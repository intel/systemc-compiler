/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"
#include <iostream>

// Record (structure/class) returned from module function
struct SinCosTuple 
{
    int sin;
    int cos = -1;
};

struct SinCos : public sc_module, public sc_interface
{
    const sc_uint<3> c = 5;
    explicit SinCos(const sc_module_name& name) : sc_module(name) 
    {}

    SinCosTuple convert_sin_cos() {
        SinCosTuple res;
        res.sin = c+1;
        return res;
    }
};

SinCosTuple f() {
    SinCosTuple res;
    res.sin = 1;
    return res;
}

class A : public sc_module {
public:
    sc_in_clk clk;
    sc_signal<bool> rstn;
    
    SinCos scr;
    sc_signal<int> s{"s"};
    
    SC_CTOR(A) : scr("scr") 
    {
        SC_METHOD(record_return_meth); sensitive << s;
        
        SC_CTHREAD(record_return1, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(record_return2, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(record_return_reset, clk.pos());
        async_reset_signal_is(rstn, false);
    }
    
    sc_signal<int> t;
    void record_return_meth() {
        SinCosTuple r = scr.convert_sin_cos();
        t = r.sin;
        SinCosTuple rr = f();
        t = rr.cos;
    }
    
    void record_return1() {
        
        wait();
        
        while (true) {
            SinCosTuple r = scr.convert_sin_cos();
            SinCosTuple rr = f();
            s = r.cos;
            wait();
            s = r.sin + rr.cos;
        }
    }

    void record_return2() {
        SinCosTuple mm;
        wait();
        
        while (true) {
            SinCosTuple m;
            if (s.read()) {
                m = scr.convert_sin_cos();
            }
            
            wait();

            if (s.read()) {
                mm = f();
            }
            s = m.sin + mm.cos;
        }
    }
    
     void record_return_reset() {
        
        SinCosTuple l  = scr.convert_sin_cos();
        SinCosTuple ll = f();
        wait();
        
        while (true) {
            s = l.sin + ll.cos;
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

