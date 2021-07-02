#pragma once

#include <map>
#include <string>
#include <unistd.h>

#include <gsky/gsky.hh>
#include <gsky/util/vessel.hh>

class gsky::net::pp::request {
    using json = gsky::util::json;
public:
    explicit request(const std::map<std::string, std::string> &map_client_info,
                  gsky::util::vessel &content); // uid for deal with offline
    ~request() {};
    void set_route(char route[]) {
        route_ = route;
    }

    char get_route(int index);
    char *route_;
    gsky::util::vessel &content_;
    std::string session_;
    const std::map<std::string, std::string> &map_client_info_;
    gsky::util::vessel *get_raw_content();
    std::string get_content();

private:
};
