#pragma once

#include <gsky/gsky.hh>

using logger = gsky::log::logger;

class gsky::server final{
public:
     server();
    ~server();
    bool run();
    bool stop();
    bool load_config();
    bool run_logger_module();
    bool run_security_module();
    bool run_network_module();
    // Set server handler
    void set_pp_handler(gsky::net::pp::server_handler h);

    void set_config_path(std::string config_path);

private:
    int number_of_thread_;
    int queue_size_;
    int port_;
    std::string log_path_;
    std::shared_ptr<gsky::log::log_thread> sp_log_thread_;
    std::shared_ptr<gsky::net::net> sp_net_;
};
