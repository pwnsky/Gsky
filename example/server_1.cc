
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
    TestError = 0x00,
    TestEcho = 0x10,
    TestPush = 0x20,
    TestMultiPush = 0x30,
};

// 服务器回调函数, 函数格式为 void func(sp_request r, sp_response w)
void server_run(net::pp::sp_request r, net::pp::sp_response w) {
    log::info() << "client: " << r->fd << " 收到数据: " << r->content() << '\n';
    switch((RouteRoot)r->route(0)) {
        case RouteRoot::TestError: {
        log::info() << "TestError";
            w->send_data("TestError");
        } break;
        case RouteRoot::TestEcho: {
        log::info() << "TestEcho";
            w->send_data(r->content());
        } break;
        case RouteRoot::TestPush: {
        log::info() << "TestPush";
            std::string data;
            data.resize(0x100000);
            w->push_data(data);
            w->push_data("Push data 1111 to you" + data);
        } break;
        case RouteRoot::TestMultiPush: {
        log::info() << "TestMultiPush";
            w->push_data("Push data 1111 to you");
            w->push_data("Push data 2222 to you");
            w->push_data("Push data 1111 to you");
            std::string data;
            data.resize(0x8000000);
            w->send_data(data); // push big data
        } break;
        default: {
            std::cout << "None\n";
            w->send_data("None");
        } break;
    }
}

// 断开连接处理函数，fd为客户端文件描述符
void offline_func(int fd) {
    std::cout << "client: " << fd << " offline ...\n";   
}

int main(int argc, char **argv) {
    ::signal(SIGINT, gsky_exit); // Ctrl + c 退出服务器
    int opt = 0;
    ser.set_logger_path("./gsky.log"); // 设置服务日志路径 logger path, Default "./gksy.log"
    ser.set_listen("0.0.0.0", 4096); // 设置服务日志路径, Defualt "0.0.0.0" 4096
    ser.set_threads(2);          // 设置服务线程数量,  Default 4
    //ser.set_protocol("pp"); // 设置协议, Default "pp"
    ser.set_pp_server_handler(server_run); // 设置服务器回调函数
    ser.set_pp_offline_handler(offline_func); // 设置客户端断开回调函数

    // 获取参数
    while((opt = getopt(argc, argv,"h::v::a::c:"))!=-1) {
        switch (opt) {
        case 'h': { // 帮助
            help();
            return 0;
        } break;
        case 'c': { 
            // 设置服务器配置文件路径
            ser.load_config(optarg);
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

    ser.run(); // 启动gsky服务器
    return 0;
}
