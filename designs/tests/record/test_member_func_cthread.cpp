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

// Record member function call in CTHREAD
class A : public sc_module {
public:
    sc_in_clk clk;
    sc_signal<bool> rstn;

    SC_CTOR(A) 
    {
        SC_CTHREAD(func_call_in_reset, clk.pos());
        async_reset_signal_is(rstn, 0);

        SC_CTHREAD(call_in_reset, clk.pos());
        async_reset_signal_is(rstn, 0);
        
        SC_CTHREAD(arr_call_in_reset, clk.pos());
        async_reset_signal_is(rstn, 0);
    }
    
    // Function call in reset for record 
    sc_signal<int> t0;
    void f(bool par) {
        bool c = par;
        t0 = c;
    }

    sc_signal<int> t1;
    void g(int par) {
        int c = par;
        t1 = c;
    }
    
    void func_call_in_reset() 
    {
        f(1);
        wait();
        
        while (true) {
            g(1);
            wait();
        }
    }

// ---------------------------------------------------------------------------
    
    struct nco_core 
    {
	sc_uint<4> nco_acc;
        
        void acc_inc(sc_uint<4> val) {
            nco_acc += val;
	}

	void acc_init(sc_uint<4> val) {
            nco_acc = val;
	}

	sc_uint<4> get_acc() {
            return nco_acc;
	}
    };
    
    // Member call in reset for record 
    nco_core  nc;
    
    sc_signal<int> t2;
    void call_in_reset() 
    {
        nc.acc_init(0);
        wait();
        
        while (true) {
            sc_uint<4> i = nc.get_acc();
            t2 = i;
            wait();
        }
    }
    
    // Member call in reset for record array element
    nco_core  ncc[3];
    sc_out<sc_uint<4>> nco_out[3];

    void arr_call_in_reset() 
    {
        for (int i = 0; i < 3; i++) {
            ncc[i].acc_init(0);
            nco_out[i] = 0;
        }
        
        while (true) {
            wait();
            
            for (int i = 0; i < 3; i++) {
                nco_out[i] = ncc[i].get_acc() >> 2;
            }
        }
    }
};

int sc_main(int argc, char *argv[]) 
{
    sc_clock clk("clk", 1, SC_NS);
    sc_signal<sc_uint<4>> nco_sig[3];
    
    A a{"a"};
    a.clk(clk);
    for (int i = 0; i < 3; i++) {
        a.nco_out[i](nco_sig[i]);
    }
    

    sc_start();
    return 0;
}

