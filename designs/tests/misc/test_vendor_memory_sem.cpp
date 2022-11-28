/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_comb_signal.h"
#include <systemc.h>
#include <string>

typedef sc_uint<8> Addr; 
typedef sc_uint<8> Data; 

// Memory with request/response semantic tests
struct memory_stub : public sc_module
{
    sc_in<bool>     clk{"clk"};
    sc_in<bool>     req{"req"};
    sc_out<bool>    resp{"resp"};

    std::string __SC_TOOL_VERILOG_MOD__ = "";
    std::string __SC_TOOL_MODULE_NAME__;
    
    SC_HAS_PROCESS(memory_stub);
    
    explicit memory_stub(const sc_module_name& name, 
                         const char* verilogName = "") :
        __SC_TOOL_MODULE_NAME__(verilogName)
    {
        SC_METHOD(asyncResp);
        sensitive << req;
        //SC_CTHREAD(syncResp, clk.pos());
    }
    
    void asyncResp() {
        resp = req;
    }

    void syncResp() {
        resp = 0;
        wait();
        
        while (true) {
            resp = req;
            wait();
        }
    }
};

struct memory_wrapper : public sc_module, sc_interface 
{
    sc_in<bool>             clk{"clk"};
    sc_in<bool>             nrst{"nrst"};
    sct_comb_signal<bool>   req{"req"};
    sc_signal<bool>         resp{"resp"};

    // Signal to check name collision
    sc_signal<bool>         req_next{"req_next"};
    
    memory_stub             stub {"stub", "specific_name"};

    SC_CTOR(memory_wrapper) 
    {
        stub.clk(clk);
        stub.req(req);
        stub.resp(resp);
    }
    
    void clear_req() {
        req = 0;
        req_next = 1;
    }
    
    void read_req() {
        req = (nrst) ? 1 : 0;
    }
    
    bool read_resp() {
        return resp;
    }
};

struct module_with_memory : public sc_module
{
    sc_in<bool>         clk{"clk"};
    sc_in<bool>         nrst{"nrst"};
    memory_wrapper      mem{"mem"};
    
    SC_CTOR(module_with_memory) 
    {
        mem.clk(clk);
        mem.nrst(nrst);
        SC_CTHREAD(proc, clk.pos());
        async_reset_signal_is(nrst, 0);
    }
    
    void proc() {
        mem.clear_req();
        wait();             // 0
        
        while (true) {
            mem.read_req();
            wait();         // 1
                        
            mem.read_resp();
            mem.clear_req();
            wait();         // 2
        }
    }
};

struct testbench : public sc_module
{
    sc_in<bool>         clk{"clk"};
    sc_signal<bool>     nrst{"nrst"};
    module_with_memory  mod_mem{"mod_mem"};
    
    SC_CTOR(testbench) 
    {
        mod_mem.clk(clk);
        mod_mem.nrst(nrst);
        SC_CTHREAD(test, clk.pos());
    }
    
    void test() {
        nrst = 0;
        wait(5); 
        
        nrst = 1;
        wait(10);   
        
        sc_stop();
    }
};


int sc_main(int argc, char **argv) {

    sc_clock clock_gen{"clock_gen", 1, SC_NS};
    testbench tb{"tb"};
    tb.clk(clock_gen);
    sc_start();

    return 0;
}

