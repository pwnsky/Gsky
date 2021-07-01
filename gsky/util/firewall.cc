#include <gsky/util/firewall.hh>

bool gsky::util::firewall::wall(int fd, const std::string &ip) {
    bool ret = false;
    for(auto iter = forbid_ips_.begin(); iter != forbid_ips_.end(); ++iter) {
        if(*iter== ip) {
            shutdown(fd, SHUT_RDWR);
            ret = true;
            break;
        }
    }
    return ret;
}

bool gsky::util::firewall::is_forbid(const std::string &ip) {
    bool ret = false;
    for(auto iter = forbid_ips_.begin(); iter != forbid_ips_.end(); ++iter) {
        if(*iter== ip) {
            ret = true;
            break;
        }
    }
    return ret;
}
