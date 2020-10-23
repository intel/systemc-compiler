//
// Created by ripopov on 12/17/18.
//

#include <systemc.h>

SC_MODULE(test_sens_arrays) {
    sc_signal<bool> clk{"clk"};

    SC_CTOR(test_sens_arrays) {
        for (size_t i = 0; i < 3; ++i) {
            ins[i] ( sigs [i] );
        }

        SC_METHOD(method0);
        sensitive << sigs[1];

        SC_METHOD(method1);
        sensitive << ins[1] << ins[2];

        SC_METHOD(method2);
        sensitive << ins[0] << ins[1] << ins[2];

        SC_THREAD(test_thread);
        sensitive << clk.posedge_event();
        async_reset_signal_is(ins[2],false);

    }

    sc_signal<bool> sigs[3];
    sc_in<bool> ins[3];

    void method0() {

    }

    void method1() {

    }

    void method2() {

    }

    void test_thread() {
        while (1) {wait();}
    }

};

SC_MODULE(top) {

    test_sens_arrays t0{"t0"};
    test_sens_arrays t1{"t1"};
    test_sens_arrays t2{"t2"};

    SC_CTOR(top) {

    }

};

int sc_main(int argc, char **argv) {
    top top_inst{"top_inst"};
    sc_start();
    return 0;
}