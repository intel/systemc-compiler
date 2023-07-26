/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by ripopov on 10/16/18.
//

#include <systemc.h>

struct wbase {
    int y;
    virtual void vwait() {
        y ++;
    }
};

struct wderived : wbase {

    void vwait() override
    {
        y += 2;
        sc_core::wait();
    }

};

void wait_wrapper() {
    sc_core::wait();
}


SC_MODULE(test_mod) {

    sc_clock clk{"clk", 10, SC_NS};
    sc_signal<bool> rstn{"rstn"};

    wbase *wptr = sc_new<wderived>();

    SC_CTOR(test_mod) {
        SC_CTHREAD(single_state_thread0, clk);
        async_reset_signal_is(rstn, false);
        SC_CTHREAD(single_state_thread1, clk);
        async_reset_signal_is(rstn, false);
        SC_CTHREAD(single_state_thread2, clk);
        async_reset_signal_is(rstn, false);
        SC_CTHREAD(single_state_thread3, clk);
        async_reset_signal_is(rstn, false);
        SC_CTHREAD(single_state_thread4, clk);
        async_reset_signal_is(rstn, false);


        SC_CTHREAD(multistate_thread0, clk);
        async_reset_signal_is(rstn, false);
        SC_CTHREAD(multistate_thread1, clk);
        async_reset_signal_is(rstn, false);
        SC_CTHREAD(multistate_thread2, clk);
        async_reset_signal_is(rstn, false);
        SC_CTHREAD(multistate_thread3, clk);
        async_reset_signal_is(rstn, false);
        SC_CTHREAD(multistate_thread4, clk);
        async_reset_signal_is(rstn, false);
        SC_CTHREAD(multistate_thread5, clk);
        async_reset_signal_is(rstn, false);
    }

    sc_signal<int> t0;
    void single_state_thread0() {
        int x = 0;
        wait();
        while (2 > 1) {
            x++;
            t0 = x;
            wait();
        }
    }

    sc_signal<int> t1;
    void single_state_thread1() {
        int x = 0;
        wait();
        for(;;) {
            t1 = x++;
            wait();
        }
    }

    sc_signal<int> t2;
    void single_state_thread2() {
        int x = 0;
        wait();
        do {
            x++;
            t2 = x;
            wait();
        } while (1 == 1);
    }

    sc_signal<int> t3;
    void single_state_thread3() {
        int x = 0;
        while (1) {
            wait();
            t3 = x++;
        }
    }

    void empty_method() {

    }

    sc_signal<int> t4;
    void single_state_thread4() {
        int x = 0;
        while (1) {
            wait();
            empty_method();
            t4 = x++;
        }
    }

    sc_signal<int> t5;
    void multistate_thread0() {
        int x = 0;
        wait();
        x++;
        while (1) {
            x++; t5 = x;
            wait();
        }
    }

    sc_signal<int> t6;
    void multistate_thread1() {
        int x = 0;
        wait();
        while (1) {
            wait();
            x++;
            t6 = x;
        }
    }

    void wait_method() {
        wait();
    }

    sc_signal<int> t7;
    void multistate_thread2() {
        int x = 0;
        while (1) {
            wait();
            wait_method();
            x++;
            t7 = x;
        }
    }

    sc_signal<int> t7a;
    void multistate_thread3() {
        int x = 0;
        while (1) {
            wait();
            wait_wrapper();
            t7a = x++;
        }
    }

    sc_signal<int> t8;
    void multistate_thread4() {
        int x = 0;
        while (1) {
            wait();
            wptr->vwait();
            x++; t8 = x;
        }
    }

    void wbase_invoker(wbase &wb) {
        wb.vwait();
    }

    wderived wd;

    sc_signal<int> t9;
    void multistate_thread5() {
        int x = 0;
        while (1) {
            wait();
            wbase_invoker(wd);
            t9 = x++;
        }
    }

};



int sc_main(int argc, char **argv) {

    test_mod t{"t"};
    sc_start(1,SC_NS);

    return 0;
}
