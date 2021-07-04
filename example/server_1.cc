
// g++ main.cc -lpthread -lgsky -o gsky
// ./gsky -c ../conf/gsky.conf
//
#include <iostream>
#include <signal.h>
#include <unistd.h>
#include <gsky/server.hh>

#define UNUSED(var) do { (void)(var); } while (false)

using namespace gsky;

server ser; // 创建服务器

void gsky_exit(int s) {
    UNUSED(s);
    ser.stop(); // 停止服务
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
    Login = 0x11,
    Echo = 0x20,
};

// 服务器回调函数, 函数格式为 void func(sp_request r, sp_response w)
void server_run(net::pp::sp_request r, net::pp::sp_response w) {
    log::info() << "收到数据: " << r->content() << '\n';
    switch((RouteRoot)r->route(0)) {
        case RouteRoot::Keep: {
            w->send_data("Keep");
        } break;
        case RouteRoot::CheckUpdate: {
            std::cout << "checking updateing\n";
        } break;
        case RouteRoot::Login: {
            std::cout << "Login\n";
        } break;
        case RouteRoot::Echo: {
            std::cout << "Echo\n";
            w->push_data("Push data 1 to you: " + r->content());
//            w->push_data("Push data 2 to you: abc");
        } break;
        default: {
            std::cout << "Error\n";
            w->send_data("ERROR");
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
            ser.set_config_path(optarg);
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
    ser.set_pp_server_handler(server_run);
    ser.run(); // 启动gsky服务器
    return 0;
}
