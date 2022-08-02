/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"
#include "sct_sel_type.h"
#include <iostream>
#include <cassert>

using namespace sct;

// Stable/rose//fell assertions and SVA generation test
template <unsigned N>
class A : public sc_module
{
public:
    sc_in<bool>         clk{"clk"};
    sc_in<bool>         rstn{"rstn"};
    
    sc_signal<int>      s;
    sc_signal<int>      _s;
    sc_signal<int>      s_d;
    sc_signal<int>      s_d2;
    sc_signal<int>      s_d3;
    sc_signal<int>*     ps;
    
    sc_signal<bool>         st;
    sc_signal<sc_uint<4>>   st1;
    sc_signal<bool>         st2;
    sc_signal<bool>         st_enbl;
    sc_signal<bool>         st_enbl_d;
    sc_signal<sc_uint<4>>   cntr;
    
    SC_HAS_PROCESS(A);
    
    explicit A(const sc_module_name& name)
    {
        ps = new sc_signal<int>("ps");
        
        SC_CTHREAD(test_thread, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(integer_thread, clk.pos());
        async_reset_signal_is(rstn, false);
    }

    SCT_ASSERT_STABLE(rstn, (0), st.read(), clk.pos());
    SCT_ASSERT_STABLE(st_enbl, (1), st1.read(), clk.pos());
    SCT_ASSERT_ROSE(st_enbl && !st_enbl_d, (1), st2.read(), clk.pos());
    SCT_ASSERT_FELL(cntr.read() == 1, (0), s.read(), clk.pos());
    SCT_ASSERT_FELL(cntr.read() == 2, (1), s.read(), clk.pos());
    SCT_ASSERT_ROSE(cntr.read() == 2, (0), s.read(), clk.pos());

    SCT_ASSERT_STABLE(cntr.read() == 8, (0), st1.read(), clk.pos());
    SCT_ASSERT_STABLE(cntr.read() == 7, (1), st1.read(), clk.pos());
    SCT_ASSERT_STABLE(cntr.read() == 8, (0, 1), st1.read(), clk.pos());
    SCT_ASSERT_STABLE(cntr.read() == 7, (1, 2), st1.read(), clk.pos());
    
    SCT_ASSERT_STABLE(cntr.read() == 7, (1, 4), st1.read(), clk.pos());
    SCT_ASSERT_STABLE(cntr.read() == 15, (1, 2), st1.read(), clk.pos());
    SCT_ASSERT_STABLE(cntr.read() == 15, (0, 2), st1.read(), clk.pos());
    
    SCT_ASSERT_STABLE(cntr.read() > 7 && cntr.read() < 15, (0, 2), st1.read(), clk.pos());
    SCT_ASSERT_STABLE(cntr.read() > 7 && cntr.read() < 15, (1, 3), st1.read(), clk.pos());
  
    
    // Provide test signals
    void test_thread() 
    {
        s = 0; s_d = 0; s_d2 = 0; s_d3 = 0; *ps = 0;
        st = 1; st1 = 0; st2 = 0;
        sc_uint<4> cntr_ = 0;
        st_enbl = 0;
        st_enbl_d = 0;
        wait();

        while (true) {
            s_d = s; s_d2 = s_d; s_d3 = s_d2;
            *ps = s;
            s = !s;
            
            //sct_assert (s, "ERR");
            //cout << cntr.read() << " " << s.read() << endl;
            
            if (cntr_ >= 7 || cntr_ == 0) st1 = 1; else st1 = s.read();
            if (st_enbl) st2 = 1; else st2 = 0;
            
            st_enbl = cntr.read() >= 7; 
            st_enbl_d = st_enbl;
            cntr = cntr_;
            cntr_++;
            //cout << "." << flush;
            wait();
        }
    }
    
    sc_signal<sc_int<12>>   i0;
    sc_signal<sc_int<12>>   i1;
    sc_signal<sc_uint<12>>  i2;
    sc_signal<sct_int<12>>  i3;
    sc_signal<sct_uint<12>> i4;
    sc_signal<sct_uint<77>> i5;
    sc_signal<sc_biguint<77>> i6;
    sc_signal<unsigned>     i7;
    sc_signal<long>         i8;
    sc_signal<int64_t>      i9;
    
    SCT_ASSERT_STABLE(rstn, (1), i0.read(), clk.pos());
    SCT_ASSERT_STABLE(i2.read(), (1), i5.read(), clk.pos());
    SCT_ASSERT_STABLE(rstn && i7.read() < 18, (1,2), i8.read(), clk.pos());
    SCT_ASSERT_ROSE(rstn, (1), i1.read(), clk.pos());        
    SCT_ASSERT_ROSE(i1.read(), (1), i2.read(), clk.pos());        
    SCT_ASSERT_ROSE(i2.read(), (1), i6.read(), clk.pos());
    SCT_ASSERT_ROSE(i2.read(), (1), i9.read(), clk.pos());        
    SCT_ASSERT_FELL(i2.read(), (0), i3.read(), clk.pos());        
    SCT_ASSERT_FELL(i2.read(), (1), i4.read(), clk.pos());        

    void integer_thread() 
    {
        i0 = 10;
        i1 = 0; i2 = 0; i3 = 1000; i4 = 1000; i5 = 0;
        i6 = 0; i7 = 0; i8 = 12; i9 = 0;
        wait();

        while (true) {
            
            i1 = i1.read() + 1; i2 = i2.read() + 1;
            i3 = i3.read() - 1; i4 = i4.read() - 1; 
            i5 = 42; i6 = i6.read() + 1;
            i7 = i7.read() + 1; 
            i8 = i7.read() < 20 ? (long)12 : (long)(i7.read() + 1);
            i9 = i9.read() + 1;
            
            wait();
        }
    }    
};

class Test_top : public sc_module
{
public:
    sc_clock clk{"clock", 10, SC_NS};
    sc_signal<bool> rstn;

    A<2> a_mod{"a_mod"};

    SC_CTOR(Test_top) {
        a_mod.clk(clk);
        a_mod.rstn(rstn);

        SC_CTHREAD(testProc, clk);
    }

    void testProc() {
    	rstn = 0;
        wait();
    	rstn = 1;
    	wait(10);
        
        cout << endl;
        cout << "--------------------------------" << endl;
        cout << "|       Test passed OK         |" << endl;
        cout << "--------------------------------" << endl;
        sc_stop();
    }
};

int sc_main(int argc, char* argv[])
{
    Test_top test_top{"test_top"};
    sc_start();
    return 0;
}

