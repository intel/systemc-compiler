/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"
#include <iostream>

// Record (structure/class) returned from class function
struct SinCosTuple 
{
    SinCosTuple()
    {}
    
    int sin = 1;
    int cos = 2;
};

struct SinCos
{
    SinCosTuple convert_sin_cos() {
        SinCosTuple res;
        sct_assert_const(res.sin == 1);
        return res;
    }
};

class A : public sc_module {
public:
    sc_in_clk clk;
    sc_signal<bool> rstn;
    
    SinCos scr;
    
    SC_CTOR(A) {
        SC_METHOD(record_meth);
        sensitive << rstn;
        
        SC_CTHREAD(record_return, clk.pos());
        async_reset_signal_is(rstn, false);
    }
    
    void record_meth() {
        SinCosTuple r;
        sct_assert_const(r.sin == 1);
        sct_assert_const(r.cos == 2);
    }
    
    void record_return() {
        
        wait();
        
        while (true) {
            SinCosTuple r = scr.convert_sin_cos();
            sct_assert_const(r.sin == 1);
            sct_assert_const(r.cos == 2);
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

