#pragma once

#include <gsky/net/net.hh>
#include <gsky/net/eventloop.hh>
#include <gsky/gsky.hh>
#include <gsky/log/log_thread.hh>
#include <gsky/log/log.hh>
#include <gsky/util/util.hh>
#include <gsky/util/firewall.hh>

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
    void set_func_handler(gsky::work::server_handler h);
    void set_config_path(std::string config_path);

private:
    int number_of_thread_;
    int queue_size_;
    int port_;
    std::string log_path_;
    std::shared_ptr<gsky::log::log_thread> sp_log_thread_;
    gsky::net::net net_;
};
