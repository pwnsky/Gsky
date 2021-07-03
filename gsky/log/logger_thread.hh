#pragma once

#include <gsky/thread/thread.hh>
#include <gsky/thread/mutex_lock.hh>
#include <gsky/thread/condition.hh>
#include <gsky/log/logger.hh>

namespace gsky{
namespace log {
class logger_thread {
public:
    logger_thread();
    ~logger_thread();
    bool creat();
    void stop();
    void set_logger_path(const std::string &logger_path);
private:
    gsky::log::logger_manager *logger_manager_ = nullptr;
    bool exiting_;
    gsky::thread::thread thread_;
    gsky::thread::mutex_lock mutex_lock_;
    gsky::thread::condition condition_;
    void thread_func();
    std::string logger_path_;
};

}
}
