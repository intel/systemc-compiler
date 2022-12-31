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

    SC_CTOR(A) {
        SC_METHOD(asyncMeth); 
        sensitive << clk.pos() << nrst.neg();

        SC_METHOD(syncMeth); 
        sensitive << clk.pos();
    }
    
    void asyncMeth() 
    {
        if (nrst) {
            a = 0;
        } else {
            a = 1;
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

