#include <gsky/util/vessel.hh>

#include <string>
namespace gsky {
namespace crypto {
class pe {
public:
    static void encode(unsigned char key[8], void *raw_data, size_t length);
    static void decode(unsigned char key[8], void *raw_data, size_t length);
    static unsigned char xor_table_[256];
private:
    static void key_random(unsigned char raw_key[8], unsigned char *out_key, unsigned char seed);
};

}
}
