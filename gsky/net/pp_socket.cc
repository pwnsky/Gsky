#include <gsky/net/pp_socket.hh>

#include <gsky/crypto/pe.hh>
#include <time.h>
#include <stdlib.h>

const __uint32_t EPOLL_DEFAULT_EVENT = EPOLLIN | EPOLLET | EPOLLONESHOT;

gsky::net::pp_socket::pp_socket(int fd,eventloop *elp) :
    fd_(fd),
    eventloop_(elp),
    sp_channel_(new channel(elp, fd)),
    connection_status_(status::connected),
    process_status_(status::parse_header),
    sp_work_(new gsky::work::work(map_client_info_, in_buffer_)) {
#ifdef DEBUG
    d_cout << "call gsky::net::pp_socket::pp_socket()\n";
#endif
    //set callback function handler
    sp_channel_->set_read_handler(std::bind(&pp_socket::handle_read, this));
    sp_channel_->set_write_handler(std::bind(&pp_socket::handle_write, this));
    sp_channel_->set_reset_handler(std::bind(&pp_socket::handle_reset, this));

    sp_work_->set_send_data_handler(std::bind(&pp_socket::send_data, this, std::placeholders::_1));
    sp_work_->set_route(header_.route);
//    session_ = gsky::util::md5(std::to_string(time(nullptr)) + "gsky").to_string();
}

gsky::net::pp_socket::~pp_socket() {
#ifdef DEBUG
    d_cout << "call gsky::net::pp_socket::~pp_socket()\n";
#endif
    close(fd_);
    // delete all
    while(this->out_buffer_queue_.size() > 0) {
            auto buf = out_buffer_queue_.front();
            out_buffer_queue_.pop();
    }
}

void gsky::net::pp_socket::reset() {
    process_status_ = status::parse_header;
    in_buffer_.clear();
}

void gsky::net::pp_socket::handle_close() {
#ifdef DEBUG
    d_cout << "call gsky::net::pp_socket::handle_close()\n";
#endif
    connection_status_ = status::disconnected;
    sp_pp_socket guard(shared_from_this()); // avoid delete
    eventloop_->remove_from_epoll(sp_channel_);
}

void gsky::net::pp_socket::new_evnet() {
    sp_channel_->set_event(EPOLL_DEFAULT_EVENT);
    eventloop_->add_to_epoll(sp_channel_);
}

gsky::net::sp_channel gsky::net::pp_socket::get_sp_channel() {
    return sp_channel_;
}

gsky::net::eventloop *gsky::net::pp_socket::get_eventloop() {
    return eventloop_;
}

// 接收数据回调
void gsky::net::pp_socket::handle_read() {
    __uint32_t &event = sp_channel_->get_event();
    do {
        if(process_status_ == status::parse_header) {
            int read_len = gsky::net::util::read(fd_, (void*)&header_, sizeof(pp::header)); // read data
            if(read_len == 0) {
#ifdef DEBUG
                d_cout << "is_disconnected 1" << std::endl;
#endif
                connection_status_ = status::disconnecting;
                return;
            }

            if(read_len != sizeof(pp::header)  || header_.magic != 0x5050) {
                handle_error(pp::status::protocol_error);
                return;
            }

            body_left_length_ = ntohl(header_.length);
            if(body_left_length_ >= 1024 * 1024 * 100) { // 大于100 MB
                handle_error(pp::status::too_big);
                return;
            }

            in_buffer_.resize(body_left_length_); // 重置缓冲区
            //logger() << "read data from " + client_ip_ + ":" + client_port_;
#ifdef DEBUG
            std::cout << "read: size: " << read_len << " migic: " << header_.magic << " length: " << body_left_length_ << std::endl;
            printf("router: ");
            for(int i = 0; i < 4; i ++) {
                printf(" %02X", header_.router[i]);
            }
            printf("\n");
#endif
            logger() << ("Request length: " + std::to_string(body_left_length_));

            process_status_ = status::recv_content;
        }

        // 接收数据部分
        if(process_status_ == status::recv_content) {
            int read_len = gsky::net::util::read(fd_, in_buffer_, body_left_length_);
            if(read_len == 0 && header_.length != 0) {
#ifdef DEBUG
                d_cout << "is_disconnected 2" << std::endl;
#endif
                connection_status_ = status::disconnecting;
                return;
            }
            body_left_length_ -= read_len;
            if(body_left_length_ <= 0) {
                process_status_ = status::work;
            }
#ifdef DEBUG
            std::cout << "pp_socket read: size: " << read_len << " body_left_length_: " << body_left_length_ << std::endl;
#endif
        }
        
        if(process_status_ == status::work) {
            // 接收完成，解密与处理请求
            if(is_sended_key_ == false) {
                if(header_.status == (unsigned char)pp::status::connect) {
                    send_key();
                }else {
                    handle_error(pp::status::invalid_transfer);
                }
                return;
            }

            // 若请求状态不是数据传输，返回错误。
            if(header_.status != (unsigned char) pp::status::data_transfer) {
                handle_error(pp::status::invalid_transfer);
                return;
            }

            // 数据解密
            crypto::pe().decode(key_, in_buffer_.data(), in_buffer_.size());

            this->handle_work();
            in_buffer_.clear();
            process_status_ = status::finish;
        }
        
    } while(false);
    // end
    if(process_status_ == status::finish) {
        this->reset();
        //if network is disconnected, do not to clean write data buffer, may be it reconnected
    } else if (connection_status_ == status::disconnected) {
        event |= EPOLLIN;
    }
}

void gsky::net::pp_socket::handle_work() {
#ifdef DEBUG
    d_cout << "call gsky::net::pp_socket::handle_work()\n";
#endif
    sp_work_->main();
}

void gsky::net::pp_socket::handle_write() {
    __uint32_t &event = sp_channel_->get_event();

    if(out_buffer_queue_.size() == 0)
        return;

    out_buffer_ = out_buffer_queue_.front();

    /*
    if(event & EPOLLOUT) {
        std::cout << "gsky::net::pp_socket::handle_write EPOLLOUT\n";
    } */
    if(connection_status_ == status::disconnected) {
        return;
    }
#ifdef DEBUG
    dbg_log("write size: " + std::to_string(out_buffer_->size()) + "] end");
#endif
    if(out_buffer_->size() > 0) {
        if(util::write(fd_, out_buffer_) < 0) {
            perror("write header data");
            sp_channel_->set_event(0);
            //out_buffer_->clear();
        }
        if(out_buffer_->size() == 0) { // 数据发送完毕后，若out_buffer_queue_还存在待发送数据，则继续发送
            if(out_buffer_queue_.size() > 0)
                out_buffer_queue_.pop();
            out_buffer_.reset();
            if(out_buffer_queue_.size() > 0) {
                event |= EPOLLOUT; // next round set event as EPOLLOUT
            }
            return;
        }
    }
    if (out_buffer_->size() > 0)
        event |= EPOLLOUT; // next round set event as EPOLLOUT
}

// 处理连接
void gsky::net::pp_socket::handle_reset() {
    __uint32_t &event = sp_channel_->get_event();

    if(connection_status_ == status::connected) {
        if(event != 0) {
            if((event & EPOLLIN) && (event & EPOLLOUT)) {
                event = 0;
                event |= EPOLLOUT;
            }
            event |= EPOLLET;
        } else {
            event |= (EPOLLIN | EPOLLET);
        }
        eventloop_->update_epoll(sp_channel_);
    } else if (connection_status_ == status::disconnecting
               && (event & EPOLLOUT)) {
        event = (EPOLLOUT | EPOLLET);
    connection_status_ = status::disconnected;
    } else {
#ifdef DEBUG
        std::cout << client_ip_ << " disconnected " << std::endl;
#endif
        eventloop_->run_in_loop(std::bind(&pp_socket::handle_close, shared_from_this()));
    }
}

/*
 * 发送数据: 采用同步方式进行数据发送
 * */

void gsky::net::pp_socket::send_data(const std::string &content) {
    std::shared_ptr<gsky::util::vessel> out_buffer = std::shared_ptr<gsky::util::vessel>(new gsky::util::vessel());
    pp::header header;
    header.magic = 0x5050;
    memcpy(header.route, header_.route, sizeof(header.route));
    header.length = htonl(content.size());
    out_buffer->append(&header, sizeof(struct pp::header));

    out_buffer->resize(content.size());

    *out_buffer << content;
    // 实现加密数据
    //
    // 发送数据
    out_buffer_queue_.push(out_buffer);
    handle_write();
}

/*
 * 数据推送: 采用异步方式进行数据推送。
 * */

void gsky::net::pp_socket::push_data(const std::string &data) {
    // 存在数据正在写入
    std::shared_ptr<gsky::util::vessel> out_buffer = std::shared_ptr<gsky::util::vessel>(new gsky::util::vessel());
    pp::header header;
    header.magic = 0x5050;
    memcpy(header.route, header_.route, sizeof(header.route));
    header.length = htonl(data.size());
    out_buffer->append(&header, sizeof(struct pp::header));
    *out_buffer << data;
    // 数据加密
    //
    out_buffer_queue_.push(out_buffer); // 将数据放入队列
    wait_event_count_ ++;
#ifdef DEBUG
    dbg_log("push_data: " + data);
#endif
    // 用于异步回调 handle_push_data_reset函数
    eventloop_->run_in_loop(std::bind(&pp_socket::handle_push_data_reset, shared_from_this()));
}

// 采用单一线程进行更新epoll事件，多线程易出现条件竞争，导致内存错误
void gsky::net::pp_socket::handle_push_data_reset() {
    wait_event_count_ --;
    __uint32_t &event = sp_channel_->get_event();
    event |= EPOLLIN | EPOLLET | EPOLLOUT;
    // 回调写入函数
    eventloop_->update_epoll(sp_channel_); // 更新事件
}
/*
 * 处理错误
 * */

void gsky::net::pp_socket::handle_error(pp::status s) {
    std::shared_ptr<gsky::util::vessel> out_buffer = std::shared_ptr<gsky::util::vessel>(new gsky::util::vessel());
    pp::header header;
    memset(&header, 0, sizeof(pp::header));
    header.magic = 0x5050;
    header.status = (unsigned char)s;
    header.type = 0;
    memcpy(header.route, header_.route, sizeof(header.route));
    header.length = 0;

    out_buffer->append(&header, sizeof(struct pp::header));
    out_buffer_queue_.push(out_buffer);
    handle_write();
    connection_status_ = status::disconnected; // 设置socket状态。
}
/*
 * 发送 pe 加解密key值给客户端，采用密钥为全0进行加密
 * */

void gsky::net::pp_socket::send_key() {
    std::shared_ptr<gsky::util::vessel> out_buffer = std::shared_ptr<gsky::util::vessel>(new gsky::util::vessel());
    unsigned char gen_key[8] = {0};
    // 随机生成key值
    srand(time(nullptr));
    unsigned int random_key_1 = random();
    unsigned int random_key_2 = random();

    memcpy(gen_key, &random_key_1, sizeof(unsigned int));
    memcpy(gen_key + sizeof(unsigned int), &random_key_2, sizeof(unsigned int));

    memcpy(key_, gen_key, 8);

    gsky::net::pp::header header;
    memset(&header, 0, sizeof(pp::header));
    header.magic = 0x5050;
    header.status = (unsigned char) gsky::net::pp::status::send_key;
    header.type = 0;
    memcpy(header.route, header_.route, 8);
    header.length = htonl(8);
    
    out_buffer->append(&header, sizeof(pp::header));
    out_buffer->append(gen_key, 8);

    // 加密传输密钥给客户端，客户端采用全0密钥进pe解密
    crypto::pe().encode(key_, out_buffer->data(), out_buffer->size());

    out_buffer_queue_.push(out_buffer);
    handle_write();
    is_sended_key_ = true;
}
