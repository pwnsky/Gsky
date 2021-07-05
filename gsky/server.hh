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
    bool load_config(const std::string &config_path);
    void set_logger_path(const std::string &logger_path);
    void set_listen(const std::string &ip, unsigned short port);
    void set_threads(int n);
    void set_protocol(const std::string &protocol);

private:
    int threads_ = 4;
    int queue_size_ = 1000;
    int port_ = 4096;
    std::string ip_ = "0.0.0.0";
    std::shared_ptr<gsky::log::logger_thread> sp_logger_thread_;
    std::shared_ptr<gsky::net::net> sp_net_;
    std::string logger_path_ = "./gsky.log";
    std::string protocol_;
};
}
