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

// Various reset: empty, common wait(), multi-wait()
class top : sc_module
{
public:
    sc_in_clk       clk;
    sc_signal<bool> arstn;
    
    sc_signal<bool> a;

    
    SC_HAS_PROCESS(top);
    top(sc_module_name)
    {
        SC_CTHREAD(incr_in_reset, clk.pos());
        async_reset_signal_is(arstn, false);

        SC_CTHREAD(sct_assert_test, clk.pos());
        async_reset_signal_is(arstn, false);

        SC_CTHREAD(var_in_reset_only1, clk.pos());
        async_reset_signal_is(arstn, false);

        SC_CTHREAD(common_wait1, clk.pos());
        async_reset_signal_is(arstn, false);

        SC_CTHREAD(common_wait2, clk.pos());
        async_reset_signal_is(arstn, false);

        SC_CTHREAD(common_wait3, clk.pos());
        async_reset_signal_is(arstn, false);

        SC_CTHREAD(common_wait4, clk.pos());
        async_reset_signal_is(arstn, false);

        SC_CTHREAD(common_wait5, clk.pos());
        async_reset_signal_is(arstn, false);


        SC_CTHREAD(no_reset1, clk.pos());
        async_reset_signal_is(arstn, false);

        SC_CTHREAD(no_reset1, clk.pos());
        // No reset here

        SC_CTHREAD(no_reset2, clk.pos());
        async_reset_signal_is(arstn, false);

        SC_CTHREAD(no_reset3, clk.pos());
        async_reset_signal_is(arstn, false);

        SC_CTHREAD(no_reset4, clk.pos());
        async_reset_signal_is(arstn, false);

        SC_CTHREAD(no_reset5, clk.pos());
        async_reset_signal_is(arstn, false);
        
        
        SC_CTHREAD(var_fcall_in_reset_only, clk.pos());
        async_reset_signal_is(arstn, false);
        
        SC_CTHREAD(comb_init_in_reset, clk.pos());
        async_reset_signal_is(arstn, false);
        
        SC_CTHREAD(not_used, clk.pos());
        async_reset_signal_is(arstn, false);
        
        SC_CTHREAD(comb_assign_in_reset, clk.pos());
        async_reset_signal_is(arstn, false);

        SC_CTHREAD(reg_init_in_reset, clk.pos());
        async_reset_signal_is(arstn, false);
        
        SC_CTHREAD(reg_assign_in_reset, clk.pos());
        async_reset_signal_is(arstn, false);

        SC_CTHREAD(read_only_in_reset, clk.pos());
        async_reset_signal_is(arstn, false);

        SC_CTHREAD(write_only_in_reset, clk.pos());
        async_reset_signal_is(arstn, false);
        
        SC_CTHREAD(array_init_in_reset, clk.pos());
        async_reset_signal_is(arstn, false);
    }
    
    // Increment/decrement in reset
    void incr_in_reset() 
    {
        const int C = 42;
        int i = 1;

        // Warning generated 
        int j = i;
        int k = a.read();  // "=" instead of "<=" for local variable
        
        // Warning generated 
        i++;
        --j;
        k += 1;
        
        // Warning generated 
        i = j + 1;
        j = 2*a.read();
        k = j ? i : i + 1;  // "=" instead of "<=" for local variable
        
        // No warnings
        i = C ? 1 : 2;
        j = C;
        k = 1 - C;
        
        wait();
        
        while (true) {
            a = i + j;
            wait();
        }
    }
    
// ----------------------------------------------------------------------------

    bool c;
    void sct_assert_test()
    {
        c = false;
        SCT_ASSERT(a, SCT_TIME(1), c);
        wait();

        while (true) {
            wait();
        }
    }
    
    // Variable used in reset only
    void var_in_reset_only1() 
    {
        int ii;
        sc_int<8> jj = 1;
        const sc_uint<16> C = 42;
        ii = jj + C;
        
        wait();
        
        while (true) {
            wait();

            if (a) {
                int kk = 1;
            }
        }
    }
    
// ----------------------------------------------------------------------------
    
    // Common wait() for reset and main loop
    void common_wait1() 
    {
        while (true) {
            wait();

            if (a) {
                int kk = 1;
            }
        }
    }
    
    sc_signal<sc_uint<3>> s1;
    void common_wait2() 
    {
        sc_uint<3> x;       // Reset local variable
        sc_uint<3> y;       // Reg
        s1 = 0;
        
        while (true) {
            wait();

            if (a) {
                s1 = 1;
                y++;
            }
        }
    }
    
    sc_signal<bool> s2[3];
    void common_wait3() 
    {
        sc_uint<3> x;       // Reset local variable
        sc_uint<3> y;       // Reg
        for (int i = 0; i < 3; i++) {
            s2[i] = 0;
        }
        
        while (true) {
            wait();         // 0

            if (a) {
                s2[s1.read()] = 1;
                y++;
                wait();     // 1
            }
        }
    }
    
    void common_wait4() 
    {
        sc_uint<3> x;       // Reg
        sc_uint<3> y;       // Write only
        x = 1;
        
        while (true) {
            wait();         // 0 
            y = x + 1;
            x = s2[x];
            
            while (!s1.read()) wait();  // 1
            x++;
            
            wait();         // 2
        }
    }
    
    // Conditional wait() after reset, after reset can start with state 0 or 1
    sc_signal<sc_uint<3>> s3;
    void common_wait5() 
    {
        sc_uint<3> x;       // Reg
        x = 1;
        s3 = 0;
        
        while (true) {
            while (!s1.read()) wait();  // 0
            x = x + s1.read();
            wait();         // 1
            
            s3 = x;
        }
    }
    
    
// ----------------------------------------------------------------------------
    // No reset section, created with and w/o reset signal

    void no_reset1() 
    {
        while (true) {
            int jj = 42;
            if (a) {
                int kk = 43;
            }
            wait();
        }
    }
    
    // Additional code for reset section, cannot be created w/o reset signal
    void no_reset2() 
    {
        int ll = 0; 
        while (true) {
            ll++;
            wait();
        }
    }
    
    // Multi-state process, cannot be created w/o reset signal
    void no_reset3() 
    {
        while (true) {
            sc_uint<3> v = s3.read();
            wait();
            
            v++;
            wait();
        }
    }
    
    int g(int val) {
        return (val+1);
    }
    
    sc_signal<sc_uint<3>> s4;
    void no_reset4() 
    {
        while (true) {
            s4 = 0;
            int w = g(s3.read());
            wait();
            
            sc_uint<3> z = w+1;
            s4 = z;
            wait();
        }
    }

    sc_signal<sc_uint<3>> s5;
    void no_reset5() 
    {
        while (true) {
            s5 = s1.read() + 1;
            sc_uint<3> z = s3.read();
            
            while (!s1.read()) wait();      // 0
            
            s5 = z; 
            wait();                         // 1
        }
    }
    
// ----------------------------------------------------------------------------

    // Function call in reset
    int f() {
        const bool A = a.read();
        bool b = a.read();
        return (A ^ b);
    }

    void var_fcall_in_reset_only() 
    {
        int i = f();
        wait();
        
        while(true) 
        {
            wait();
        }
    }
    
// ----------------------------------------------------------------------------

    // Combinational variables in reset
    void comb_init_in_reset() 
    {
        sc_uint<4> i;
        int j;
        int k = 1;
        a = k;
        
        wait();
        
        while (true) {
            i = 1;
            j = 1;
            k = 1;
            
            wait();
        }
    }
    
    void not_used() 
    {
        int jj;
        sc_uint<4> kk;
        wait();

        while (true) {
            wait();
        }
    }
    
    void comb_assign_in_reset() 
    {
        int i;
        int j; j = 1;
        int k; k = 2;
        a = k;
        int l = 0;
        l -= 1;
        sc_uint<2> x;
        x += 1;
        
        wait();
        
        while (true) {
            i = 1;
            j = 1;
            k = 1;
            l = 1;
            x = 1;
            
            wait();
        }
    }
    
    // Register variables in reset
    void reg_init_in_reset() 
    {
        int i;
        int j = 1;
        int k = 2;
        a = k;
        
        wait();
        
        while (true) {
            a = i;
            a = j;
            a = k;
            
            wait();
        }
    }

    int arr0[3];
    void reg_assign_in_reset() 
    {
        int i;
        int j; j = 1;
        int k; k = 2;
        a = k;
        int l = 0;
        l -= 1;
        sc_uint<2> x;
        x += 1;
        
        arr0[1] = 0;
        arr0[1] += 1;
        
        wait();
        
        while (true) {
            a = i;
            a = j;
            a = k;
            a = l;
            a = x;
            a = arr0[1];
            
            wait();
        }
    }
    
    int aa;
    void read_only_in_reset() 
    {
        aa = 1;
        int ii;
        int jj = 0; 
        int aaa = jj + aa;
        
        wait();
        
        while (true) {
            wait();
        }
    }
    
    void write_only_in_reset() 
    {
        int i = 0;
        int j = 1; 
        
        wait();
        
        while (true) {
            wait();
        }
    }

// ----------------------------------------------------------------------------

    // Array elements initialization, array cannot be combinational variable yet
    sc_int<2>           arr1[3];
    sc_signal<bool>     arr2[3];
    
    void array_init_in_reset() 
    {
        arr1[0] = 1;
        arr2[1] = true;
        int arr3[2] = {1, 2};
        
        wait();
        
        while (true) {
            // Partial defined not used to determined it as comb variable
            arr1[0] = 1; arr1[1] = 2; 
            a = arr1[0];
            
            a = arr2[1];
            a = arr3[0];
            
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

