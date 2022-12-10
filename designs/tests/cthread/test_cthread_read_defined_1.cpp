/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"
#include <sct_assert.h>

using namespace sc_core;

// Defined and read in IF/? with complex condition (&&/||)
class A : public sc_module
{
public:
    sc_in<bool>         clk{"clk"};
    sc_in<bool>         nrst{"nrst"};
    sc_in<bool>         a{"a"};
    sc_out<bool>        b{"b"};

    sc_signal<sc_uint<4>> s;
    
    SC_CTOR(A)
    {
        SC_METHOD(if_complex_stmt1); sensitive << a;
        SC_METHOD(if_complex_stmt2); sensitive << a;
        SC_METHOD(if_complex_stmt3); sensitive << a;
        
        SC_METHOD(if_complex_const1); sensitive << a;
        SC_METHOD(if_complex_const2); sensitive << a;
        SC_METHOD(if_complex_const3); sensitive << a;
        SC_METHOD(if_complex_const4); sensitive << a;

        SC_METHOD(if_complex_par1); sensitive << a;
        SC_METHOD(if_complex_par2); sensitive << a;
        
        SC_METHOD(if_complex_assign1); sensitive << a;
        SC_METHOD(if_complex_assign2); sensitive << a;

        SC_METHOD(cond_oper1); sensitive << s;
        SC_METHOD(cond_oper2); sensitive << s;
        SC_METHOD(cond_oper3); sensitive << s;
        SC_METHOD(cond_oper4); sensitive << s;
        SC_METHOD(cond_oper5); sensitive << s;
    }

    // complex IF condition
    void if_complex_stmt1()
    {
        int i, j;
        i = a;
        if (i || j) {
        }
        sct_assert_read(i);
        sct_assert_read(j);
    }
    
    void if_complex_stmt2()
    {
        int i, j;
        j++;
        if (i || j) {
        }
        sct_assert_read(i);
        sct_assert_read(j);
    }
    
    void if_complex_stmt3()
    {
        int i, j;
        if (i || bool(j)) {
        }
        sct_assert_read(i);
        sct_assert_read(j);
    }
    
    void if_complex_const1()
    {
        int i, j;
        if (0 || i || j) {
        }
        sct_assert_read(i);
        sct_assert_read(j);
    }
    
    void if_complex_const2()
    {
        int i, j;
        if (1 || i || j) {
        }
        sct_assert_read(i, false);
        sct_assert_read(j, false);
    }

    void if_complex_const3()
    {
        int i, j;
        if (1 && i || j) {
        }
        sct_assert_read(i);
        sct_assert_read(j);
    }

    void if_complex_const4()
    {
        int i, j;
        if (0 && i || j && 0) {
        }
        sct_assert_read(i, false);
        sct_assert_read(j);
    }

    void if_complex_par1()
    {
        int i, j;
        if (i || (j)) {
        }
        sct_assert_read(i);
        sct_assert_read(j);
    }
    
    void if_complex_par2()
    {
        int i, j, k;
        if (i || (j || (k))) {
        }
        sct_assert_read(i);
        sct_assert_read(j);
        sct_assert_read(k);
    }
    
    void if_complex_assign1()
    {
        int i, j;
        if (i || (j == a)) {
        }
        sct_assert_read(i);
    }
    
    void if_complex_assign2()
    {
        int i, j;
        if (i || (j == 1)) {
        }
        sct_assert_read(i);
    }
    
// ---------------------------------------------------------------------------    
    
    void cond_oper1()
    {
        int i, j;
        int c = i == 1 ? j : 0;
        sct_assert_read(i);
        sct_assert_read(j);
    }
    
    void cond_oper2()
    {
        int i, j, k;
        i = 1;
        int c = i == 1 ? j : k + 1;
        sct_assert_read(i);
        sct_assert_read(j);
        sct_assert_read(k ,false);
    }

    void cond_oper3()
    {
        int i, j, k;
        int c = s.read() && i ? 1 : j + k;
        sct_assert_read(i);
        sct_assert_read(j);
        sct_assert_read(k);
    }

    void cond_oper4()
    {
        int i, j, k;
        int c = 0 && i ? j : k;
        sct_assert_read(i, false);
        sct_assert_read(j, false);
        sct_assert_read(k);
    }

    void cond_oper5()
    {
        int i, j, k;
        int c = 1 && i || 1 || j ? 0 : k;
        sct_assert_read(i);
        sct_assert_read(j, false);
        sct_assert_read(k, false);
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

