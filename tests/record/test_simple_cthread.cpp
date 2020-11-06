/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

using namespace sc_core;

// Record member (structure/class, non-module) accessed from CTHREAD
class A : public sc_module {
public:
    sc_in_clk clk{"clk"};
    sc_signal<bool> rst;
    sc_signal<sc_uint<2>> sig;

    SC_CTOR(A) 
    {
        SC_CTHREAD(record_in_thread1, clk.pos());
        async_reset_signal_is(rst, true);
        
        SC_CTHREAD(record_in_thread2, clk.pos());
        async_reset_signal_is(rst, true);

        SC_CTHREAD(record_in_thread3, clk.pos());
        async_reset_signal_is(rst, true);

        SC_CTHREAD(record_in_thread3a, clk.pos());
        async_reset_signal_is(rst, true);

        SC_CTHREAD(record_in_thread4, clk.pos());
        async_reset_signal_is(rst, true);

        SC_CTHREAD(record_in_thread_call, clk.pos());
        async_reset_signal_is(rst, true);
        
        SC_CTHREAD(record_in_thread_call2, clk.pos());
        async_reset_signal_is(rst, true);
        
//        SC_METHOD(record_in_method1);
//        sensitive << sig;
//        
//        SC_METHOD(record_in_method2);
//        sensitive << sig;
//
//        SC_METHOD(array_in_method1);
//        sensitive << sig;
    }
    
    struct Simple {
        bool a;

        void setA(bool par) {
            a = par;
        }

        bool getA() {
            return a;
        }
        
        bool localVar(bool par) {
            bool l;
            l = par || a;
            return l;
        }
    };
    
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

    // Access member record in CTHREAD
    Simple s[3];
    void record_in_thread1()
    {
        s[1].a = false;
        wait();
        
        while (true) {
            s[1].a = true;
            sct_assert_const(s[1].a);
            sct_assert_array_defined(s[1].a);
            
            bool b = s[2].a;
            sct_assert_unknown(b);
            sct_assert_read(s[2].a);
            wait();
        }
    }

    Simple ss[2][3];
    void record_in_thread2()
    {
        ss[1][1].a = false;
        wait();
        
        while (true) {
            ss[1][2].a = true;
            sct_assert_const(ss[1][2].a);
            sct_assert_array_defined(ss[1][2].a);

            bool b = ss[1][0].a;
            sct_assert_unknown(b);
            sct_assert_read(ss[1][0].a);
            wait();
        }
    }

    // Array of records with inner record 
    Outer oarr[3];
    void record_in_thread3()
    {
        oarr[1].b = false;
        oarr[1].r.a = false;
        wait();
        
        while (true) 
        {
            oarr[1].b = true;
            oarr[2].r.a = true;
            bool c = oarr[2].b || oarr[1].r.a;
            wait();
        }
    }
    
    // Array of records with inner record non-determinable index access 
    Outer oarr2[2];
    void record_in_thread3a()
    {
        for (int i = 0; i < 2; i++) {
            oarr2[i].b = false;
            oarr2[i].r.a = true;
        }
        sct_assert_const(!oarr2[0].b);
        sct_assert_const(oarr2[0].r.a);
        sct_assert_const(!oarr2[1].b);
        sct_assert_const(oarr2[1].r.a);
        wait();
        
        while (true)
        {
            sc_uint<2> j = sig.read();
            oarr2[j].b = 1;
            oarr2[j].r.a = 0;
            bool d = oarr2[j+1].b + oarr2[j-1].r.a;
            
            sct_assert_read(oarr2[0].b);
            sct_assert_register(oarr2[0].b);
            sct_assert_read(oarr2[0].r.a);
            sct_assert_register(oarr2[0].r.a);
            
            wait();
        }
    }
    
    // Record with array of inner records 
    OuterArr orec;
    void record_in_thread4()
    {
        orec.b = true;
        orec.r[0].a = false;
        wait();
        
        while (true) 
        {
            sc_uint<2> j = sig.read();
            bool c; 
            c = orec.r[j].a || j;
            orec.r[j].a = c && orec.b;
            wait();
        }
    }
    
    Simple t[3];
    void record_in_thread_call()
    {
        wait();
        
        while (true) 
        {
            sc_uint<2> j = sig.read();
            bool b = t[j].getA();
            t[j+1].setA(t[1].getA());
            
            sct_assert_read(t[0].a);
            sct_assert_array_defined(t[0].a);
            wait();
        }
    }
    
    Simple tt[3][2];
    void record_in_thread_call2()
    {
        tt[2][1].setA(false);
        wait();
        
        while (true) 
        {
            bool b = 1;
            sc_uint<2> j = sig.read();
            for (int i = 0; i < 2; i++) {
                b = b ^ tt[j][i].getA();
            }
            tt[j+1][0].setA(tt[1][1].getA());
            wait();
        }
    }

/*    void array_in_method1()
    {
        i[0] = 1; i[1] = 2;
        int c = i[0];
    }
    
    // Record array in method
    Simple s[3];
    void record_in_method1()
    {
        s[1].a = true;
        bool b = s[2].a;
    }
    
    // Record 2D array in method
    void record_in_method2()
    {
        ss[1][2].a = true;
        //bool b = ss[2][3].a;
    }
*/
};

int sc_main(int argc, char *argv[]) {
    sc_clock clk("clk", 1, SC_NS);
    A a{"a"};
    a.clk(clk);
    sc_start();
    return 0;
}
 
