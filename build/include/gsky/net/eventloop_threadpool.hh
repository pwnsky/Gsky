#pragma once

#include <gsky/gsky.hh>
#include <gsky/net/eventloop.hh>
#include <gsky/net/eventloop_thread.hh>

class gsky::net::eventloop_threadpool {
public:
    eventloop_threadpool(eventloop *base_eventloop, int number_of_thread);
    ~eventloop_threadpool() {};
    void start();
    void stop();
    eventloop *get_next_eventloop();
private:
    bool started_;
    eventloop *base_eventloop_;
    int number_of_thread_;
    int next_thread_indx_;
    std::vector<sp_eventloop_thread> v_sp_eventloop_threads_;
};
