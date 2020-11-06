/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

struct Simple {
    bool a;
    sc_uint<4> b;
};

// Record local variable and member in MIF 
struct mod_if : public sc_module, sc_interface 
{
    sc_in_clk           clk{"clk"};
    sc_signal<bool>     nrst;
    sc_signal<bool>     s {"s"};

    SC_CTOR(mod_if) 
    {
        SC_CTHREAD(localRecThread, clk.pos());
        async_reset_signal_is(nrst, false);

        SC_CTHREAD(localRecArrThread, clk.pos());
        async_reset_signal_is(nrst, false);

        SC_CTHREAD(memRecThread, clk.pos());
        async_reset_signal_is(nrst, false);

        SC_CTHREAD(memRecArrThread, clk.pos());
        async_reset_signal_is(nrst, false);
    }
    
    // Local record
    void localRecThread() 
    {
        Simple  t;      // reg
        Simple  tt;     // comb
        t.a = false; t.b = 1;
        wait();
   
        while (true) {
            tt.a = s.read();
            tt.b = tt.a ? t.b : (sc_uint<4>)0;
            int i = t.b + tt.b;
            wait();
            
            t.b = t.b + 1;
        }
    }
    
    // Local record array
    void localRecArrThread() 
    {
        Simple  v[2];      // reg
        Simple  vv[4];     // comb
        for (int i = 0; i < 2; ++i) {
            v[i].a = false; v[i].b = i+1;
        }
        wait();
   
        while (true) {
            vv[1].a = s.read();
            vv[2].b = vv[1].a ? v[0].b : v[1].b;
            
            int sum = 0;
            for (int i = 0; i < 4; ++i) {
                vv[i].b = (i < 2) ? v[i].b : (sc_uint<4>)i;
                sum += vv[i].b;
            }
            
            wait();
        }
    }
     
    // Member record
    Simple  r;      // reg
    Simple  rr;     // comb

    void memRecThread() 
    {
        r.a = false; r.b = 1;
        wait();
   
        while (true) {
            rr.a = s.read();
            rr.b = rr.a ? r.b : (sc_uint<4>)0;
            int i = r.b + rr.b;
            wait();
            
            r.b = r.b + 1;
        }
    }
    
    // Member record array
    Simple  w[2];      // reg
    Simple  ww[4];     // comb

    void memRecArrThread() 
    {
        for (int i = 0; i < 2; ++i) {
            w[i].a = false; w[i].b = i+1;
        }
        wait();
   
        while (true) {
            ww[1].a = s.read();
            ww[2].b = ww[1].a ? w[0].b : w[1].b;
            
            int sum = 0;
            for (int i = 0; i < 4; ++i) {
                ww[i].b = (i < 2) ? w[i].b : (sc_uint<4>)i;
                sum += ww[i].b;
            }
            
            wait();
        }
    }
};

SC_MODULE(Top) 
{

    sc_in_clk       clk{"clk"};
    mod_if          minst{"minst"};
    mod_if          ninst{"ninst"};

    SC_CTOR(Top) {
        minst.clk(clk);
        ninst.clk(clk);
    }
    
    sc_signal<bool>     w;
};

int sc_main(int argc, char **argv) 
{
    sc_clock  clk("clk", sc_time(1, SC_NS));
    Top top{"top"};
    top.clk(clk);
    
    sc_start();

    return 0;
}
