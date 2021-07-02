#include <gsky/net/pp/request.hh>

gsky::net::pp::request::request(const std::map<std::string, std::string> &client_info,
                      gsky::util::vessel &content) :
    client_info_(client_info),
    content_(content) {
}


void gsky::net::pp::request::set_route(unsigned char route[]) {
    route_ = route;
}

unsigned char gsky::net::pp::request::get_route(int index) {
    if(index < 0 || index > 8) {
        d_cout << "get_route: out of orange (1 - 8)\n";
        return 0;
    }
    return route_[index];
}

std::string gsky::net::pp::request::get_content() {
    return "";
}

gsky::util::vessel &gsky::net::pp::request::get_raw_content() {
    return content_;
}
