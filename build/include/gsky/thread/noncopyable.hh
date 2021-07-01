#pragma once
#include <gsky/gsky.hh>

class gsky::thread::noncopyable {
protected:
    noncopyable() {}
    ~noncopyable() {}
private:
    noncopyable(const noncopyable &);
    const noncopyable &operator=(const noncopyable&);
};
