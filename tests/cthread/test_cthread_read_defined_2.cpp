/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"
#include <sct_assert.h>

using namespace sc_core;

// Variables and variable arrays defined and read tests
class A : public sc_module
{
public:
    sc_in_clk         clk{"clk"};
    sc_signal<bool>   nrst{"nrst"};

    sc_signal<sc_uint<4>> s;
    
    sc_uint<4> a;
    bool b;
    sc_int<16> c;
    sc_biguint<42> d;
    sc_bigint<72> e;
    
    sc_uint<4> arra[4];
    bool arrb[8][8];
    long int arrc[2][1];
    unsigned arrd[7];
    int arre[2];
    
    SC_CTOR(A)
    {
        SC_CTHREAD(var1, clk.pos());
        async_reset_signal_is(nrst, false);

        SC_CTHREAD(arr1, clk.pos());
        async_reset_signal_is(nrst, false);

        SC_METHOD(for_stmt1); sensitive << s;
        SC_METHOD(while_stmt1); sensitive << s;
    }

    void var1()
    {
        a = 1;
        if (s.read()) {
            c = a;
        } 
        sct_assert_defined(a);
        sct_assert_read(a);
        sct_assert_array_defined(c);
        wait();
        
        while (1) {
            int l = d.to_int() + a;
            sct_assert_defined(l);
            sct_assert_defined(l);
            sct_assert_register(a);
            
            while (l < e) {
                sct_assert_read(e);
                sct_assert_read(l);
                wait();
            }
            
            wait();
        }
    }

    void arr1()
    {
        for (int i = 0; i < 4; i++) {
            arra[i] = i;
            arrd[i] = 0;
        }
        sct_assert_array_defined(arra);
        sct_assert_array_defined(arrd);
        wait();
        
        while (1) {
            if (arra[1] == s.read()) {
                arrb[s.read()][s.read()+1] = arra[2];
            }
            sct_assert_array_defined(arrb);
            sct_assert_read(arra);

            wait();
            
            int i = arra[s.read()];
            do {
                i--;
                arrc[i+1][0] =arrb[i][i] ? s.read().to_int() : 1;
            } while (i > 0 && arrc[i][0]);
            sct_assert_array_defined(arrc);
            sct_assert_read(arrb);
            wait();
            
            arre[0] = 1; 
            sct_assert_array_defined(arre);
        }
    }

    void for_stmt1()
    {
        int ii, jj;
        int lo = 1, hi = 2;
        for (int i = lo; i < hi; i++) {
            int k = ii;
            k = jj + 1;
        }
        sct_assert_read(hi);
        sct_assert_read(lo);
        sct_assert_read(ii);
    }
    
    void while_stmt1()
    {
        int lo = 1, hi = 2;
        int i = lo;
        while (i < hi) {
            i++;
        }
        sct_assert_read(i);
        sct_assert_defined(i);
        sct_assert_read(hi);
        sct_assert_read(lo);
    }
};

int sc_main(int argc, char* argv[])
{
    sc_clock clk{"clk", 1, SC_NS};
    A a_mod{"a_mod"};
    a_mod.clk(clk);
    
    sc_start();
    return 0;
}

