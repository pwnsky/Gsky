#pragma once

#include <string>
#include <pthread.h>
#include <cstdio>
#include <cassert>
#include <unistd.h>
#include <syscall.h>
#include <sys/prctl.h>

#include <gsky/thread/noncopyable.hh>
#include <gsky/thread/count_down_latch.hh>
#include <gsky/gsky.hh>

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

// 该函数用于获取该线程的一个id值, 该值是内核中独一无二的值, 主要用于辨别不同进程中不同的线程
inline pid_t get_tid() {
/* #define likely(x) __builtin_expect(!!(x), 1)   //x likely as true
 * #define unlikely(x) __builtin_expect(!!(x), 0) //x likely as false
 */
    // 这个表示很有可能为假, 即在编译的时候告诉编译器, 若为假, 就不用跳转, 这是在汇编代码中体现的, 主要用于优化代码, 提高执行效率
    if(__builtin_expect(current_thread::tid == 0, 0)) {
        current_thread::tid = ::syscall(SYS_gettid);  //get real trhead id
    }
    return current_thread::tid;
}
}}}

/* 线程类
 */
class gsky::thread::thread : gsky::thread::noncopyable {
public:
    explicit thread(const gsky::util::callback &call_back, const std::string &name = std::string());
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
    gsky::util::callback func_;
    std::string name_;
    count_down_latch count_down_latch_;
};


class gsky::thread::thread_data {
public:
    thread_data(const gsky::util::callback&func, const std::string &name,
               pid_t *tid, gsky::thread::count_down_latch *count_down_latch);
    ~thread_data();
    void run();

private:
    gsky::util::callback func_;
    std::string name_;
    pid_t *tid_;
    gsky::thread::count_down_latch *count_down_latch_;
};
