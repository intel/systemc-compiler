/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"
#include <iostream>
using namespace sc_core;

// Record (structure/class) non-module tests
template <unsigned N>
class A : public sc_module {
public:
    sc_signal<bool> dummy{"dummy"};

    SC_CTOR(A) {
        //SC_METHOD(base_record);  // partially work, need to support base class ctor
        //sensitive << dummy;
        
        SC_METHOD(record_name_coll);
        sensitive << dummy;
        
        SC_METHOD(record_local_complex);
        sensitive << dummy;
        
        SC_METHOD(record_local_complex2);
        sensitive << dummy;
        
        SC_METHOD(record_fcall1);  
        sensitive << dummy;
                
        SC_METHOD(record_fcall2);  
        sensitive << dummy;
                
        SC_METHOD(record_fcall3);  
        sensitive << dummy;
                
        SC_METHOD(record_in_bracket);  
        sensitive << dummy;
         
        SC_METHOD(record_return1);  
        sensitive << dummy;
        
        SC_METHOD(record_assign1);  
        sensitive << dummy;
        
        // Inner records
        //SC_METHOD(inner_record1_complex);
        //sensitive << dummy;
        
        //SC_METHOD(inner_record2);  
        //sensitive << dummy;
        
        //SC_METHOD(inner_record3);  
        //sensitive << dummy;
    }
    
    
    struct Simple {
        bool a;
        int b;
    };
    
    
//-----------------------------------------------------------------------------
    // Record name collision
    Simple g3() {
       Simple r;
       r.a = true;
       return r;
    }
    
    void record_name_coll()
    {
        Simple r = g3();
        r.a = false;
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
        sc_uint<16> var16;
        sc_uint<1> flag;
        sc_uint<32> var32;

        
        Rec1() : y(1) {
            x = 1;
        }
    };
    
    // Local variables structure/class type
    void record_local_complex()
    {
        Rec1 r1;
        Rec1 r2;
        Rec1 r3;
        r1.x = r1.y + 2;
        r2.x = r1.y + r2.y;
        r1.var32= 0xFFFF; // 0b111
        r2.var32=0xA0A0A0A0;
        r3.x = r1.x | r2.x;
        r3.flag = r2.flag && (r1.flag || r1.var32[0]);

        if (r1.y==0) {
            r2.x=r1.x+r2.y*2;
        }
        for (int i = 0; i<10; i++) {
            r2.var16[i] = r1.var32[i*2];
        }

    }
    
    
    struct Rec2 {
        int x;
        sc_uint<32> y;
        sc_uint<8> z = 3;
        int t{4};
        int tt{5};
        int s;
        
        Rec2(int x_, int y_) : x(x_) {

            if (y_>1) {
                y = y_-1;
            } else {
                if (x_>0){
                    y = y_+x_;
                } else {
                    y=y_+1;
                }
            }
        }
        
        Rec2() : x(1){

        }
    };
    
    int f(int i) {return (i+1);}
    
    // Local variables structure/class type
    void record_local_complex2()
    {
        Rec2 c(2,3);
        Rec2 d(3,1);
        Rec2 e;

        c.x = 4;
        d.x = c.x + e.x;

        sct_assert_const(c.x == 4);
        sct_assert_const(c.y == 2);

        sct_assert_unknown(c.s);
        sct_assert_defined(c.x);
        
        int a = f(1);
    }

//-----------------------------------------------------------------------------
    // Function with records
    void f1(Simple& par) {
        bool b = par.a;
        par.b = 2;
    }
    
    void f1_const(const Simple& par) {
        bool b = !par.a;
    }
    
    void f2(Simple par) {
        bool b = par.a;
        par.b = 2;
    }
    
    bool f3(Simple& par1, Simple& par2) {
        bool b = par1.a;
        par2.a = b;
        return (par2.a || par1.a);
    }
    
    void f3_(Simple par1, Simple par2) {
        //bool b = par1.a;    // par2 instead of par1 !!!!
        //par2.a = b;         // par1 instead of par2 !!!!
    }
    void f3_() {
            //bool b = par1.a;    // par2 instead of par1 !!!!
            //par2.a = b;         // par1 instead of par2 !!!!
        }

    bool f3_2(Simple& par1, Simple& par2) {
        seta(par1,true);
        seta(par2,false);

        return (par1.a | par2.a);
    }

    void record_fcall1() 
    {
        Simple s;
        s.b = 1;
        f1(s);       
        f1_const(s); 
        
        sct_assert_read(s.a);
        sct_assert_defined(s.b);
    }

    void record_fcall2() 
    {
        Simple s;
        s.b = 1;
        f2(s);
    }

    void seta(Simple& op1){
        op1.a=true;
    }
    void seta(Simple& op1, bool tf){
        op1.a = tf;
    }

    bool f3ref(Simple& par1, Simple& par2) {
        bool b = par1.a;
        par2.a = b;
        return (par2.a || par1.a);
    }

    bool f3noref(Simple par1, Simple par2) {
        bool b = par1.a;
        par2.a = b;
        return (par2.a || par1.a);
    }

    Simple rec;

    void record_fcall3() 
    {
        rec.a = 1; // rec_a = 1;
        int rec_a;
        rec_a=5;
        sct_assert_const(rec_a==5);
        Simple s; Simple r;
        Simple s1; Simple s2;

        f3_();
        bool flag;
        bool flag2;
        seta(s1);
        seta(s2);
        //flag = f3noref(s1,s2);
        flag = f3ref(s1,s2);
        sct_assert_const(flag);

        s1.a = 0;

        if (rec.a) {
            int s1;
            int rec_a;
            s1 = 1;
            rec_a = 10;
            sct_assert_const(rec.a == 1);
            sct_assert_const(rec_a == 10);

        }

        sct_assert_const(s1.a==0);

    }
    

//-----------------------------------------------------------------------------
    // Function with record in return

    Simple g1() {
       Simple r;
       r.b = 2;
       return r;
   }
   Simple g2() {
       Simple r;
       r.b = 2;
       return (r);
   }
     
    void record_in_bracket() 
    {
        Simple s;
        (s).b = 1;
    }
    
    void record_return1() 
    {
        Simple s = g1();
        Simple r = g2();
        cout << "s.b = " << s.b << endl;
        cout << "r.b = " << r.b << endl;
        //assert(s.b==2);
        //sct_assert_const(s.b==2);
    }
    
//-----------------------------------------------------------------------------
    // Record assignment in declaration and binary operator
     
    void record_assign1() 
    {
        Simple r;
        Simple s = r;
        r = s;
    }
        
    
//-----------------------------------------------------------------------------
    // Inner structure
    
    struct Inner {
        sc_uint<3> x = 1;
        sc_uint<32> z = 0xF;

        Inner() {}
    };
    
    struct Outer {
        sc_uint<4> y;
        Inner r;
        
        Outer() {
            y = 2;
        }
    };
    
    void inner_record1_complex()
    {
        Outer o;
        Outer o2;
        Outer o3;
        Outer o4;

        o.y = 3;
        o2.y = 0x4;
        o3.y = o.y | 0x4;
        o.r.x = o.y + 4;
        o4.r.x = o.y - 1;

        o2.r.x = o.r.x;
        o3.r.x = o.r.x & o2.r.x ^ o4.r.x;

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

