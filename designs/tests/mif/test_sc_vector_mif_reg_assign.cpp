/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"

// @sc_vector of MIF to check extra signals assignment "sig_next = sig" 
// in thread process is removed, use signals in MIF process 

struct B : sc_module, sc_interface {

    sc_in<bool>  clk;
    sc_in<bool>  rstn;
    
    sc_signal<int>      SC_NAMED(s);
    
    B(sc_module_name) 
    {
        SC_CTHREAD(sig_assign, clk.pos());
        async_reset_signal_is(rstn, true);
    }
    
    sc_out<int> o1;
    sc_out<int> o2;
    sc_signal<int> t1;
    sc_signal<int> s1;
    sc_signal<int> s2;
    sc_signal<int> s3;
    sc_signal<sc_uint<16>> s4;
    sc_signal<sc_bigint<16>> s5;
    sc_signal<sc_uint<16>> s6;
    sc_signal<unsigned> a1[3];
    
    void sig_assign() {
        o1 = 1;
        o2 = 1;
        s1 = 0;
        s2 = 1;
        s3 = 0;
        s4 = 0;
        a1[0] = 0;
        wait();
        
        while (true) {
            o1 = s.read();          // No @o1_next = o1
            t1 = s.read() ? o1.read() : o2.read();
            s1 = s.read();          // No @s1_next = s1
            t1 = s1 + s2.read();    // No @t1_next = t1
            if (s1.read()) {
                s3 = 42;
                s5 = 1;
                s6 = 2;
            } else {
                s4 = 0;
                s5 = 2;             // No @s5_next = s5
            }
            t1 = s3.read() + s4.read() + s5.read().to_int();
            t1 = s6.read();  
            t1 = a1[s.read()];
            wait();
        }
    }
};

SC_MODULE(Top) {

    sc_in<bool>  clk;
    sc_signal<bool>  rstn;

    const static unsigned M = 2;
    sc_vector<B> mif_b{"mif_b", M};
    sc_vector<sc_signal<int>> o1{"o1", M};
    sc_vector<sc_signal<int>> o2{"o2", M};
    
    SC_CTOR(Top) 
    {
        for (int i = 0; i < M; ++i) {
            mif_b[i].clk(clk); mif_b[i].rstn(rstn);
            mif_b[i].o1(o1[i]);
            mif_b[i].o2(o2[i]);
        }
    }
};

int sc_main(int argc, char **argv) {

    sc_clock clk("clk", 1, SC_NS);
    Top top("top");
    top.clk(clk);
    
    sc_start();

    return 0;
}
