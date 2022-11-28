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

// Tests for pointer to Module
struct A: public sc_module, sc_interface 
{
    sc_in_clk           clk;
    sc_signal<bool>     nrst;
    sc_signal<bool>     dummy;

    SC_CTOR(A){
        SC_METHOD(methProcA);
        sensitive << dummy;
        
        SC_CTHREAD(threadProcA, clk.pos());
        async_reset_signal_is(nrst, false);
    }
    
    sc_uint<4> var1;
    sc_uint<4> var2;
    sc_uint<4> var3;
    sc_uint<4> var4;

    sc_uint<4> getvar() {
        return var1;
    }

    void methProcA()
    {
        sc_uint<4> i = 0;
        i++;
        var2 = i;
        var3 = i + var2;
    }

    void threadProcA()
    {
        var4 = 0;
        wait();
        
        while(true){
            var4++;
            wait();
        }

    }
};

struct B: public sc_module
{
    sc_in_clk           clk;
    sc_signal<bool>     nrst;
    sc_signal<bool>     dummy;
    
    SC_HAS_PROCESS(B);
    
    explicit B(const sc_module_name& name) : sc_module(name) 
    {
        SC_METHOD(methProcB);
        sensitive << dummy;
        
        //SC_CTHREAD(threadProcB, clk.pos());
        //async_reset_signal_is(nrst, false);
    }

    void methProcB(){
        int i = 0;
    }

    void threadProcB(){
        wait();
        while(true){
            wait();
        }
    }
};

struct Dut : public sc_module 
{
    sc_in_clk           clk;
    sc_signal<bool>     nrst;
    sc_signal<bool>     dummy;

    A*  a1;
    A*  a2;
    
    B* BInst;

    SC_CTOR(Dut)
    {
        a1 = new A("a1");
        a1->clk(clk);

        a2 = new A("a2");
        a2->clk(clk);
        
        BInst = new B("BInst");
        BInst->clk(clk);

        SC_METHOD(methProcD);
        sensitive << dummy;
         
        SC_CTHREAD(threadProcD, clk.pos());
        async_reset_signal_is(nrst, false);
    }

    void methProcD()
    {
        sc_uint<4> i = a1->getvar();
        i = a2->getvar();
    }

    void threadProcD()
    {
        while(true) {
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

