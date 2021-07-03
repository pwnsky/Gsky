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
void gsky::server::set_pp_server_handler(gsky::net::pp::server_handler h) {
    gsky::net::pp::server_handler_ = h; 
}


// Set server configure file path
void gsky::server::set_config_path(std::string config_path) {
    config_path_ = config_path;
}

bool gsky::server::stop() {
    sp_net_->stop();
    sp_logger_thread_->stop();
    info() << "server stoped!\n";
    return true;
}

bool gsky::server::run() {
    bool err = true;
    //setbuf(stdout, nullptr);
    do {
        if(load_config() == false) {
            error() << "Load config file failed!\n";
            break;
        }

        if(run_security_module() == false) {
            error() << "Run security module failed!\n";
            break;
        }

        if(run_logger_module() == false) {
            error() << "Run logger module failed!\n";
            break;
        }

        logger() << "*************  start gsky pp server...  ***************";
        info() << "\ngsky server port: " << port_ << "  number of thread: " << number_of_thread_ << "\n"
            << " Log file at: " << logger_path_ << "\n";

        if(false == this->run_network_module()) {
            error() << "Run network module failed!\n";
            break;
        }

        err = false;
    }while(false);
    return err;
}

// Load config file
bool gsky::server::load_config() {
#define MAX_BUF_SIZE 0x1000
    std::string file_json;
    FILE* config_file_ptr = fopen(config_path_.c_str(), "r");
    if(config_file_ptr == nullptr) {
        error() << "open config file: " << config_path_ << " failed!\n";
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
        number_of_thread_ = obj["number_of_thread"];
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

bool gsky::server::run_network_module() {
    if(port_ <= 1024 && getuid() != 0) {
       error() << "Must be root\n";
       return false;
    }

    sp_net_->init(port_, number_of_thread_);
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
