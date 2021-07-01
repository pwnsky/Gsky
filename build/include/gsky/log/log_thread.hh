#pragma once

#include <gsky/thread/thread.hh>
#include <gsky/thread/mutex_lock.hh>
#include <gsky/thread/condition.hh>
#include <gsky/log/log.hh>

class gsky::log::log_thread {
public:
    log_thread();
    ~log_thread();
    log *creat();
    void stop();
    void set_log_path(const std::string &log_path) {
        log_path_ = log_path;
    }
private:
    log *log_;
    bool exiting_;
    gsky::thread::thread thread_;
    gsky::thread::mutex_lock mutex_lock_;
    gsky::thread::condition condition_;
    void thread_func();
    std::string log_path_;
};
