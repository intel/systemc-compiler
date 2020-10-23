//
// Created by ripopov on 2/17/18.
//


#include <systemc.h>

struct virt_base {
    sc_signal<int> sig_virt{"sig_virt"};
};

struct base0 : virtual virt_base {
    sc_in <int> in_0{"in_0"};
};

struct base1 : virtual virt_base {
    sc_out <int> out_0{"out_0"};
};

struct top : sc_module, base0, base1 {

    SC_CTOR(top) {
        in_0.bind(sig_virt);
        out_0.bind(sig_virt);
    }
};

int sc_main(int argc, char** argv)
{
    cout << "test virtual base\n";
    auto t0 = std::make_unique<top>("top_inst");
    sc_start();
    return 0;
}
