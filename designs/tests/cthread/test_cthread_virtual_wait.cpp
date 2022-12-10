/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// wait() call in virtual function overridden in child module
struct base : sc_module {

    sc_in_clk clk{"clk"};
    sc_signal<bool> nrst;

    SC_CTOR(base) {
        SC_THREAD(test_thread);
        sensitive << clk.pos();
        async_reset_signal_is(nrst, true);
    }

    virtual void virtual_wait() {
        wait();
    }

    void test_thread() {
        wait();
        while (1) {
            virtual_wait();
        }
    }

};

struct derived : base {

    derived(sc_module_name nm) : base(nm) {}

    sc_signal <bool> x{"x"};

    void virtual_wait() override
    {
        x = 0;
        wait();
        x = 1;
        wait();
    }

};


struct top : sc_module {

    sc_signal<bool> clk{"clk"};
    derived d{"d"};

    SC_CTOR(top) {
        d.clk(clk);
    }

};

int sc_main(int argc, char **argv) {

    top t{"t"};

    sc_start();
    return 0;
}
