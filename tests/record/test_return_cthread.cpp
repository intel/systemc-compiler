/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"
#include <iostream>

// Record (structure/class) returned from function
struct SinCosTuple {
    int sin;
    int cos;
    sc_uint<4> coeff;
    
    SinCosTuple() {}
};

SinCosTuple convert_sin_cos() {
    SinCosTuple res;
    res.sin = 4;
    return res;
}

class A : public sc_module {
public:
    sc_in_clk clk;
    sc_signal<bool> rstn;
    
    SC_CTOR(A) {
        SC_CTHREAD(record_return, clk.pos());
        async_reset_signal_is(rstn, false);
    }
    
    void record_return() {
        
        wait();
        
        while (true) {
            SinCosTuple r = convert_sin_cos();
            sct_assert_const(r.sin == 4);
            wait();
        }
    }
};

int sc_main(int argc, char *argv[]) 
{
    sc_clock clk("clk", 1, SC_NS);
    A a{"a"};
    a.clk(clk);
    
    sc_start();
    return 0;
}

