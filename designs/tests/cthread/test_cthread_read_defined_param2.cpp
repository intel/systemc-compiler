/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>
#include "sct_assert.h"

// Use/Def analysis for function calls with value and reference parameters
// used after wait -- must be register
class top : sc_module {
public:
    sc_in_clk clk{"clk"};
    sc_signal<bool> rstn{"rstn", 1};
    sc_signal<int> out{"out"};
    sc_signal<int> in{"in"};
    
    sc_signal<sc_uint<3>> s;
    static const unsigned A = 2;
    const unsigned B = 3;

    SC_HAS_PROCESS(top);
    top(sc_module_name)
    {
        // Constant reference simulation test
        //SC_CTHREAD(tmp, clk.pos());
        //async_reset_signal_is(rstn, false);

        //SC_CTHREAD(const_ref_call0, clk.pos());
        //async_reset_signal_is(rstn, false);

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

        SC_CTHREAD(val_call5, clk.pos());
        async_reset_signal_is(rstn, false);
        
        SC_CTHREAD(def_param1, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(def_param2, clk.pos());
        async_reset_signal_is(rstn, false);
    }

   
// ------------------------------------------------------------------------
    // Constant reference simulation test
    sc_signal<sc_uint<3>> s0;
    sc_signal<sc_biguint<3>> s1;
    sc_signal<sc_uint<3>> sarr[3];
    void cref_void(const sc_uint<3>& val) {
        int l;
        cout << sc_time_stamp() << " 1 " << val << endl;
        wait();             // 1
        cout << sc_time_stamp() << " 2 " << val << endl;
        l = val;
    }

    void const_ref_call0() {
        wait();             // 0
        
        while (true) { 
            //cref_void(s0.read()+1);
            //cref_void(s1.read());
            //cref_void(sarr[1]);
            const sc_uint<3>& ref = s0.read();
            cout << sc_time_stamp() << " 1 " << ref << endl;
            
            wait();         // 2
            
            cout << sc_time_stamp() << " 2 " << ref << endl;
        }
    }
    
    void tmp() {
        s0 = 0;
        s1 = 0;
        sarr[1] = 0;
        wait(); 
        
        for (int i = 0; i < 5; ++i) {
            s0 = s0.read() + 1;
            s1 = s1.read() + 1;
            sarr[1] = sarr[1].read() + 1;
            wait();
        }
        
        sc_stop();
    }
    
// ------------------------------------------------------------------------
    // Constant reference

    sc_uint<4> cref(const sc_uint<3>& val) {
        wait();             // 1
        return val;
    }
    
    // Literal and RValue passed via constant reference
    void const_ref_call1() {
        int i = 0;
        wait();             // 0
        
        while (true) {
            i = cref(1);
            i = cref(in.read() + 2);
            wait();         // 3
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

    // @a register
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
        sc_uint<3> b = 1;
        wait();             // 0
        
        while (true) {
            i = cref(b);
            wait();         // 2
        }
    }

    
    // Range for constant reference
    sc_uint<2> range_cref(const sc_uint<3>& val) {
        bool c = val.bit(0);
        wait();             // 1
        sc_uint<2> l = val.range(2,1);
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
        wait();             // 1
        int l = val;
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
        wait();
        bool ll = r.read();
        return ll;
    }
    
    void chan_ref1() 
    {
        s = 0;
        wait();         
        
        while (true) {
            bool d = fch_ref(s);
            wait();     
        }
    }    
    
    // ------------------------------------------------------------------------
    // Constant value
    sc_uint<4> cval(const sc_uint<3> val) {
        int l = val;
        wait();             // 1
        return val;
    }
    
    void const_val_call1() {
        int i = 0;
        wait();             // 0
        
        while (true) {
            i = cval(1);
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
        wait();             // 1
        int l = val;
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
    
    void val_call4() {
        int i = 0;
        sc_uint<3> a = 1;
        wait();             // 0
        
        while (true) {
            i = fval(a);
            wait();         // 2
        }
    }    
    
    void val_call5() {
        int i = 0;
        wait();             // 0
        
        while (true) {
            i = fval(5);
            wait();         
            
            i = fval(6);
        }
    }    

// ------------------------------------------------------------------------

    // Default parameters
    sc_uint<4> def_cref(const sc_uint<3>& val = 1) {
        int l = val;
        wait();             // 1
        l = val-1;
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
        l = val-1;
        return l;
    }

    void def_param2() {
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

