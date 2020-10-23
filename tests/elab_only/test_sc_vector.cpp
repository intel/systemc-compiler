//
// Created by ripopov on 1/29/18.
//

#include <systemc.h>

struct top : sc_module {
    sc_vector<sc_in<int>> in_vec{"in_vec", 2};
    sc_vector<sc_signal<int>> sig_vec{"sig_vec", 2};

    SC_CTOR(top) {
        in_vec.bind(sig_vec);
    }
};

int sc_main(int argc, char** argv)
{
    auto t0 = std::make_unique<top>("top_inst");
    sc_start();
    return 0;
}
