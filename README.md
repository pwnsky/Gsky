# GSKY游戏服务器框架

GSKY GAME SERVER FRAMEWORK



## 介绍

为了便于更快速开发高性能游戏服务器，特意基于lgx web服务器框架，二次开发且封装为一个服务器库。
gsky是一个基于epoll 边缘触发架构的高性能游戏服务器库，采用更快速的pp (pwnsky protocol)二进制加密双向协议进行传输数据，服务端支持异步消息推送，日志打印与日志文件写入，协程等，让使用者更专注与游戏逻辑开发。

[pp协议sdk](https://github.com/pwnsky/pp)

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
│       ├── crypto
│       │   ├── pe.hh
│       │   └── pmd5.hh
│       ├── gsky.hh
│       ├── log
│       │   ├── logger.hh
│       │   ├── logger_thread.hh
│       │   └── log.hh
│       ├── net
│       │   ├── channel.hh
│       │   ├── epoll.hh
│       │   ├── eventloop.hh
│       │   ├── eventloop_thread.hh
│       │   ├── eventloop_threadpool.hh
│       │   ├── http
│       │   ├── net.hh
│       │   ├── pp
│       │   │   ├── pp.hh
│       │   │   ├── request.hh
│       │   │   ├── response.hh
│       │   │   └── socket.hh
│       │   ├── socket.hh
│       │   └── util.hh
│       ├── server.hh
│       ├── thread
│       │   ├── condition.hh
│       │   ├── count_down_latch.hh
│       │   ├── mutex_lock.hh
│       │   ├── noncopyable.hh
│       │   └── thread.hh
│       └── util
│           ├── firewall.hh
│           ├── json.hh
│           ├── url.hh
│           ├── util.hh
│           └── vessel.hh
└── lib
    └── libgsky.so

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
    log::info() << "收到数据: " << r->content() << '\n';
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
```


编译的时候

```
g++ -lgsky -lpthread main.cc
```

运行该程序需要指定下配置文件，一般常用配置文件在 

https://github.com/pwnsky/gsky/blob/main/conf/gsky.conf

```
{
    "protocol": "pp",
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

由于服务器采用pp协议进行传输的，使用test/client.py进行测试，若想使用pp协议客户端，则访问 [pp](https://github.com/pwnsky/pp) 下载相应的客户端pp协议sdk。

```
pp connecting
request get key: route: b'\x00\x00\x00\x00\x00\x00' length: 8 status : 0x31
code: b'e\xfa'
b'key: \xbf\xe6\x80Ve\xfaR3'
b'code: e\xfa'
route: b'0\x00\x00\x00\x00\x00' length: 5 status : 0x30
code: b'e\xfa'
b'recv: ERROR'
b'key: \xbf\xe6\x80Ve\xfaR3'
b'code: e\xfa'
route: b' \x00\x00\x00\x00\x00' length: 40 status : 0x30
code: b'e\xfa'
b'recv: Push data 1 to you: Yeah you are online!'

```

若运行出现以上内容，则服务器正常运行了。



服务端API详情，请看[服务器样例](https://github.com/pwnsky/gsky/tree/main/example)下的例子。

服务端测试详情，请看[测试](https://github.com/pwnsky/gsky/tree/main/test)下的例子。



## 开发者

i0gan



## 开发日志

2021-06-30 开启gsky项目

2021-07-01 修改psp协议名称为pp协议，增加对称加密传输，加密方采用 PE (Pwnsky Encryption) 加密。

2021-07-02 完善pp协议

2021-07-03 更模块化，完善日志系统

2021-07-04 更改vessel容器，修复无法接受大数据包 + vessel测试，客户端测试，pe解密测试，修复et模式无法发送大包问题。

2021-07-05 异步消息推送实现。

