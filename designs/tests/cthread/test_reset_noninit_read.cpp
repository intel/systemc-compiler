/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"
#include <sct_assert.h>

using namespace sc_core;

// Read not initialized variables/arrays in reset -- error/warning reported
class A : public sc_module
{
public:
    sc_in<bool>         clk{"clk"};
    sc_signal<bool>     nrst{"nrst"};
    
    int* p;
    int m = 11;
    int* q = &m;
    int n = 12;
    int& r = n;
    int mm = 13;
    
    SC_CTOR(A)
    {
        p = sc_new<int>();

        SC_CTHREAD(readProc1, clk.pos());
        async_reset_signal_is(nrst, false);

        SC_CTHREAD(readProc2, clk.pos());
        async_reset_signal_is(nrst, false);
    }
    
    sc_signal<int>     s{"s"};
    
    void f1(int par_a[3]) {
        auto ii = par_a[1];
    }
    void f2(int (&par_a)[3], int i) {
        auto ii = par_a[i];
    }
    void f3(int par_a[3][3], int i) {
        auto ii = par_a[i+1][i+2];
    }

    sc_signal<int>     s0;
    void readProc1() 
    {
        int aa[3];
        int bb[3] = {0,1,2};
        int cc[3][3];
        int dd[3];

        f1(aa);             // Warning
        f2(bb, 1);
        f3(cc, 0);          // Warning
        int i = dd[1];      // Warning
        int j;
        int& r = j;          
        i = r;              // Warning
        int l;
        i = l + 1;          // Warning

        s0 = i;             
        
        wait();
        
        while(1) {
            wait();
        }
    }
    
    sc_signal<int>     s1;
    void readProc2() 
    {
        int i;
        i = *p;             // Warning
        i = mm;             
        int j = *q;         
        j = r;              

        s1 = i + j;     
        
        wait();
        
        while(1) {
            s1 = mm + 1;    
            wait();
        }
    }
};

int sc_main(int argc, char* argv[])
{
    sc_clock clk("clk", 1, SC_NS);
    A a_mod{"a_mod"};
    a_mod.clk(clk);
    sc_start();
    return 0;
}