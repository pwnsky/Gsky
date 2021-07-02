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
    unsigned char get_route(int index);
    void set_route(unsigned char route[]);
    gsky::util::vessel &content_;
    const std::map<std::string, std::string> &client_info_;

    std::string session_;
    std::string get_content();
    gsky::util::vessel &get_raw_content();
private:
    unsigned char *route_;
};

}
}
}
