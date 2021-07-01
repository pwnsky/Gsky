#include <gsky/log/log_thread.hh>

gsky::log::log_thread::log_thread() :
    log_(nullptr),
    exiting_(false),
    thread_(std::bind(&log_thread::thread_func, this), "log eventloop thread!"),
    mutex_lock_(),
    condition_(mutex_lock_) {
}

gsky::log::log_thread::~log_thread() {
    exiting_ = true;
    if(log_ != nullptr) {
        log_->quit();
        thread_.join();
    }
}

gsky::log::log *gsky::log::log_thread::creat() {
    assert(thread_.is_started() == false);
    thread_.start();
    //Waiting for run
    gsky::thread::mutex_lock_guard mutex_lock_guard(mutex_lock_);
    while(log_ == nullptr)
        condition_.wait();
    // return a new log object ptr
    return log_;
}

void gsky::log::log_thread::thread_func() {
    log l;
    log_ = &l;
    log_->set_log_path(log_path_); // set the path to log
    condition_.signal();           // Notify Main thread then realize Sync
    log_->loop();                  // run event
    log_ = nullptr;
}

void gsky::log::log_thread::stop() {
     log_->quit();
     gsky::data::p_log = nullptr;
}

