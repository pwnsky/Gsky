#pragma once
#include <functional>
#include <vector>
#include <memory>

#include <gsky/thread/thread.hh>
#include <gsky/net/epoll.hh>
#include <gsky/net/util.hh>

#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <atomic>

namespace gsky {
namespace net {
class eventloop {
public:
    eventloop();
    void set_name(const std::string name) { name_ = name; };
    ~eventloop() { //std::cout << "~eventloop(): \n";
    };
    int create_event_fd();
    void loop();
    void quit();
    void run_in_loop(std::function<void()> &&func);
    void push_back(std::function<void()> &&func);
    bool is_in_loop_thread();              // 判断是否在事件循环的线程中
    void assert_in_loop_thread();          // 在线程中断言
    void shutdown(sp_channel spc);                        // 关闭fd的写端
    void remove_from_epoll(sp_channel spc);                 // 移除事件
    void update_epoll(sp_channel spc); // 更新epoll事件
    void add_to_epoll(sp_channel spc);  // 添加epoll事件
private:
    bool looping_;
    int awake_fd_;
    std::atomic<bool> quit_;
    bool event_handling_;
    std::string name_ = "none";
    const pid_t thread_id_;
    sp_epoll sp_epoll_;
    sp_channel sp_awake_channel_;           // 用于唤醒的Channel
    mutable gsky::thread::mutex_lock mutex_lock_; // 互斥锁
    std::vector<std::function<void()>> pending_callback_functions_;
    bool calling_pending_callback_function_;

    void wake_up(); //for write one byte to client
    void run_pending_callback_func();
    void handle_read();
    void handle_reset();
};

}}
