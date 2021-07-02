#pragma once

#include <memory>
#include <functional>
#include <iostream>
#include <pthread.h>
#include <list>
#include <unordered_map>
#include <queue>

#include <gsky/util/json.hh>

#define DEBUG
//#define d_cout std::cout << "[" << __FILE__ << " line: " << __LINE__ << " thread id: " << std::hex <<  pthread_self() << std::oct << "] "
#define d_cout std::cout << "[" << __FILE__ << " line: " << __LINE__ << "] "
#define dbg_log(x) d_cout << x << std::endl
#define log_dbg(x) "DEBUG:" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "\n" + std::string(x) + "\n"

#define MAX_CONNECTED_FDS_NUM 0x100000
#define EPOLL_MAX_EVENT_NUM   0x1000
#define EPOLL_WAIT_TIME       0x1000

//#define MAX_HTTP_RECV_BUF_SIZE 0x4000
#define GSKY_VERSION "1.0"
#define SERVER_NAME "gsky " GSKY_VERSION

#define HTTP_MAX_NOT_FOUND_TIMES 25

namespace gsky {

namespace thread {
class noncopyable;
class thread;
class thread_data;
class mutex_lock;
class mutex_lock_guard;
class count_down_latch;
class condition;
namespace current_thread {}

}
// namespace net start
namespace net {
class net;
class channel;
class epoll;
class channel;
class eventloop;
class eventloop_thread;
class eventloop_threadpool;
class socket;

using sp_socket = std::shared_ptr<gsky::net::socket>;
using sp_epoll = std::shared_ptr<gsky::net::epoll>;
using sp_eventloop = std::shared_ptr<gsky::net::eventloop>;
using sp_eventloop_thread = std::shared_ptr<gsky::net::eventloop_thread>;
using sp_channel = std::shared_ptr<gsky::net::channel>;

namespace pp { // pp协议
class socket;
class request;
class response;
typedef void (*server_handler)(std::shared_ptr<gsky::net::pp::request> , std::shared_ptr<gsky::net::pp::response> );
extern server_handler server_handler_;
}

namespace http { // http 协议
class socket;
class request;
class response;
typedef void (*server_handler)(std::shared_ptr<gsky::net::http::request> , std::shared_ptr<gsky::net::http::response> );
extern server_handler server_handler_;
}

}

// namespace net end

// namespace util start
namespace util {
//using std::function<void()> = std::function<void()>;
//using std::function<void()>1 = std::function<void(const std::string &)>;
//using std::function<void()>2 = std::function<void(const std::string &, const std::string &)>;

class vessel;
using json = nlohmann::json;
std::string date_time();
std::string cat_file(const std::string &file_name);
std::string popen(const std::string &cmd);
class color;
class url;
class firewall;
}

// namespace util end

// namespace crypto start
namespace crypto {
    class pmd5;
    class pe;
}
// namespace crypto start

// namespace log start
namespace log {
class io;
class log;
class logger;
class log_thread;
}
// namespace log end

// namespace data start
namespace data {
extern std::string protocol;
extern gsky::log::log *p_log;
extern std::string log_path;
extern std::vector<std::string> forbid_ips;
extern gsky::util::firewall *firewall;
extern std::string config_path;
extern std::string os_info;
}

// namespace data end
class server;
const char *version();
}
