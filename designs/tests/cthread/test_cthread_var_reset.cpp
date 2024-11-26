/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Register variables/channels/arrays in CTHREAD

struct Simple {
    bool a;
    int b;

    bool operator ==(const Simple& oth) {
        return (a == oth.a && b == oth.b);
    }
};

namespace std {
inline ::std::ostream& operator << (::std::ostream& os, const Simple&) 
{return os;}
}

struct A : public sc_module
{
    sc_in_clk               clk;
    sc_in<bool>             rst;

    sc_in<int>              in;     
    sc_out<int>             out;
    sc_signal<int>          a;

    SC_CTOR(A) 
    {
        SC_CTHREAD(local_var1, clk.pos());
        async_reset_signal_is(rst, true);

        SC_CTHREAD(local_var2, clk.pos());
        async_reset_signal_is(rst, true);

        SC_CTHREAD(local_rec_var1, clk.pos());
        async_reset_signal_is(rst, true);

        SC_CTHREAD(local_rec_var2, clk.pos());
        async_reset_signal_is(rst, true);
    }

    sc_signal<int> s1;
    void local_var1()
    {
        int w = 0;
        int wa[2] = {};
        wait();
        
        while (true) {
            int v = 2;
            int va[2] = {1,2};
            wait();
            s1 = w + v + wa[a.read()] + va[a.read()];
        }
    }
    
    int f1(int par) {
        while (a.read()) {par += 1; wait();}
        return par;
    }
    
    int f2(int par) {
        while (a.read()) {par += 1; wait();}
        return par;
    }

    sc_signal<int> s2;
    void local_var2()
    {
        int w = 0;
        wait();
        
        while (true) {
            int v = a.read();
            s2 = f1(v);
            s2 = f2(w);
            wait();
        }
    }

    sc_signal<int> r1;
    void local_rec_var1()
    {
        Simple w; w.b = 0;
        Simple wa[2]; wa[0].b = 0; wa[1].b = 0;
        wait();
        
        while (true) {
            Simple v; v.a = a.read(); v.b = 42;
            Simple va[2]; va[0].b = 43; va[1].b = 44;
            wait();
            r1 = (v.a ? w.b : v.b) + wa[a.read()].b + va[a.read()].b;
        }
    }
    
    Simple g1(Simple par) {
        while (a.read()) {par.b = par.b+1; wait();}
        return par;
    }
    
    Simple g2(Simple par) {
        while (a.read()) {par.b = par.b+1; wait();}
        return par;
    }
    
    sc_signal<Simple> r2;
    void local_rec_var2()
    {
        Simple w; w.b = 0;
        wait();
        
        while (true) {
            Simple v; v.a = a.read(); v.b = 42;
            r2 = g1(v);
            r2 = g2(w);
            wait();
        }
    }
};

int sc_main(int argc, char **argv) 
{
    sc_clock clk {"clk", sc_time(1, SC_NS)};
    sc_signal<bool> rst;
    sc_signal<int>  t;      
    
    A a_mod{"a_mod"};
    a_mod.clk(clk);
    a_mod.rst(rst);
    a_mod.in(t);
    a_mod.out(t);
    
    sc_start();

    return 0;
}
