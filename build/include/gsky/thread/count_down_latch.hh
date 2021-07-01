#pragma once
#include <gsky/thread/noncopyable.hh>
#include <gsky/thread/condition.hh>
#include <gsky/thread/mutex_lock.hh>
/*
 * 这个类的主要功能是确保在主线程运行之前确保创建的线程启动成功
 */
class gsky::thread::count_down_latch : noncopyable {
public:
    explicit count_down_latch(int count) :
        count_(count), mutex_(), condition_(mutex_) {
    }
    ~count_down_latch() {}

    void wait() {
        mutex_lock_guard mlg(mutex_);
        while(count_ > 0) condition_.wait();
    }
    void count_down() {
        mutex_lock_guard mlg(mutex_);
        --count_;
        if(count_ <= 0)  condition_.broadcast();
    }
private:
    int count_;
    mutable mutex_lock mutex_;
    condition condition_;
};
