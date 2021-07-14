#pragma once

#include <map>
#include <string>
#include <unistd.h>
#include <gsky/gsky.hh>
#include <gsky/util/vessel.hh>

namespace gsky{
namespace net{
namespace pp{

class request {
public:

    explicit request(const std::map<std::string, std::string> &client_info,
                  gsky::util::vessel &content); // uid for deal with offline
    unsigned char route(int index);
    const std::map<std::string, std::string> &client_info_;
    std::string content();
    gsky::util::vessel &raw_content();
    void set_route(unsigned char route[]);
    int fd;
private:
    unsigned char *route_;
    std::string session_;
    gsky::util::vessel &content_;
};

}
}
}
