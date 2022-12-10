/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

using namespace sc_core;

// Local variables name conflicts with empty method names
class mod_if : public sc_module, sc_interface 
{
public:
    sc_in_clk       clk;
    sc_signal<bool> rstn;
    sc_signal<int>  s;

    SC_CTOR(mod_if) 
    {
        SC_METHOD(empty_proc);
        SC_METHOD(local_var); sensitive << s;
        SC_CTHREAD(reg_var, clk.pos()); async_reset_signal_is(rstn, 0);
    }
        
    void empty_proc() {
        int i = 1;
        s = i + 1;
    }
    
    void local_var() 
    {
        int i = 1;
        int sum = i + 1;
    }
    
    void reg_var() 
    {
        int i = 0;
        wait();
        
        while (true) {
            
            long sum = i;
            wait();
        }
    }
};

SC_MODULE(Top) 
{
    sc_in_clk clk;
    mod_if    minst{"minst"};
    mod_if*   marr[2];

    SC_CTOR(Top) {
        minst.clk(clk);
        for (int i = 0; i < 2; ++i) {
            marr[i] = new mod_if("mod_if");
            marr[i]->clk(clk);
        }
    }
};

int sc_main(int argc, char *argv[]) 
{
    sc_clock clk{"clk", 10, SC_NS};
    Top top{"top"};
    top.clk(clk);
    
    sc_start();
    return 0;
}

