/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

using namespace sc_core;

// Record (structure/class) non-module simple and loop counter name conflict tests
template <unsigned N>
class A : public sc_module {
public:
    sc_in<bool>         clk;
    sc_signal<bool>     rst;
    sc_signal<bool>     dummy;

    SC_CTOR(A) 
    {
        SC_METHOD(record_const); 
        sensitive << dummy << s << sa[0] << sa[1] << st1 << st2;
        
        // Check constant in member record is considered as readOnly/useVals
        SC_METHOD(record_decl1);  sensitive << dummy;
        SC_CTHREAD(record_decl2, clk.pos());  
        async_reset_signal_is(rst, 0);
        
        SC_METHOD(record_local_var1);  
        sensitive << dummy;
        
        SC_METHOD(record_local_var2);  
        sensitive << dummy;
        
        SC_METHOD(record_loop_conflict1);
        sensitive << dummy;
        
        SC_METHOD(record_loop_conflict2);
        sensitive << dummy;

        SC_METHOD(record_loop_conflict3);
        sensitive << dummy;

        // Base record not supported yet
        //SC_METHOD(base_record);  // partially work, need to support base class ctor
        //sensitive << dummy;
        
        // Inner records not supported yet
        //SC_METHOD(inner_record1);  
        //sensitive << dummy;
        
        //SC_METHOD(inner_record2);  
        //sensitive << dummy;
        
        //SC_METHOD(inner_record3);  
        //sensitive << dummy;
    }
    
    struct ScSimple {
        const sc_uint<2> A = 1; 
        static const unsigned B = 2;
        sc_uint<2> a;
        sc_uint<3> b;
        
        ScSimple& operator=(const ScSimple& other)
        {a = other.a; b = other.b; return *this;}
        inline friend bool operator==(const ScSimple& lhs, const ScSimple& rhs)
        {return (lhs.a == rhs.a && lhs.b == rhs.b);}
        inline friend std::ostream& operator<< (std::ostream& os, const ScSimple &obj)
        {return os;}
        inline friend void sc_trace(sc_trace_file*& f, const ScSimple& val, std::string name) 
        {}
    };

    template <unsigned BB>
    struct ScTempl {
        //const unsigned A = 1;   -- not allowed for record in channel, error reported
        static const unsigned B = BB;
        sc_uint<2> a;

        // Required for constant field @A
        ScTempl<BB>& operator=(const ScTempl<BB>& other)
        {a = other.a; return *this;}
        
        inline friend bool operator==(const ScTempl<BB>& lhs, const ScTempl<BB>& rhs)
        {return lhs.a == rhs.a;}
        inline friend std::ostream& operator<< (std::ostream& os, const ScTempl<BB> &obj)
        {return os;}
        inline friend void sc_trace(sc_trace_file*& f, const ScTempl<BB>& val, std::string name) 
        {}
    };

    ScSimple scRec;
    ScSimple scRec_;
    ScSimple scRecArr[2];
    
    sc_signal<unsigned> s;
    sc_signal<ScTempl<1>> st1;
    sc_signal<ScTempl<2>> st2;
    sc_signal<ScTempl<3>> sa[2];
    void record_const() {
        int i;
    
        i = st1.read().B;
        i = st2.read().B;
        i = sa[0].read().B + sa[1].read().B;
        
        i = scRec.A + scRec.B + scRec.a;
        i = ScSimple::B + scRecArr[0].a;
        i = scRecArr[0].a + ScSimple::B +  scRecArr[1].b;
        i = scRecArr[0].a + scRecArr[0].B +  scRecArr[1].b;
        i = ScSimple::B + scRecArr[1].A + scRecArr[0].a + scRecArr[1].b;
        i = scRecArr[1].A + scRecArr[0].a + ScSimple::B + scRecArr[0].b;
        i = scRecArr[1].A + scRecArr[0].a + scRecArr[1].B + scRecArr[0].b;
        
        i = scRecArr[s.read()].a + scRecArr[s.read()+1].A;
        i = scRecArr[s.read()].b + scRecArr[s.read()-1].B;
        i = scRec_.B + scRecArr[0].B;
        
        int m[3];
        m[0] = ScSimple::B + scRecArr[0].B + scRecArr[0].A;
    }

//-----------------------------------------------------------------------------

    // Check constant in member record is considered as useVals
    ScSimple scRec1;
    void record_decl1() 
    {
        const int L = scRec1.A;
        sc_uint<5> c = (scRec1.a, scRec1.b);
    }
    
    // Check constant in member record is considered as readOnly
    ScSimple scRec2;
    void record_decl2() 
    {
         wait();
         while (true) {
            const int LL = scRec2.A;
            ScSimple scRecLoc;
            wait();
         }
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
    // Record array access in loop, check name conflict (#244 -- fixed)
    
    struct Rec3 {
        int x[3];
        
        int loop() {
            int res = 0;
            for (int i = 0; i < 3; i++) {  
                res += x[i];
            }
            return res;
        }
    };

    Rec3 rr[3];
    Rec3 rrr[3];
    int i_1;
    
    void ff1() {
        for (int i = 0; i < 3; i++) {  
            int a = i;
        }
    }
    
    void ff2() {
        for (int i = 0; i < 3; i++) {  
            int a = rrr[i].loop();
        }
    }

    void record_loop_conflict1() 
    {
        Rec3 r[3];

        r[0].loop();
        
        ff1();

        for (int i = 0; i < 3; i++) {  
            int i_2 = r[i].loop();
        }
    }
    
    void record_loop_conflict2() 
    {
        int i;
        
        for (int i = 0; i < 3; i++) {  
            int a = rr[i].loop();
        }
        
        i_1 = 0;
        for (int i = 0; i < 3; i++) {  
            i_1 += rr[i].loop();
        }
        int i_3 = i_1;
    }
    
    void record_loop_conflict3() 
    {
        for (int i = 0; i < 3; i++) {  
            int i_2 = rrr[i].loop();
        }

        ff2();
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


int sc_main(int argc, char *argv[]) 
{
    sc_clock clk("clk", 1, SC_NS);
    A<1> a_mod{"a_mod"};
    a_mod.clk(clk);
    
    sc_start();
    return 0;
}

