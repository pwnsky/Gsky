#include <gsky/log/log.hh>

std::string gsky::log::color::red(const std::string &str) {
        std::string ret_str  = "\033[31m";
        ret_str += str;
        return ret_str;
}

std::string gsky::log::color::green(const std::string &str) {
    std::string ret_str  = "\033[32m";
    ret_str += str;
    return ret_str;
}

std::string gsky::log::color::yellow(const std::string &str) {
    std::string ret_str  = "\033[33m";
    ret_str += str;
    return ret_str;
}

std::string gsky::log::color::blue(const std::string &str) {
    std::string ret_str  = "\033[34m";
    ret_str += str;
    return ret_str;
}

std::string gsky::log::color::fuchsia(const std::string &str) {
    std::string ret_str  = "\033[35m";
    ret_str += str;
    return ret_str;
}

std::string gsky::log::color::cyan(const std::string &str) {
    std::string ret_str  = "\033[36m";
    ret_str += str;
    return ret_str;
}

std::string gsky::log::color::white(const std::string &str) {
    std::string ret_str  = "\033[37m";
    ret_str += str;
    return ret_str;
}

std::string gsky::log::color::reset(const std::string &str) {
    std::string ret_str  = "\033[0m";
    ret_str += str;
    return ret_str;
}



/*
 * 打印日志思路: 采用log_base作为基础，在构造的时候实现头部打印，中途直接调用std::cout
 * */

gsky::log::log_base::log_base(std::string header){
    std::cout << header;
}

gsky::log::log_base &gsky::log::log_base::operator<<(const std::string &m) {
    std::cout << m;
    return *this;
}

gsky::log::log_base &gsky::log::log_base::operator<<(char m) {
    std::cout << m;
    return *this;
}

gsky::log::log_base &gsky::log::log_base::operator<<(unsigned char m) {
    std::cout << m;
    return *this;
}

gsky::log::log_base &gsky::log::log_base::operator<<(short m) {
    std::cout << m;
    return *this;
}

gsky::log::log_base &gsky::log::log_base::operator<<(unsigned short m) {
    std::cout << m;
    return *this;
}

gsky::log::log_base &gsky::log::log_base::operator<<(int m) {
    std::cout << m;
    return *this;
}

gsky::log::log_base &gsky::log::log_base::operator<<(unsigned int m) {
    std::cout << m;
    return *this;
}


gsky::log::log_base &gsky::log::log_base::operator<<(ssize_t m) {
    std::cout << m;
    return *this;
}

gsky::log::log_base &gsky::log::log_base::operator<<(size_t m) {
    std::cout << m;
    return *this;
}

gsky::log::log_base &gsky::log::log_base::operator<<(float m) {
    std::cout << m;
    return *this;
}

gsky::log::log_base &gsky::log::log_base::operator<<(double m) {
    std::cout << m;
    return *this;
}

gsky::log::info::info(): log_base("\033[32m[INFO*] \033[0m") {
}

gsky::log::debug::debug(): log_base("\033[35m[DEBUG*] \033[0m") {
}

gsky::log::warning::warning(): log_base("\033[33m[WARNING*] \033[0m") {
}

gsky::log::error::error(): log_base("\033[31m[ERROR*] \033[0m") {
}
