//
// Created by ripopov on 1/29/18.
//

#include <systemc.h>

struct top : sc_module {
    SC_CTOR(top) {

    }

    sc_int<16> large_scint_array[1024*1024];
    int large_mdim_array[10][1024][1024];

    sc_int<16> *scint_ptr = &large_scint_array[100000];
    int *mdim_ptr = &large_mdim_array[1][0][0];

};

int sc_main(int argc, char** argv)
{
    auto t0 = std::make_unique<top>("top_inst");
    sc_start();
    return 0;
}
