/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include <systemc.h>

// Various reset: empty, common wait()
class top : sc_module
{
public:
    sc_in_clk       clk;
    sc_signal<bool> arstn;
    
    sc_signal<bool> a;

    
    SC_HAS_PROCESS(top);
    top(sc_module_name)
    {
        SC_CTHREAD(sct_assert_test, clk.neg());
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
    
// ----------------------------------------------------------------------------

    bool c;
    sc_signal<int> t0;
    void sct_assert_test()
    {
        c = false;
        SCT_ASSERT_THREAD(a, SCT_TIME(1), c, clk.neg());
        t0 = c;
        wait();

        while (true) {
            wait();
        }
    }
    
    // Variable used in reset only
    sc_signal<int> t1;
    void var_in_reset_only1() 
    {
        int ii;
        sc_int<8> jj = 1;
        const sc_uint<16> C = 42;
        ii = jj + C;
        t1 = ii;
        
        wait();
        
        while (true) {
            wait();

            if (a) {
                int kk = 1;
                t1 = kk;
            }
        }
    }
    
// ----------------------------------------------------------------------------
    
    // Common wait() for reset and main loop
    sc_signal<int> t1a;
    void common_wait1() 
    {
        while (true) {
            wait();

            if (a) {
                int kk = 1;
                t1a = kk;
            }
        }
    }
    
    sc_signal<sc_uint<3>> s1;
    sc_signal<int> t2;
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
            t2 = y;
        }
    }
    
    sc_signal<bool> s2[3];
    sc_signal<int> t3;
    void common_wait3() 
    {
        sc_uint<3> x;       // Reset local variable
        sc_uint<3> y;       // Reg
        for (int i = 0; i < 3; i++) {
            s2[i] = 0;
        }
        t3 = s2[0];
        
        while (true) {
            wait();         // 0

            if (a) {
                s2[s1.read()] = 1;
                y++;
                wait();     // 1
            }
            t3 = y;
        }
    }
    
    sc_signal<int> t4;
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
            t4 = x;
            
            wait();         // 2
        }
    }
    
    // Conditional wait() after reset, after reset can start with state 0 or 1
    sc_signal<sc_uint<3>> s3;
    void common_wait5() 
    {
        s3 = 0;
        
        while (true) {
            wait();                     // 0
            while (!s1.read()) wait();  // 1
            s3 = 1;
        }
    }
    
    
//    void common_wait5() 
//    {
//        while (true) {
//            wait();                     // 1
//            s3 = 1;
//        }
//    }
    
// ----------------------------------------------------------------------------
    // No reset section, created with and w/o reset signal

    sc_signal<int> t5;
    void no_reset1() 
    {
        while (true) {
            int jj = 42;
            if (jj) {
                int kk = 43;
                t5 = kk;
            }
            wait();
        }
    }
    
    // Additional code for reset section, cannot be created w/o reset signal
    sc_signal<int> s10;
    void no_reset2() 
    {
        int ll = 0; 
        while (true) {
            s10 = ll;
            wait();
        }
    }
    
    // Multi-state process, cannot be created w/o reset signal
    sc_signal<int> s10a;
    void no_reset3() 
    {
        while (true) {
            sc_uint<3> v = 0;
            wait();
            
            v += s3.read();
            s10a = v;
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
            wait();
            
            int w = g(s3.read());
            sc_uint<3> z = w+1;
            s4 = z;
            wait();
        }
    }

    sc_signal<sc_uint<3>> s5;
    void no_reset5() 
    {
        while (true) {
            wait();                         // 0
            s5 = s1.read() + 1;
            sc_uint<3> z = s3.read();
            
            while (!s1.read()) wait();      // 1
            
            s5 = z; 
        }
    }
    
// ----------------------------------------------------------------------------

    // Function call in reset
    int f() {
        const bool A = 1;
        bool b = 2;
        return (A ^ b);
    }
    
    void f1(int& par) {
        int l = par+1;
        par = l;
    }

    sc_signal<int> s7;
    void var_fcall_in_reset_only() 
    {
        int i = f();
        f1(i);
        s7 = i;
        wait();
        
        while(true) 
        {
            wait();
        }
    }
    
// ----------------------------------------------------------------------------

    // Combinational variables in reset
    sc_signal<int> t6;
    void comb_init_in_reset() 
    {
        sc_uint<4> i;
        int j;
        int k = 1;
        a = k;
        t6 = a;
        
        wait();
        
        while (true) {
            i = 1;
            j = 1;
            k = 1;
            t6 = k;
            
            wait();
        }
    }
    
    sc_signal<int> t7;
    void not_used() 
    {
        int jj;
        sc_uint<4> kk;
        wait();

        while (true) {
            t7 = kk;
            wait();
        }
    }
    
    sc_signal<int> t8;
    void comb_assign_in_reset() 
    {
        int i;
        int j; j = 1;
        int k; k = 2;
        int l = 0;
        l -= 1;
        sc_uint<2> x;
        x += 1;
        t8 = x + l;
        
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
    sc_signal<int> t8a;
    void reg_init_in_reset() 
    {
        int i;
        int j = 1;
        int k = 2;
        t8a = k;
        
        wait();
        
        while (true) {
            t8a = i;
            t8a = j;
            t8a = k;
            
            wait();
        }
    }

    int arr0[3];
    sc_signal<int> t9;
    sc_signal<int> t8b;
    void reg_assign_in_reset() 
    {
        int i;
        int j; j = 1;
        int k; k = 2;
        t8b = k;
        int l = 0;
        sc_uint<2> x;
        x = 1;
        
        arr0[1] = 0;
        arr0[2] = 1;
        t9 = arr0[2] + x;
        
        wait();
        
        while (true) {
            t8b = i;
            t8b = j;
            t8b = k;
            t8b = l;
            t8b = x;
            t8b = arr0[1];
            
            wait();
        }
    }
    
    sc_signal<int> t10;
    int aa;
    void read_only_in_reset() 
    {
        aa = 1;
        int ii;
        int jj = 0; 
        int aaa = jj + aa;
        t10 = aaa;
        
        wait();
        
        while (true) {
            wait();
        }
    }
    
    sc_signal<int> t11;
    void write_only_in_reset() 
    {
        int i = 0;
        int j = 1; 
        t11 = 0;
        
        wait();
        
        while (true) {
            wait();
        }
    }

// ----------------------------------------------------------------------------

    // Array elements initialization, array cannot be combinational variable yet
    sc_int<2>           arr1[3];
    sc_signal<bool>     arr2[3];
    int arr4[2];
    
    sc_signal<int> t12;
    void array_init_in_reset() 
    {
        arr1[0] = 1;
        arr2[1] = true;
        int arr3[2] = {1, 2};
        
        for (int i = 0; i < 2; ++i) {
            arr4[i] = arr3[i];
        }
        
        wait();
        
        while (true) {
            // Partial defined not used to determined it as comb variable
            arr1[0] = 1; arr1[s7.read()] = 2; 
            t12 = arr1[0];
            
            t12 = arr2[1] + arr4[s7.read()];
            
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

