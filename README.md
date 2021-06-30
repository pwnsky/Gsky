# gsky

## 介绍

为了便于更快速开发高性能游戏服务器，特意基于lgx web服务器框架，二次开发且封装为一个服务器库。
gsky是一个基于epoll架构的高性能游戏服务器库，采用更快速的psp (pwnsky protocol)二进制协议进行传输数据。


## 如何使用？

git clone ...

make -j

make install

make默认编译为.so文件

编译安装好之后，可以更方便的写游戏服务器了。



编译的时候

```
g++ -lgsky -lpthread main.cc
```


详情请看测试目录下的例子
