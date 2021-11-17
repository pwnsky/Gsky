#pragma once

#include <gsky/net/eventloop.hh>
#include <gsky/thread/thread.hh>
#include <gsky/thread/mutex_lock.hh>
#include <gsky/thread/condition.hh>

namespace gsky {
namespace net {
class eventloop_thread {
public:
     explicit eventloop_thread();
    ~eventloop_thread();
    eventloop *start_loop();
    void set_name(const std::string name) { name_ = name; }
    void stop_loop();
    eventloop *get_eventloop() { return eventloop_; }

private:
    eventloop *eventloop_;
    std::string name_ = "none";
    bool exiting_;
    gsky::thread::thread thread_;
    gsky::thread::mutex_lock mutex_lock_;
    gsky::thread::condition condition_;
    void thread_func();
};

}
}
