#include <gsky/net/pp/socket.hh>

#include <time.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include <gsky/crypto/pe.hh>
#include <gsky/log/log.hh>
#include <gsky/net/util.hh>
#include <gsky/util/firewall.hh>
#include <gsky/util/util.hh>
/*
 * 加密：
 * 数据加密采用 Pwnsky Encryption进行加密，客户端在初始连接的时候先请求密钥，服务端发送8字节随机密钥用于本次连接，
 * 后面的数据除了头部前8字节未加密，其余部分全部加密，其中包括头部的6字节的route字节和两字节的校验值。
 * */

using logger = gsky::log::logger;
gsky::net::pp::server_handler gsky::net::pp::server_handler_;

gsky::net::pp::socket::socket(int fd) :
    fd_(fd),
    status_(status::parse_header),
    request_(new gsky::net::pp::request(client_info_, in_buffer_)),
    response_(new gsky::net::pp::response()) {

#ifdef DEBUG
    d_cout << "call gsky::net::pp::socket::socket()\n";
#endif
    //set std::function<void()> function handler
    //response_.set_send_data_handler(std::bind(&pp::socket::send_data, this, std::placeholders::_1));
    //request_.set_route(header_.route);
}

gsky::net::pp::socket::~socket() {
#ifdef DEBUG
    d_cout << "call gsky::net::pp::socket::~socket()\n";
#endif
}

void gsky::net::pp::socket::reset() {
#ifdef DEBUG
    d_cout << "gsky::net::pp::socket::reset()\n";
#endif

    status_ = status::parse_header;
    in_buffer_.clear();
}

void gsky::net::pp::socket::handle_close() {
#ifdef DEBUG
    d_cout << "call gsky::net::pp::socket::handle_close()\n";
#endif
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
                d_cout << "gsky::net::pp::socket::handle_read()" << std::endl;
#endif
    do {
        if(status_ == status::parse_header) {
            int read_len = gsky::net::util::read(fd_, (void*)&header_, sizeof(pp::header)); // read data
            if(read_len == 0) {
#ifdef DEBUG
                d_cout << "is_disconnected 1" << std::endl;
#endif
                handle_disconnect();
                break;
            }

            if(read_len != sizeof(pp::header)  || header_.magic != 0x5050) {
                handle_error(pp::status::protocol_error);
                break;
            }


            body_left_length_ = ntohl(header_.length);
            if(body_left_length_ >= 1024 * 1024 * 100) { // 大于100 MB
                handle_error(pp::status::too_big);
                break;
            }

            // 验证头部信息, checkcode
            if(is_invalid()) {
                handle_disconnect();
                break; // 验证失败，断开连接。
            }

            in_buffer_.resize(body_left_length_); // 重置缓冲区

            //logger() << "read data from " + client_ip_ + ":" + client_port_;
#ifdef DEBUG
            std::cout << "read: size: " << read_len << " migic: " << header_.magic << " length: " << body_left_length_ << std::endl;
            printf("router: ");
            for(int i = 0; i < 8; i ++) {
                printf(" %02X", header_.route[i]);
            }
            printf("\n");
#endif
            logger() << ("Request length: " + std::to_string(body_left_length_));

            status_ = status::recv_content;
        }

        // 接收数据部分
        if(status_ == status::recv_content) {


            int read_len = gsky::net::util::read(fd_, in_buffer_, body_left_length_);
            if(read_len == 0 && header_.length != 0) {
#ifdef DEBUG
                d_cout << "is_disconnected 2" << std::endl;
#endif
                handle_disconnect();
                break;
            }
            body_left_length_ -= read_len;
            if(body_left_length_ <= 0) {
                status_ = status::work;
            }
#ifdef DEBUG
            std::cout << "pp::socket read: size: " << read_len << " body_left_length_: " << body_left_length_ << std::endl;
#endif
        }
        
        if(status_ == status::work) {
            // 接收完成，解密与处理请求
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

            // 数据解密
            crypto::pe().decode(key_, in_buffer_.data(), in_buffer_.size());
            this->handle_work();
            in_buffer_.clear();
            status_ = status::finish;
        }
        
    } while(false);

    // end
    if(status_ == status::finish) {
        this->reset();
        //if network is disconnected, do not to clean write data buffer, may be it reconnected
    }
}

void gsky::net::pp::socket::handle_work() {
#ifdef DEBUG
    d_cout << "call gsky::net::pp::socket::handle_work()\n";
#endif
    if(server_handler_ == nullptr) {
        d_cout << "server_handler is nullptr\n";
        return;
    }
    server_handler_(request_, response_);
}

void gsky::net::pp::socket::send_data(const std::string &content) {
    std::shared_ptr<gsky::util::vessel> out_buffer = std::shared_ptr<gsky::util::vessel>(new gsky::util::vessel());
    pp::header header;
    header.magic = 0x5050;
    memcpy(header.route, header_.route, 6);
    header.length = htonl(content.size());
    out_buffer->append(&header, sizeof(struct pp::header));
    out_buffer->resize(content.size());
    *out_buffer << content;
    // 实现加密数据
    gsky::crypto::pe().encode(key_, out_buffer->data(), out_buffer->size());
    
    // 发送数据
    if(send_data_handler_) {
        send_data_handler_(out_buffer);
    }
}

/*
 * 数据推送: 采用异步方式进行数据推送。
 * */

void gsky::net::pp::socket::push_data(const std::string &data) {
    // 存在数据正在写入
    std::shared_ptr<gsky::util::vessel> out_buffer = std::shared_ptr<gsky::util::vessel>(new gsky::util::vessel());
    pp::header header;
    header.magic = 0x5050;
    memcpy(header.route, header_.route, 6);
    header.length = htonl(data.size());
    out_buffer->append(&header, sizeof(struct pp::header));
    *out_buffer << data;

    // 数据加密
    gsky::crypto::pe().encode(key_, out_buffer->data(), out_buffer->size());

    if(push_data_handler_) {
        push_data_handler_(out_buffer);
    }
}

/*
 * 处理错误
 * */

void gsky::net::pp::socket::handle_error(pp::status s) {
    std::shared_ptr<gsky::util::vessel> out_buffer = std::shared_ptr<gsky::util::vessel>(new gsky::util::vessel());
    pp::header header;
    memset(&header, 0, sizeof(pp::header));
    header.magic = 0x5050;
    header.status = (unsigned char)s;
    header.type = 0;
    memcpy(header.route, header_.route, sizeof(header.route));
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
    d_cout << " gsky::net::pp::socket::send_key()\n";
#endif
    std::shared_ptr<gsky::util::vessel> out_buffer = std::shared_ptr<gsky::util::vessel>(new gsky::util::vessel());
    unsigned char gen_key[8] = {0};
    // 随机生成key值
    srand(time(nullptr));
    unsigned int random_key_1 = random();
    unsigned int random_key_2 = random();
#ifdef DEBUG
    std::cout << "send key " << std::hex << random_key_1 << "  " << random_key_2 << "\n";
#endif

    memcpy(gen_key, &random_key_1, sizeof(unsigned int));
    memcpy(gen_key + sizeof(unsigned int), &random_key_2, sizeof(unsigned int));
    memcpy(check_code_, &random_key_1, 2);

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
    crypto::pe().encode(key_, out_buffer->data() + 8, out_buffer->size() - 8);

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
