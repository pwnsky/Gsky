#include "util.hh"
#include <sys/stat.h>
#include <fcntl.h>


std::string gsky::util::date_time() {
    char time_str[128] = {0};
    struct timeval tv;
    time_t time;
    gettimeofday(&tv, nullptr);
    time = tv.tv_sec;
    struct tm *p_time = localtime(&time);
    strftime(time_str, 128, "%Y-%m-%d %H:%M:%S", p_time);
    return std::string(time_str);
}

std::string gsky::util::cat_file(const std::string &file_name) {
    int fd = open(file_name.c_str(), O_RDONLY);
    if(-1 == fd) {
        return "";
    }
    //lseek(fd, 0, SEEK_SET);
    return "";
}

std::string gsky::util::popen(const std::string &cmd) {
    return "";
}
