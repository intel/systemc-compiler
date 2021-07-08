/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by Mikhail Moiseev
//

#include "sct_assert.h"
#include <systemc.h>

// Code after reset wait() before main loop
class top : sc_module
{
public:
    sc_in_clk       clk;
    sc_signal<bool> rstn;
    
    sc_signal<bool> s;

    
    SC_HAS_PROCESS(top);
    top(sc_module_name)
    {
        SC_CTHREAD(after1, clk.pos());
        async_reset_signal_is(rstn, false);
      
        SC_CTHREAD(after2, clk.pos());
        async_reset_signal_is(rstn, false);
        
        SC_CTHREAD(after_multi1, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(after_multi2, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(after_multi3, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(after_multi4, clk.pos());
        async_reset_signal_is(rstn, false);
    }
    
    sc_signal<sc_uint<4>> s1;
    void after1() 
    {
        sc_uint<4> a = 1;
        s1 = 0;
        
        wait();
        
        sc_uint<4> b = 2;
        s1 = a + b;
        
        while (true) {
            a++;
            wait();
            s1 = a;
        }
    }
    
    // if and for after reset
    sc_signal<sc_uint<4>> s2;
    sc_signal<sc_uint<4>> s3[3];
    void after2() 
    {
        sc_uint<4> a = 1;
        s2 = 0;
        for (int i = 0; i < 3; i++) {
            s3[i] = 0;
        }
        
        wait();
        
        if (s1.read() == 1) {
            s2 = a + 1;
        }
        
        for (int i = 0; i < 3; i++) {
            s3[i] = s1.read() + 1;
        }
        
        while (true) {
            a++;
            s2 = a;
            wait();
        }
    }
    
    // multiple wait() after reset
    sc_signal<sc_uint<4>> s4;
    void after_multi1() 
    {
        sc_uint<4> c = 1;
        wait();         // 0

        s4 = 1;
        wait();         // 1
        
        s4 = 2;
        wait();         // 2
        
        if (s1.read()) {
            s4 = 3;
            wait();     // 3
        }
        
        while (true) {
            c++;
            wait();     // 4
            s4 = c;
        }
    }
    
    sc_signal<sc_uint<4>> s5[3];
    void after_multi2() 
    {
        sc_biguint<3> d = 2;
        for (int i = 0; i < 3; i++) {
            s5[i] = 0;
        }
        wait();         // 0

        for (int i = 0; i < 3; i++) {
            s5[i] = s1.read();
            wait();
        }
        
        while (true) {
            d++;
            s5[0] = d;
            wait();     
            s5[2] = s5[1];
            s5[1] = d;
        }
    }
    
    // While with wait() after reset
    sc_signal<int> s6;
    void after_multi3() 
    {
        int i = 0;
        wait();   

        while (!s5[s4.read()].read()) {
            i++;
            wait();
        }
        s6 = i;
        
        while (true) {
            s6 = i+1;
            wait();
        }
    }
    
    sc_signal<int> s7;
    void after_multi4() 
    {
        int i = 0;
        wait();   

        while (s6.read()) {
            if (s4.read()) break;
            i++;
            wait();
        }
        s7 = i;
        
        while (true) {
            s7 = i+1;
            wait();
        }
    }
};

int sc_main(int argc, char *argv[])
{
    sc_clock clk{"clk", 10, SC_NS};
    top top_inst{"top_inst"};
    top_inst.clk(clk);
    sc_start(100, SC_NS);
    return 0;
}

