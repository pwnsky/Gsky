#pragma once
#include <gsky/gsky.hh>
namespace gsky {
namespace thread {
class noncopyable {
protected:
    noncopyable() {}
    ~noncopyable() {}
private:
    noncopyable(const noncopyable &);
    const noncopyable &operator=(const noncopyable&);
};

}
}
