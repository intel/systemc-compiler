/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

// Record with function contains wait()
class A : public sc_module {
public:
    sc_in_clk           clk;
    sc_signal<bool>     nrst;
    sc_signal<int>      sig{"sig"};

    SC_CTOR(A) 
    {
        SC_CTHREAD(wait_in_record1, clk.pos());
        async_reset_signal_is(nrst, 0);
        
        SC_CTHREAD(wait_in_record2, clk.pos());
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(wait_in_record3, clk.pos());
        async_reset_signal_is(nrst, 0);
    }
  
    struct WaitRec {
        WaitRec() {}
        
        void waitFunc() {
            sc_core::wait();
        }

        int waitFunc(int par) {
            par += 1;
            sc_core::wait();
            return par;
        }
        
        void waitFuncRef(int& par) {
            par++;
            sc_core::wait();
            par--;
        }
    }; 
    
    
    WaitRec wr;
    void wait_in_record1()
    {
        wait();
        
        while (true) {
            
            wr.waitFunc();
        }
    }
    
    WaitRec wr2;
    void wait_in_record2()
    {
        wait();
        
        while (true) {
            
            auto i = wr2.waitFunc(10);
            sig = i;
        }
    }
    
    void wait_in_record3()
    {
        WaitRec wr3;
        wait();
        
        while (true) {
            int i = sig.read();
            wr3.waitFuncRef(i);
        }
    }
};

int sc_main(int argc, char *argv[]) {
    sc_clock clk{"clk", 1, SC_NS};
    A a_mod{"a_mod"};
    a_mod.clk(clk);
    
    sc_start();
    return 0;
}

