/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by htrampur on 07/19/19
//

#include <systemc.h>

// Tests for combinational variables in MIF threads
struct A: public sc_module, sc_interface 
{
    sc_in_clk               clk;
    sc_signal<bool>         rst;

    sc_signal<sc_uint<4>>   s {"s"};
    sc_signal<sc_uint<4>>   ss {"ss"};
    sc_uint<4>              v;
    sc_uint<4>              vv;
    
    SC_CTOR(A)
    {
        SC_CTHREAD(thread1, clk.pos());
        async_reset_signal_is(rst, 1);

        SC_CTHREAD(thread2, clk.pos());
        async_reset_signal_is(rst, 1);
    }
    
    // Thread with combinational variable
    void thread1()
    {
        s = 1;
        v = 2;
        wait();
        
        while (true) {
            v = 3;
            s = 4;
            auto a = v + s.read();
            wait();
        }
    }
    
    void thread2()
    {
        wait();
        
        while (true) {
            auto a = ss.read();
            wait();
        }
    }
};

struct Dut : public sc_module 
{
    sc_in_clk           clk;
    sc_signal<bool>     rst;
    
    A*  a2;

    SC_CTOR(Dut)
    {
        a2 = new A("a2");
        a2->clk(clk);
        
        SC_CTHREAD(top_thread, clk.pos());
        async_reset_signal_is(rst, 1);
    }
    
    void top_thread() 
    {
        a2->ss = 1;
        a2->vv = 2; 
        wait();
        
        while (true) {
            a2->ss = 3;
            a2->vv = 4;
            auto b = a2->vv + a2->ss.read();
            wait();
        }
    }
};

int sc_main(int argc, char **argv) 
{
    Dut dut{"dut"};
    sc_clock clk("clk", 1, SC_NS);
    dut.clk(clk);

    sc_start();
    return 0;
}

