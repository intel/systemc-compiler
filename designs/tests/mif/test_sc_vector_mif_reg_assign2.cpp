/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"

// @sc_vector of MIF to check extra signals assignment "sig_next = sig" 
// in thread process is removed, use signals in parent module process

struct B : sc_module, sc_interface {

    sc_in<bool>  clk;
    sc_in<bool>  rstn;
    
    B(sc_module_name) 
    {
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
    
};

SC_MODULE(Top) {

    sc_in<bool>  clk;
    sc_signal<bool>  rstn;

    const static unsigned M = 2;
    sc_vector<B> mif_b{"mif_b", M};
    sc_vector<sc_signal<int>> o1{"o1", M};
    sc_vector<sc_signal<int>> o2{"o2", M};
    
    sc_signal<int>      SC_NAMED(s);
    
    SC_CTOR(Top) 
    {
        for (int i = 0; i < M; ++i) {
            mif_b[i].clk(clk); mif_b[i].rstn(rstn);
            mif_b[i].o1(o1[i]);
            mif_b[i].o2(o2[i]);
        }

        SC_CTHREAD(sig_assign, clk.pos());
        async_reset_signal_is(rstn, true);
    }

    void sig_assign() {
        mif_b[0].o1 = 1;
        mif_b[0].o2 = 1; mif_b[1].o2 = 1;
        for (int i = 0; i != M; ++i) {
            mif_b[i].s1 = 0;
            mif_b[i].s2 = 1;
            mif_b[i].s3 = 0;
            mif_b[i].s4 = 0;
            mif_b[i].a1[0] = 0;
        }
        wait();
        
        while (true) {
            mif_b[0].o1 = s.read();          // No @o1_next = o1
            mif_b[0].t1 = s.read() ? mif_b[0].o1.read() : mif_b[1].o2.read();
            
            for (int i = 0; i != M; ++i) {
                mif_b[i].s1 = s.read();          // No @s1_next = s1
                mif_b[i].t1 = mif_b[i].s1 + mif_b[i].s2.read();    // No @t1_next = t1
                if (mif_b[i].s1.read()) {
                    mif_b[i].s3 = 42;
                    mif_b[i].s5 = 1;
                    mif_b[i].s6 = 2;
                } else {
                    mif_b[i].s4 = 0;
                    mif_b[i].s5 = 2;             // No @s5_next = s5
                }
                mif_b[i].t1 = mif_b[i].s3.read() + mif_b[i].s4.read() + 
                              mif_b[i].s5.read().to_int();
                mif_b[i].t1 = mif_b[i].s6.read();  
                mif_b[i].t1 = mif_b[i].a1[s.read()];
            }
            wait();
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
