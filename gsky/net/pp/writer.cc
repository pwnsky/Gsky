#include <gsky/net/pp/writer.hh>

#include <sstream>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <pthread.h>
#include <iterator>

gsky::net::pp::writer::writer() {

}

gsky::net::pp::writer::~writer() {

}

void gsky::net::pp::writer::clean_handler() {
    send_data_handler_ = nullptr;
    push_data_handler_ = nullptr;
}

void gsky::net::pp::writer::set_send_data_handler(std::function<void(const std::string &)> h) {
    send_data_handler_ = h;
}

void gsky::net::pp::writer::set_push_data_handler(std::function<void(const std::string &)> h) {
    push_data_handler_ = h;
}

void gsky::net::pp::writer::set_route_handler(std::function<void(unsigned char [])> h) {
    set_route_handler_ = h;
}

bool gsky::net::pp::writer::send_data(const std::string &content) {
    if(send_data_handler_) {
        send_data_handler_(content);
        return true;
    }
    return false;
}

bool gsky::net::pp::writer::push_data(const std::string &content) {
    if(push_data_handler_) {
        push_data_handler_(content);
        return true;
    }
    return false;
}

bool gsky::net::pp::writer::send_json(json &json_obj) {
    std::ostringstream json_sstream;
    json_sstream << json_obj;
    std::string data = json_sstream.str();
    return send_data(data);
}

bool gsky::net::pp::writer::push_json(json &json_obj) {
    std::ostringstream json_sstream;
    json_sstream << json_obj;
    std::string data = json_sstream.str();
    return push_data(data);
}

std::string gsky::net::pp::writer::json_to_string(json &json_obj) {
    std::ostringstream json_sstream;
    json_sstream << json_obj;
    return json_sstream.str();
}

void gsky::net::pp::writer::set_route(unsigned char route []) {
    if(set_route_handler_) {
        set_route_handler_(route);
    }
}
