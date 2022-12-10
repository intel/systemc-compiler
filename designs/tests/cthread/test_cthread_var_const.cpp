/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>
#include "sct_assert.h"

// Member and local constant variables declaration as registers/read only
class top : sc_module {
public:
    sc_in_clk       clk{"clk"};
    sc_signal<bool> rstn{"rstn", 1};
    sc_signal<int>  in{"in"};
    sc_signal<int>  out{"out"};
    
    sc_signal<sc_uint<3>> s;
    static const unsigned A = 2;
    const unsigned B = 3;

    SC_HAS_PROCESS(top);
    top(sc_module_name)
    {
        SC_METHOD(const_init_meth); sensitive << s;
        
        SC_CTHREAD(const_init_thread, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(local_rnd1, clk.pos());
        async_reset_signal_is(rstn, false);
        
        SC_CTHREAD(local_rnd2, clk.pos());
        async_reset_signal_is(rstn, false);
        
        SC_CTHREAD(local_rnd3, clk.pos());
        async_reset_signal_is(rstn, false);
        
        SC_CTHREAD(local_def_read1, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(local_def_read2, clk.pos());
        async_reset_signal_is(rstn, false);
    }

    sc_signal<int> r0;
    void const_init_meth() {
        if (s.read()) {
            int i = s.read();
            const unsigned M1 = i+42;
            r0 = M1;
        }
        const sc_uint<16> M2;

        r0 = M2 + A + B;
    }
   
    sc_signal<int> r1;
    void const_init_thread() {
        if (s.read()) {
            int i = 41;
            const unsigned R1 = i+1;
            r1 = R1;
        }
        const unsigned R2 = 42;
        r1 = R2 + B;
        
        wait();             
        
        while (true) {
            const unsigned N1 = 43;
            const unsigned R1 = 44;
            wait();
            r1 = N1 + R2 + R1 + A + B;
        }
    }

// ---------------------------------------------------------------------------
    
    // Constant not defined before read in some state
    void local_rnd1() {
        int i = 0;
        const int C = in.read();
        wait();             
        
        while (true) {
            const sc_uint<3> D = s.read();
            wait();         
            if (C) i = D;
        }
    }
    
    sc_signal<sc_uint<3>> s2;
    void local_rnd2() {
        int i = 0;
        const int ARR[3] = {1, 2, in.read()};
        wait();             
        
        while (true) {
            const sc_uint<3> G = ARR[2];
            wait();
            
            if (s.read()) i = ARR[s.read()];
            wait();
            
            s2 = G + i;
        }
    }

    // Constant in local scopes
    sc_signal<sc_uint<3>> s3;
    void local_rnd3() {
        int i = 0;
        if (A) {
            const sc_uint<4> Z = in.read();
            wait();
            i = Z + 1;
        }
        wait();             
        
        while (true) {
            if (s.read()) {
                const sc_int<4> Y = i;
                wait();                     // 2
                
                for (int j = 0; j < 3; ++j) {
                    const sc_biguint<4> X = Y+j;
                    wait();                 // 3
                    s3 = X;
                }
            }
            wait();                         // 4
        }
    }
    
    // Constant defined before read in all states
    void local_def_read1() {
        const int E = in.read();
        int i = E;
        wait();             
        
        while (true) {
            const sc_uint<3> F = s.read();
            i = F;
            wait();         
        }
    }
    
    sc_signal<sc_uint<3>> s4;
    void local_def_read2() {
        const sc_uint<3> ARRA[3] = {1, 2, 3};
        const sc_uint<3> K = ARRA[0];
        wait();
        
        while (true) {
            // Reg as accessed at unknown index
            const int ARRB[3] = {in.read()+1, in.read()+2, in.read()+3};
            const sc_uint<3> L = ARRB[K+s.read()];
            wait();
            s4 = L;
        }
    }
};

int sc_main(int argc, char *argv[])
{
    sc_clock clk{"clk", 10, SC_NS};
    top top_inst{"top_inst"};
    top_inst.clk(clk);
    sc_start();
    return 0;
}

