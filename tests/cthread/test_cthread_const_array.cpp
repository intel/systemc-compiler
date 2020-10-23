//
// Created by ripopov on 12/19/18.
//

#include <systemc.h>


struct S {

    static constexpr int ARR2[2] = {40,41};

};

static constexpr int ARR1[2] = {30,31};

SC_MODULE(test) {

    static constexpr int ARR0[2] = {20,21};

    sc_signal<bool> clk{"clk"};
    sc_signal<bool> rstn{"rstn"};

    SC_CTOR(test) {
        SC_METHOD(test_method); sensitive << clk << idx;
        
        SC_CTHREAD(test_thread, clk);
        async_reset_signal_is(rstn, false);
    }

    void test_method() {
            outs = ARR0[idx.read()];
            outs = ARR1[idx.read()];
            outs = S::ARR2[idx.read()];
    }

    sc_signal<bool> idx{"idx"};
    sc_signal<int> outs{"outs"};

    void test_thread() {
        wait();
        while (1) {
            outs = ARR0[idx.read()];
            outs = ARR1[idx.read()];
            outs = S::ARR2[idx.read()];
            wait();
        }
    }

};

int sc_main(int argc, char **argv) {

    test test_inst{"test_inst"};
    sc_start();

    return 0;
}