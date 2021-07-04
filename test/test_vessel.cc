#include <gsky/util/vessel.hh>
using namespace gsky::util;

void test1() {
    vessel v;    
    v << "abcdefghijk";
    std::cout << v.to_string() << '\n';

    v.sub(2);

    std::cout << v.to_string() << '\n';

    v << "*****";
    std::cout << v.to_string() << '\n';

    //v.sub(100);
    //std::cout << v.to_string() << '\n';

    v.resize(1);
    std::cout << "resize: " << v.to_string() << '\n';

    v.resize(0x100);
    std::cout << "resize: " << v.to_string() << '\n';
    std::cout << "size: " << v.size() <<  " capacity: " << v.capacity() << '\n';
    v.sub(0x80);
    std::cout << "size: " << v.size() <<  " capacity: " << v.capacity() << '\n';
}
int main() {
    test1();
    return 0;
}
