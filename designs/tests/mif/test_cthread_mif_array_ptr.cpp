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
    sc_uint<4>              vc;
    sc_uint<4>              vr;
    sc_uint<4>              vo;
    int  vm;

    SC_CTOR(mod_if) 
    {
        SC_CTHREAD(thread_member_ro, clk.pos());
        async_reset_signal_is(rst, 1);

        SC_CTHREAD(thread_member_comb, clk.pos());
        async_reset_signal_is(rst, 1);

        SC_CTHREAD(thread_member_reg, clk.pos());
        async_reset_signal_is(rst, 1);
        
        SC_CTHREAD(thread_loc_reg, clk.pos());
        async_reset_signal_is(rst, 1);

        //SC_METHOD(meth);
        //sensitive << s;
    }
    
    void meth()
    {
        sc_uint<4> a = s;
    }

    void thread_member_ro()        
    {
        wait();
        
        while (true) {
            sc_uint<4> a = s;
            wait();
        }
    }
     
    sc_uint<4>     v;       // comb
    void thread_member_comb() 
    {
        v = 0;
        auto b = v + 1;     // comb
        wait();
        
        while (true) {
            v = 1;
            sc_uint<4> a = v;   // comb
            wait();
        }
    }

    sc_uint<5>    va;       // reg
    void thread_member_reg() 
    {
        va = 0;
        auto b = va + 1;    // reg
        wait();
        
        while (true) {
            sc_uint<5> a = va + b;  // reg
            wait();
            va = a;
        }
    }

    void thread_loc_reg() 
    {
        sc_uint<6> a = 0;   // reg
        wait();
        
        while (true) {
            sc_uint<6> b = a;   // comb
            a++;
            wait();
        }
    }
    
    void f() {
        v = s;
    }

    bool  vf = 0;
    
    void f_loc_reg() {
        bool a = vf;
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

        SC_CTHREAD(top_thread_ro, clk.pos());
        async_reset_signal_is(rst, 1);

        SC_CTHREAD(top_thread_fcall, clk.pos());
        async_reset_signal_is(rst, 1);
        
        //SC_METHOD(top_method);
        //sensitive << minst[0]->s << minst[1]->s << t;
    }

    // Checking combinational variable member of MIF array accessed 
    void top_thread_comb() 
    {
        minst[1]->vc = 1;
        wait();
        
        while (true) {
            minst[1]->vc = 2;
            sc_uint<4> a = minst[1]->vc;
            wait();
        }
    }
    
    // Checking register variable member of MIF array accessed
    void top_thread_reg() 
    {
        minst[1]->vr = 1;
        wait();
        
        while (true) {
            minst[1]->vr = 2;
            wait();
            sc_uint<4> a = minst[1]->vr;
        }
    }

    // Checking read-only variable member of MIF array accessed
    void top_thread_ro() 
    {
        wait();
        
        while (true) {
            sc_uint<4> a = minst[1]->vo;
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

    void top_method() 
    {
        minst[1]->vm = 2;
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
