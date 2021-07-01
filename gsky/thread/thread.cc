#include <gsky/thread/thread.hh>

__thread pid_t  gsky::thread::current_thread::tid = 0;
__thread gsky::thread::current_thread::State gsky::thread::current_thread::state = current_thread::State::stopped;
__thread char *gsky::thread::current_thread::name;

gsky::thread::thread::thread(const gsky::util::callback &call_back, const std::string &name) :
    started_(false),
    joined_(false),
    pthread_id(0),
    tid_(0),
    func_(call_back),
    name_(name),
    count_down_latch_(1) { // 等待机制, 单独一次开启一个开启线程, 就设置为1
#ifdef DEBUG
        dbg_log("gsky::thread::thread::thread()");
#endif
}

gsky::thread::thread::~thread() {
#ifdef DEBUG
        dbg_log("gsky::thread::thread::~thread()");
#endif
    //std::cout << "~thread\n";
    if(started_ && !joined_)
        pthread_detach(pthread_id);
}

void gsky::thread::thread::set_name(const std::string &name) {
    name_ = name;
}

void gsky::thread::thread::start() {
    assert(!started_);
    started_ = true;
    gsky::thread::thread_data *tdp = new thread_data(func_, name_, &tid_, &count_down_latch_);

    if(pthread_create(&pthread_id, nullptr, &gsky::thread::thread::run, tdp)) {
        started_ = false;
        std::cout << "create thread fail\n";
        delete tdp;
    }else {
        count_down_latch_.wait();
        assert(tid_ > 0);
    }
}

void *gsky::thread::thread::run(void *arg) {
    gsky::thread::thread_data *tdp = static_cast<thread_data *>(arg);
    tdp->run();
    delete tdp;
    return nullptr;
}

int gsky::thread::thread::join() {
    assert(started_);
    assert(!joined_);
    joined_ = true;
    return pthread_join(pthread_id, nullptr);
}

bool gsky::thread::thread::is_started() {
    return started_;
}

// 用于创建线程传递参数
gsky::thread::thread_data::thread_data(const gsky::util::callback &func, const std::string &name,
                              pid_t *tid, count_down_latch *cdlp) :
    func_(func),
    name_(name),
    tid_(tid),
    count_down_latch_(cdlp) {
}

gsky::thread::thread_data::~thread_data() {

}

void gsky::thread::thread_data::run() {

    *tid_ = current_thread::get_tid();
    count_down_latch_->count_down(); // 线程运行成功 --count
    tid_ = nullptr;                 // 不用该值
    count_down_latch_ = nullptr;    // 不用该值
    current_thread::name = const_cast<char *>(name_.empty() ? "thread" : name_.c_str());
    current_thread::state = current_thread::State::starting;
    prctl(PR_SET_NAME, current_thread::name); // 设置当前线程名称, 在内核中, 进程与线程都是单独的
    func_();                       //执行回调函数, 这里的回调函数是创建gsky::thread对象传递过来的回调

    current_thread::name = const_cast<char *>("finished"); // 设置当前线程名称为finish
    current_thread::state = current_thread::State::stopped;
}
