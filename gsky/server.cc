#include <gsky/server.hh>

#include <iostream>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>

#include <gsky/util/util.hh>
#include <gsky/util/json.hh>
#include <gsky/net/eventloop.hh>

extern gsky::log::logger_manager *gsky::log::logger_manager_instance;

using json = gsky::util::json;
using namespace gsky::log;

gsky::server::server() :
    sp_logger_thread_(new gsky::log::logger_thread),
    sp_net_(new gsky::net::net) {
}

gsky::server::~server() {
//    if(gsky::data::firewall) {
//        delete gsky::data::firewall;
//    }
}

// Set server handler
//void gsky::server::set_pp_server_handler(gsky::net::pp::server_handler h) {
//    gsky::net::pp::server_handler_ = h; 
//}

// Set server's client offline handler
//void gsky::server::set_pp_offline_handler(gsky::net::pp::offline_handler h) {
//    gsky::net::pp::offline_handler_ = h; 
//}
//

/*
void request(sp_request r, sp_writer w) {
#ifdef INFO
    info() << "server fd: " << r->fd << "request\n";
#endif
}

void offline(int fd) {
#ifdef INFO
    info() << "server fd: " << fd << "offline\n";
#endif
}*/

bool gsky::server::stop() {
    sp_net_->stop();
    sp_logger_thread_->stop();
#ifdef INFO
    info() << "server stoped!\n";
#endif
    return true;
}

bool gsky::server::run() {
    bool err = true;
#ifdef DEBUG
    setbuf(stdout, nullptr);
#endif
    gsky::net::pp::request_handler = std::bind(&gsky::server::request, this, std::placeholders::_1, std::placeholders::_2);
    gsky::net::pp::offline_handler = std::bind(&gsky::server::offline, this, std::placeholders::_1);

    do {
        if(run_security_module() == false) {
            error() << "Run security module failed!\n";
            break;
        }

        if(run_logger_module() == false) {
            error() << "Run logger module failed!\n";
            break;
        }

        logger() << "*************  start gsky pp server...  ***************";
#ifdef INFO
        info() << "\ngsky server port: " << port_ << "  threads: " << threads_ << "\n"
            << " Log file at: " << logger_path_ << "\n";
#endif

        if(false == this->run_network_module()) {
            error() << "Run network module failed!\n";
            break;
        }

        err = false;
    }while(false);
    return err;
}

// Load config file
// Set server configure file path
bool gsky::server::load_config(const std::string &config_path) {
#define MAX_BUF_SIZE 0x1000
    std::string file_json;
    FILE* config_file_ptr = fopen(config_path.c_str(), "r");
    if(config_file_ptr == nullptr) {
        error() << "open config file: " << config_path << " failed!\n";
        exit(-1);
    }
    while(!feof(config_file_ptr)) {
        char buffer[MAX_BUF_SIZE];
        int len = fread(buffer, 1, MAX_BUF_SIZE, config_file_ptr);
        file_json += std::string(buffer, len);
    }

    fclose(config_file_ptr);
    //std::cout << "json [" << file_json << "]\n";
    util::json obj;
    try{
        obj = gsky::util::json::parse(file_json);
    } catch(util::json::parse_error &e) {
        error() << e.what();
        return false;
    }
    // Parse json
    try {
        port_ = obj["port"];
        threads_ = obj["threads"];
        protocol_ = obj["protocol"];
        logger_path_ = obj["log"];
    } catch (util::json::exception &e) {
        error() << e.what();
        error() << "Parse error configure file\n";
        return false;
    }

    /*
    // 防火墙禁用特定ip
    auto ips = obj["firewall"];
    for(auto iter = ips.begin(); iter != ips.end(); ++iter) {
        std::string ip_key, ip;
        try {
            ip_key = iter.value();
            ip = iter.value();
        }  catch (util::json::type_error e) {
            d_cout << e.what() << '\n';
            return false;
        }
        gsky::data::firewall->forbid(ip);
    } */
    return true;
}

void gsky::server::set_logger_path(const std::string &logger_path) {
    logger_path_ = logger_path;
}

void gsky::server::set_protocol(const std::string &protocol) {
    protocol_ = protocol;
}

void gsky::server::set_threads(int n) {
    threads_ = n;
}

void gsky::server::set_listen(const std::string &ip, unsigned short port) {
    ip_ = ip;
    port_ = port;
}

bool gsky::server::run_network_module() {
    if(port_ <= 1024 && getuid() != 0) {
       error() << "Must be root\n";
       return false;
    }
    sp_net_->init(ip_, port_, threads_);
    sp_net_->start();
    return true;
}

bool gsky::server::run_logger_module() {
    sp_logger_thread_->set_logger_path(logger_path_);
    sp_logger_thread_->creat();
    return true;
}

bool gsky::server::run_security_module() {
    //gsky::data::firewall = new gsky::util::firewall();
    //gsky::data::firewall->set_forbid_ips(gsky::data::forbid_ips);
    return true;
}
