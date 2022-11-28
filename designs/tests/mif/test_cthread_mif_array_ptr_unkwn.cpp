/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Array of modular interface pointers accessed at unknown index
// from local and parent CTHREAD processes
// Modular interface contains array and array pointer
struct mod_if : public sc_module, sc_interface 
{
    sc_in_clk       clk;
    sc_in<bool>     rst;
    
    sc_signal<sc_uint<4>>   s {"s"};
    sc_uint<4>  d;       
    int         ad[3];
    int*        adp[2];
    sc_uint<4>  y;       
    int         ay[3];
    int*        ayp[2];
    sc_uint<4>  z;       
    int         az[2];
    int*        azp[2];

    SC_CTOR(mod_if) 
    {
        for (int i = 0; i < 2; i++) {
            avp[i] = sc_new<bool>();
            awp[i] = sc_new<bool>();
            axp[i] = sc_new<int>();
            adp[i] = sc_new<int>();
            ayp[i] = sc_new<int>();
            azp[i] = sc_new<int>();
        }
        
        SC_CTHREAD(thread_member_comb, clk.pos());
        async_reset_signal_is(rst, 1);
        
        SC_CTHREAD(thread_member_ro, clk.pos());
        async_reset_signal_is(rst, 1);

        SC_CTHREAD(thread_member_reg, clk.pos());
        async_reset_signal_is(rst, 1);
    }
     
    sc_uint<4>  v;          // comb
    int         av[2];      // comb
    int         avv[2][3];  // comb
    bool*       avp[2];     // comb
    
    void thread_member_comb() 
    {
        v = 0;
        av[0] = v + 1;
        avv[1][2] = av[0];
        wait();
        
        while (true) {
            v = 1;
            av[0] = v;
            avv[1][2] = v;
            int k = av[0] + avv[1][2];
            
            *avp[1] = k;
            *avp[0] = *avp[1];   
            wait();
        }
    }
    
    const sc_uint<8>  t = 42;            // ro
    const int         at[3] = {1, 2, 3}; // ro
    
    void thread_member_ro() 
    {
        int i = t - at[1];
        wait();
        
        while (true) {
            int k = t + at[2];
            wait();
        }
    }
    
    sc_uint<4>  w;          // reg
    int         aw[2];      // reg
    bool*       awp[2];     // reg
    
    void thread_member_reg() 
    {
        w = 0;
        aw[0] = w + 1;    
        wait();
        
        while (true) {
            int l = aw[0] + w + s.read();
            *awp[l] = 0;
            l = *awp[1];
            wait();
        }
    }

    sc_uint<4>  x;          // reg
    int         ax[2];      // reg
    int*        axp[2];     // reg
    
    void f() {
        int l = ax[x];
        x = 2;
        wait();
        
        ax[l] = x;
        x = ax[1];
        *axp[l] = 1;
        x = *axp[0];
    }
};

SC_MODULE(Top) {

    sc_in_clk       clk{"clk"};
    sc_signal<bool> rst;
    
    sc_signal<int>  sig;
    mod_if*         minst[2];

    SC_CTOR(Top) {
        for (int i = 0; i < 2; i++) {
            minst[i] = new mod_if("mod_if");
            minst[i]->clk(clk);
            minst[i]->rst(rst);
        }
        
        SC_CTHREAD(top_thread_comb_reset, clk.pos());
        async_reset_signal_is(rst, 1);
        
        SC_CTHREAD(top_thread_comb, clk.pos());
        async_reset_signal_is(rst, 1);
        
        SC_CTHREAD(top_thread_reg, clk.pos());
        async_reset_signal_is(rst, 1);

        SC_CTHREAD(top_thread_fcall, clk.pos());
        async_reset_signal_is(rst, 1);

        SC_CTHREAD(top_thread_ptr_array, clk.pos());
        async_reset_signal_is(rst, 1);
    }
    
    void top_thread_comb_reset() 
    {
        int j = sig.read();
        minst[j]->d = 1;
        minst[j]->ad[2] = 2;
        *minst[j]->adp[j+1] = 3;
        wait();
        
        while (true) {
            wait();
        }
    }

    // Checking combinational variable member of MIF array accessed 
    void top_thread_comb() 
    {
        for (int i = 0; i < 2; i++) {
            minst[i]->y = i;
            for (int k = 0; k < 2; k++) {
                minst[i]->ay[k] = i+k;
                *minst[i]->ayp[k] = i-k;
            }
        }
        int j = sig.read();
        wait();
        
        while (true) {
            minst[1]->y = 2;
            int i = minst[1]->y;    // comb
            
            minst[1]->ay[1] = 3;
            i = minst[1]->ay[1];
            
            *minst[1]->ayp[1] = 4;
            i = *minst[1]->ayp[1];
            
            wait();
        }
    }
    
    // Checking register variable member of MIF array accessed
    void top_thread_reg() 
    {
        int j = sig.read();
        wait();
        
        while (true) {
            int i;
            minst[j+1]->z = 2;        // reg
            i = minst[j]->z;
            
            minst[j]->az[j+1] = 3;
            i = minst[j]->az[1];
            
            wait();
        }
    }

    void top_thread_fcall() 
    {
        int jj = sig.read();
        wait();
        
        while (true) {
            minst[jj]->f();
            wait();
        }
    }

    void top_thread_ptr_array() 
    {
        int kk = sig.read();
        wait();
        
        while (true) {
            int i;
            *minst[kk]->azp[0] = 4;
            i = *minst[1]->azp[kk];
            
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
