/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"
#include <sct_assert.h>

using namespace sc_core;

// Defined and read for pointer/channel arrays and function calls
class A : public sc_module
{
public:
    sc_in<bool>         clk{"clk"};
    sc_in<bool>         nrst{"nrst"};
    sc_in<bool>         a{"a"};
    sc_out<bool>        b{"b"};

    sc_signal<bool>     c{"c"};
    sc_signal<bool>     d{"d"};
    sc_signal<bool>     e{"e"};
    sc_signal<bool>     f{"f"};

    const int           ci = 1;
    int*                pa[3];
    sc_signal<bool>     ca[3];
    sc_signal<bool>*    pca[3];
    sc_signal<bool>*    ppca[3][2];

    sc_signal<bool>*    pc;
    int*                pi;
    
    int                 m;
    int                 k;
    int                 n;

    sc_signal<sc_uint<8>> s;
    
    int* p1;
    sc_int<2>* p2;
    int* p3;
    
    int arr[2];
    int* p4;

    SC_CTOR(A)
    {
        // Dynamically allocated array
        pc = sc_new_array< sc_signal<bool> >(2);
        pi = sc_new_array<int>(2);
        
        for (int i = 0; i < 3; i++) {
            pa[i] = sc_new<int>();
            pca[i] = new sc_signal<bool>("pca");
            for (int j = 0; j < 2; j++) {
                ppca[i][j] = new sc_signal<bool>("ppca");
            }
        }
        
        p1 = sc_new<int>();
        p2 = new sc_int<2>();
        p3 = &m;
        p4 = &arr[0];
        
        SC_METHOD(array_of_pointers); sensitive << a;
        SC_METHOD(array_of_channels); sensitive << a << *pca[0] << ca[0] << *ppca[0][0];

        SC_METHOD(if1); sensitive << a;

        SC_METHOD(latch1); sensitive << a << c << d << e << f;

        SC_CTHREAD(multistate, clk.pos());
        async_reset_signal_is(nrst, false);

        SC_CTHREAD(func_call, clk.pos());
        async_reset_signal_is(nrst, false);
        SC_CTHREAD(func_call_sc_type, clk.pos());
        async_reset_signal_is(nrst, false);
        SC_CTHREAD(func_call_arr, clk.pos());
        async_reset_signal_is(nrst, false);
        SC_CTHREAD(func_call_arr2, clk.pos());
        async_reset_signal_is(nrst, false);

        SC_METHOD(return_value); sensitive << a;
    }
    
    // Global pointer initialization, NOT SUPPORTED YET 
    void glob_pointer_init() {
        int* p = p1;
        *p = 1;
        sct_assert_read(p1, false);
        sct_assert_defined(p1);
        
        sc_int<2>* x;
        x = p2;         // Not supported in GEN yet, bug #51
        bool b = *x > 0;
        sct_assert_read(p2);

        p = p3;         // Not supported in GEN yet, bug #51
        *p = *p;
        sct_assert_read(m);
        sct_assert_defined(m);
    }

    // Local pointer initialization, NOT SUPPORTED YET
    void local_pointer_init() {
        int mm;
        int* p = &mm;   // Not supported in GEN yet, bug #51
        *p = 1;
        sct_assert_read(mm, false);
        sct_assert_defined(mm);

        int kk;
        p = &kk;        // Not supported in GEN yet, bug #51
        int i = *p;
        sct_assert_read(kk);

        p = &m;
        i = *p + 1;
        *p = i;
        sct_assert_read(m);
        sct_assert_defined(m);
    }

    // Bug #37 -- fixed
    void array_of_pointers()
    {
        *(pa[0]) = 1;
        int i = *(pa[1]);
        sct_assert_read(*pa[1]);
    }
    
    // Array of channels
    void array_of_channels()
    {
        int i = ca[1];
        sct_assert_read(ca);

        int j = pca[1]->read();
        sct_assert_read(*pca[1]);

        int k = ppca[1][0]->read();
        sct_assert_read(*ppca[1][0]);
    }
    
    int ret_func() {
        int i = 1;
        return i;
    }
    
    void return_value()
    {
        int j = ret_func();
        
        // i must be in @defined and @read
    }

    int                 m4;
    int                 n4;
    int                 k4;

    void if1()
    {
        if (a.read()) {
            n4 = m4 + k4;  // k, m -> @readndef, n -> @defined
            m4 = k4 - 1;  // m -> @defined

        } else {
            k4 = m4;  // m -> @readndef, k -> @defined
            n4 = 1;  // n -> @defined
        }
        sct_assert_defined(n4);
        sct_assert_defined(m4, false);
        sct_assert_register(k4);
        sct_assert_register(n4, false);
        sct_assert_read(k4);
        sct_assert_read(n4, false);
    }

    void latch1()
    {
        if (a.read()) {
            c = 1;
            d = 1;
            e = f;
        } else {
            d = 2;
        }
        e = 2;
        
        sct_assert_latch(c);
        sct_assert_latch(d, false);
        sct_assert_latch(e, false);
        sct_assert_latch(f, false);
    }

    int global_register;
    int global_comb;
    sc_signal<int> ms_sig{"ms_sig"};
    sc_signal<bool> ms_bsig{"ms_bsig"};
    int ms_array[10];

    void multistate() {
        int ms_loc_comb;
        int ms_loc_reg;
        ms_loc_comb = 0;
        global_register = 0;
        ms_sig = 0;
        ms_loc_reg = 1;
        ms_array[1] = 10;
        wait();
        ms_sig = 0;

        while (1) {
            global_register = global_register + 1;
            global_comb = 1;
            ms_sig = ms_array[2];

            if (ms_bsig) {
                ms_loc_comb = 1;
                ms_loc_reg = 12;
            } else {
                ms_loc_comb = 2;
            }

            ms_sig = ms_loc_comb + ms_loc_reg + global_comb;

            wait();
            wait(10);
        }
    }

    void func_int1(int par) {
    }
    void func_int2(int& par) {
        // @par not read/defined in function
    }
    void func_int_read(int& par) {
        // @par read in function
        bool b = par == 0;
    }
    void func_int_defined(int& par) {
        // @par defined in function
        par = 0;
    }
    void func_sc1(sc_int<2> par) {
    }
    void func_sc2(sc_int<2>& par) {
        // @par not read/defined in function
    }
    void func_sc_read(sc_int<2>& par) {
        // @par read in function
        bool b = par == 0;
    }
    void func_call() {
        int aa;
        func_int1(aa);
        sct_assert_read(aa);

        int bb;
        func_int2(bb);
        sct_assert_read(bb, false);

        int cc;
        func_int_read(cc);
        sct_assert_read(cc);

        int dd;
        func_int_defined(dd);
        sct_assert_defined(dd);
        sct_assert_read(dd, false);
        wait();
        
        while(1) wait();
    }
    
    void func_call_sc_type() 
    {
        sc_int<2> cc;
        func_sc1(cc);
        sct_assert_read(cc);

        sc_int<2> dd;
        func_sc2(dd);
        sct_assert_read(dd, false);

        sc_int<2> ee;
        func_sc_read(ee);
        sct_assert_read(ee);
        wait();
        
        while(1) wait();
    }
    
    // Function calls
    void func_arr1(int par_a[3]) {
    }
    void func_arr2(int (&par_a)[3]) {
    }
    void func_arr3(int par_a[2][2]) {
    }
    void func_call_arr() {
        wait();
        
        while(1) {
            int aa[3];
            int bb[3];
            func_arr1(aa);
            func_arr2(bb);
            sct_assert_read(aa, false);
            sct_assert_read(bb, false);

            int cc[2][2];
            func_arr3(cc);
            sct_assert_read(cc, false);
            
            wait();
        }
    }
    
    sc_signal<int> sig;
    
    void func_arr1a(int par_a[3]) {
        auto ii = par_a[1];
    }
    void func_arr2a(int (&par_a)[3]) {
        auto ii = par_a[sig.read()];
    }
    void func_arr3a(int par_a[2][2]) {
        auto ii = par_a[sig.read()][1];
    }
    void func_call_arr2() {
        wait();
        
        while(1) {
            int aa[3];
            int bb[3] = {0,1,2};
            func_arr1a(aa);
            func_arr2a(bb);
            sct_assert_read(aa, true);
            sct_assert_read(bb, true);

            int cc[2][2];
            func_arr3a(cc);
            sct_assert_read(cc, true);

            wait();
        }
    }
    
};

class B_top : public sc_module
{
    sc_signal<bool> a{"a"};
    sc_signal<bool> b{"b"};
    sc_signal<bool> clk{"clk"};
    sc_signal<bool> nrst{"nrst"};

public:
    A a_mod{"a_mod"};

    SC_CTOR(B_top) {
        a_mod.clk(clk);
        a_mod.nrst(nrst);
        a_mod.a(a);
        a_mod.b(b);
    }
};

int sc_main(int argc, char* argv[])
{
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}

