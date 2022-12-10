/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Various types in for loop statement
class top : sc_module
{
public:
    sc_clock clk{"clk", 10, SC_NS};
    sc_signal<bool> arstn{"arstn", 1};
    sc_signal<int> in{"in"};
    sc_signal<int> out{"out"};

    SC_HAS_PROCESS(top);
    top(sc_module_name)
    {
        SC_THREAD(for_stmt_other_types_fns);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
    }

    // ------------------------------------------------------------------------
    // For DAC paper
    sc_signal<int> enabled{"enabled"};
    sc_signal<int> reset{"reset"};
    
    void thread_break() {
        wait();          // STATE 0
        while (true) {
            wait();      // STATE 1
            while (!enabled) {
                if (reset) break;
                wait();  // STATE 2
            }
        }
    }

    // ------------------------------------------------------------------------

    void for_stmt_other_types_fns()
    {
        wait();
    	while (true) {
            for_stmt_no_wait(sc_uint<10>(0));
            for_stmt_no_wait(sc_int<20>(0));
            for_stmt_no_wait(sc_bigint<40>(0));
            for_stmt_no_wait(sc_biguint<55>(0));

            for_stmt_wait0(sc_uint<15>(0));
            for_stmt_wait0(sc_int<25>(0));
            for_stmt_wait0(sc_bigint<35>(0));
            for_stmt_wait0(sc_biguint<45>(0));

            for_stmt_wait1(sc_uint<5>(0));
            for_stmt_wait1(sc_int<15>(0));
            for_stmt_wait1(sc_bigint<25>(0));
            for_stmt_wait1(sc_biguint<56>(0));

            for_stmt_wait2(sc_uint<14>(0));
            for_stmt_wait2(sc_int<23>(0));
            for_stmt_wait2(sc_bigint<33>(0));
            for_stmt_wait2(sc_biguint<60>(0));

            for_stmt_wait_noiter(sc_uint<8>(0));
            for_stmt_wait_noiter(sc_int<31>(0));
            for_stmt_wait_noiter(sc_bigint<33>(0));
            for_stmt_wait_noiter(sc_biguint<64>(0));

            wait();
    	}
    }

    template<typename T1>
    void for_stmt_no_wait(T1 par1)
    {
    	T1 i = par1;
        int k = 0;
        for (i = 0; i < 2; i++) {
            k = i.to_int();
        }
    }
    
    template<typename T1>
    void for_stmt_wait0(T1 par1)
    {
    	T1 i = par1;
        int k = 0;
        k = 1;                          // B6
        for (i = 0; i < 2; i++) {   // B5
            k = i.to_int()+2;                      // B4
        }                               // B3
            k = 3;                          // B2
    }
    
    template<typename T1>
    void for_stmt_wait1(T1 par1)
    {
    	T1 i = par1;
    	int k = 0;
        for (i = 0; i < 2; i++) {
             k = 1;
        }
        k = 2;
    }

    template<typename T1>
    void for_stmt_wait2(T1 par1)
    {
    	T1 i = par1;
    	int k = 0;
        for (i = 0; i < 2; i++) {
            k = 1;
        }
        k = 2;
        k = 3;
    }
    
    // For with wait() no iteration
    // Not supported yet
    template<typename T1>
    void for_stmt_wait_noiter(T1 par1)
    {
    	T1 i = par1;
    	int k = 0;
        k = 1;
        for (i = 0; i < 0; i++) {
            k = 2;
        }
        k = 3;
    }
};


int sc_main(int argc, char *argv[])
{
    top top_inst{"top_inst"};
    sc_start(100, SC_NS);
    return 0;
}

