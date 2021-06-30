#pragma once
#include <pthread.h>
#include <time.h>
#include <errno.h>

#include <gsky/gsky.hh>
#include <gsky/thread/noncopyable.hh>
#include <gsky/thread/mutex_lock.hh>

class gsky::thread::condition : noncopyable {
  public:
    explicit condition(mutex_lock &ml) : mutex_(ml) {
        pthread_cond_init(&cond_, nullptr);
    }
    ~condition() {
        pthread_cond_destroy(&cond_);
    }

    void wait() {
        pthread_cond_wait(&cond_, mutex_.get_mutex());
    }
    void signal() {
        pthread_cond_signal(&cond_);
    }
    void broadcast() {
        pthread_cond_broadcast(&cond_);
    }
    bool wait_for_seconds(int seconds) {
        struct timespec time_spec;
        clock_gettime(CLOCK_REALTIME, &time_spec);
        time_spec.tv_sec += static_cast<__time_t>(seconds);
        return ETIMEDOUT == pthread_cond_timedwait(&cond_, mutex_.get_mutex(), &time_spec);
    }

private:
    mutex_lock &mutex_;
    pthread_cond_t cond_ = PTHREAD_COND_INITIALIZER;
};

