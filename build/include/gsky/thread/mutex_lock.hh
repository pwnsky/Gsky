#pragma once
#include <gsky/thread/noncopyable.hh>
#include <pthread.h>

class gsky::thread::mutex_lock : noncopyable {
public:
    mutex_lock() {
        pthread_mutex_init(&mutex_, nullptr);
    }
    ~mutex_lock() {
        // 线程调用该函数让互斥锁上锁，如果该互斥锁已被另一个线程锁定和拥有，则调用该线程将阻塞，直到该互斥锁变为可用为止
        pthread_mutex_lock(&mutex_);
        // 销毁互斥锁
        pthread_mutex_destroy(&mutex_);
    }
    void lock() {
        pthread_mutex_lock(&mutex_);
    }
    void unlock() {
        pthread_mutex_unlock(&mutex_);
    }

    pthread_mutex_t *get_mutex() {
        return &mutex_;
    }
private:
    pthread_mutex_t mutex_ = PTHREAD_MUTEX_INITIALIZER;
    //Friend classes are not affected by access rights
    friend class Condition;
};

class gsky::thread::mutex_lock_guard : gsky::thread::noncopyable {
public:
    explicit mutex_lock_guard(gsky::thread::mutex_lock &mutex_lock) : mutex_lock_(mutex_lock){
        mutex_lock_.lock();
    }
    ~mutex_lock_guard(){
        mutex_lock_.unlock();
    }

private:
    mutex_lock &mutex_lock_;
};

