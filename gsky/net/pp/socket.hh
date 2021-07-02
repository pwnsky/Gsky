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

    void response_error(int error_number, std::string message);
    void handle_read();
    void set_send_data_handler(std::function<void(std::shared_ptr<gsky::util::vessel>)> func);
    void set_push_data_handler(std::function<void(std::shared_ptr<gsky::util::vessel>)> func);
    void set_disconnecting_handler(std::function<void()> func);
    void set_client_info(const std::string &ip, const std::string &port);

private:
    int fd_;
    std::string uid_ =  "none";
    bool is_sended_key_ = false;
    gsky::util::vessel in_buffer_;
    unsigned char check_code_[2];
    bool is_invalid();

    status status_;
    
    std::function<void()> disconnecting_handler_;
    std::function<void(std::shared_ptr<gsky::util::vessel>)> send_data_handler_;
    std::function<void(std::shared_ptr<gsky::util::vessel>)> push_data_handler_;

    std::shared_ptr<gsky::net::pp::request>  request_;   // pp协议，request
    std::shared_ptr<gsky::net::pp::response> response_; // pp协议，response

    gsky::net::pp::server_handler server_handler_; // 回调函数

    unsigned char key_[8] = {0}; // 初始化为0
    std::map<std::string, std::string> client_info_;
    gsky::net::pp::header header_;
    int  body_left_length_ = 0;
    void handle_disconnect();
    void handle_work();
    void handle_error(pp::status s);
    void send_key(); // 发送pe key给客户端
    void send_data(const std::string &data);
    void push_data(const std::string &data);
};
