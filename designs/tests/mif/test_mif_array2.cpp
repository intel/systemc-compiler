/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by mmoiseev on 06/14/19.
//

#include <systemc.h>

struct Simple {
    bool a;
    sc_uint<4> b;
};


// Array of modular interface pointers
struct mod_if : public sc_module, sc_interface 
{
    sc_in_clk           clk{"clk"};
    sc_in<bool>         rst{"rst"};
    
    sc_signal<bool>     s {"s"};
    sc_uint<3>          v;
    sc_uint<4>          vv;
    sc_uint<5>          vvv;
    sc_uint<3>          w[3];

    SC_CTOR(mod_if) 
    {
        SC_METHOD(metProc);
        sensitive << s;

        SC_METHOD(locRecMeth);
        sensitive << s;

        SC_METHOD(locRecArrMeth);
        sensitive << s;

        SC_METHOD(memRecMeth);
        sensitive << s;

        SC_METHOD(memRecArrMeth);
        sensitive << s;

        SC_CTHREAD(thrProc, clk.pos());
        async_reset_signal_is(rst, true);
    }

    // Variables and signal members of MIF
    void metProc() {
        v = 1; 
        vv = 2;
        w[2] = 3;
        
        bool c = v;
        bool d = vv;
        
        v = s.read() ? 0 : 1;
        d = !s.read() && v;
    }
    
    // Local record 
    void locRecMeth() {
        Simple t;
        Simple tt;
        
        t.a = false;
        t.b = 4;
        
        tt.a = true;
        tt.b = 5;
        
        sc_uint<4> x = t.b + tt.b;
    }
    
    // Local record array member of MIF
    void locRecArrMeth() {
        Simple t[2];
        Simple tt[3];
        
        t[1].a = false;
        t[1].b = 4;
        
        for (int i = 0; i < 3; ++i) {
            tt[i].a = i == 0;
            tt[i].b = i;
            if (i < 2) t[i].b = tt[i].b + 1;
        }
        
        sc_uint<4> x = t[1].b + tt[2].b;
    }
    
    // Member record and record array member of MIF
    Simple  r;
    Simple  rr;

    void memRecMeth() {
        r.a = false;
        r.b = 4;
        
        rr.a = true;
        rr.b = 5;
        
        sc_uint<4> x = r.b + rr.b;
    }

    Simple  p[2];
    Simple  pp[3];
    
    void memRecArrMeth() {
        p[1].a = false;
        p[1].b = 4;
        
        pp[2].a = false;
        pp[2].b = 4;
        
        sc_uint<4> x = p[1].b + pp[2].b;
    }
    
    void thrProc() {
        sc_uint<3> i = 0;   // Register
        sc_uint<3> j = 1;   // Comb variable
        s = 0;
        vvv = 0;
        wait();
        
        while (1) {
            s = i++;
            j = vvv++;
            wait();
        }
    }

    void func() {
        bool d = s;
        //bool e = v;
        //return s;
    }
};

SC_MODULE(top) {

    sc_in_clk       clk{"clk"};
    sc_in<bool>     rst;
    
    //Simple        ss[2];
    mod_if*         minst[2];

    sc_uint<3>      y;    

    SC_CTOR(top) {
        for (int i = 0; i < 2; i++) {
            minst[i] = new mod_if("mod_if");
            minst[i]->clk(clk);
            minst[i]->rst(rst);
        }
        
        //SC_METHOD(top_method);
        //sensitive << rst; //minst[0]->s << minst[1]->s;

        //SC_CTHREAD(top_thread, clk.pos());
        //async_reset_signal_is(rst, true);
    }

    void top_method() {
        //bool c = minst[0]->s || minst[1]->s;
        //minst[0]->func();
        //minst[1]->func();
        y = 1;
        bool b = y;
    }
    
     void top_thread() {
        y = 1;
        wait();
        bool b = y;
    }

};

SC_MODULE(tb) {

    sc_clock        clk {"clk", sc_time(1, SC_NS)};
    sc_signal<bool> rst {"rst"};

    top top_inst{"top_inst"};

    SC_CTOR(tb) {
        top_inst.clk(clk);
        top_inst.rst(rst);
    }

};

int sc_main(int argc, char **argv) {

    cout << "test_modular_iface_proc\n";

    tb tb_inst{"tb_inst"};
    sc_start();

    return 0;
}
