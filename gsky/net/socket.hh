#pragma once

#include <memory>
#include <cassert>
#include <queue>
#include <gsky/net/pp/socket.hh>
#include <gsky/log/log.hh>

namespace gsky {
namespace net {
class channel;
class eventloop;

class socket final : public std::enable_shared_from_this<socket> {
enum class status {
    connected = 0,
    disconnecting,
    disconnected,
};
public:
    explicit socket(int fd, eventloop *elp);
    ~socket();
    void reset();
    std::shared_ptr<gsky::net::channel> get_channel();
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
    std::shared_ptr<gsky::net::channel> sp_channel_;
    std::shared_ptr<gsky::util::vessel> out_buffer_ = nullptr;
    std::queue<std::shared_ptr<gsky::util::vessel>> out_buffer_queue_;
    int wait_event_count_ = 0; //用于计数等待事件的数量
    std::shared_ptr<gsky::net::pp::socket> sp_pp_socket_;
    status status_;
    void handle_read();
    void handle_write();
    void handle_reset();
    void handle_push_data_reset();
};

}
}
