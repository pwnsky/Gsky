#include <gsky/log/logger.hh>

//extern std::string gsky::data::logger_path;
#include <sstream>
#include <queue>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <atomic>
#include <sys/sem.h>
#include <semaphore.h>
#include <iostream>

gsky::log::logger_manager *gsky::log::logger_manager_instance;

gsky::log::logger_io::logger_io(){
}

gsky::log::logger_io::~logger_io() {
}

bool gsky::log::logger_io::open(const std::string &logger_path) {
    logger_fd_ = ::open(logger_path.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0755);
    //std::cout << "log file: " << gsky::data::logger_path.c_str() << '\n';
    if(-1 == logger_fd_) {
        perror(("Open log file: " + logger_path).c_str());
        return false;
    }
    return true;
}

void gsky::log::logger_io::close() {
    ::close(logger_fd_);
}

void gsky::log::logger_io::write() {
    gsky::thread::mutex_lock_guard mutex_lock_guard(write_mutex_lock_);
    if(logs_.size() > 0) {
        std::string log = logs_.front();
        logs_.pop();
        ::write(logger_fd_, log.data(), log.size());
    }
}

void gsky::log::logger_io::push(const std::string &log) {
    gsky::thread::mutex_lock_guard mutex_lock_guard(write_mutex_lock_);
    logs_.push(log);
}


gsky::log::logger_manager::logger_manager() : quit_(false) ,
    cond_(log_wait_){
    quit_ = false;
}

gsky::log::logger_manager::~logger_manager() {

}

void gsky::log::logger_manager::set_logger_path(const std::string &logger_path) {
    logger_path_ = logger_path;
}

void gsky::log::logger_manager::push(const std::string &log) {
    logger_io_.push(log);
    cond_.signal();
}

void gsky::log::logger_manager::loop() {
    if(logger_io_.open(logger_path_) == false) {
        quit_ = true;
        cond_.broadcast();
        exit(-1);
        return;
    }

    while (!quit_) {
        cond_.wait();
        logger_io_.write();
    }
    std::cout << "stop log module\n";
    logger_io_.close();
}

void gsky::log::logger_manager::quit() {
    quit_ = true;
    cond_.broadcast();
}

void gsky::log::logger::operator<< (const std::string &t) {
    if(logger_manager_instance)
        logger_manager_instance->push('\n' + gsky::util::date_time() + '\n' + t);
}
