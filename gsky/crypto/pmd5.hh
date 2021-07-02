#pragma once

#include <string>
#include <fstream>
#include <cstring>

namespace gsky {
namespace crypto {

/* Type define */
typedef unsigned char byte;
typedef unsigned long ulong;

/* Pwnsky md5 declaration. */
class pmd5 {
public:
	pmd5();
	pmd5(const void *input, size_t length);
    pmd5(const std::string &str);
    pmd5(std::ifstream &in);
	void update(const void *input, size_t length);
    void update(const std::string &str);
    void update(std::ifstream &in);
	const byte* digest();
    std::string to_string();
    std::string to_lower_case_string();
	void reset();
private:
	void update(const byte *input, size_t length);
	void final();
	void transform(const byte block[64]);
	void encode(const ulong *input, byte *output, size_t length);
	void decode(const byte *input, ulong *output, size_t length);
    std::string bytes_to_hex_string(const byte *input, size_t length);

	/* class uncopyable */
	pmd5(const pmd5&);
	pmd5& operator=(const pmd5&);
private:
	ulong _state[4];	/* state (ABCD) */
	ulong _count[2];	/* number of bits, modulo 2^64 (low-order word first) */
	byte _buffer[64];	/* input buffer */
	byte _digest[16];	/* message digest */
	bool _finished;		/* calculate finished ? */

	static const byte PADDING[64];	/* padding for calculate */
	static const char HEX[16];
	static const size_t BUFFER_SIZE = 1024;
};

}
}
