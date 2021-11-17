#pragma once
#include <functional>
//#include <unordered_map>
//#include <memory>
//#include <gsky/net/eventloop.hh>
#include <gsky/net/socket.hh>

//#include <gsky/gsky.hh>
namespace gsky {
namespace net {
class eventloop;

class channel {
public:
    channel(eventloop *elp);
    channel(eventloop *elp, int fd);
    ~channel();
    void set_fd(int fd);
    int get_fd();
    std::shared_ptr<socket> get_holder();
    void set_holder(std::shared_ptr<socket> sock);

    void handle_read();
    void handle_write();
    void handle_reset(); // handle new connect
    void handle_event();
    void handle_error();
    void set_revent(__uint32_t revent);
    void set_event(__uint32_t event);
    void set_read_handler (std::function<void()> &&read_handler);
    void set_write_handler(std::function<void()> &&write_handler);
    void set_error_handler(std::function<void()> &&error_handler);
    void set_reset_handler(std::function<void()> &&reset_handler);
    __uint32_t &get_event();
    __uint32_t get_last_event();
    void update_last_evnet();
    bool is_last_event();

private:
    eventloop *elp_ = nullptr;
    int fd_ = -1;
    __uint32_t event_ = 0;
    __uint32_t revent_ = 0;
    __uint32_t last_event_ = 0;
    std::weak_ptr<gsky::net::socket> holder_;
    std::function<void()> read_handler_ = nullptr;
    std::function<void()> write_handler_ = nullptr;
    std::function<void()> error_handler_ = nullptr;
    std::function<void()> reset_handler_ = nullptr;
};

}
}
