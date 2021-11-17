#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <iostream>
#include <cassert>
#include <string.h>

#include <gsky/net/channel.hh>
#include <gsky/gsky.hh>

namespace gsky {
namespace net {
class channel;
class socket;
class epoll {
public:
    epoll();
    ~epoll();
    void add(std::shared_ptr<gsky::net::channel> spc);
    void mod(std::shared_ptr<gsky::net::channel> spc);
    void del(std::shared_ptr<gsky::net::channel> spc);
    std::vector<std::shared_ptr<gsky::net::channel>> get_all_event_channels();
    std::vector<std::shared_ptr<gsky::net::channel>> get_event_channels_after_get_events(int number_of_events);
    int get_epoll_fd();
private:
    int epoll_fd_;
    std::vector<epoll_event> v_events_;
    std::shared_ptr<channel> sp_channels_[MAX_CONNECTED_FDS_NUM];
    std::shared_ptr<socket> sp_sockets_[MAX_CONNECTED_FDS_NUM];
};

}
}
