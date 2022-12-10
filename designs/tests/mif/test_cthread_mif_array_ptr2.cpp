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

// Array of modular interface pointers accessed from parent module THREAD
struct mod_if : public sc_module, sc_interface 
{
    sc_in_clk       clk;
    sc_in<bool>     rst;
    
    sc_signal<sc_uint<4>>   s {"s"};
    sc_signal<sc_uint<4>>   ss {"ss"};
    sc_uint<4>              v;
    sc_uint<4>              vv;

    SC_CTOR(mod_if) 
    {
        SC_METHOD(meth);
        sensitive << s;
    }
    
    void meth()
    {
        sc_uint<4> a = s;
    }

    void f() {
        v = s;
    }

    void f_loc() {
        bool a = s.read();
    }
    
    void f_loc_reg() {
        bool a = v;
        wait();
        bool b = a;
    }
};

SC_MODULE(Top) {

    sc_in_clk       clk{"clk"};
    sc_signal<bool> rst;
    
    sc_uint<7>      k;
    sc_signal<int>  t;
    mod_if*         minst[2];

    SC_CTOR(Top) {
        for (int i = 0; i < 2; i++) {
            minst[i] = new mod_if("mod_if");
            minst[i]->clk(clk);
            minst[i]->rst(rst);
        }
        
        SC_CTHREAD(top_thread_comb, clk.pos());
        async_reset_signal_is(rst, 1);
        
        SC_CTHREAD(top_thread_reg, clk.pos());
        async_reset_signal_is(rst, 1);

        SC_CTHREAD(top_thread_sig_reg, clk.pos());
        async_reset_signal_is(rst, 1);

        SC_CTHREAD(top_thread_ro, clk.pos());
        async_reset_signal_is(rst, 1);

        SC_CTHREAD(top_thread_fcall, clk.pos());
        async_reset_signal_is(rst, 1);
        
        SC_CTHREAD(top_thread_fcall_loc, clk.pos());
        async_reset_signal_is(rst, 1);

        SC_METHOD(top_method);
        sensitive << minst[0]->s << minst[1]->s << t;
    }

    // Checking combinational variable member of MIF array accessed 
    void top_thread_comb() 
    {
        minst[1]->vv = 1;
        wait();
        
        while (true) {
            minst[1]->vv = 2;
            sc_uint<4> a = minst[1]->vv;
            wait();
        }
    }
    
    // Checking register variable member of MIF array accessed
    void top_thread_reg() 
    {
        minst[1]->vv = 1;
        wait();
        
        while (true) {
            minst[1]->vv = 2;
            wait();
            sc_uint<4> a = minst[1]->vv;
        }
    }

    void top_thread_sig_reg() 
    {
        minst[1]->ss = 1;
        wait();
        
        while (true) {
            minst[1]->ss = 2;
            wait();
            sc_uint<4> a = minst[1]->ss.read();
        }
    }

    // Checking read-only variable member of MIF array accessed
    void top_thread_ro() 
    {
        wait();
        
        while (true) {
            sc_uint<4> a = minst[1]->ss.read();
            wait();
        }
    }
    
    void top_thread_fcall() 
    {
        wait();
        
        while (true) {
            minst[1]->f_loc_reg();
            wait();
        }
    }

    void top_thread_fcall_loc() 
    {
        wait();
        
        while (true) {
            minst[1]->f_loc();
            wait();
        }
    }

    void top_method() 
    {
        minst[1]->vv = 2;
    }
};

int sc_main(int argc, char **argv) 
{
    sc_clock clk {"clk", sc_time(1, SC_NS)};
    Top top{"top"};
    top.clk(clk);
    
    sc_start();

    return 0;
}
