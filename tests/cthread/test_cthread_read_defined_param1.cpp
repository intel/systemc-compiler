/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>
#include "sct_assert.h"

// Use/Def analysis for function calls with value and reference parameters
class top : sc_module {
public:
    sc_in_clk clk{"clk"};
    sc_signal<bool> rstn{"rstn", 1};
    sc_signal<int> out{"out"};
    sc_signal<int> in{"in"};
    
    sc_signal<sc_uint<3>> ch;
    static const unsigned A = 2;
    const unsigned B = 3;

    SC_HAS_PROCESS(top);
    top(sc_module_name)
    {
        SC_CTHREAD(const_ref_call1, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(const_ref_call2, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(const_ref_call3, clk.pos());
        async_reset_signal_is(rstn, false);
        
        SC_CTHREAD(const_ref_call4, clk.pos());
        async_reset_signal_is(rstn, false);
        
        SC_CTHREAD(const_ref_call_range, clk.pos());
        async_reset_signal_is(rstn, false);
        

        SC_CTHREAD(ref_call3, clk.pos());
        async_reset_signal_is(rstn, false);
        
        SC_CTHREAD(ref_call4, clk.pos());
        async_reset_signal_is(rstn, false);
        
        SC_CTHREAD(chan_ref1, clk.pos());
        async_reset_signal_is(rstn, false);

        
        SC_CTHREAD(const_val_call1, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(const_val_call2, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(const_val_call3, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(const_val_call4, clk.pos());
        async_reset_signal_is(rstn, false);
        
        
        SC_CTHREAD(val_call3, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(val_call4, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(def_param1, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(def_param2, clk.pos());
        async_reset_signal_is(rstn, false);
    }

   
    // ------------------------------------------------------------------------
    // Constant reference

    sc_uint<4> cref(const sc_uint<3>& val) {
        int l = val;
        wait();             // 1
        return l;
    }

    // Literal and RValue passed via constant reference
    void const_ref_call1() {
        int i = 0;
        wait();             // 0
        
        while (true) {
            i = cref(1);
            i = cref(3+in.read());
            wait();         // 2
        }
    }
    
    void const_ref_call2() {
        int i = 0;
        wait();             // 0
        
        while (true) {
            i = cref(A);
            i = cref(B);
            wait();         // 2
        }
    }

    // @a not register
    void const_ref_call3() {
        int i = 0;
        wait();             // 0
        
        while (true) {
            sc_uint<3> a = 1;
            i = cref(a);
            wait();         // 2
        }
    }
    
    // @a register
    void const_ref_call4() {
        int i = 0;
        sc_uint<3> a = 1;
        wait();             // 0
        
        while (true) {
            i = cref(a);
            wait();         // 2
        }
    }

    
    // Range for constant reference
    sc_uint<2> range_cref(const sc_uint<3>& val) {
        bool c = val.bit(0);
        sc_uint<2> l = val.range(2,1);
        wait();             // 1
        return l;
    }
    
     void const_ref_call_range() {
        int i = 0;
        wait();             // 0
        
        while (true) {
            i = range_cref(1);
            wait();         // 2
        }
    }
    
    
    // ------------------------------------------------------------------------
    // Non-constant reference
    
    sc_uint<4> fref(sc_uint<3>& val) {
        int l = val;
        wait();             // 1
        return l;
    }

    // @a not register
    void ref_call3() {
        int i = 0;
        wait();             // 0
        
        while (true) {
            sc_uint<3> a = 1;
            i = fref(a);
            wait();         // 2
        }
    }

    // @a register
    void ref_call4() {
        int i = 0;
        sc_uint<3> a = 1;
        wait();             // 0
        
        while (true) {
            i = fref(a);
            wait();         // 2
        }
    }

    // Channel reference
    bool fch_ref(sc_signal<sc_uint<3>>& r) {
        bool ll = r.read();
        wait();
        return ll;
    }
    
    void chan_ref1() 
    {
        ch = 0;
        wait();         
        
        while (true) {
            bool d = fch_ref(ch);
            wait();     
        }
    }    
    
    // ------------------------------------------------------------------------
    // Constant value
    sc_uint<4> cval(const sc_uint<3> val) {
        int l = val;
        wait();             // 1
        return l;
    }
    
    sc_uint<4> cval2(const sc_uint<3> val) {
        int l = val.bit(ch.read());
        wait();             // 1
        return l;
    }
        
    void const_val_call1() {
        int i = 0;
        wait();             // 0
        
        while (true) {
            i = cval2(1);
            i = cval(in.read());
            wait();         // 2
        }
    }
    
     void const_val_call2() {
        int i = 0;
        wait();             // 0
        
        while (true) {
            i = cval(A+1);
            i = cval(B);
            wait();         // 2
        }
    }
     
    void const_val_call3() {
        int i = 0;
        wait();             // 0
        
        while (true) {
            sc_uint<3> a = 1;
            i = cval(a);
            wait();         // 2
        }
    }
    
    void const_val_call4() {
        int i = 0;
        sc_uint<3> a = 1;
        wait();             // 0
        
        while (true) {
            i = cval(a);
            wait();         // 2
        }
    }
    
// ------------------------------------------------------------------------
    // Non-constant value
    sc_uint<4> fval(sc_uint<3> val) {
        int l = val;
        wait();             // 1
        return l;
    }
    
    void val_call3() {
        int i = 0;
        wait();             // 0
        
        while (true) {
            sc_uint<3> a = 1;
            i = fval(a);
            wait();         // 2
        }
    }
    
     sc_uint<4> fval2(sc_uint<3> val) {
        int l = val.bit(ch.read());
        wait();             // 1
        return l;
    }
    
    void val_call4() {
        int i = 0;
        sc_uint<3> a = 1;
        wait();             // 0
        
        while (true) {
            i = fval2(a);
            wait();         // 2
        }
    }    

// ------------------------------------------------------------------------

    // Default parameters
    sc_uint<4> def_cref(const sc_uint<3>& val = 1) {
        int l = val;
        wait();             // 1
        return l;
    }

    void def_param1() {
        int i = 0;
        wait();             // 0
        
        while (true) {
            i = def_cref();
            wait();         // 2
        }
    }

    sc_uint<4> def_cval(const sc_uint<3> val = 1) {
        int l = val;
        wait();             // 1
        return l;
    }

    void def_param2() 
    {
        int i = 0;
        wait();             // 0
        
        while (true) {
            i = def_cval();
            wait();         // 2
        }
    }
};

int sc_main(int argc, char *argv[])
{
    sc_clock clk{"clk", 10, SC_NS};
    top top_inst{"top_inst"};
    top_inst.clk(clk);
    sc_start();
    return 0;
}

