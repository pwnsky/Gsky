#pragma once
#include <gsky/gsky.hh>
#include <sys/socket.h>
#include <vector>

class gsky::util::firewall {
public:
    firewall() {}
    ~firewall() {}
    bool wall(int fd, const std::string &ip); // fire wall
    void forbid(const std::string &ip) {
        forbid_ips_.push_back(ip);
    };

    bool is_forbid(const std::string &ip);
    void set_forbid_ips(const  std::vector<std::string> &forbid_ips) {
        forbid_ips_ = forbid_ips;
    }
private:
    std::vector<std::string> forbid_ips_;
};
