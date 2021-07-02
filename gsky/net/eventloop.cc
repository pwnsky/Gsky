#include "eventloop.hh"

gsky::net::eventloop::eventloop() :
    looping_(false),
    awake_fd_(eventloop::create_event_fd()),
    quit_(false),
    event_handling_(false),
    thread_id_(gsky::thread::current_thread::get_tid()),
    sp_epoll_(new epoll()),
    sp_awake_channel_(new channel(this, awake_fd_))
{
    sp_awake_channel_->set_event(EPOLLIN | EPOLLET); //init epoll event
    sp_awake_channel_->set_read_handler(std::bind(&eventloop::handle_read, this));
    sp_awake_channel_->set_reset_handler(std::bind(&eventloop::handle_reset, this));
    sp_epoll_->add(sp_awake_channel_);
}

// 读取传输到缓冲区前的数据
void gsky::net::eventloop::handle_read() {
    char buf[8];
    ssize_t read_len = gsky::net::util::read(awake_fd_, &buf, sizeof(buf));
    if(read_len != sizeof (buf)) {
        std::cout << "eventloop::hand_read() reads " << read_len << "instead of 8\n";
    }
}

void gsky::net::eventloop::handle_reset() {
    sp_awake_channel_->set_event(EPOLLIN | EPOLLET);
}

void gsky::net::eventloop::update_epoll(sp_channel spc) {
    sp_epoll_->mod(spc);
}

void gsky::net::eventloop::add_to_epoll(sp_channel spc) {
    sp_epoll_->add(spc);
}


int gsky::net::eventloop::create_event_fd() {
    /*
    Enable the close-on-exec flag for the new file descriptor.
    Specifying this flag permits a program to avoid additional
    fcntl(2) F_SETFD operations to set the FD_CLOEXEC flag.
    */
    int event_fd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if(event_fd < 0) {
        perror("eventfd:");
        abort();
    }
    return event_fd;
}

void gsky::net::eventloop::loop() {
    assert(looping_ == false);
    assert(is_in_loop_thread());
    looping_ = true;
    quit_ = false;
    std::vector<sp_channel> v_sp_event_channels;
    while(!quit_) {
#ifdef DEBUG
        d_cout << "event looping!\n";
#endif
        v_sp_event_channels.clear();
        v_sp_event_channels = sp_epoll_->get_all_event_channels(); // get all unhandle events
        event_handling_ = true;
        for (auto &sp_channel : v_sp_event_channels) {
            if(sp_channel)
                sp_channel->handle_event(); // handle event
#ifdef DEBUG
        d_cout << "call handle_event\n";
#endif
        }
        event_handling_  = false;
        run_pending_callback_func();   // run pending callback function
        //sp_epoll_->handle_expired_event();
    }
}

void gsky::net::eventloop::quit() {
    quit_ = true;
    if(is_in_loop_thread() == false) {
        wake_up();
    }
}
void gsky::net::eventloop::run_in_loop(std::function<void()> &&func) {
    if(is_in_loop_thread())
        func();
    else
        push_back(std::move(func)); //加入待处理函数中
}

void gsky::net::eventloop::push_back(std::function<void()> &&func) {
    gsky::thread::mutex_lock_guard mutex_lock_guard(mutex_lock_);
    pending_callback_functions_.emplace_back(std::move(func));
    if(!is_in_loop_thread() || calling_pending_callback_function_)
        wake_up();
}

//判断是否在事件循环的线程中
bool gsky::net::eventloop::is_in_loop_thread() {
    // 只需通过tid来进行判断, 若是事件循环的线程, 那么是通过Thread创建出来的, 所以Thread::CurrentThread::get_tid()的值是子线程的值,
    // 这就可以判断是否为子线程了, 而thread_id_始终是父线程的id
    return thread_id_ == gsky::thread::current_thread::get_tid();
}

// 从epoll中移除事件
void gsky::net::eventloop::remove_from_epoll(sp_channel sp_channel) {
    sp_epoll_->del(sp_channel);
}

// 用于保持长连接避免断开连接
void gsky::net::eventloop::wake_up() {
    char buf[8];
    ssize_t write_len = gsky::net::util::write(awake_fd_, buf, sizeof (buf));
    if(write_len != sizeof (buf)) {
        d_cout << "eventloop::wakeup write:" << write_len << " instead of 8\n";
    }
}

// 运行待运行的回调函数
void gsky::net::eventloop::run_pending_callback_func() {
    std::vector<std::function<void()>> v_callback_functions;
    calling_pending_callback_function_ = true;
    gsky::thread::mutex_lock_guard mutex_lock_guard(mutex_lock_); // 保证线程单个线程执行

    //Calling all functions in pending vecotr
    v_callback_functions.swap(pending_callback_functions_); // 获取所有等待的毁掉函数
    // 依次运行回调函数
    for( size_t idx = 0; idx < v_callback_functions.size(); ++idx) {
        std::function<void()> func = v_callback_functions[idx];
        if(func)
            func(); // 依次运行
    }
    calling_pending_callback_function_ = false;
}

void gsky::net::eventloop::assert_in_loop_thread() {
    assert(is_in_loop_thread());
}

void gsky::net::eventloop::shutdown(sp_channel spc) {
    gsky::net::util::shutdown_write_fd(spc->get_fd());
}
