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

#include <gsky/log/log_thread.hh>
#include <gsky/log/log.hh>
#include <gsky/util/util.hh>
#include <gsky/util/firewall.hh>
#include <gsky/net/net.hh>
#include <gsky/net/eventloop.hh>

extern gsky::log::log *gsky::data::p_log;
extern std::string gsky::data::log_path;
std::string gsky::data::config_path;
std::string gsky::data::protocol;

gsky::net::pp::server_handler gsky::net::pp::server_handler_;
gsky::net::http::server_handler gsky::net::http::server_handler_;

std::vector<std::string> gsky::data::forbid_ips;
gsky::util::firewall *gsky::data::firewall = nullptr;

gsky::server::server() :
    sp_log_thread_(new gsky::log::log_thread),
    sp_net_(new gsky::net::net) {
}

gsky::server::~server() {
    if(gsky::data::firewall) {
        delete gsky::data::firewall;
    }
}
// Set server handler
void gsky::server::set_pp_handler(gsky::net::pp::server_handler h) {
    gsky::net::pp::server_handler_ = h; 
}


// Set server configure file path
void gsky::server::set_config_path(std::string config_path) {
    gsky::data::config_path = config_path;
}

bool gsky::server::stop() {
    sp_net_->stop();
    sp_log_thread_->stop();
    return true;
}

bool gsky::server::run() {
    bool error = true;
    //setbuf(stdout, nullptr);
    do {
        if(load_config() == false) {
            std::cout << "Load config file failed!\n" << std::endl;
            logger() << log_dbg("Load config file failed!");
            break;
        }

        if(run_security_module() == false) {
            std::cout << "Run security module failed\n" << std::endl;
            logger() << log_dbg("Run security module failed!");
            break;
        }

        if(run_logger_module() == false) {
            std::cout << "Run logger module failed\n" << std::endl;
            logger() << log_dbg("Run logger module failed!");
            break;
        }

        logger() << "*************  start gsky pp server...  ***************";
        std::cout << "\ngsky server port: " << port_ << "  number of thread: " << number_of_thread_ << "\n"
            << " Log file at: " << gsky::data::log_path;

        if(false == this->run_network_module()) {
            std::cout << "Run network module failed!\n";
            logger() << log_dbg("Run network module failed!");
            break;
        }

        error = false;
    }while(false);
    return error;
}

// Load config file
bool gsky::server::load_config() {
    std::string file_json;
    FILE* config_file_ptr = fopen(gsky::data::config_path.c_str(), "r");
    if(config_file_ptr == nullptr) {
        std::cout << "open config file: " << gsky::data::config_path << " failed!\n";
        exit(-1);
    }
    while(!feof(config_file_ptr)) {
        char buffer[MAX_BUF_SIZE];
        int len = fread(buffer, 1, MAX_BUF_SIZE, config_file_ptr);
        file_json += std::string(buffer, len);
    }

    fclose(config_file_ptr);
    //std::cout << "json [" << file_json << "]\n";
    gsky::util::json obj;
    try{
        obj = gsky::util::json::parse(file_json);
    } catch(util::json::parse_error &e) {
        d_cout << e.what() << '\n';
        return false;
    }
    // Parse json
    try {
        port_ = obj["port"];
        number_of_thread_ = obj["number_of_thread"];
        gsky::data::protocol = obj["protocol"];
        gsky::data::log_path = obj["log"];
    } catch (util::json::exception &e) {
        //d_cout << e.what() << '\n';
        std::cout << "Parse error configure file\n";
        return false;
    }

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
    }
    return true;
}

bool gsky::server::run_network_module() {
    if(port_ <= 1024 && getuid() != 0) {
       std::cout << "Must be root" << std::endl;
       return false;
    }

    sp_net_->init(port_, number_of_thread_);
    sp_net_->start();
    return true;
}

bool gsky::server::run_logger_module() {
    sp_log_thread_->set_log_path(gsky::data::log_path);
    gsky::data::p_log = sp_log_thread_->creat();
    return true;
}

bool gsky::server::run_security_module() {
    gsky::data::firewall = new gsky::util::firewall();
    gsky::data::firewall->set_forbid_ips(gsky::data::forbid_ips);
    return true;
}
