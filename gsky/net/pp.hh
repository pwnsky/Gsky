#pragma once
#include <gsky/gsky.hh>

/*
 * Define pwnsky protocol
 * */

namespace gsky {
namespace net {
enum class pp_status {
    // Client request code
    request_connect = 0x0,
    data_transfer = 0x1,

    // Server response code
    protocol_error = 0x20,
    ok = 0x30,

    redirct = 0x40, // server set route
};

enum class pp_data_type {
    binary_stream = 0x00,
    image = 0x01,
    video = 0x02,
    music = 0x03,

    text = 0x10,
    json = 0x11,
    xml  = 0x12,
};

// Size 32 bytes
struct pp_header {
    unsigned short magic;    // "PP" 0x5050
    unsigned char status;    // The status code of client or server
    unsigned char type;      // The type of data
    char route[8];           // The route of request
    union {
        char key[16];        // The key of PE (Pwnsky Encryption)
        char token[16];      // The token of client
    };
    unsigned int length;     // The length of data
};

}
}
