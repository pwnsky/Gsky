#pragma once

#include <string>
#include <pthread.h>
#include <cassert>
#include <functional>

#include <gsky/thread/noncopyable.hh>
#include <gsky/thread/count_down_latch.hh>

namespace gsky {
namespace thread {
namespace current_thread {
enum State{
    starting,
    runing,
    stopped
};

// __thread 代表每个线程会分配一个独立的空间
extern __thread pid_t  tid;
extern __thread State  state;
extern __thread char *name;
pid_t get_tid();
}

/* 线程类
 */
class thread : noncopyable {
public:
    explicit thread(const std::function<void()> &call_back, const std::string &name = std::string());
    ~thread();
    void start();
    int join();
    bool is_started();
    pid_t get_tid;
    const std::string &get_name();
    void set_name(const std::string &name);

private:
    static void *run(void *arg);
    bool started_;
    bool joined_;
    pthread_t pthread_id;
    pid_t tid_;
    std::function<void()> func_;
    std::string name_;
    count_down_latch count_down_latch_;
};


class thread_data {
public:
    thread_data(const std::function<void()> &func, const std::string &name,
               pid_t *tid, gsky::thread::count_down_latch *count_down_latch);
    ~thread_data();
    void run();

private:
    std::function<void()> func_;
    std::string name_;
    pid_t *tid_;
    gsky::thread::count_down_latch *count_down_latch_;
};

}}
