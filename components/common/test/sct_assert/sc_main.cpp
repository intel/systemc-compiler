/******************************************************************************
 * Copyright (c) 2023, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 *
 *****************************************************************************/

#include "sct_assert.h"
#include "sct_sel_type.h"
#include "systemc.h"

using namespace sct;

// Assertions and SVA generation test
class A : public sc_module 
{
public:

    sc_in<bool>         clk{"clk"};
    sc_in<bool>         rstn{"rstn"};
    
    sc_in<sc_uint<4>>   a{"a"};
    sc_out<sc_uint<4>>  b{"b"};

    sc_signal<int>      s;
    sc_signal<int>      s_d;
    sc_signal<int>      s_d2;
    sc_signal<int>      s_d3;
    
    sc_signal<int>*     ps;

    bool                arr[3];
    sc_signal<bool>     sarr[3];
    sc_signal<bool>     sarr_d[3];
    sc_signal<bool>     sarr2[3][2];
    
    static const size_t N = 4;
    const unsigned long M = 1;
    
    SC_HAS_PROCESS(A);
    
    A(sc_module_name, unsigned m)
    {
        cout << "sct_assert " << endl;
#ifdef __SC_TOOL__
        cout << "__SC_TOOL__ defined" << endl;
#endif
        ps = new sc_signal<int>("ps");
        
        auto& val = const_cast<unsigned long&>(M);
        val = m;
        cout << "M " << M << endl;
        
        SC_CTHREAD(test_thread, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(integer_thread, clk.pos());
        async_reset_signal_is(rstn, false);
        
        SC_CTHREAD(sct_assert_thread1, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(sct_assert_thread2, clk.pos());
        async_reset_signal_is(rstn, false);
        
        SC_CTHREAD(sct_assert_pointer, clk.pos());
        async_reset_signal_is(rstn, false);
        
        SC_CTHREAD(sct_assert_loop, clk.pos());
        async_reset_signal_is(rstn, false);
    }
    
    // Equivalent 4 and 2-arguments assertions
    SCT_ASSERT(!rstn, (0), !s, clk.pos());
    SCT_ASSERT(rstn || !s, clk.pos());
    
    SCT_ASSERT(s_d2, (0), s, clk.pos());
    SCT_ASSERT(!s_d2 || s, clk.pos());

    // 4-arguments assertions
    SCT_ASSERT(s, (1,N), s_d, clk.pos());
    
    SCT_ASSERT(s, SCT_TIME(M+1), s_d, clk.pos());
    SCT_ASSERT(s, SCT_TIME(M+1,M+2), s_d, clk.pos());
    
    SCT_ASSERT(s, (N+3), s_d, clk.pos());
    SCT_ASSERT(s, (1,N+1), s_d, clk.pos());
    
    SCT_ASSERT(s, (1), s_d, clk.pos());
    SCT_ASSERT(s_d, (1), s_d2, clk.pos());
    SCT_ASSERT(a.read() == 1, (1), b.read() == 3, clk.pos());
    SCT_ASSERT(*ps, (1), s_d2, clk.pos());
    SCT_ASSERT((*this ).s, SCT_TIME(0), this->s, clk.pos());

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
    
    SCT_ASSERT_ROSE(cst, (1), cntr.read(), clk.pos());        
    
    sc_signal<int>          st;
    sc_signal<sc_uint<4>>   st1;
    sc_signal<bool>         st2;
    sc_signal<bool>         st_enbl;
    sc_signal<bool>         st_enbl_d;
    sc_signal<sc_uint<4>>   cntr;
    sc_signal<bool>         cst;
    
    // Provide test signals
    void test_thread() 
    {
        s = 0; s_d = 0; s_d2 = 0; s_d3 = 0; *ps = 0;
        st = 1; st1 = 0; st2 = 0;
        sc_uint<4> cntr_ = 0;
        st_enbl = 0;
        st_enbl_d = 0;
        cst = 0;
        wait();

        while (true) {
            s_d = s; s_d2 = s_d; s_d3 = s_d2;
            *ps = s;
            s = !s;
            
            //cout << sc_time_stamp() << " " << cst.read() << " "  << cntr.read() << endl;
            
            if (cntr_ > 3 && cntr_ < 12) cst = 1; else cst = 0;
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
    
    SCT_ASSERT_STABLE(rstn, (1), i0.read(), clk.pos());
    SCT_ASSERT_STABLE(i2.read(), (1), i5.read(), clk.pos());
    SCT_ASSERT_STABLE(rstn && i7.read() < 18, (1,2), i8.read(), clk.pos());
    SCT_ASSERT_ROSE(rstn, (1), i1.read(), clk.pos());        
    SCT_ASSERT_ROSE(i1.read(), (1), i2.read(), clk.pos());        
    SCT_ASSERT_ROSE(i2.read(), (1), i6.read(), clk.pos());
    SCT_ASSERT_ROSE(i2.read(), (1), i9.read(), clk.pos());        
    SCT_ASSERT_FELL(i2.read(), (0), i3.read(), clk.pos());        
    SCT_ASSERT_FELL(i2.read(), (1), i4.read(), clk.pos());        

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

    void sct_assert_thread() 
    {
        SCT_ASSERT_THREAD(s, SCT_TIME(1), s_d, clk.pos());
        wait();

        while (true) {
            //std::cout << "thread " << sc_time_stamp() << " dc " << sc_delta_count() << std::endl;
            wait();
        }
    }
    
    // Simple immediate assertion test
    void sct_assert_thread1() 
    {
        SCT_ASSERT_THREAD(s, (0), s, clk.pos());
        SCT_ASSERT_THREAD(s, SCT_TIME(1), s_d, clk.pos());
        SCT_ASSERT_THREAD(s, SCT_TIME(2,3), s_d3, clk.pos());
        wait();

        while (true) {
            wait();
        }
    }
    
    // Multiple-wait thread
    void sct_assert_thread2() {
        wait();
        SCT_ASSERT_THREAD(s, SCT_TIME(1), s_d, clk.pos());
        SCT_ASSERT_THREAD(s || s_d, (1,2), s_d2, clk.pos());

        while (true) {
            wait();
        }
    }    
    
    // Use pointer in assertion
    void sct_assert_pointer()
    {
        wait();
        SCT_ASSERT_THREAD(ps->read(), (0), s_d, clk.pos());

        while (true) {
            wait();
        }
    }
    
    void sct_assert_loop() 
    {
        sc_uint<3> k = 0;
        for (int i = 0; i < 3; i++) {
            sarr[i] = 0; sarr_d[i] = 0;
        }
        wait();

        for (int i = 0; i < 3; i++) {
            // Check @k is captured by reference
            SCT_ASSERT_LOOP(sarr[i], (1), sarr_d[i] || k == 1, clk.pos(), i);
            for (int j = 0; j < 2; j++) {
                SCT_ASSERT_LOOP(sarr2[i][j], SCT_TIME(0), sarr2[i][j], clk.pos(), i, j);
            }
        }
        
        while (true) {
            for (int i = 0; i < 3; i++) {
                sarr[i] = !sarr[i];
                //if (i == 1) continue;
                if (k) sarr_d[i] = sarr[i];
            }
            k = (k != 2) ? k+1 : 0;
            //cout << k << " ";
            wait();
        }
    }
};

class Test_top : public sc_module
{
public:
    sc_signal<sc_uint<4>>  a{"a"};
    sc_signal<sc_uint<4>>  b{"b"};
    sc_signal<bool>        rstn{"rstn"};
    sc_clock clk{"clock", 10, SC_NS};

    A a_mod{"a_mod", 4};

    SC_CTOR(Test_top) {
        a_mod.a(a);
        a_mod.b(b);
        a_mod.clk(clk);
        SC_CTHREAD(resetProc, clk);
        a_mod.rstn(rstn);

    }

    void resetProc() {
    	rstn = 0;
        a = 0;
        wait(2);
    	rstn = 1;
        a = 0;
    	wait(30);
        
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
