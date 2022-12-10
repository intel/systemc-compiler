/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_comb_signal.h"
#include <systemc.h>
#include <string>

// Call virtual methods define din base class @memory_wrapper_base
class mem_wrapper_if : public sc_interface {
public:
    virtual void clear_req() = 0;
    virtual void read_req()  = 0;
};

struct memory_wrapper_base : public sc_module, mem_wrapper_if
{
    sc_in<bool>              clk{"clk"};
    sct_comb_signal<bool>    req{"req"};

    SC_CTOR(memory_wrapper_base) 
    {
    }
    
    virtual void clear_req() {
        req = 0;
    }
    
    virtual void read_req() {
        req = 1;
    }
    
};

struct memory_wrapper : public memory_wrapper_base 
{
    explicit memory_wrapper(const sc_module_name& name) :
        memory_wrapper_base(name)
    {
    }
};


struct module_with_memory : public sc_module
{
    sc_in<bool>         clk{"clk"};
    sc_in<bool>         rst{"rst"};
    memory_wrapper      mem{"mem"};
    
    SC_CTOR(module_with_memory) 
    {
        mem.clk(clk);
        SC_CTHREAD(test, clk.pos());
        async_reset_signal_is(rst, 0);
    }
    
    void test() {
        mem.clear_req();
        wait();             
        
        while (true) {
            mem.read_req();
            wait();         
            mem.clear_req();
            wait();         
        }
    }
};

int sc_main(int argc, char **argv) {

    sc_clock clock_gen{"clock_gen", 10, SC_NS};
    sc_signal<bool> rst{"rst"};
    module_with_memory mod{"mod"};
    mod.clk(clock_gen);
    mod.rst(rst);
    sc_start();

    return 0;
}

