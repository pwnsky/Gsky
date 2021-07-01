#pragma once
#include <gsky/gsky.hh>

/*
 * Define pwnsky protocol
 * */

namespace gsky {
namespace net {
namespace pp {

enum class status {
    // Client request code
    connect = 0x10,
    data_transfer = 0x11,

    // Server response code
    protocol_error = 0x20,
    too_big = 0x21, // The content size too big
    invalid_transfer = 0x22, // 
    ok = 0x30,
    send_key = 0x31,

    redirct = 0x40, // server set route
};

enum class data_type {
    binary_stream = 0x00,
    image = 0x01,
    video = 0x02,
    music = 0x03,

    text = 0x10,
    json = 0x11,
    xml  = 0x12,
};

// Size 32 bytes
struct header {
    unsigned short magic;    // "PP" 0x5050
    unsigned char status;    // The status code of client or server
    unsigned char type;      // The type of data
    unsigned int length;     // The length of data
    char route[8];           // The route of request
};
}
}
}
