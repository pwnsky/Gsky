#pragma once
#include <memory>

#include <gsky/gsky.hh>
#include <gsky/net/eventloop.hh>
//#include <gsky/net/pp/socket.cc>

class gsky::net::socket final : public std::enable_shared_from_this<socket> {
enum class status {
    connected = 0,
    disconnecting,
    disconnected,
};
public:
    explicit socket(int fd, eventloop *elp);
    ~socket();
    void reset();
    sp_channel get_sp_channel();
    eventloop *get_eventloop();
    void handle_close();
    void new_evnet();
    void set_client_info(const std::string &ip, const std::string &port); // 设置客户端信息
    void send_data(std::shared_ptr<gsky::util::vessel> v);
    void push_data(std::shared_ptr<gsky::util::vessel> v); // 数据推送
    bool is_deleteble(); //是否可删除
    void set_disconnected();   //设置状态为已断开连接
    void set_disconnecting();  //设置状态为断开连接中

private:
    int fd_;
    eventloop *eventloop_;
    sp_channel sp_channel_;
    std::shared_ptr<gsky::util::vessel> out_buffer_ = nullptr;
    std::queue<std::shared_ptr<gsky::util::vessel>> out_buffer_queue_;
    int wait_event_count_ = 0; //用于计数等待事件的数量
    //gsky::net::pp::socket pp_socket_;
    status status_;
    std::string ip_;
    std::string port_;
    void handle_read();
    void handle_write();
    void handle_reset();
    void handle_push_data_reset();
};
