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

    void single_state_thread0() {
        int x = 0;
        wait();
        while (2 > 1) {
            x++;
            wait();
        }
    }

    void single_state_thread1() {
        int x = 0;
        wait();
        for(;;) {
            x++;
            wait();
        }
    }

    void single_state_thread2() {
        int x = 0;
        wait();
        do {
            x++;
            wait();
        } while (1 == 1);
    }

    void single_state_thread3() {
        int x = 0;
        while (1) {
            wait();
            x++;
        }
    }

    void empty_method() {

    }

    void single_state_thread4() {
        int x = 0;
        while (1) {
            wait();
            empty_method();
            x++;
        }
    }

    void multistate_thread0() {
        int x = 0;
        wait();
        x++;
        while (1) {
            x++;
            wait();
        }
    }

    void multistate_thread1() {
        int x = 0;
        wait();
        while (1) {
            wait();
            x++;
        }
    }

    void wait_method() {
        wait();
    }

    void multistate_thread2() {
        int x = 0;
        while (1) {
            wait();
            wait_method();
            x++;
        }
    }

    void multistate_thread3() {
        int x = 0;
        while (1) {
            wait();
            wait_wrapper();
            x++;
        }
    }

    void multistate_thread4() {
        int x = 0;
        while (1) {
            wait();
            wptr->vwait();
            x++;
        }
    }

    void wbase_invoker(wbase &wb) {
        wb.vwait();
    }

    wderived wd;

    void multistate_thread5() {
        int x = 0;
        while (1) {
            wait();
            wbase_invoker(wd);
            x++;
        }
    }

};



int sc_main(int argc, char **argv) {

    test_mod t{"t"};
    sc_start(1,SC_NS);

    return 0;
}
