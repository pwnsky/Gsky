#pragma once

#include <gsky/util/util.hh>
#include <gsky/thread/condition.hh>
#include <gsky/gsky.hh>

extern gsky::log::log *gsky::data::p_log;
extern std::string gsky::data::log_path;

#include <atomic>
// handling in log eventloop thread
class gsky::log::io {
public:
    io();
    ~io();
    void close();
    bool open(const std::string &log_path);
    void write();
    void push(const std::string &log);
    void wait();
private:
    int log_fd_ = -1;
    std::queue<std::string> logs_;
    thread::mutex_lock write_mutex_lock_; //avoid call write at same time
};

class gsky::log::log {
public:
    log();
    ~log();
    void loop();
    void quit();
    void set_log_path(const std::string &log_path) {
        log_path_ = log_path;
    }
    void push(const std::string &log) {
        io_.push(log);
        cond_.signal();
    }

private:
    std::atomic<bool> quit_;
    io io_;
    std::string log_path_;
    gsky::thread::mutex_lock log_wait_;
    gsky::thread::condition cond_;
};

class gsky::log::logger {
public:
    void operator<< (const std::string &t) {
        if(gsky::data::p_log)
            gsky::data::p_log->push('\n' + gsky::util::date_time() + '\n' + t);
    }
private:

};
