#include <gsky/work/work.hh>

gsky::work::work::work(const std::map<std::string, std::string> &map_client_info,
                      gsky::util::vessel &content) :
    map_client_info_(map_client_info),
    content_(content),
    send_data_handler_(nullptr) {

}

void gsky::work::work::set_send_data_handler(gsky::util::callback1 send_data_handler) {
    send_data_handler_ = send_data_handler;
}

// Call server_handler function
void gsky::work::work::main() {
    if(gsky::work::server_handler_ != nullptr) {
        gsky::work::server_handler_(this);
    }else {
        d_cout << "server_handler is not set!\n";
    }

}

void gsky::work::work::send_data(const std::string &content) {
    if(send_data_handler_)
        send_data_handler_(content);
}

void gsky::work::work::send_json(json &json_obj) {
    std::ostringstream json_sstream;
    json_sstream << json_obj;
    std::string data = json_sstream.str();
    send_data(data);
}

std::string gsky::work::work::json_to_string(json &json_obj) {
    std::ostringstream json_sstream;
    json_sstream << json_obj;
    return json_sstream.str();
}

std::string gsky::work::work::get_date_time() {
    char time_str[128] = {0};
    struct timeval tv;
    time_t time;
    gettimeofday(&tv, nullptr);
    time = tv.tv_sec;
    struct tm *p_time = localtime(&time);
    strftime(time_str, 128, "%Y-%m-%d %H:%M:%S", p_time);
    return std::string(time_str);
}

