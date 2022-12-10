/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

using namespace sc_core;

// Function with wait() call in multiple states  
class A : public sc_module {
public:
    sc_in<bool>     clk;
    sc_signal<bool> nrst{"nrst"};
    sc_signal<sc_uint<4>> s{"s"};

    SC_CTOR(A) 
    {   
        SC_CTHREAD(multi1, clk.pos());   
        async_reset_signal_is(nrst, 0);
        
        SC_CTHREAD(multi2, clk.pos());   
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(multi3, clk.pos());   
        async_reset_signal_is(nrst, 0);
        
        SC_CTHREAD(multi2, clk.pos());   
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(multi3, clk.pos());   
        async_reset_signal_is(nrst, 0);
    }
    
    // Function with wait()
    int cref_wait1(const int& par) {
        int res;
        wait();                             // 1, 3
        res = par+1;
        return res;
    }

    void multi1()
    {
        int i = 0;
        wait(); 
        
        while (true) {
            int arra[3];
            if (s.read()) {
                int res = cref_wait1(arra[2]);   // 1
            }
            wait();                              // 2

            if (s.read() < 3) {
                i = s.read();
                int arrb[3];
                cref_wait1(arrb[i]);             // 3
            }
        }
    }
    
    int cref_wait2(const int& par) {
        wait();                             // 1, 3
        return par;
    }

    void multi2()
    {
        int a, b;
        wait(); 
        
        while (true) {                              // B6
            if (s.read() == 1) {                    // B5
                cref_wait2(a);             // 1     // B4
            }
            if (s.read() == 2) {                    
                cref_wait2(b);             // 2     // B2
            }
            wait();                        // 3     // B3
        }
    }
    
    
    int cref_wait3(int& par) {
        wait();                             // 1, 3
        return par;
    }

    void multi3()
    {
        int a, b;
        wait(); 
        
        while (true) {                              // B6
            if (s.read() == 1) {                    // B5
                cref_wait3(a);             // 1     // B4
            }
            wait();                        // 2     // B3

            if (s.read() == 2) {                    
                cref_wait3(b);             // 3     // B2
            }
        }                                           // B1
    }
    
 
// ----------------------------------------------------------------------------    
    // BUG: index of referenced array modified, #182
    void multi4()
    {
        wait(); 
        
        while (true) {
            int cwrr[3];
            cref_wait2(cwrr[s.read()]);  // Incorrect code generated, see #182
            wait();                         
        }
    }
    
    // BUG: index of referenced array modified, #182
    void multi5()
    {
        bool arrc[3];
        int j = s.read();
        wait(); 
        
        while (true) {
            bool& b = arrc[j];          // Incorrect code generated, see #182
            wait();                         
            j = 1;
            b = 2;
        }
    }
};

int sc_main(int argc, char *argv[]) 
{
    sc_clock clk("clk", 1, SC_NS);
    A a_mod{"a_mod"};
    a_mod.clk(clk);
    
    sc_start();
    return 0;
}

