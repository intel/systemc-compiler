/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"
#include <iostream>

// Record (structure/class) returned from static function
struct SinCos
{
    struct SinCosTuple {
        int sin;
        int cos;
    };

    static SinCosTuple convert_sin_cos() {
        SinCosTuple res;
        res.sin = 0;
        res.cos = 1;
        return res;
    }
};


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
            SinCos::SinCosTuple r = SinCos::convert_sin_cos();
            wait();
            
            int a = r.sin;
            sct_assert_const(r.cos == 1);
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

