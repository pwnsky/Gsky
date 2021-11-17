#pragma once


#include <gsky/net/channel.hh>
#include <gsky/net/eventloop_threadpool.hh>

namespace gsky {
namespace net {
class net final{
public:
    net();
    ~net();
    void init(const std::string &ip, int port, int threads);
    void start();
    void stop();
    void handle_new_connection();
    void handle_reset();

private:
    int listen();    // Bind port_ and listen
    bool started_;   // Store state of net if started
    bool listened_;  // Store state of net if listend
    std::string ip_;
    int port_;       // Listen port
    int threads_; // The number of thread
    eventloop *base_eventloop_ = nullptr;
    int listen_fd;
    std::shared_ptr<channel> accept_channel_;
    std::unique_ptr<eventloop_threadpool> up_eventloop_threadpool_;
    int accept_fd_sum = 0;
};

}
}
