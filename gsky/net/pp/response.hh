#pragma once
#include <map>
#include <string>

#include <gsky/gsky.hh>
#include <gsky/util/vessel.hh>

namespace gsky {
namespace net {
namespace pp {

class response {
    using json = gsky::util::json;
public:
    explicit response(); // uid for deal with offline
    ~response();
    void set_send_data_handler(std::function<void(const std::string &)> send_data_handler);
    void send_data(const std::string &content);
    void send_json(gsky::util::json &json_obj);

private:
    std::string json_to_string(json &json_obj);
    std::function<void(const std::string &)> send_data_handler_;
};

}
}
}
