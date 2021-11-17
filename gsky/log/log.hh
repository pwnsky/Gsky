#pragma once

#include <string>
#include <iostream>

#define dlog gsky::log::debug() << "[" << __FILE__ << ": " << __LINE__ << "] "

namespace gsky {
namespace log {

class color {
public:
    static std::string red(const std::string &str);
    static std::string green(const std::string &str);
    static std::string yellow(const std::string &str);
    static std::string blue(const std::string &str);
    static std::string fuchsia(const std::string &str);
    static std::string cyan(const std::string &str);
    static std::string white(const std::string &str);
    static std::string reset(const std::string &str);
};


class log_base {
public:
    log_base(std::string header);
    virtual log_base &operator<<(const std::string &m);
    virtual log_base &operator<<(char m);
    virtual log_base &operator<<(unsigned char m);
    virtual log_base &operator<<(short m);
    virtual log_base &operator<<(unsigned short m);
    virtual log_base &operator<<(int m);
    virtual log_base &operator<<(unsigned int m);
    virtual log_base &operator<<(long int m);
    virtual log_base &operator<<(unsigned long int m);
    //virtual log_base &operator<<(ssize_t m);
    virtual log_base &operator<<(float m);
    virtual log_base &operator<<(double m);
//    virtual log_base &operator<<(char *m);
};

class info : public log_base {
public:
    info();
};

class debug : public log_base {
public:
    debug();
};


class warning : public log_base {
public:
    warning();
};

class error : public log_base {
public:
    error();

};

}
}
