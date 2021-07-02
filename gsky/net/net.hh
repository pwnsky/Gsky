#pragma once
#include <unistd.h>
#include <iostream>
#include <cstdio>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <string>
#include <errno.h>
#include <cstring>
#include <openssl/ssl.h>
#include <openssl/err.h>


#include <gsky/gsky.hh>
#include <gsky/net/channel.hh>
#include <gsky/net/eventloop_threadpool.hh>

using logger = gsky::log::logger;

class gsky::net::net final{
public:
    net();
    net(int port, int number_of_thread);
    ~net();
    void init(int port, int number_of_thread);
    void start();
    void stop();
    void handle_new_connection();
    void handle_reset();
private:
    int listen();    // Bind port_ and listen
    bool started_;   // Store state of net if started
    bool listened_;  // Store state of net if listend
    int port_;       // Listen port
    int number_of_thread_; // The number of thread
    eventloop *base_eventloop_ = nullptr;
    int listen_fd;
    sp_channel accept_channel_;
    std::unique_ptr<eventloop_threadpool> up_eventloop_threadpool_;
    int accept_fd_sum = 0;
};

