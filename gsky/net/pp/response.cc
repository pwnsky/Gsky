#include <gsky/net/pp/response.hh>

#include <sstream>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <pthread.h>
#include <iterator>

gsky::net::pp::response::response() {

}

gsky::net::pp::response::~response() {

}

void gsky::net::pp::response::set_send_data_handler(std::function<void(const std::string &)> send_data_handler) {
    send_data_handler_ = send_data_handler;
}

void gsky::net::pp::response::set_push_data_handler(std::function<void(const std::string &)> push_data_handler) {
    push_data_handler_ = push_data_handler;
}

void gsky::net::pp::response::send_data(const std::string &content) {
    if(send_data_handler_) {
        send_data_handler_(content);
    }
}

void gsky::net::pp::response::push_data(const std::string &content) {
    if(push_data_handler_) {
        push_data_handler_(content);
    }
}

void gsky::net::pp::response::send_json(json &json_obj) {
    std::ostringstream json_sstream;
    json_sstream << json_obj;
    std::string data = json_sstream.str();
    send_data(data);
}

std::string gsky::net::pp::response::json_to_string(json &json_obj) {
    std::ostringstream json_sstream;
    json_sstream << json_obj;
    return json_sstream.str();
}
