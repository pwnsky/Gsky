
// g++ main.cc -lpthread -lgsky -o gsky
// ./gsky -c ../conf/gsky.conf
//
#include <iostream>
#include <signal.h>
#include <unistd.h>

#include <gsky/gsky.hh>
#include <gsky/server.hh>

#define UNUSED(var) do { (void)(var); } while (false)

//using namespace gsky;

gsky::server server; // 创建gsky服务

void gsky_exit(int s) {
    UNUSED(s);
    server.stop(); // 停止服务
}

int main(int argc, char **argv) {
    ::signal(SIGINT, gsky_exit); // Ctrl + c 退出服务器
    int opt = 0;

    // 获取参数
    while((opt = getopt(argc, argv,"h::v::a::c:"))!=-1) {
        switch (opt) {
        case 'c': { 
            // 设置服务器配置文件路径
            server.set_config_path(optarg);
        } break;
        default: {
            std::cout << "./gsky -c config_file_path" << std::endl;
            return -1;
        }
        }
    }
    server.run(); // 启动gsky服务器
    std::cout << "\033[40;33mgsky quited! \n\033[0m";
    return 0;
}
