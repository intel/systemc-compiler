//
// Created by ripopov on 7/8/18.
//

#include <systemc.h>
#include <sct_assert.h>

SC_MODULE(top) {

    sc_clock    clk_gen{"clk_gen", 10 , SC_NS};

    SC_CTOR(top) {
        SC_THREAD(test_thread);
        sensitive << clk_gen.posedge_event();
    }

    sc_signal<bool> din;
    sc_signal<bool> din2;

    int x;
    int y;

    void wait_wrap () {
        x++;

        sct_assert_const(x == 2);
        wait();
        x ++;
        sct_assert_const(x == 3);
    }

    void dwait_wrap () {
        x++;
        wait_wrap();
        x++;
    }

    void test_thread() {
        x = 0;
        y = 0;

        dwait_wrap();
        sct_assert_const(x == 4);

    }

};

int sc_main (int argc, char ** argv ) {

    top t_inst{"t_inst"};
    sc_start(100, SC_NS);

    return 0;
}