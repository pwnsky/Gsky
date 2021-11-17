#include <gsky/log/logger_thread.hh>

gsky::log::logger_thread::logger_thread() :
    logger_manager_(nullptr),
    exiting_(false),
    thread_(std::bind(&logger_thread::thread_func, this), "logger eventloop thread!"),
    mutex_lock_(),
    condition_(mutex_lock_) {
}

gsky::log::logger_thread::~logger_thread() {
    exiting_ = true;
    if(logger_manager_!= nullptr) {
        logger_manager_->quit();
        thread_.join();
    }
}

bool gsky::log::logger_thread::creat() {
    assert(thread_.is_started() == false);
    thread_.start();
    //Waiting for run
    gsky::thread::mutex_lock_guard mutex_lock_guard(mutex_lock_);
    while(logger_manager_ == nullptr)
        condition_.wait();
    logger_manager_instance = logger_manager_;
    return true;
}

void gsky::log::logger_thread::thread_func() {
    logger_manager l;
    logger_manager_ = &l;
    logger_manager_->set_logger_path(logger_path_); // set the path to log
    condition_.signal();           // Notify Main thread then realize Sync
    logger_manager_->loop();                  // run event
    logger_manager_ = nullptr;
}

void gsky::log::logger_thread::stop() {
    logger_manager_->quit();
    logger_manager_instance = nullptr;
}

void gsky::log::logger_thread::set_logger_path(const std::string &logger_path) {
    logger_path_ = logger_path;
}
