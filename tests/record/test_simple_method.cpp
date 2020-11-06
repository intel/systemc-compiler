/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

using namespace sc_core;

// Record (structure/class) non-module tests
template <unsigned N>
class A : public sc_module {
public:
    sc_signal<bool> dummy{"dummy"};

    SC_CTOR(A) {
        //SC_METHOD(base_record);  // partially work, need to support base class ctor
        //sensitive << dummy;
        
        SC_METHOD(record_concat1);  
        sensitive << dummy;
        
        SC_METHOD(record_concat2);  
        sensitive << dummy;
        
        SC_METHOD(record_local_var1);  
        sensitive << dummy;
        
        SC_METHOD(record_local_var2);  
        sensitive << dummy;
        
        // Inner records not supported yet
//        SC_METHOD(inner_record1);  
//        sensitive << dummy;
//        
//        SC_METHOD(inner_record2);  
//        sensitive << dummy;
//        
//        SC_METHOD(inner_record3);  
//        sensitive << dummy;
    }
    
    struct ScSimple {
        sc_uint<2> a;
        sc_uint<3> b;
    };
    
    ScSimple scRec;
    
    void record_concat1() 
    {
        sc_uint<5> c = (scRec.a, scRec.b);
    }
    
    void record_concat2() 
    {
        ScSimple scRecLoc;
        sc_uint<5> c = (scRecLoc.a, scRecLoc.b);
    }
    
//-----------------------------------------------------------------------------

    struct BaseRec {
        int a;
        int b = 2;
        int c;
        
        BaseRec(int a_) : a(a_) { 
            c = a + b;
        }
    };

    struct InheritRec : BaseRec {
        int d = 4;
        int e;
         
        InheritRec() : BaseRec(1) {
            e = 5;
        }
    };

    void base_record() {
        InheritRec ir;
    }

//-----------------------------------------------------------------------------
    
    struct Rec1 {
        int x;
        sc_int<2> y;
        
        Rec1() : y(1) {
            x = 1;
        }
    };
    
    // Local variables structure/class type
    void record_local_var1() 
    {
        Rec1 r;
        r.x = r.y + 2;
    }
    
    
    struct Rec2 {
        int x;
        sc_int<2> y;
        sc_uint<8> z = 3;
        int t{4};
        int tt{5};
        int s;
        
        Rec2(int x_, int y_) : x(x_) {
            y = y_-1;
        }
        
        Rec2() {
            x = 1;
        }
    };
    
    int f(int i) {return (i+1);}
    
    // Local variables structure/class type
    void record_local_var2() 
    {
        Rec2 c(2,3);
        c.x = 4;
        sct_assert_const(c.x == 4);
        sct_assert_unknown(c.s);
        sct_assert_defined(c.x);
        
        int a = f(1);
    }

//-----------------------------------------------------------------------------
    // Inner structure
    
    struct Inner {
        sc_uint<3> x = 1;
        
        Inner() {}
    };
    
    struct Outer {
        sc_uint<4> y;
        Inner r;
        
        Outer() {
            y = 2;
        }
    };
    
    void inner_record1() 
    {
        Outer o;
        o.y = 3;
        o.r.x = o.y + 4;
        sc_uint<3> a = o.r.x;
        sct_assert_const(o.y == 3);
        sct_assert_const(o.r.x == 7);

        sct_assert_read(o.y);
        sct_assert_defined(o.y);
        sct_assert_read(o.r.x);
        sct_assert_defined(o.r.x);
    }
  
    void inner_record2() 
    {
        Outer o1;
        Outer o2;
        
        o1.r.x = 5;
        o2.r.x = o1.r.x; 

        sct_assert_const(o1.r.x == 5);
        sct_assert_read(o1.r.x);
        sct_assert_defined(o1.r.x);
        sct_assert_defined(o2.r.x);
    }
        
    // --------------------
    
    struct Inner2 {
        sc_uint<3> x = 1;
        
        Inner2(sc_uint<3> x_) : x(x_) {}
        Inner2() {}
    };
    
    struct Outer2 {
        Inner2 r1;
        // Inner2 r1{2};  -- not supported yet
        Inner2 r2;
        
        Outer2() : r1(2) {
            r2.x = 4;
        }
    };
    
    void inner_record3() 
    {
        Outer2 o1;
        Outer2 o2;
        o1.r1.x = 5;
        o2.r2.x = o1.r1.x;

        sct_assert_const(o1.r1.x == 5);
        sct_assert_read(o1.r1.x);
        sct_assert_defined(o1.r1.x);
        sct_assert_defined(o2.r2.x);
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

