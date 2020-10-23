//
// Created by ripopov on 3/31/18.
//

#include <systemc.h>


struct top : sc_module {

    SC_CTOR(top) {

    }

    void test_method() { }

};

int sc_main(int argc, char** argv)
{
    auto t0 = std::make_unique<top>("top_inst");
    sc_start();
    return 0;
}
