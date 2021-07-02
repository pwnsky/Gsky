#include <gsky/net/pp/socket.hh>

#include <time.h>
#include <stdlib.h>

#include <gsky/crypto/pe.hh>
#include <gsky/log/log.hh>
#include <gsky/net/util.hh>
#include <gsky/util/firewall.hh>
#include <gsky/util/util.hh>

using logger = gsky::log::logger;

gsky::net::pp::socket::socket(int fd) :
    fd_(fd),
    status_(status::parse_header) {

#ifdef DEBUG
    d_cout << "call gsky::net::pp::socket::socket()\n";
#endif
    //set callback function handler
    //response_.set_send_data_handler(std::bind(&pp::socket::send_data, this, std::placeholders::_1));
    //request_.set_route(header_.route);
}

gsky::net::pp::socket::~socket() {
#ifdef DEBUG
    d_cout << "call gsky::net::pp::socket::~socket()\n";
#endif
}

void gsky::net::pp::socket::reset() {
    status_ = status::parse_header;
    in_buffer_.clear();
}

void gsky::net::pp::socket::handle_close() {
#ifdef DEBUG
    d_cout << "call gsky::net::pp::socket::handle_close()\n";
#endif
}

// 接收数据回调
void gsky::net::pp::socket::handle_read() {
    do {
        if(status_ == status::parse_header) {
            int read_len = gsky::net::util::read(fd_, (void*)&header_, sizeof(pp::header)); // read data
            if(read_len == 0) {
#ifdef DEBUG
                d_cout << "is_disconnected 1" << std::endl;
#endif
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

            status_ = status::recv_content;
        }

        // 接收数据部分
        if(status_ == status::recv_content) {
            int read_len = gsky::net::util::read(fd_, in_buffer_, body_left_length_);
            if(read_len == 0 && header_.length != 0) {
#ifdef DEBUG
                d_cout << "is_disconnected 2" << std::endl;
#endif

                return;
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

    //server_handler_(&request_, &response_);
}

void gsky::net::pp::socket::send_data(const std::string &content) {
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
    //
}

/*
 * 数据推送: 采用异步方式进行数据推送。
 * */

void gsky::net::pp::socket::push_data(const std::string &data) {
    // 存在数据正在写入
    std::shared_ptr<gsky::util::vessel> out_buffer = std::shared_ptr<gsky::util::vessel>(new gsky::util::vessel());
    pp::header header;
    header.magic = 0x5050;
    memcpy(header.route, header_.route, sizeof(header.route));
    header.length = htonl(data.size());
    out_buffer->append(&header, sizeof(struct pp::header));
    *out_buffer << data;
    // 数据加密
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

}

/*
 * 发送 pe 加解密key值给客户端，采用密钥为全0进行加密
 * */

void gsky::net::pp::socket::send_key() {
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
    is_sended_key_ = true;
}
