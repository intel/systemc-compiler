/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

struct Simple {
    int         a;
    sc_uint<3>  b;
    
    Simple() = default;
    Simple(sc_uint<3> par) : b(par) {a = 42;}

    Simple& operator=(const Simple& other)
    {a = other.a; b = other.b; return *this;}
    inline friend bool operator==(const Simple& lhs, const Simple& rhs)
    {return (lhs.a == rhs.a && lhs.b == rhs.b);}
    inline friend std::ostream& operator<< (std::ostream& os, const Simple &obj)
    {return os;}
    inline friend void sc_trace(sc_trace_file*& f, const Simple& val, std::string name) 
    {}
};

struct FArray {
    static const unsigned A = 12;
    bool        a[2];
    sc_uint<3>  b[3];
    
    FArray() = default;
    FArray(sc_uint<3> par) {
        a[0] = par;
        a[1] = par+1;
    }

    FArray& operator=(const FArray& other) {
        a[0] = other.a[0]; a[1] = other.a[1];
        b[0] = other.b[0]; b[1] = other.b[1]; b[2] = other.b[2];
        return *this;
    }
    inline friend bool operator==(const FArray& lhs, const FArray& rhs) {
        return (lhs.a[0] == rhs.a[0] && lhs.a[1] == rhs.a[1] &&
                lhs.b[0] == rhs.b[0] && lhs.b[1] == rhs.b[1] && lhs.b[2] == rhs.b[2]);
    }
    inline friend std::ostream& operator<< (std::ostream& os, const FArray &obj)
    {return os;}
    inline friend void sc_trace(sc_trace_file*& f, const FArray& val, std::string name) 
    {}
};
    
// Check unused variables/statements remove for records
struct A : public sc_module 
{
    sc_in<bool>         clk;
    sc_signal<bool>     nrst;
    sc_signal<sc_uint<4>> s;
    sc_signal<Simple> r;

    sc_signal<Simple>  t0;
    sc_signal<Simple>  t1;
    sc_signal<Simple>  t2;
    sc_signal<int>  t3;
    sc_signal<int>  t4;
    sc_signal<int>  t5;
    sc_signal<FArray>  t6;
    sc_signal<FArray>  t7;
    sc_signal<int>  t8;
    
    SC_HAS_PROCESS(A);

    A(const sc_module_name& name) : 
        sc_module(name) {
        
        SC_METHOD(remove_local); sensitive << s << r;
        SC_METHOD(remove_member); sensitive << s << r;
        SC_METHOD(remove_array); sensitive << s << r;
        SC_METHOD(remove_field_array); sensitive << s << r << t6;

        SC_CTHREAD(remove_local_thread, clk.pos());
        async_reset_signal_is(nrst, 0);
        SC_CTHREAD(remove_member_thread, clk.pos());
        async_reset_signal_is(nrst, 0);
        SC_CTHREAD(remove_array_thread, clk.pos());
        async_reset_signal_is(nrst, 0);
        SC_CTHREAD(remove_field_array_thread, clk.pos());
        async_reset_signal_is(nrst, 0);
    }
    
    void remove_local() {
        Simple r1;              // not removed
        Simple r2(s.read());    // not removed
        Simple r3{s.read()};    // not removed
        
        Simple r4;              // not removed
        t0 = r4;
        
        Simple r5 = r.read();   // not removed    
        
        Simple r6;              // not removed    
        Simple r7;              // not removed    
        r6 = r7;                // not removed    
        Simple r8 = r7;
    }
    
    void remove_local_thread() 
    {
        Simple r0;              // not removed
        wait();

        while (true) {
            Simple r1;              // not removed
            Simple r2(s.read());    // not removed
            Simple r3{s.read()};    // not removed

            Simple r4;              // not removed
            t0 = r4;

            Simple r5 = r.read();   // not removed    

            Simple r6;              // not removed    
            Simple r7;              // not removed    
            r6 = r7;                // not removed    
            Simple r8 = r7;
            wait();
        }
    }
    
    Simple m1;                  // removed
    Simple m2;                  // not removed    
    Simple m2t;                  // not removed    
    Simple m3;                  // not removed    
    Simple m3t;                  // not removed    
    Simple m4;                  // not removed    
    Simple m4t;                  // not removed    
    Simple m5;                  // removed  
    Simple m5t;                  // not removed    
    Simple m6;                  // not removed    
    Simple m6t;                  // not removed    
    Simple m7;                  // not removed    
    Simple m7t;                  // not removed    
    Simple m8;                  // not removed    
    Simple m8t;                  // not removed    
    Simple m9;                  // not removed    
    Simple m9t;                  // not removed    
    Simple m10;                 // not removed    
    Simple m10t;                // not removed    
    
    void remove_member() {
        Simple r1;
        m2 = r1;
        r1 = m3;
        
        m4.a = 42;
        auto l1 = m5.b;
        auto l2 = m6.a;
        t3 = l2;
        
        m7 = m8;
        
        Simple r2 = m9;
        t3 = r2.a;
        t1 = m10;
    }
    
    void remove_member_thread() 
    {
        Simple r1;
        m2t = r1;
        r1 = m3t;
        wait();

        while (true) {
            m4t.a = 42;
            auto l1 = m5t.b;
            auto l2 = m6t.a;
            t3 = l2;

            m7t = m8t;

            Simple r2 = m9t;
            t3 = r2.a;
            t1 = m10t;
            wait();
        }
    }
    
   
    Simple ma0[2];              // not removed    
    Simple ma1[1][2];           // removed

    Simple ma2[2];              // not removed    
    Simple ma3[2];              // not removed    
    Simple ma4[2][2];           // not removed
    Simple ma5[2][2];           // not removed
    Simple ma6[2][2];           // not removed
    Simple ma7[2];              // not removed    
    Simple ma8[1][2];           // not removed
    Simple ma9[1][2];           // not removed
    
    void remove_array() {
        Simple a;
        Simple a0[2];           // not removed    
        Simple a1[2][3];        // not removed    
        ma0[s.read()] = a;
        a1[0][0] = a;
        
        Simple a2[2];           // not removed    
        Simple a3[2][3];        // not removed    
        
        a2[0] = ma2[0];         // not removed
        t4 = a2[0].a;

        a3[s.read()][0] = ma3[1];                       // not removed  
        a3[1][s.read()] = ma4[s.read()][r.read().b];    // not removed

        auto l1 = ma5[s.read()][1];
        auto l2 = ma6[1][s.read()];
        t4 = l2.a;
        
        auto l3 = ma7[s.read()];
        Simple l4; l4.a = ma8[0][s.read()].a;
        Simple l5; l5.a = ma9[s.read()][s.read()].a;
        t4 = l5.a;
    }
    
    Simple ma0t[2];              // not removed    
    Simple ma1t[1][2];           // removed

    Simple ma2t[2];              // not removed    
    Simple ma3t[2];              // not removed    
    Simple ma4t[2][2];           // not removed
    Simple ma5t[2][2];           // not removed
    Simple ma6t[2][2];           // not removed
    Simple ma7t[2];              // not removed    
    Simple ma8t[1][2];           // not removed
    Simple ma9t[1][2];           // not removed
    
    void remove_array_thread() 
    {
        Simple a;
        Simple a0[2];           // not removed    
        Simple a1[2][3];        // not removed    
        wait();

        while (true) {
            ma0[s.read()] = a;
            a1[0][0] = a;

            Simple a2[2];           // not removed    
            Simple a3[2][3];        // not removed    

            a2[0] = ma2t[0];         // not removed
            t4 = a2[0].a;

            a3[s.read()][0] = ma3t[1];                       // not removed  
            a3[1][s.read()] = ma4t[s.read()][s.read()];      // not removed

            auto l1 = ma5t[s.read()][1];
            auto l2 = ma6t[1][s.read()];
            t4 = l2.a;

            auto l3 = ma7t[s.read()];
            Simple l4; l4.a = ma8t[0][s.read()].a;
            Simple l5; l5.a = ma9t[s.read()][s.read()].a;
            t4 = l5.a;
            wait();
        }
    }

    
    
    FArray fm1;                 // removed
    FArray fm2[2];              // removed
    FArray fm3;                 // not removed
    FArray fm4;                 // not removed
    FArray fm5[2];              // not removed
    FArray fm6[2];              // not removed
    FArray fm7;                 // not removed
    FArray fm8[2];              // not removed
    
    void remove_field_array() {
        FArray f1;              // not removed
        FArray f2[2];           // not removed
        
        fm3 = t6.read();
        t7 = fm4;
        fm5[s.read()] = t6.read();
        t7 = fm6[s.read()];
        
        t8 = fm7.b[s.read()];
        t8 = fm8[s.read()].a[s.read()];
    }
    
    
    FArray fm1t;                 // removed
    FArray fm2t[2];              // removed
    FArray fm3t;                 // not removed
    FArray fm4t;                 // not removed
    FArray fm5t[2];              // not removed
    FArray fm6t[2];              // not removed
    FArray fm7t;                 // not removed
    FArray fm8t[2];              // not removed

    void remove_field_array_thread() 
    {
        FArray f1;              // not removed
        FArray f2[2];           // not removed
        wait();

        while (true) {
            fm3t = t6.read();
            t7 = fm4t;
            fm5t[s.read()] = t6.read();
            t7 = fm6t[s.read()];

            t8 = fm7t.b[s.read()];
            t8 = fm8t[s.read()].a[s.read()];
            wait();
        }
    }

};

int sc_main(int argc, char *argv[]) 
{
    sc_clock clk{"clk", 1, SC_NS};
    A a_mod{"a_mod"};
    a_mod.clk(clk);

    sc_start();
    return 0;
}

