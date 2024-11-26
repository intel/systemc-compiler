/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

using namespace sc_core;

// Sequential method sensitive to clock edge, not supported for now, see #304
class A : public sc_module {
public:
    sc_in_clk       clk{"clk"};
    sc_in<bool>     nrst{"nrst"};
    sc_signal<int>  a{"a"};
    sc_signal<int>  b{"b"};
    sc_signal<int>  c{"c"};

    SC_CTOR(A) {
        SC_METHOD(asyncMeth);
        sensitive << clk.pos() << nrst << a; // nrst.neg();

        //SC_CTHREAD(thrdProc, clk.pos()); 
        //async_reset_signal_is(nrst, 0);

        //SC_METHOD(syncMeth); 
        //sensitive << clk.pos();
    }
    
    void asyncMeth() 
    {
        if (nrst) 
            a = 0;
         else {
            if (b) a = 1;// else a = 2;
            //a = a + 1;
        }
    }
    
    void thrdProc() {
        c = 0;
        wait();
        while(true) {
            c = c + 1;
            wait();
        }
    }

    void syncMeth() 
    {
        if (nrst) {
            b = 0;
        } else {
            b = 1;
        }
    }
};

int sc_main(int argc, char *argv[]) {
    sc_clock clock{"clock", 1, SC_NS};
    sc_signal<bool>  nrst{"nrst"};
    A a{"a"};
    a.clk(clock);
    a.nrst(nrst);
    
    sc_start();
    return 0;
}

