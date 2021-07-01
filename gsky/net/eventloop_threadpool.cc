#include "eventloop_threadpool.hh"

gsky::net::eventloop_threadpool::eventloop_threadpool(eventloop *base_eventloop, int number_of_thread) :
    started_(false),
    base_eventloop_(base_eventloop),
    number_of_thread_(number_of_thread),
    next_thread_indx_(0) {
    if(number_of_thread <= 0) {
        d_cout << "The number of thread must be >= 1\n";
        abort();
    }
}

gsky::net::eventloop *gsky::net::eventloop_threadpool::get_next_eventloop() {
    base_eventloop_->assert_in_loop_thread();
    assert(started_);
    eventloop *eventloop = base_eventloop_;
    if(v_sp_eventloop_threads_.empty() == false) {
        eventloop = v_sp_eventloop_threads_[next_thread_indx_]->get_eventloop();
        next_thread_indx_ = (next_thread_indx_ + 1) % (number_of_thread_);
    }
    return eventloop;
}

void gsky::net::eventloop_threadpool::start() {
    started_ = true;
    base_eventloop_->assert_in_loop_thread();
    for(int idx = 0; idx < number_of_thread_; ++idx) {
        sp_eventloop_thread sp_elt(new eventloop_thread());
        sp_elt->set_name("thread eventloop " + std::to_string(idx));
        sp_elt->start_loop();
        v_sp_eventloop_threads_.push_back(sp_elt);
    }
}

void gsky::net::eventloop_threadpool::stop() {
    started_ = false;
    for(auto iter = v_sp_eventloop_threads_.begin(); iter != v_sp_eventloop_threads_.end(); ++iter) {
        sp_eventloop_thread et = *iter;
        et->stop_loop();
    }
}
