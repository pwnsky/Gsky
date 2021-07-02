#pragma once

#include <memory>
#include <map>

#include <gsky/gsky.hh>
#include <gsky/net/pp/pp.hh>
#include <gsky/net/pp/request.hh>
#include <gsky/net/pp/response.hh>
#include <gsky/util/vessel.hh>

// pp::socket (pwnsky protocl socket)
class gsky::net::pp::socket {
public:
enum class status {
    parse_header,
    recv_content,
    work,
    finish,
};

public:
    explicit socket(int fd);
    ~socket();
    void reset();
    sp_channel get_sp_channel();
    eventloop *get_eventloop();
    void handle_close();
    void new_evnet();
    void set_client_info(const std::string &ip, const std::string &port) {
            map_client_info_["client_ip"] = ip;
            map_client_info_["client_port"] = port;
            //map_client_info_["session"] = session_;
            client_ip_ = ip;
            client_port_ = port;
    }
    void push_data(const std::string &data); // 数据推送
    void response_error(int error_number, std::string message);
    void handle_read();
private:
    int fd_;
    std::string uid_ =  "none";
    bool is_sended_key_ = false;
    gsky::util::vessel in_buffer_;

    status status_;

    std::string client_ip_;
    std::string client_port_;

    //gsky::net::pp::request request_;   // pp协议，request
    //gsky::net::pp::response response_; // pp协议，response

    gsky::net::pp::server_handler server_handler_; // 回调函数

    unsigned char key_[8] = {0}; // 初始化为0
    std::map<std::string, std::string> map_client_info_;
    gsky::net::pp::header header_;
    int  body_left_length_ = 0;
    void handle_work();
    void send_data(const std::string &content);
    void handle_error(pp::status s);
    void send_key(); // 发送pe key给客户端
};
