#include <gsky/net/pp/socket.hh>

#include <time.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#include <gsky/crypto/pe.hh>
#include <gsky/log/logger.hh>
#include <gsky/net/util.hh>
#include <gsky/util/firewall.hh>
#include <gsky/util/util.hh>

/*
 * 加密：
 * 数据加密采用 Pwnsky Encryption进行加密，客户端在初始连接的时候先请求密钥，服务端发送8字节随机密钥用于本次连接，
 * 后面的数据除了头部前8字节未加密，其余部分全部加密，其中包括头部的6字节的route字节和两字节的校验值。
 * */

using logger = gsky::log::logger;
gsky::net::pp::server_handler gsky::net::pp::server_handler_ = nullptr;
gsky::net::pp::offline_handler gsky::net::pp::offline_handler_ = nullptr; //客户端断开连接调用该函数

gsky::net::pp::socket::socket(int fd) :
    fd_(fd),
    status_(status::parse_header),
    request_(new gsky::net::pp::request(client_info_, in_buffer_)),
    response_(new gsky::net::pp::response()) {

#ifdef DEBUG
    dlog << "call gsky::net::pp::socket::socket()\n";
#endif
    //set std::function<void()> function handler
    response_->set_send_data_handler(std::bind(&pp::socket::send_data, this, std::placeholders::_1));
    response_->set_push_data_handler(std::bind(&pp::socket::push_data, this, std::placeholders::_1));
    response_->set_route_handler(std::bind(&pp::socket::set_route, this, std::placeholders::_1));
    request_->set_route(header_.route);
    request_->fd = fd_;
}

gsky::net::pp::socket::~socket() {
#ifdef DEBUG
    dlog << "call gsky::net::pp::socket::~socket()\n";
#endif
}

void gsky::net::pp::socket::reset() {
#ifdef DEBUG
    dlog << "gsky::net::pp::socket::reset()\n";
#endif

    status_ = status::parse_header;
    in_buffer_.clear();
}

void gsky::net::pp::socket::handle_close() {
#ifdef DEBUG
    dlog << "call gsky::net::pp::socket::handle_close()\n";
#endif
    response_->clean_handler();
}

void gsky::net::pp::socket::set_send_data_handler(std::function<void(std::shared_ptr<gsky::util::vessel>)> func) {
    send_data_handler_ = func;
}

void gsky::net::pp::socket::set_push_data_handler(std::function<void(std::shared_ptr<gsky::util::vessel>)> func) {
    push_data_handler_ = func;

}

void gsky::net::pp::socket::set_disconnecting_handler(std::function<void()> func) {
    disconnecting_handler_ = func;
}

// 接收数据回调
void gsky::net::pp::socket::handle_read() {
#ifdef DEBUG
            dlog << "gsky::net::pp::socket::handle_read()\n";
#endif
    do {
        // 由于采用epoll ET边缘触发模式，读取数据必须一次性读完缓冲区。
        if(status_ == status::parse_header) {
            int read_len = gsky::net::util::read(fd_, in_buffer_); // read data
#ifdef INFO
            std::cout << std::hex;
        info() << "Recv PP header.... read length: " << read_len  << "\n";
#endif
            if(read_len == 0) {
#ifdef DEBUG
                dlog << "is_disconnected 1\n";
#endif
                handle_disconnect();
                break;
            }else if(read_len < 0) {
                break; // 重新接收
            }
            memcpy(&header_, in_buffer_.data(), sizeof(pp::header)); //拷贝头部
            if(header_.magic != 0x5050) {
                handle_error(pp::status::protocol_error);
                break;
            }
            header_.length = ntohl(header_.length);

            if(header_.length >= 1024 * 1024 * 100) { // 大于100 MB
#ifdef INFO
     info() << "request data too_big: " << client_info_["ip"] << ":" << client_info_["port"] << "\n"; 
#endif
                handle_error(pp::status::too_big);
                break;
            }

            // 验证头部信息, checkcode
            if(is_invalid()) {
#ifdef INFO
     info() << "invalid_transfer: " << client_info_["ip"] << ":" << client_info_["port"] << "\n"; 
#endif
                handle_disconnect();
                break; // 验证失败，断开连接。
            }
#ifdef INFO
        info() << "Buffer size: " << in_buffer_.size() << " Total length: " << (header_.length + 0x10);
#endif
            in_buffer_.resize(header_.length + sizeof(pp::header)); // 修改缓冲区大小
            //logger() << "read data from " + client_ip_ + ":" + client_port_;
#ifdef INFO
            info() << "read: size: " << read_len << " migic: " << header_.magic << " length: " << header_.length;
            printf("router: ");
            for(int i = 0; i < 6; i ++) {
                printf(" %02X", header_.route[i]);
            }
            printf("\n");
#endif
            logger() << ("Request length: " + std::to_string(header_.length));

            if(in_buffer_.size() >= header_.length + sizeof(pp::header)) { //已经收完整个数据包
                status_ = status::work;
#ifdef INFO
        info() << "Recv all pp data\n";
#endif
            }else {
                status_ = status::recv_content; //未收完数据包
            }
        }

        // 接收剩余数据部分
        if(status_ == status::recv_content) {
    /*
            if(in_buffer_.size() >= header_.length + sizeof(pp::header)) { //已经收完整个数据包
                status_ = status::work;
            }
    */

            int read_len = gsky::net::util::read(fd_, in_buffer_);
#ifdef INFO
        info() << "Recv PP body.... read length: " << read_len  << "\n";
#endif
            if(read_len == 0) {
                // 对socket再次判断是否断开连接
                struct tcp_info tcp_info;
                int len= sizeof(tcp_info);
                getsockopt(fd_, IPPROTO_TCP, TCP_INFO, &tcp_info, (socklen_t *)&len);
                if((tcp_info.tcpi_state != TCP_ESTABLISHED)) { // 断开连接
#ifdef INFO
                info() << "is_disconnected 2 : recv_len" << read_len << " header_length: " << header_.length << "\n";
#endif
                    handle_disconnect();
                }

                break;
            }else if(read_len < 0) {
#ifdef INFO
                info() << "Continue recv.... " << "readed data size" << in_buffer_.size() << " left" << (header_.length + 16 - in_buffer_.size());
#endif
                break; // 重新接收
            }

            if(in_buffer_.size() >= header_.length + sizeof(pp::header)) { //已经收完整个数据包
                status_ = status::work;
            }
        }
        
        if(status_ == status::work) {
#ifdef INFO
        info() << "Recved OK works: \n";
#endif
            // 接收完成，解密与处理请求
            in_buffer_.sub(sizeof(pp::header)); // 把头部信息去去掉。

            if(is_sended_key_ == false) {
                if(header_.status == (unsigned char)pp::status::connect) {
                    send_key();
                    status_ = status::finish;
                }else {
                    handle_error(pp::status::invalid_transfer);
                }
                break;
            }

            // 若请求状态不是数据传输，返回错误。
            if(header_.status != (unsigned char) pp::status::data_transfer) {
                handle_error(pp::status::invalid_transfer);
                break;
            }

#ifdef INFO
            info() << "";
            std::cout << "decode size: " << std::hex << in_buffer_.size() << "\n";
#endif
            // 数据解密
            crypto::pe().decode(key_, in_buffer_.data(), in_buffer_.size());
            handle_work();
            //in_buffer_.clear();
            status_ = status::finish;
        }
        
    } while(false);

    // end
    if(status_ == status::finish) {
        reset();
        //if network is disconnected, do not to clean write data buffer, may be it reconnected
    }
}

void gsky::net::pp::socket::handle_work() {
#ifdef DEBUG
    dlog << "call gsky::net::pp::socket::handle_work()\n";
#endif
    if(gsky::net::pp::server_handler_ == nullptr) {
        dlog << "server_handler is nullptr\n";
        return;
    }
    gsky::net::pp::server_handler_(request_, response_);
}

void gsky::net::pp::socket::send_data(const std::string &data) {
    std::shared_ptr<gsky::util::vessel> out_buffer = std::shared_ptr<gsky::util::vessel>(new gsky::util::vessel());
    pp::header header;
    header.magic = 0x5050;
    header.status = (unsigned char) pp::status::ok;
    header.type   = (unsigned char) pp::data_type::binary_stream;
    header.length = htonl(data.size());
    memcpy(header.route, header_.route, 6);
    memcpy(header.code, check_code_, 2);

    out_buffer->append(&header, sizeof(struct pp::header));
    *out_buffer << data;
    // 实现加密数据
    crypto::pe().encode(key_, out_buffer->data() + 8, 8); // 加密头部后8字节
    crypto::pe().encode(key_, out_buffer->data() + sizeof(pp::header), out_buffer->size() - sizeof(pp::header)); // 加密传输内容

    // 发送数据
    if(send_data_handler_) {
        send_data_handler_(out_buffer);
    }
}

/*
 * 数据推送: 采用异步方式进行数据推送。
 * */

void gsky::net::pp::socket::push_data(const std::string &data) {
#ifdef DEBUG
    dlog << "push_data()\n";
#endif
    // 存在数据正在写入
    std::shared_ptr<gsky::util::vessel> out_buffer = std::shared_ptr<gsky::util::vessel>(new gsky::util::vessel());
    pp::header header;
    header.magic = 0x5050;
    header.status = (unsigned char) pp::status::ok;
    header.type   = (unsigned char) pp::data_type::binary_stream;
    header.length = htonl(data.size());
    memcpy(header.route, header_.route, 6);
    memcpy(header.code, check_code_, 2);

    out_buffer->append(&header, sizeof(struct pp::header));
    *out_buffer << data;

    // 数据加密
    //gsky::crypto::pe().encode(key_, out_buffer->data(), out_buffer->size());

    crypto::pe().encode(key_, out_buffer->data() + 8, 8); // 加密头部后8字节
    crypto::pe().encode(key_, out_buffer->data() + sizeof(pp::header), out_buffer->size() - sizeof(pp::header)); // 加密传输内容

    if(push_data_handler_) {
        push_data_handler_(out_buffer);
    }
}

/*
 * 处理错误
 * */

void gsky::net::pp::socket::handle_error(pp::status s) {

#ifdef DEBUG
    info() << "handle_error: status: ";
    std::cout << std::hex << (unsigned int)s << std::endl;
#endif

    std::shared_ptr<gsky::util::vessel> out_buffer = std::shared_ptr<gsky::util::vessel>(new gsky::util::vessel());
    pp::header header;
    memset(&header, 0, sizeof(pp::header));
    header.magic = 0x5050;
    header.status = (unsigned char)s;
    header.type = 0;
    memcpy(header.route, header_.route, 6);
    header.length = 0;
    out_buffer->append(&header, sizeof(struct pp::header));

    if(send_data_handler_) {
        send_data_handler_(out_buffer);
    }

    handle_disconnect(); // 断开连接
}

/*
 * 发送 pe 加解密key值给客户端，采用密钥为全0进行加密
 * */

void gsky::net::pp::socket::send_key() {
#ifdef DEBUG
    dlog << " gsky::net::pp::socket::send_key()\n";
#endif
    std::shared_ptr<gsky::util::vessel> out_buffer = std::shared_ptr<gsky::util::vessel>(new gsky::util::vessel());
    unsigned char gen_key[8] = {0};
    // 随机生成key值
    srand(time(nullptr));
    unsigned int random_key_1 = random();
    unsigned int random_key_2 = random();
#ifdef INFO
    info() << "send key " << random_key_1 << "  " << random_key_2 << "\n";
#endif

    memcpy(gen_key, &random_key_1, sizeof(unsigned int));
    memcpy(gen_key + sizeof(unsigned int), &random_key_2, sizeof(unsigned int));
    memcpy(check_code_, &random_key_2, 2);

    gsky::net::pp::header header;
    memset(&header, 0, sizeof(pp::header));
    header.magic = 0x5050;
    header.status = (unsigned char) gsky::net::pp::status::send_key;
    header.type = 0;
    memcpy(header.route, header_.route, 6);
    memcpy(header.code, check_code_, 2); // 复制check code给客户端
    header.length = htonl(8);
    
    out_buffer->append(&header, sizeof(pp::header));
    out_buffer->append(gen_key, 8);

    // 加密传输密钥给客户端，客户端采用全0密钥进pe解密
    // 加密包括头部最后8字节与内容部分，这里发送内容为密钥。
    crypto::pe().encode(key_, out_buffer->data() + 8, 8); // 加密头部后8字节
    crypto::pe().encode(key_, out_buffer->data() + sizeof(pp::header), out_buffer->size() - sizeof(pp::header)); // 加密传输内容

    memcpy(key_, gen_key, 8); // 保存密钥
    if(send_data_handler_) {
        send_data_handler_(out_buffer);
    } 
    is_sended_key_ = true;
}

bool gsky::net::pp::socket::is_invalid() {
    if(!is_sended_key_) return false; // 未发送密钥，直接返回。
    crypto::pe().encode(key_, &header_.route,  8);  // 解密头部8字节: 6 字节 route + 2 字节code
    if(memcmp(&header_.code, check_code_, 2) != 0)  // 检查头部code
        return true;
    return false;
}

void gsky::net::pp::socket::handle_disconnect() {
    if(disconnecting_handler_) {
        disconnecting_handler_();
    }
}

void gsky::net::pp::socket::set_client_info(const std::string &ip, const std::string &port) {
        client_info_["ip"] = ip;
        client_info_["port"] = port;
        //map_client_info_["session"] = session_;
}

// copy route 
void gsky::net::pp::socket::set_route(unsigned char route[]) {
#ifdef DEBUG
    dlog << "set_route()";
#endif

    memcpy(header_.route, route, 6);
}
