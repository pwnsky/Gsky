#include <gsky/net/eventloop_thread.hh>

gsky::net::eventloop_thread::eventloop_thread() :
    eventloop_(nullptr),
    exiting_(false),
    thread_(std::bind(&eventloop_thread::thread_func, this), "eventloop_thread"),
    mutex_lock_(),
    condition_(mutex_lock_) {

}

gsky::net::eventloop_thread::~eventloop_thread() {
    exiting_ = true;
    if(eventloop_ != nullptr) {
        eventloop_->quit();
        thread_.join();
    }
}
gsky::net::eventloop *gsky::net::eventloop_thread::start_loop() {
    assert(thread_.is_started() == false);
    thread_.start();

    //Waiting for run
    gsky::thread::mutex_lock_guard mutex_lock_guard(mutex_lock_);
    while(eventloop_ == nullptr)
        condition_.wait();

    // return a new eventloop object ptr
    return eventloop_;
}
void gsky::net::eventloop_thread::thread_func() {
    // create a new eventloop
    eventloop el;
    el.set_name(name_);
    eventloop_ = &el;
    condition_.signal(); // Notify Main thread then realize Sync
    eventloop_->loop();  //run event
    eventloop_ = nullptr;
}

void gsky::net::eventloop_thread::stop_loop() {
    if(eventloop_)
        eventloop_->quit();
}
