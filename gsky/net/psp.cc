#include <gsky/net/psp.hh>

const __uint32_t EPOLL_DEFAULT_EVENT = EPOLLIN | EPOLLET | EPOLLONESHOT;

gsky::net::psp::psp(int fd,eventloop *elp) :
    fd_(fd),
    eventloop_(elp),
    sp_channel_(new channel(elp, fd)),
    psp_connection_state_(PSPConnectionState::CONNECTED),
    psp_process_state_(PSPRecvState::PARSE_HEADER),
    sp_work_(new gsky::work::work(map_client_info_, in_buffer_)) {
#ifdef DEBUG
    d_cout << "call gsky::net::psp::psp()\n";
#endif
    //set callback function handler
    sp_channel_->set_read_handler(std::bind(&psp::handle_read, this));
    sp_channel_->set_write_handler(std::bind(&psp::handle_write, this));
    sp_channel_->set_reset_handler(std::bind(&psp::handle_reset, this));

    sp_work_->set_send_data_handler(std::bind(&psp::send_data, this, std::placeholders::_1));
    sp_work_->set_router(header_.router);
//    session_ = gsky::util::md5(std::to_string(time(nullptr)) + "gsky").to_string();
}

gsky::net::psp::~psp() {
#ifdef DEBUG
    d_cout << "call gsky::net::psp::~psp()\n";
#endif
    close(fd_);
    // delete all
    while(this->out_buffer_queue_.size() > 0) {
            auto buf = out_buffer_queue_.front();
            out_buffer_queue_.pop();
    }
}

void gsky::net::psp::reset() {
    psp_process_state_ = PSPRecvState::PARSE_HEADER;
    in_buffer_.clear();
}

void gsky::net::psp::handle_close() {
#ifdef DEBUG
    d_cout << "call gsky::net::psp::handle_close()\n";
#endif
    psp_connection_state_ = PSPConnectionState::DISCONNECTED;
    sp_psp guard(shared_from_this()); // avoid delete
    eventloop_->remove_from_epoll(sp_channel_);
}

void gsky::net::psp::new_evnet() {
    sp_channel_->set_event(EPOLL_DEFAULT_EVENT);
    eventloop_->add_to_epoll(sp_channel_);
}

gsky::net::sp_channel gsky::net::psp::get_sp_channel() {
    return sp_channel_;
}

gsky::net::eventloop *gsky::net::psp::get_eventloop() {
    return eventloop_;
}

// 接收数据回调
void gsky::net::psp::handle_read() {
    __uint32_t &event = sp_channel_->get_event();
    do {
        if(psp_process_state_ == PSPRecvState::PARSE_HEADER) {
            int read_len = gsky::net::util::read(fd_, (void*)&header_, sizeof(protocol)); // read data
            if(read_len == 0) {
#ifdef DEBUG
                d_cout << "is_disconnected 1" << std::endl;
#endif
                psp_connection_state_ = PSPConnectionState::DISCONNECTING;
                return;
            }

            if(read_len != sizeof(protocol)  || strcmp(header_.magic, "PSP")) {
                handle_error();
                return;
            }

            body_left_length_ = ntohl(header_.length);
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

            psp_process_state_ = PSPRecvState::RECV_CONTENT;
        }

        if(psp_process_state_ == PSPRecvState::RECV_CONTENT) {
            int read_len = gsky::net::util::read(fd_, in_buffer_, body_left_length_);
            if(read_len == 0) {
#ifdef DEBUG
                d_cout << "is_disconnected 2" << std::endl;
#endif
                psp_connection_state_ = PSPConnectionState::DISCONNECTING;
                return;
            }
            body_left_length_ -= read_len;
            if(body_left_length_ <= 0) {
                psp_process_state_ = PSPRecvState::WORK;
            }
#ifdef DEBUG
            std::cout << "psp read: size: " << read_len << " body_left_length_: " << body_left_length_ << std::endl;
#endif
        }
        
        if(psp_process_state_ == PSPRecvState::WORK) {
            this->handle_work();
            in_buffer_.clear();
            psp_process_state_ = PSPRecvState::FINISH;
        }
        
    } while(false);
    // end
    if(psp_process_state_ == PSPRecvState::FINISH) {
        this->reset();
        //if network is disconnected, do not to clean write data buffer, may be it reconnected
    } else if (psp_connection_state_ == PSPConnectionState::DISCONNECTED) {
        event |= EPOLLIN;
    }
}

void gsky::net::psp::handle_work() {
#ifdef DEBUG
    d_cout << "call gsky::net::psp::handle_work()\n";
#endif
    sp_work_->main();
}

void gsky::net::psp::handle_write() {
    __uint32_t &event = sp_channel_->get_event();

    if(out_buffer_queue_.size() == 0)
        return;

    out_buffer_ = out_buffer_queue_.front();

    /*
    if(event & EPOLLOUT) {
        std::cout << "gsky::net::psp::handle_write EPOLLOUT\n";
    } */
    if(psp_connection_state_ == PSPConnectionState::DISCONNECTED) {
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

void gsky::net::psp::send_data(const std::string &content) {
    std::shared_ptr<gsky::util::vessel> out_buffer = std::shared_ptr<gsky::util::vessel>(new gsky::util::vessel());
    protocol header;
    memcpy(header.magic, "PSP", sizeof(header.magic));
    memcpy(header.router, header_.router, sizeof(header.router));
    header.length = htonl(content.size());
    out_buffer->append(&header, sizeof(struct protocol));
    *out_buffer << content;
    out_buffer_queue_.push(out_buffer);
    handle_write();
}

// 处理连接
void gsky::net::psp::handle_reset() {
    __uint32_t &event = sp_channel_->get_event();

    if(psp_connection_state_ == PSPConnectionState::CONNECTED) {
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
    } else if (psp_connection_state_ == PSPConnectionState::DISCONNECTING
               && (event & EPOLLOUT)) {
        event = (EPOLLOUT | EPOLLET);
    psp_connection_state_ = PSPConnectionState::DISCONNECTED;
    } else {
#ifdef DEBUG
        std::cout << client_ip_ << " disconnected " << std::endl;
#endif
        eventloop_->run_in_loop(std::bind(&psp::handle_close, shared_from_this()));
    }
}

void gsky::net::psp::push_data(const std::string &data) {
    // 存在数据正在写入
    std::shared_ptr<gsky::util::vessel> out_buffer = std::shared_ptr<gsky::util::vessel>(new gsky::util::vessel());
    protocol header;
    memcpy(header.magic, "PSP", sizeof(header.magic));
    memcpy(header.router, header_.router, sizeof(header.router));
    header.length = htonl(data.size());
    out_buffer->append(&header, sizeof(struct protocol));
    *out_buffer << data;
    out_buffer_queue_.push(out_buffer);
    wait_event_count_ ++;
#ifdef DEBUG
    dbg_log("push_data: " + data);
#endif
    // 采用目标psp的线程进行更新epoll事件
    eventloop_->run_in_loop(std::bind(&psp::handle_push_data_reset, shared_from_this()));
}

// 采用单一线程进行更新epoll事件，多线程易出现条件竞争，导致内存错误
void gsky::net::psp::handle_push_data_reset() {
    wait_event_count_ --;
    __uint32_t &event = sp_channel_->get_event();
    event |= EPOLLIN | EPOLLET | EPOLLOUT;
    // 回调写入函数
    eventloop_->update_epoll(sp_channel_); // 更新事件
}

void gsky::net::psp::handle_error() {
    std::shared_ptr<gsky::util::vessel> out_buffer = std::shared_ptr<gsky::util::vessel>(new gsky::util::vessel());
    std::string error_msg = "ERROR";
    protocol header;
    memcpy(header.magic, "PSP", sizeof(header.magic));
    memcpy(header.router, header_.router, sizeof(header.router));
    header.length = htonl(error_msg.size());
    out_buffer->append(&header, sizeof(struct protocol));
    *out_buffer << error_msg;
    //std::cout << "send: " << out_buffer->to_string();
    out_buffer_queue_.push(out_buffer);
    handle_write();
    psp_connection_state_ = PSPConnectionState::DISCONNECTED;
}
