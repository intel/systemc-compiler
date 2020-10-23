//
// Created by ripopov on 2/27/18.
//

#include <systemc.h>

struct mod_if : sc_module, sc_interface {

    sc_in_clk       clk{"clk"};
    sc_signal<bool> enable{"enable"};

    SC_CTOR(mod_if) {
        SC_METHOD(test_method);
        sensitive << clk.pos();
    }

    void test_method() {}
};

SC_MODULE(top) {

    sc_signal<bool>   clk{"clk"};
    sc_vector<mod_if> mod_vec{"mod_vec", 3};

    SC_CTOR(top) {
        SC_METHOD(test_method);

        for (auto &mod: mod_vec) {
            mod.clk(clk);
            sensitive << mod.enable;
        }
    }

    void test_method() {}
};

int sc_main (int argc, char **argv) {
    cout << "test_modif_vector\n";
    top t0{"t0"};
    sc_start();
    return 0;
}
