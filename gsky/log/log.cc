#include <gsky/log/log.hh>

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

gsky::log::log *gsky::data::p_log;
std::string gsky::data::log_path;


gsky::log::io::io(){
}

gsky::log::io::~io() {
}

bool gsky::log::io::open(const std::string &log_path) {
    log_fd_ = ::open(log_path.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0755);
    //std::cout << "log file: " << gsky::data::log_path.c_str() << '\n';
    if(-1 == log_fd_) {
        perror(("Open log file: " + log_path).c_str());
        return false;
    }
    return true;
}

void gsky::log::io::close() {
    ::close(log_fd_);
}

void gsky::log::io::write() {
    gsky::thread::mutex_lock_guard mutex_lock_guard(write_mutex_lock_);
    if(logs_.size() > 0) {
        std::string log = logs_.front();
        logs_.pop();
        ::write(log_fd_, log.data(), log.size());
    }
}

void gsky::log::io::push(const std::string &log) {
    gsky::thread::mutex_lock_guard mutex_lock_guard(write_mutex_lock_);
    logs_.push(log);
}


gsky::log::log::log() : quit_(false) ,
    cond_(log_wait_){
    quit_ = false;
}

gsky::log::log::~log() {

}
void gsky::log::log::loop() {
    if(io_.open(log_path_) == false) {
        quit_ = true;
        cond_.broadcast();
        exit(-1);
        return;
    }

    while (!quit_) {
        cond_.wait();
        io_.write();
    }
    std::cout << "stop log module\n";
    io_.close();
}

void gsky::log::log::quit() {
    quit_ = true;
    cond_.broadcast();
}
