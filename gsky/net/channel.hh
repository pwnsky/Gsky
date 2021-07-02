#pragma once
#include <functional>
#include <unordered_map>
#include <memory>
#include <iostream>
#include <sys/epoll.h>

#include <gsky/net/eventloop.hh>
#include <gsky/net/socket.hh>
#include <gsky/gsky.hh>

class gsky::net::channel {
public:
    channel(eventloop *elp);
    channel(eventloop *elp, int fd);
    ~channel();
    void set_fd(int fd);
    int get_fd();
    gsky::net::sp_socket get_holder();
    void set_holder(sp_socket sock);

    void handle_read();
    void handle_write();
    void handle_reset(); // handle new connect
    void handle_event();
    void handle_error();
    void set_revent(__uint32_t revent);
    void set_event(__uint32_t event);
    void set_read_handler (gsky::util::callback &&read_handler);
    void set_write_handler(gsky::util::callback &&write_handler);
    void set_error_handler(gsky::util::callback &&error_handler);
    void set_reset_handler(gsky::util::callback &&reset_handler);
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
    gsky::util::callback read_handler_ = nullptr;
    gsky::util::callback write_handler_ = nullptr;
    gsky::util::callback error_handler_ = nullptr;
    gsky::util::callback reset_handler_ = nullptr;
};
