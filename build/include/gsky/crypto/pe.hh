#include <gsky/gsky.hh>
#include <gsky/util/vessel.hh>

#include <string>

class gsky::crypto::pe {
    pe();
    ~pe();
    void encode(char key[], gsky::util::vessel &v, unsigned int length);
    void decode(char key[], gsky::util::vessel &v, unsigned int length);
};
