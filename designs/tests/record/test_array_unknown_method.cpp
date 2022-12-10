/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"
#include "sct_assert.h"

using namespace sc_core;

// Record array access at non-determinable index leads to all elements unknown
class A : public sc_module {
public:
    
    sc_signal<bool>  dummy{"dummy"};
    sc_signal<int>   sig{"sig"};
    
    SC_CTOR(A) 
    {
        SC_METHOD(simple_array1); 
        sensitive << sig;

        SC_METHOD(record_array1); 
        sensitive << sig;

        SC_METHOD(record_array2); 
        sensitive << sig;
        
        // Inner records
        SC_METHOD(inner_record_array1); 
        sensitive << sig;
        
        SC_METHOD(inner_record_array2); 
        sensitive << sig;
    }
    
    struct Simple {
        bool a;
        int b;
    };
    
    
//-----------------------------------------------------------------------------
    // Simple array and record array
    bool b[3];
    void simple_array1() 
    {
        b[1] = true;
        sct_assert_unknown (b[0]);
        sct_assert (b[1]);
        sct_assert_unknown (b[2]);
        
        int i = sig.read();
        b[i] = false;
        sct_assert_unknown (b[1]);
    }
    
    Simple r[3];
    void record_array1() 
    {
        r[1].a = true;
        sct_assert_const (r[1].a);
        
        r[0].a = false;
        sct_assert_const (r[1].a);
        sct_assert_const (!r[0].a);
        
        int i = sig.read();
        r[i].a = false;
        sct_assert_unknown (r[1].a);
    }

    Simple rr[3][2];
    void record_array2() 
    {
        rr[0][0].a = true;
        rr[1][1].a = true;
        sct_assert_const (rr[0][0].a);
        sct_assert_const (rr[1][1].a);
        
        rr[0][1].b = 42;
        rr[2][0].b = 43;
        rr[2][1].b = 44;
        sct_assert_const (rr[0][1].b == 42);
        sct_assert_const (rr[2][0].b == 43);
        sct_assert_const (rr[2][1].b == 44);
        
        int i = sig.read();
        
        rr[i][0].a = false;
        sct_assert_unknown (rr[0][0].a);
        sct_assert_unknown (rr[1][1].a);
        sct_assert_const (rr[0][1].b == 42);
        sct_assert_const (rr[2][0].b == 43);
        
        rr[0][i].b = 0;
        sct_assert_unknown (rr[0][1].b);
        sct_assert_const (rr[2][0].b == 43);
        sct_assert_const (rr[2][1].b == 44);
    }
    
//-----------------------------------------------------------------------------
    // Inner record array
    struct Inner {
        sc_uint<3> x = 1;
        
        Inner() {}
    };
    
    struct Outer {
        sc_uint<4> y;
        Inner r;
        
        Outer() {
        }
    };   
    
    struct OuterArr {
        bool b;
        Inner r[3];
    };
    
    // Array of records with inner record 
    Outer o[3];
    void inner_record_array1() 
    {
        o[1].y = 3;
        o[2].r.x = 4;
        sct_assert_const (o[1].y == 3);
        sct_assert_const (o[2].r.x == 4);
        
        int i = sig.read();
        sct_assert_unknown (o[i].y);
        sct_assert_unknown (o[i].r.x);
        
        o[i].y = 0;
        sct_assert_unknown (o[1].y);
        sct_assert_const (o[2].r.x == 4);

        o[i].r.x = 0;
        sct_assert_unknown (o[1].y);
        sct_assert_unknown (o[2].r.x);
        
        i = o[i].r.x;
    }
    
    // Record with array of inner records 
    OuterArr orec;
    void inner_record_array2()
    {
        orec.b = true;
        orec.r[0].x = 1;
        sct_assert_const (orec.b);
        sct_assert_const (orec.r[0].x == 1);
        
        orec.r[1].x = 2;
        sct_assert_const (orec.b);
        sct_assert_const (orec.r[0].x == 1);
        sct_assert_const (orec.r[1].x == 2);
        
        int i = sig.read();
        orec.r[i].x = i;
        sct_assert_const (orec.b);
        sct_assert_unknown (orec.r[0].x);
        sct_assert_unknown (orec.r[1].x);
        sct_assert_unknown (orec.r[2].x);
        
        i = orec.r[i].x;
    }    
};

class B_top : public sc_module {
public:
    A a_mod{"a_mod"};

    SC_CTOR(B_top) {
    }
};

int sc_main(int argc, char *argv[]) {
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}

