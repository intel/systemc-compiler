/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Loop with return from function
class top : sc_module
{
public:
    sc_in_clk clk;
    sc_signal<bool> arstn;

    SC_HAS_PROCESS(top);
    top(sc_module_name)
    {
        SC_CTHREAD(test_thread, clk.pos());
        async_reset_signal_is(arstn, false);
    }

    sc_vector<sc_signal<int>> s{"s", 4};
    
    // Example from DME
    int getNextLdSlot(int curSlot)
    {
        while (true) {
            int nextSlot = curSlot;
            for (unsigned i = 0; i < 4; ++i) {
                if (!s[nextSlot])
                    return nextSlot;
                
                nextSlot = (nextSlot == 4 - 1) ? 0 : nextSlot + 1;
            }
            wait();
        }
    }    
    
    void test_thread()
    {
        int slot = 0;
        wait();
        
        while (1) 
        {
            slot = getNextLdSlot(slot);
            wait();
        }
    }
};

int sc_main(int argc, char *argv[])
{
    sc_clock clk{"clk", 10, SC_NS};
    top top_inst{"top_inst"};
    top_inst.clk(clk);
    sc_start(100, SC_NS);
    return 0;
}

