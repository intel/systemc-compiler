//
// Created by ripopov on 2/14/18.
//

#include <systemc.h>

struct top : sc_module {

    static const int x = 42;

    static const int B = 12;
    static const unsigned C = 3;
    static const unsigned A = unsigned((1 + B / C));
    
    SC_CTOR(top) {
    }

};


int sc_main(int argc, char** argv)
{
    cout << "test_static_member\n";
    auto t0 = std::make_unique<top>("top_inst");
    sc_start();
    return 0;
}
