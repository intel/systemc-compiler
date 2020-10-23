//
// Created by ripopov on 2/27/18.
//

#include <systemc.h>

struct mod_if : sc_module, sc_interface {

    sc_in_clk   clk{"clk"};

    SC_CTOR(mod_if) {
        SC_METHOD(test_method);
        sensitive << clk.pos();
    }

    void test_method() {}
};

SC_MODULE(top) {

    sc_signal<bool> clk{"clk"};
    mod_if mif{"mif"};

    SC_CTOR(top) {
        mif.clk(clk);
        SC_METHOD(test_method);
        sensitive << clk.posedge_event();
    }

    void test_method() {}
};

int sc_main (int argc, char **argv) {
    cout << "test_modular_if0\n";
    top t0{"t0"};
    sc_start();
    return 0;
}
