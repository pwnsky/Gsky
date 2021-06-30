#pragma once

#include <map>
#include <string>
#include <unistd.h>
#include <sstream>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unordered_map>
#include <pthread.h>
#include <iterator>

#include <gsky/gsky.hh>
#include <gsky/log/log.hh>
#include <gsky/util/url.hh>
#include <gsky/util/vessel.hh>
#include <gsky/net/psp.hh>

using logger = gsky::log::logger;
class gsky::work::work {
    using json = gsky::util::json;
public:
    explicit work(const std::map<std::string, std::string> &map_client_info,
                  gsky::util::vessel &content); // uid for deal with offline
    ~work() {};
    void set_send_data_handler(gsky::util::callback1 send_data_handler);
    void set_router(char router[]) {
        router_ = router;
    }
    void main();
    void send_data(const std::string &content);
    void send_json(gsky::util::json &json_obj);
    std::string json_to_string(json &json_obj);
    char *router_;
    gsky::util::vessel &content_;
    std::string session_;
    const std::map<std::string, std::string> &map_client_info_;
    std::string get_date_time();

private:
    //std::string &uid_;
    gsky::util::callback1 send_data_handler_;
};
