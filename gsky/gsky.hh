#pragma once

#define MAX_CONNECTED_FDS_NUM 0x100000
#define EPOLL_MAX_EVENT_NUM   0x1000
#define EPOLL_WAIT_TIME       0x1000

//#define MAX_HTTP_RECV_BUF_SIZE 0x4000
#define GSKY_VERSION "1.0"
#define SERVER_NAME "gsky " GSKY_VERSION

#define HTTP_MAX_NOT_FOUND_TIMES 25
namespace gsky {
const char *version();
}
