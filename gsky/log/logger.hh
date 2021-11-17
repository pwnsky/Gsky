#pragma once

#include <gsky/util/util.hh>
#include <gsky/thread/condition.hh>
#include <atomic>
#include <queue>

// handling in log eventloop thread
namespace gsky {
namespace log {
class logger_manager;
extern logger_manager *logger_manager_instance;

class logger_io {
public:
    logger_io();
    ~logger_io();
    void close();
    bool open(const std::string &log_path);
    void write();
    void push(const std::string &log);
    void wait();
private:
    int logger_fd_ = -1;
    std::queue<std::string> logs_;
    thread::mutex_lock write_mutex_lock_; //avoid call write at same time
};

class logger_manager {
public:
    logger_manager();
    ~logger_manager();
    void loop();
    void quit();
    void set_logger_path(const std::string &logger_path);
    void push(const std::string &log);

private:
    std::atomic<bool> quit_;
    logger_io logger_io_;
    std::string logger_path_;
    gsky::thread::mutex_lock log_wait_;
    gsky::thread::condition cond_;
};

class logger {
public:
    void operator<< (const std::string &t);
private:

};

}
}
