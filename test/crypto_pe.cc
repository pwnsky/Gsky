#include <gsky/crypto/pe.hh>
#include <gsky/util/vessel.hh>
#include <iostream>
using namespace gsky;

int main() {
    unsigned char key[8] = {3, 2, 3, 4, 5, 6, 7, 8};
    crypto::pe p;    
    util::vessel v; 
    std::string str = "12345678asdfj12345678\n";
    v << str;

    std::cout << "加密前: " << v.to_string();
    p.encode(key, v);
    std::cout << "加密后: " << v.to_string();
    p.decode(key, v);
    std::cout << "\n解密后: " << v.to_string();
}
