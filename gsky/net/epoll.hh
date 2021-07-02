#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <iostream>
#include <cassert>
#include <string.h>

#include <gsky/net/channel.hh>
#include <gsky/gsky.hh>

class gsky::net::epoll {
public:
    epoll();
    ~epoll();
    void add(sp_channel sp_channel);
    void mod(sp_channel sp_channel);
    void del(sp_channel sp_channel);
    std::vector<sp_channel> get_all_event_channels();
    std::vector<sp_channel> get_event_channels_after_get_events(int number_of_events);
    int get_epoll_fd();
private:
    int epoll_fd_;
    std::vector<epoll_event> v_events_;
    sp_channel sp_channels_[MAX_CONNECTED_FDS_NUM];
    sp_socket sp_sockets_[MAX_CONNECTED_FDS_NUM];
};
