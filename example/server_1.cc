
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

using namespace gsky;

server server; // 创建gsky服务

void gsky_exit(int s) {
    UNUSED(s);
    server.stop(); // 停止服务
}

void help() {
    std::cout << "Usage: ./gsky [OPTION...] [SECTION] PAGE...\n"
                "-c   load configure file\n"
                "-h   help of gsky server\n"
                "-v   check version of gsky server\n"
                 ;
}

enum class RouteRoot {
    Keep = 0,
    CheckUpdate = 0x10,
    Login,
};

// 服务器回调函数, 函数格式为 void func(gsky::work::work *)
void server_run(work::work *w) {

    switch((RouteRoot)w->route_[0]) {
        case RouterRoot::Keep: {
            w->send_data("Keep");
        } break;
        case RouteRoot::CheckUpdate: {
            std::cout << "checking updateing\n";
        } break;
        case RouteRoot::Login: {
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

    // 获取参数
    while((opt = getopt(argc, argv,"h::v::a::c:"))!=-1) {
        switch (opt) {
        case 'h': { // 帮助
            help();
            return 0;
        } break;
        case 'c': { 
            // 设置服务器配置文件路径
            server.set_config_path(optarg);
        } break;
        case 'v': {
            // 显示 gsky lib 的版本号
            std::cout << "gsky version: " << gsky::version() << '\n';
            return 0;
        } break;
        default: {
            std::cout << "-h get more info" << std::endl;
            return -1;
        }
        }
    }

    // 设置服务器回调函数
    server.set_func_handler(server_run);
    server.run(); // 启动gsky服务器
    std::cout << "\033[40;33mgsky quited! \n\033[0m";
    return 0;
}
