#include <gsky/net/pp/request.hh>
#include <gsky/log/log.hh>
gsky::net::pp::request::request(const std::map<std::string, std::string> &client_info,
                      gsky::util::vessel &content) :
    client_info_(client_info),
    content_(content) {
}

void gsky::net::pp::request::set_route(unsigned char route[]) {
    route_ = route;
}

unsigned char gsky::net::pp::request::route(int index) {
    if(index < 0 || index > 6) {
        warning() << "get_route: out of orange (1 - 6)\n";
        return 0;
    }
    return route_[index];
}

std::string gsky::net::pp::request::content() {
    return content_.to_string();
}

gsky::util::vessel &gsky::net::pp::request::raw_content() {
    return content_;
}
