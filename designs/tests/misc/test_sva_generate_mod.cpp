/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"
#include <iostream>
#include <cassert>

class B : public sc_module {
public:
    sc_signal<int>      o;
    sc_signal<int>      r;

    SC_CTOR(B) 
    {}
};

// Assertions and SVA generation test
template <unsigned N>
class A : public B 
{
public:
    sc_in<bool>         clk{"clk"};
    sc_in<bool>         rstn{"rstn"};
    
    sc_signal<int>      s;
    sc_signal<int>      _s;
    sc_signal<int>      s_d;
    sc_signal<int>      s_d2;
    sc_signal<int>      s_d3;

    sc_signal<sc_uint<8>> u;
    sc_signal<sc_uint<8>>* up;
    
    sc_uint<8>          m;
    sc_uint<16>         mm;
    const sc_uint<8>    marr[3] = {1, 2 ,3};
    
    sc_signal<int>      r;
    sc_signal<int>      reg;

    sc_in<bool>         nrpi{"nrpi"};
    sc_out<bool>        nrpo{"nrpo"};
    sc_signal<bool>     nrs;
    sc_signal<bool>     nrs2;
    sc_signal<bool>     nrs3;
    sc_signal<bool>     nrarr[3];
    sc_signal<bool>*    nrptr;
    
    sc_signal<int>*     ps;
    const bool          CONST_B = true;
    const bool          arr[3] = {true, true, false};
    sc_signal<int>      sarr[3];
    sc_signal<sc_uint<3>>* psarr[3];
    
    const unsigned NN = N;
    
    struct Local {
        bool a;
        sc_uint<4> b;
    };
    
    Local rec;
    
    SC_HAS_PROCESS(A);
    
    explicit A(const sc_module_name& name) : B(name)
    {
        ps = new sc_signal<int>("ps");
        up = new sc_signal<sc_uint<8>>("up");
        nrptr = new sc_signal<bool>("nrptr");
        
        for (int i = 0; i < 3; i++) {
            psarr[i] = new sc_signal<sc_uint<3>>("psarr");
        }
        
        SC_CTHREAD(test_thread, clk.pos());
        async_reset_signal_is(rstn, false);

    }

    // Various time parameters
    SCT_ASSERT(s, (0), s, clk.pos());  
    SCT_ASSERT(s, SCT_TIME(1) , s_d, clk.pos());
    SCT_ASSERT(s, SCT_TIME(2*N+1,NN-1), s_d, clk.pos());
    SCT_ASSERT(s, (3,N+1), s_d3, clk.pos());
    
    // Operations
    SCT_ASSERT(s || s_d, SCT_TIME(1), s_d2 && s_d3, clk.pos());
    SCT_ASSERT(m + 1 == N || CONST_B, SCT_TIME(1), mm - marr[1] > 3, clk.pos());
    SCT_ASSERT(m % 2 != NN || mm == s.read(), SCT_TIME(1), marr[2], clk.pos());
    
    // Bit and range
    SCT_ASSERT(u.read().bit(1), SCT_TIME(0), s, clk.pos());
    SCT_ASSERT(up->read().bit(2), SCT_TIME(0), s, clk.pos());   
    SCT_ASSERT(u.read().range(2,1), SCT_TIME(0), s, clk.pos());
    SCT_ASSERT(up->read().range(5,3), SCT_TIME(0), s, clk.pos());

    // Using non-read port and signal
    SCT_ASSERT(nrpi, SCT_TIME(1), r && nrpo, clk.pos());
    SCT_ASSERT(nrs, SCT_TIME(0), nrs, clk.pos());
    SCT_ASSERT(r || nrs2, SCT_TIME(0), !r && nrs3, clk.pos()); // Evaluated to const
    SCT_ASSERT(NN || nrs2, SCT_TIME(0), !NN && nrs3, clk.pos()); // Evaluated to const

    // Signal access with read()
    SCT_ASSERT(s.read( ) == 1, SCT_TIME(3) , s_d3, clk.pos());
    SCT_ASSERT(ps->read(), SCT_TIME(1,2) , s || s.read(), clk.pos());

    // Array access 
    SCT_ASSERT(arr[1], SCT_TIME(1) , s, clk.pos());
    SCT_ASSERT(sarr[2].read(), SCT_TIME(0), sarr[0], clk.pos());
    SCT_ASSERT(psarr[1]->read(), SCT_TIME(1), sarr[2], clk.pos());

    // Array access at variable index, may be incorrect if @i is not constant
    int i = 0;
    SCT_ASSERT(arr[i], SCT_TIME(1), s, clk.pos());
    SCT_ASSERT(sarr[1+i], SCT_TIME(0) , arr[0]+i, clk.pos());
    SCT_ASSERT(psarr[1]->read(), SCT_TIME(1), (*psarr[2+i]).read(), clk.pos());
    
    // Access with @this
    SCT_ASSERT(this->s, SCT_TIME(1), s_d || this->o, clk.pos());
    SCT_ASSERT(this->o, SCT_TIME(0), B::o, clk.pos());
    
    // Record
    SCT_ASSERT(rec.a, SCT_TIME(0), rec.b+1, clk.pos());
    
    // Not used signals/ports and constants
    SCT_ASSERT(nrpi, SCT_TIME(1), nrpo || nrs, clk.pos());
    SCT_ASSERT(nrarr[1], SCT_TIME(1), nrptr->read(), clk.pos());
    SCT_ASSERT(s.read() == N, SCT_TIME(1), ps->read() != NN, clk.pos());
    
    static const unsigned   CC1 = 42;
    static const unsigned   CC2 = 0;
    sc_signal<int>          nu1; // not used
    sc_signal<bool>         nu2; // not used
    
    SCT_ASSERT(!CC1 || s.read() || nu1.read() == 11, SCT_TIME(1), !CC2 && nu2.read(), clk.pos());
    
    // Name collisions
    SCT_ASSERT(reg, SCT_TIME(1), r || B::r, clk.pos());

    // 2-argument assertions
    SCT_ASSERT(rstn || !s, clk.pos());
    SCT_ASSERT(!s_d2 || s, clk.pos());
    
    void test_thread() 
    {
        nrs = 0;
        s = 0; s_d = 0; s_d2 = 0; s_d3 = 0;   
        wait();

        while (true) {
            s_d = s; s_d2 = s_d; s_d3 = s_d2;
            s = !s;
            cout << "." << flush;
            wait();
        }
    }
};

class Test_top : public sc_module
{
public:
    sc_clock clk{"clock", 10, SC_NS};
    sc_signal<bool> rstn;
    sc_signal<bool> nrp;

    A<2> a_mod{"a_mod"};

    SC_CTOR(Test_top) {
        a_mod.clk(clk);
        SC_CTHREAD(testProc, clk);
        a_mod.rstn(rstn);
        a_mod.nrpi(nrp);
        a_mod.nrpo(nrp);
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

