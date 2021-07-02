#include <gsky/net/socket.hh>

#include <time.h>
#include <stdlib.h>
//#include <arpa/inet.h>
#include <pthread.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

#include <gsky/net/pp/socket.hh>
#include <gsky/net/eventloop.hh>

const __uint32_t EPOLL_DEFAULT_EVENT = EPOLLIN | EPOLLET | EPOLLONESHOT;

gsky::net::socket::socket(int fd, eventloop *elp) :
    fd_(fd),
    eventloop_(elp),
    sp_channel_(new channel(elp, fd)),
    sp_pp_socket_(new gsky::net::pp::socket(fd)),
    status_(status::connected) {

    //sp_work_(new gsky::work::work(map_client_info_, in_buffer_)) {
#ifdef DEBUG
    d_cout << "call gsky::net::socket::socket()\n";
#endif
    //set callback function handler
    sp_channel_->set_read_handler(std::bind(&socket::handle_read, this));
    sp_channel_->set_write_handler(std::bind(&socket::handle_write, this));
    sp_channel_->set_reset_handler(std::bind(&socket::handle_reset, this));

    sp_pp_socket_->set_send_data_handler(std::bind(&socket::send_data, this, std::placeholders::_1));
    sp_pp_socket_->set_push_data_handler(std::bind(&socket::push_data, this, std::placeholders::_1));
    sp_pp_socket_->set_disconnecting_handler(std::bind(&socket::set_disconnecting, this));
    //sp_pp_socket->set_disconnected_handler(std::bind(&socket::push_data, this));

}

gsky::net::socket::~socket() {
#ifdef DEBUG
    d_cout << "call gsky::net::socket::~socket()\n";
#endif
    close(fd_);
    // delete all
    while(this->out_buffer_queue_.size() > 0) {
            auto buf = out_buffer_queue_.front();
            out_buffer_queue_.pop();
    }
}

void gsky::net::socket::set_disconnected() {
    __uint32_t &event = sp_channel_->get_event();
    status_ = status::disconnected;
    event |= EPOLLIN;
}

void gsky::net::socket::set_disconnecting() {
#ifdef DEBUG
    d_cout << "gsky::net::socket::set_disconnecting\n";
#endif
    status_ = status::disconnecting;
}

// 关闭释放连接
void gsky::net::socket::handle_close() {
#ifdef DEBUG
    d_cout << "call gsky::net::socket::handle_close()\n";
#endif
    sp_socket guard(shared_from_this()); // avoid delete
    eventloop_->remove_from_epoll(sp_channel_);
}

void gsky::net::socket::new_evnet() {
    sp_channel_->set_event(EPOLL_DEFAULT_EVENT);
    eventloop_->add_to_epoll(sp_channel_);
}

gsky::net::sp_channel gsky::net::socket::get_sp_channel() {
    return sp_channel_;
}

gsky::net::eventloop *gsky::net::socket::get_eventloop() {
    return eventloop_;
}

// 接收数据回调
void gsky::net::socket::handle_read() {

    // 接收数据进行pp协议解析
    sp_pp_socket_->handle_read();

    /*
    int len= sizeof(info);
    getsockopt(fd_, IPPROTO_TCP, TCP_INFO, &info, (socklen_t *)&len);
    if((info.tcpi_state != TCP_ESTABLISHED)) { // 断开连接
            std::cout << "断开\n";
    }else {
        // 数据处理
        std::cout << "接收数据\n";
    }
    */
}


// 写入数据函数，负责循环剩余数据或队列中的数据。
void gsky::net::socket::handle_write() {
    __uint32_t &event = sp_channel_->get_event();

    if(out_buffer_queue_.size() == 0)
        return;

    out_buffer_ = out_buffer_queue_.front();

    if(status_ == status::disconnected) {
        return;
    }
#ifdef DEBUG
    dbg_log("write size: " + std::to_string(out_buffer_->size()) + "] end");
#endif
    if(out_buffer_->size() > 0) {
        if(util::write(fd_, out_buffer_) < 0) {
            perror("write header data");
            sp_channel_->set_event(0);
            //out_buffer_->clear();
        }
        if(out_buffer_->size() == 0) { // 数据发送完毕后，若out_buffer_queue_还存在待发送数据，则继续发送
            if(out_buffer_queue_.size() > 0)
                out_buffer_queue_.pop();
            out_buffer_.reset();
            if(out_buffer_queue_.size() > 0) {
                event |= EPOLLOUT; // next round set event as EPOLLOUT
            }
            return;
        }
    }
    if (out_buffer_->size() > 0)
        event |= EPOLLOUT; // next round set event as EPOLLOUT
}

// channel调用后handle函数后处理
void gsky::net::socket::handle_reset() {
    __uint32_t &event = sp_channel_->get_event();

    if(status_ == status::connected) {
        if(event != 0) {
            if((event & EPOLLIN) && (event & EPOLLOUT)) {
                event = 0;
                event |= EPOLLOUT;
            }
            event |= EPOLLET;
        } else {
            event |= (EPOLLIN | EPOLLET);
        }
        eventloop_->update_epoll(sp_channel_);
    } else if (status_ == status::disconnecting
               && (event & EPOLLOUT)) {
        event = (EPOLLOUT | EPOLLET); // 再次回调，处理断开连接。
        status_ = status::disconnected;
    } else { // disconnected

#ifdef DEBUG
        std::cout << " disconnected " << std::endl;
#endif
        // 异步处理关闭
        eventloop_->run_in_loop(std::bind(&socket::handle_close, shared_from_this()));
    }
}

/*
 * 发送数据: 采用同步方式进行数据发送
 * */
void gsky::net::socket::send_data(std::shared_ptr<gsky::util::vessel> v) {
    out_buffer_queue_.push(v);
    handle_write();
}

/*
 * 数据推送: 采用异步方式进行数据推送。
 * */

void gsky::net::socket::push_data(std::shared_ptr<gsky::util::vessel> v) {
    out_buffer_queue_.push(v); // 将数据放入队列
    wait_event_count_ ++; // 计数++
    // 用于异步回调 handle_push_data_reset函数进行更新epoll事件
    eventloop_->run_in_loop(std::bind(&socket::handle_push_data_reset, shared_from_this()));
}

// 采用单一线程进行更新epoll事件，多线程易出现条件竞争，导致内存错误
void gsky::net::socket::handle_push_data_reset() {
    wait_event_count_ --; // 计数--
    __uint32_t &event = sp_channel_->get_event();
    event |= EPOLLIN | EPOLLET | EPOLLOUT;
    // 回调写入函数
    eventloop_->update_epoll(sp_channel_); // 更新事件
}

void gsky::net::socket::set_client_info(const std::string &ip, const std::string &port) {
    sp_pp_socket_->set_client_info(ip, port);
}

bool gsky::net::socket::is_deleteble() {
    if(wait_event_count_ > 0)
        return false;
    return true;
}
