# gsky

## 介绍

为了便于更快速开发高性能游戏服务器，特意基于lgx web服务器框架，二次开发且封装为一个服务器库。
gsky是一个基于epoll架构的高性能游戏服务器库，采用更快速的psp (pwnsky protocol)二进制协议进行传输数据。


## 如何使用？

```
git clone https://github.com/pwnsky/gsky.git
cd gsky
make -j # 编译gsky库
make install # 安装gsky库
```

该库提供编译好的.so文件以及api头文件，更利于开发和编译服务器。

编译成功后，所有文件在build目录下，如下。

```
build
├── include
│   └── gsky
│       ├── gsky.hh # gsky服务器总声明
│       ├── log # 日志模块
│       │   ├── log.hh
│       │   └── log_thread.hh
│       ├── net # 网络模块
│       │   ├── channel.hh
│       │   ├── epoll.hh
│       │   ├── eventloop.hh
│       │   ├── eventloop_thread.hh
│       │   ├── eventloop_threadpool.hh
│       │   ├── net.hh
│       │   ├── psp.hh
│       │   └── util.hh
│       ├── server.hh
│       ├── thread # 线程模块
│       │   ├── condition.hh
│       │   ├── count_down_latch.hh
│       │   ├── mutex_lock.hh
│       │   ├── noncopyable.hh
│       │   └── thread.hh
│       ├── util # 工具模块
│       │   ├── firewall.hh
│       │   ├── json.hh
│       │   ├── md5.hh
│       │   ├── url.hh
│       │   ├── util.hh
│       │   └── vessel.hh
│       └── work # 逻辑调用模块
│           └── work.hh
└── lib
    └── libgsky.so # 编译好的gsky动态库
```

详情请查看头文件api。



编译安装好库之后，可以更方便的写游戏服务器了。

例如: https://github.com/pwnsky/gsky/blob/main/example/server_1.cc

```c++

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
            w->send_data("Keep"); // 发送给客户端"Keep"字符串
        } break;
        case RouterRoot::CheckUpdate: {
            std::cout << "checking updateing\n";
        } break;
        case RouterRoot::Login: {
            std::cout << "Login\n";
        } break;
        default: {
            w->send_data(w->content_.to_string()); // 回显输入的内容
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
```


编译的时候

```
g++ -lgsky -lpthread main.cc
```

运行该程序需要指定下配置文件，一般常用配置文件在https://github.com/pwnsky/gsky/blob/main/conf/gsky.conf

```
{
    "protocol": "psp",
    "port" : 8080,
    "number_of_thread": 4,
    "log" : "./gsky.log",
    "firewall" : {
    }
}

```

设置好配置文件之后加上`-c`参数指定配置文件路径

```
./gsky -c ../conf/gsky.conf
```

若出现

```
gsky server port: 8080  number of thread: 4
```

则说明运行成功了。

测试服务器

由于服务器采用psp协议进行传输的，使用example/client.py进行测试，若想使用psp协议客户端，则访问https://github.com/pwnsky/psp下载相应的客户端psp协议库。

```
./client.py
send...
recv: 
router: b'\x00\x00\x00\x00' length: 4
b'Keep'
```

若运行出现以上内容，则服务器正常运行了。



详情更多，请看[测试目录](https://github.com/pwnsky/gsky/tree/main/example)下的例子。



开发者:

i0gan



