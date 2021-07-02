#include <gsky/net/pp/request.hh>

gsky::net::pp::request::request(const std::map<std::string, std::string> &map_client_info,
                      gsky::util::vessel &content) :
    map_client_info_(map_client_info),
    content_(content ){
}

char gsky::net::pp::request::get_route(int index) {
    if(index < 0 || index > 8) {
        d_cout << "get_route: out of orange (1 - 8)\n";
        return 0;
    }
    return route_[index];
}

std::string gsky::net::pp::request::get_content() {
    return "";
}

gsky::util::vessel *gsky::net::pp::request::get_raw_content() {
    return &content_;
}
