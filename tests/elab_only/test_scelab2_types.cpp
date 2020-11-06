/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by ripopov on 9/7/18.
//

#include <systemc.h>
#include <typeinfo>

struct vbase {
    short x0 = 100;
    short x00 = 1000;
};

struct vbase2 {
    int x1 = -1;
};

struct vder0 : virtual vbase {
    long x2 = 2;
};

struct vder1 : virtual vbase {
    uint64_t x3 = 3;
};

struct vder2 : vder0, vder1 {
    short  x4 = 4;
};

struct derived : vder2, virtual vbase2  {
    uint32_t x5 = 5;
};

struct test_mod : sc_module {

    sc_signal<bool> clk{"clk"};

    sc_in<bool> clkin{"clkin"};
    sc_in<bool> clkin2{"clkin2"};

    SC_CTOR(test_mod) {

        SC_METHOD(test_method);
//        sensitive << clkin.pos();
//        sensitive << clkin.neg();
        sensitive << clk.negedge_event();
        sensitive << clk;
//        sensitive << clk;
//        sensitive << clk.posedge_event();
//        sensitive << clk.negedge_event();
//        sensitive << clk.posedge_event();
//        sensitive << clk.negedge_event();

        clkin(clk);
        clkin2(clk);

        SC_CTHREAD(test_cthread, clk);
        SC_THREAD(test_thread);
        async_reset_signal_is(clk, false);
        reset_signal_is(clk, true);
    }

    void test_method () {
    };

    void test_cthread() {
        while (1) {wait();}
    }

    void test_thread() {
        while (1) {wait();}
    }

protected:

};

int sc_main(int argc, char **argv) {
    auto tb_inst = new test_mod {"tb_inst"};
    sc_start();
    return 0;
}

