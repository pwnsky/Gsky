#include <gsky/net/channel.hh>

#include <iostream>
#include <sys/epoll.h>

gsky::net::channel::channel(eventloop *elp) {
    elp_ = elp;
}

gsky::net::channel::channel(eventloop *elp, int fd) {
    elp_ = elp;
    fd_ = fd;
}

gsky::net::channel::~channel() {

}

void gsky::net::channel::set_fd(int fd) {
    fd_ = fd;
}

int gsky::net::channel::get_fd() {
    return fd_;
}

void gsky::net::channel::set_holder(std::shared_ptr<gsky::net::socket> sock) {
    holder_ = sock;
}

std::shared_ptr<gsky::net::socket> gsky::net::channel::get_holder() {
    std::shared_ptr<gsky::net::socket> ret = holder_.lock();
    return ret;
}

void gsky::net::channel::handle_read() {
    if(read_handler_) {
        read_handler_();
    }
}

void gsky::net::channel::handle_write() {
    if(write_handler_) {
        write_handler_();
    }
}

void gsky::net::channel::handle_reset() {
    if(reset_handler_) {
        reset_handler_();
    }
}
void gsky::net::channel::handle_error() {
    if(error_handler_) {
        error_handler_();
    }
}

void gsky::net::channel::handle_event() {
#ifdef DEBUG
    dlog << "call gsky::net::channel::handle_event\n";
#endif
    event_ = 0; //处理后的事件清0
    if((revent_ & EPOLLHUP) && !(revent_ & EPOLLIN)) {
        event_ = 0;
#ifdef DEBUG
    dlog << "call gsky::net::channel::handle_event-> clean\n";
#endif
        return;
    }
    // revent是上一个事件
    // 处理错误
    if(revent_ & EPOLLERR) {
        if(error_handler_) handle_error();
        event_ = 0;
#ifdef DEBUG
    dlog << "call gsky::net::channel::handle_event-> error\n";
#endif
        return ;
    }

    // 优先处理有数据将要写入
    if(revent_ & EPOLLOUT) {
#ifdef DEBUG
    dlog << "call gsky::net::channel::handle_event-> write\n";
#endif
        handle_write();
    }

    // 有数据来临
    if(revent_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
#ifdef DEBUG
    dlog << "call gsky::net::channel::handle_event-> read\n";
#endif
        handle_read();
    }

    //*******
    handle_reset();
}

void gsky::net::channel::set_revent(__uint32_t revent) {
    revent_ = revent;
}
void gsky::net::channel::set_event(__uint32_t event) {
    event_ = event;
}

void gsky::net::channel::set_read_handler(std::function<void()>  &&read_handler) {
    read_handler_ = read_handler;
}

void gsky::net::channel::set_write_handler(std::function<void()>  &&write_handler) {
    write_handler_ = write_handler;
}

void gsky::net::channel::set_error_handler(std::function<void()>  &&error_handler) {
    error_handler_ = error_handler;
}

// For deal with connected client event
void gsky::net::channel::set_reset_handler(std::function<void()>  &&reset_handler) {
    reset_handler_ = reset_handler;
}

__uint32_t &gsky::net::channel::get_event() {
    return event_;
}

__uint32_t gsky::net::channel::get_last_event() {
    return last_event_;
}

void gsky::net::channel::update_last_evnet() {
    last_event_ = event_;
}

bool gsky::net::channel::is_last_event() {
    return last_event_ == event_;
}
