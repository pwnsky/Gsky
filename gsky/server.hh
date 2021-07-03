#pragma once

#include <gsky/gsky.hh>
#include <gsky/log/logger_thread.hh>
#include <gsky/net/net.hh>

using logger = gsky::log::logger;
namespace gsky {
class server final{
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
    void set_pp_server_handler(gsky::net::pp::server_handler h);

    void set_config_path(std::string config_path);

private:
    int number_of_thread_;
    int queue_size_;
    int port_;
    std::shared_ptr<gsky::log::logger_thread> sp_logger_thread_;
    std::shared_ptr<gsky::net::net> sp_net_;
    std::string config_path_;
    std::string logger_path_;
    std::string protocol_;
};
}
