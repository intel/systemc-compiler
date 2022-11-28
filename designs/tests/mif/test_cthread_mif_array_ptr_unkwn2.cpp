/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Array of 2D modular interface pointers accessed at unknown index
// from local and parent CTHREAD processes
// Modular interface array [1][2], contains signal/signal pointer arrays
struct mod_if : public sc_module, sc_interface 
{
    sc_in_clk       clk;
    sc_in<bool>     rst;
    
    sc_signal<sc_uint<4>>   s;
    sc_signal<sc_uint<4>>   as[3];
    sc_signal<sc_uint<4>>*  asp[3];
    
    sc_uint<4>  y;       
    int         ay[3];

    sc_uint<4>  z;       
    int         az[2];

    SC_CTOR(mod_if) 
    {
        for (int i = 0; i < 3; i++) {
            asp[i] = new sc_signal<sc_uint<4>>("");
        }
        
        SC_CTHREAD(thread_member_sig, clk.pos());
        async_reset_signal_is(rst, 1);
        
        SC_CTHREAD(thread_member_comb, clk.pos());
        async_reset_signal_is(rst, 1);
        
        SC_CTHREAD(thread_member_reg, clk.pos());
        async_reset_signal_is(rst, 1);
    }
     
    void thread_member_sig() 
    {
        sc_uint<4> k; int j; y = 1; ay[2] = 2;
        j = s.read() + y;
        k = as[1].read() + ay[2];
        wait();
        
        while (true) {
            j = s.read() + y;
            k = as[1].read() + ay[2];
            wait();
        }
    }
    
    
    sc_uint<4>  v;          // comb
    int         av[2];      // comb
    int         avv[2][3];  // comb
    
    void thread_member_comb() 
    {
        int j = s.read();
        v = as[1];
        av[0] = v + asp[j]->read();
        avv[1][j] = av[0];
        wait();
        
        while (true) {
            v = 1;
            av[0] = v;
            avv[1][2] = v;
            int k = av[0] + avv[1][2];
            wait();
        }
    }
    
    sc_uint<4>  w;          // reg
    int         aw[2];      // reg
    
    void thread_member_reg() 
    {
        int j = s.read();
        as[0] = j;
        *asp[j] = 0;
        wait();
        
        while (true) {
            int l = w + aw[0] + as[1].read() + asp[2]->read();
            wait();
        }
    }
};

SC_MODULE(Top) {

    sc_in_clk       clk{"clk"};
    sc_signal<bool> rst;
    
    sc_signal<int>  sig;
    mod_if*         minst[1][2];

    SC_CTOR(Top) {
        for (int i = 0; i < 1; i++) 
        for (int j = 0; j < 2; j++) {
            minst[i][j] = new mod_if("mod_if");
            minst[i][j]->clk(clk);
            minst[i][j]->rst(rst);
        }
        
        SC_CTHREAD(top_thread_comb, clk.pos());
        async_reset_signal_is(rst, 1);
        
        SC_CTHREAD(top_thread_reg, clk.pos());
        async_reset_signal_is(rst, 1);
    }
    
    // Checking combinational variable member of MIF array accessed 
    void top_thread_comb() 
    {
        for (int i = 0; i < 2; i++) {
            minst[0][i]->y = i;
            for (int k = 0; k < 2; k++) {
                minst[0][i]->ay[k] = i+k;
            }
        }
        int j = sig.read();
        wait();
        
        while (true) {
            minst[0][1]->y = minst[j][0]->asp[0]->read();
            int i = minst[0][1]->y; 
            
            minst[0][0]->ay[1] = minst[0][j]->as[j].read();
            i = minst[0][0]->ay[1];
            
            wait();
        }
    }
    
    // Checking register variable member of MIF array accessed
    void top_thread_reg() 
    {
        int j = minst[0][0]->s.read();
        wait();
        
        while (true) {
            int i;
            minst[j][j+1]->z = 2;        
            i = minst[j][j+1]->z;
            
            minst[0][0]->az[j+1] = 3;
            i = minst[0][0]->az[1];
            
            wait();
        }
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
