/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

using namespace sc_core;

// Member(not local) record arrays
template <unsigned N>
class A : public sc_module {
public:
    sc_signal<bool> dummy{"dummy"};

    SC_CTOR(A) 
    {
        SC_METHOD(rec_mod_arr1);  
        sensitive << dummy;

        // Simple inner record works for now 
        SC_METHOD(rec_mod_inner0);
        sensitive << dummy;

        // Inner records 
        SC_METHOD(rec_mod_inner1);  
        sensitive << dummy;

        SC_METHOD(rec_mod_inner2);  
        sensitive << dummy;
    }
   
    struct Simple {
        bool a = true;
        int b = 42;
    };
    
    Simple marr1[2];
    Simple marr2[2][3];
    
    void rec_mod_arr1() 
    {
        marr1[1].a = false;
        marr2[1][2].a = !marr1[1].a;
        int c = marr2[1][0].b + marr1[0].b;
        
        sct_assert_const(!marr1[1].a);
        sct_assert_const(marr2[1][2].a);
        
        sct_assert_array_defined(marr2[1][2].a);
        sct_assert_read(marr2[1][0].b);
    }
    

//---------------------------------------------------------------------------
// Inner structure array    
    struct Inner {
        bool a;
    };
    
    struct Outer {
        bool b;
        Inner r;
    };
    
    struct OuterArr {
        bool b;
        Inner r[3];
    };

    Outer oo;

    void rec_mod_inner0() 
    {
        oo.r.a = true;
        bool b = !oo.r.a; 
    }
    
    Outer oarr[2];
        
    void rec_mod_inner1() 
    {
        oarr[0].b = false;
        oarr[1].r.a = !oarr[0].b;

        sct_assert_const(!oarr[0].b);
        sct_assert_const(oarr[1].r.a);
        
        sct_assert_array_defined(oarr[1].r.a);
        sct_assert_read(oarr[0].b);
    }

    OuterArr orec;
        
    void rec_mod_inner2() 
    {
        orec.b = false;
        orec.r[1].a = !orec.b;

        sct_assert_const(orec.r[1].a);
    }    
};

class B_top : public sc_module {
public:
    A<1> a_mod{"a_mod"};

    SC_CTOR(B_top) {
    }
};

int sc_main(int argc, char *argv[]) {
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}

