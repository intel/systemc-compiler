/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Combinational and register arrays and pointers in CTHREAD
class top : sc_module
{
public:
    sc_clock clk{"clk", 10, SC_NS};
    sc_signal<bool> arstn{"arstn", 1};
    sc_signal<int> in{"in"};
    sc_signal<int> out{"out"};

    sc_uint<2>  arr[2];
    sc_uint<2>  arr1[2];
    sc_uint<2>  arr2[2];
    sc_uint<2>  arr3[2];
    sc_uint<3>  arr4[3][4];
    sc_uint<3>  arr5[3][4];
    
    sc_uint<3>  a;
    sc_uint<4>  b;
    sc_uint<5>  c;
    sc_uint<6>  d;
    
    const sc_uint<7>  e = 42;
    sc_uint<8>  f = 43;
    
    sc_uint<9>* p;
    sc_uint<9>* q;
    
    SC_HAS_PROCESS(top);
    top(sc_module_name)
    {
        p = sc_new<sc_uint<9> >();
        q = sc_new<sc_uint<9> >();
                
        SC_THREAD(comb_arr_in_reset);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(comb_arr_in_reset1);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(comb_arr_in_reset1a);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(comb_arr_in_reset2);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(comb_arr_in_reset2D);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(comb_ptr_in_reset);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(init_list1);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(init_list2);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
    }

    // Combinational array with initialization in reset
    // No read from array -- combinational array
    void comb_arr_in_reset()
    {
        arr[0] = 0;
        wait();
        
        while (true) {
            arr[1] = 3;
            wait();
        }
    }
    
    // Read from array -- register array
    void comb_arr_in_reset1()
    {
        arr1[0] = 0;
        wait();
        
        while (true) {
            arr1[1] = 3;
            out = arr1[0];      // @arr1 is reg
            wait();
        }
    }

    void comb_arr_in_reset1a()
    {
        arr2[0] = 0; arr2[1] = 1;
        wait();
        
        while (true) {
            wait();
            out = arr2[1];      // @arr2 is reg
        }
    }
    
    // Read from array -- comb array
    void comb_arr_in_reset2()
    {
        arr3[0] = 0;
        wait();
        
        while (true) {
            arr3[0] = 3;
            out = arr3[0];      // @arr3 is comb
            wait();
        }
    }
    
    // No read from 2D array -- combinational array
    sc_signal<int> r3;
    void comb_arr_in_reset2D()
    {
        arr4[0][1] = 0;
        arr5[0][1] = 0;
        wait();
        
        while (true) {
            arr4[1][2] = 3;
            arr5[1][2] = 3;
            r3 = arr4[1][2];    // @arr4 is comb
            wait();
            r3 = arr5[1][2];    // @arr5 is reg
        }
    }
    
    // Combinational pointer with initialization in reset
    sc_signal<int> r4;
    void comb_ptr_in_reset()
    {
        *p = 0;
        wait();
        
        while (true) {
            *p = 3;
            *q = 4;
            r4 = *p + 1;        // @p is comb
            wait();
            
            r4 = *q + 1;        // @q is reg
        }
    }
    
    // Initialization list
    int n;
    void init_list1() {
        n = 0;
        int m = 1;
        // l1 is register
        int l1[2] = {0, m};
        wait();
        
        while (true) {
            // l2 is not register
            int l2[2] = {m, n};
            out = l1[1] + l2[0];
            
            int l3[2] = {1, 2}; // Comb
            int l4[2] = {3, 4}; // Reg
            wait();
            
            l3[1] = 2; l4[1] = 4;
            out = l3[1] + l4[0];
            n = m;
        }
    }

    void init_list2() {
        // ll1 is not register
        int ll1[2] = {0, 1};
        int j = ll1[1];
        wait();
        
        while (true) 
        {
            // ll2 is not register
            int ll2[3] = {3, 2, j};
            out = ll2[2];
            wait();
        }
    }
};

int sc_main(int argc, char *argv[])
{
    top top_inst{"top_inst"};
    
    sc_start(100, SC_NS);
    return 0;
}

