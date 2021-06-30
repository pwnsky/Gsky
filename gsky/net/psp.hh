#pragma once
#include <memory>
#include <unordered_map>
#include <map>
#include <pthread.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>

#include <gsky/gsky.hh>
#include <gsky/net/eventloop.hh>
#include <gsky/util/vessel.hh>
#include <gsky/log/log.hh>
#include <gsky/util/firewall.hh>
#include <gsky/util/md5.hh>
#include <gsky/util/util.hh>
#include <gsky/work/work.hh>

using logger = gsky::log::logger;

enum class gsky::net::PSPRecvState {
    PARSE_HEADER = 0,
    RECV_CONTENT,
    WORK,
    FINISH
};

enum class gsky::net::PSPConnectionState {
    CONNECTED = 0,
    DISCONNECTING,
    DISCONNECTED
};

class gsky::net::psp final : public std::enable_shared_from_this<psp> {
struct protocol {
    char magic[4]; // "PSP\x00" 0x00505350  5264208
    char router[4];    // router
    unsigned int length;
};

public:
    explicit psp(int fd,eventloop *elp);
    ~psp();
    void reset();
    sp_channel get_sp_channel();
    eventloop *get_eventloop();
    void handle_close();
    void new_evnet();
    void set_client_info(const std::string &ip, const std::string &port) {
            map_client_info_["client_ip"] = ip;
            map_client_info_["client_port"] = port;
            //map_client_info_["session"] = session_;
            client_ip_ = ip;
            client_port_ = port;
    }
    void push_data(const std::string &data); // 数据推送
    int is_deleteble() {
        if(wait_event_count_ > 0)
            return false;
        return true;
    }
    void response_error(int error_number, std::string message);
private:
    int fd_;
    std::string uid_ =  "none";
    eventloop *eventloop_;
    sp_channel sp_channel_;
    gsky::util::vessel in_buffer_;
    std::shared_ptr<gsky::util::vessel> out_buffer_ = nullptr;
    std::queue<std::shared_ptr<gsky::util::vessel>> out_buffer_queue_;
    PSPConnectionState psp_connection_state_;
    PSPRecvState psp_process_state_;
    int wait_event_count_ = 0; //用于计数等待事件的数量
    std::string client_ip_;
    std::string client_port_;
    std::shared_ptr<gsky::work::work> sp_work_;

    std::map<std::string, std::string> map_client_info_;
    protocol header_;
    int  body_left_length_ = 0;
    void handle_read();
    void handle_write();
    void handle_reset();
    void handle_push_data_reset();
    void handle_work();
    void send_data(const std::string &content);
    void handle_error();
};