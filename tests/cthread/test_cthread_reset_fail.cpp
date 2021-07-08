/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include <systemc.h>

// Register variable modified and used in reset -- error/warning reported
class top : sc_module
{
public:
    sc_in_clk       clk;
    sc_signal<bool> arstn;
    
    int* p;
    int m;
    int* q = &m;
    int n;
    int& r = n;
    
    SC_HAS_PROCESS(top);
    top(sc_module_name)
    {
        p = sc_new<int>();
        pp = sc_new<int>();

        SC_CTHREAD(read_modify_nonreg, clk.pos());
        async_reset_signal_is(arstn, false);

        SC_CTHREAD(read_modify_reg_in_reset, clk.pos());
        async_reset_signal_is(arstn, false);
        
        SC_CTHREAD(read_modify_arr_in_reset, clk.pos());
        async_reset_signal_is(arstn, false);
    }
    
    // Check no errors for member used as combinational variables
    int mm;
    int* pp;
    sc_signal<int> s10;
    void read_modify_nonreg() 
    {
        mm = 0;
        mm++;   
        *pp = 42;
        (*pp) += 1;
        s10 = mm + *pp;
        
        wait();
        
        while (true) {
            s10 = 0;
            wait();
        }
    }
    
    sc_signal<int> s0;
    sc_signal<int> s1;
    sc_signal<int> s2;
    
    void read_modify_reg_in_reset() 
    {
        int j = 1;
        j++;
        j += 2;
        s0 = j;
        
        int i = 1;
        i++;            // Error
        i += 2;         // Error
        
        sc_uint<2> x;
        x++;            // Error
        x += 2;         // Error

        // No warnings
        i = 2;
        m = 42;
        *p = 43;
        n = 44;
        
        if (i) {s1 = 0;} // @i is evaluated, no warning
        
        for (int k = 0; k < i; ++k) { // Warning
            s0 = k;
        }

        int& ri = i;    // Warning
        s1 = ri;        // Warning 
        s1 = i;         // Warning
        s1 = 1+i;       // Warning
        s1 = true || i; 
        s1 = false || i;  // Warning

        s1 = *q;        // Warning
        s1 = n;         // Warning
        s1 = r;         // Warning
        
        int* lp = p;    // Warning
        s1 = *lp;       // Warning
        s1 = *p;        // Warning
        
        wait();
        
        while (true) {
            s2 = i + *q + *p + r + x;
            wait();
        }
    }
    
    int marr[3];
    
    sc_signal<int> s3;
    void read_modify_arr_in_reset() 
    {
        int arr1[3] = {1,2,3};
        arr1[0]++;
        
        int arr2[3] = {1,2};
        arr2[2] = arr1[0];
        arr2[0] = 42;
        
        arr2[0]++;      // Error
        arr2[0] += 1;   // Error    
        
        marr[0] = 42;
        marr[1] = 42 + arr1[0];
        
        marr[0]++;      // Error
        marr[0] += 1;   // Error    
        
        s3 = arr2[1];   // Warning
        s3 = marr[0];   // Warning
        
        wait();
        
        while (true) {
            s3 = arr2[s0.read()] + marr[s1.read()];
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

