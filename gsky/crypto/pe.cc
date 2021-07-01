#include <gsky/crypto/pe.hh>

gsky::crypto::pe::pe() {
    
}

gsky::crypto::pe::~pe() {
    
}

// key length is 8 bytes
// 加密概述
// 采用密钥重叠循环，查表来进行异或。
//
void gsky::crypto::pe::encode(unsigned char key[8], gsky::util::vessel &v) {
    unsigned char keys[8];
    memcpy(keys, key, 8);
    size_t length = v.size();
    char *data = v.data();

    for(int i = 0; i < length; i ++) {
        data[i] ^= keys[i % 8];
        unsigned char n = ((keys[i % 8] + keys[(i + 1) % 8]) * keys[(i + 2) % 8]) & 0xff;
        data[i] ^= n ^ xor_table_[n];
        keys[i % 8] = (n * 2) % 0x100;
    }
}

// 解密
void gsky::crypto::pe::decode(unsigned char key[8], gsky::util::vessel &v) {
    unsigned char keys[8];
    memcpy(keys, key, 8);
    size_t length = v.size();
    char *data = v.data();
    for(int i = 0; i < length; i ++) {
        char t_key = keys[i % 8];
        unsigned char n = ((keys[i % 8] + keys[(i + 1) % 8]) * keys[(i + 2) % 8]) & 0xff;
        data[i] ^= n ^ xor_table_[n];
        data[i] ^= t_key;
        keys[i % 8] = (n * 2) % 0x100;
    }
}
