/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>
#include <string>

// Signal which is multiple bound to different modules

SC_MODULE(A) {
    sc_in<bool>     in{"in"};
    sc_signal<bool> t{"t"};
    
    SC_CTOR(A) {
        SC_METHOD(meth);
        sensitive << in;
    }
    
    void meth()
    {
        bool b = in;
        t = b;
    }
};

SC_MODULE(B) {
    sc_in<bool>     clk{"clk"};
    sc_in<bool>     rstn{"rstn"};
    sc_signal<bool> s{"s"};
    
    A a{"a"};
    
    SC_CTOR(B) {
        a.in(s);
        
        SC_CTHREAD(thread, clk.pos());
        async_reset_signal_is(rstn, 0);
    }
    
    void thread() {
        s = 0;
        wait(); 
        while (1) {
            s = !s;
            wait();
        }
    }
};

SC_MODULE(C) 
{
    sc_in<bool> in{"in"};
    sc_signal<bool> v{"v"};

    SC_CTOR(C) {
        SC_METHOD(meth);
        sensitive << in;
    }
    
    void meth() {
        bool b = in;
        v = b;
    }
};

SC_MODULE(D) 
{
    sc_in<bool>     clk{"clk"};
    sc_in<bool>     rstn{"rstn"};

    B b{"b"};
    C c{"c"};

    SC_CTOR(D) {
        b.clk(clk);
        b.rstn(rstn);
        c.in(b.s);
    }
};

int sc_main(int argc, char **argv) {
    sc_clock clk("clk", 1, SC_NS);
    sc_signal<bool> rstn;
    
    D d{"d"};
    d.clk(clk);
    d.rstn(rstn);
    
    sc_start();

    return 0;
}
