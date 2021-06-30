
// g++ main.cc -lpthread -lgsky -o gsky
// ./gsky -c ../conf/gsky.conf
//
#include <iostream>
#include <signal.h>
#include <unistd.h>

#include <gsky/gsky.hh>
#include <gsky/server.hh>
#include <gsky/work/work.hh>

#define UNUSED(var) do { (void)(var); } while (false)

//extern std::string gsky::data::config_path;
//
std::string gsky::data::os_info;
gsky::server server;

void gsky_exit(int s) {
    UNUSED(s);
    server.stop();
}

void help() {
    std::cout << "Usage: ./gsky [OPTION...] [SECTION] PAGE...\n"
                "-c   load configure file\n"
                "-h   help of gsky server\n"
                "-v   check version of gsky server\n"
                 ;
}

enum class RouterRoot {
    Keep = 0,
    CheckUpdate,
    Login,
};

// 服务器回调函数, 函数格式为 void func(gsky::work::work *)
void server_run(gsky::work::work *w) {

    switch((RouterRoot)w->router_[0]) {
        case RouterRoot::Keep: {
            w->send_data("Keep");
        } break;
        case RouterRoot::CheckUpdate: {
            std::cout << "checking updateing\n";
        } break;
        case RouterRoot::Login: {
            std::cout << "Login\n";
        } break;
        default: {
            w->send_data(w->content_.to_string());
        } break;
    }
}

int main(int argc, char **argv) {
    ::signal(SIGINT, gsky_exit); // Ctrl + c 退出服务器
    int opt = 0;
    gsky::data::config_path = DEFAULT_CONFIG_FILE;
    while((opt = getopt(argc, argv,"h::v::a::c:"))!=-1) {
        switch (opt) {
        case 'h': {
            help();
            exit(0);
        } break;
        case 'c': {
            // 设置服务器配置文件路径
            server.set_config_path(optarg);
        } break;
        case 'v': {
            // 显示 gsky lib 的版本号
            std::cout << "gsky version: " << gsky::version() << '\n';
            exit(0);
        } break;
        default: {
            std::cout << "-h get more info" << std::endl;
            exit(0);
        }
        }
    }

    // 设置服务器回调函数
    server.set_func_handler(server_run);
    server.run(); // 启动gsky服务器
    std::cout << "\033[40;33mgsky quited! \n\033[0m";
    return 0;
}
