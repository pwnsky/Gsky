#pragma once

#include <gsky/gsky.hh>
#include <gsky/log/logger_thread.hh>
#include <gsky/net/net.hh>
#include <gsky/net/pp/socket.hh>

using logger = gsky::log::logger;
using namespace gsky::net::pp;

namespace gsky {
class server {
public:
     server();
    ~server();
    bool run();
    bool stop();
    bool load_config();
    bool run_logger_module();
    bool run_security_module();
    bool run_network_module();
    bool load_config(const std::string &config_path);
    void set_logger_path(const std::string &logger_path);
    void set_listen(const std::string &ip, unsigned short port);
    void set_threads(int n);
    void set_protocol(const std::string &protocol);

    virtual void request(sp_request r, sp_writer w) {}
    virtual void offline(int fd) {}

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
