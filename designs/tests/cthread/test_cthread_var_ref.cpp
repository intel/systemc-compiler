/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Register, combinational and read-only reference variables/records and 
// pointers in CTHREAD
class top : sc_module
{
public:
    sc_in_clk clk;
    sc_signal<bool> rstn;

    sc_uint<2>  arr[2];
    sc_uint<2>  arr1[2];
    sc_uint<3>  arr2[3][4];
    
    sc_uint<3>  m;
    const int* cp;
    int* p;
    sc_uint<4>* q;
    
    SC_HAS_PROCESS(top);
    top(sc_module_name)
    {
        cp = sc_new<int>(42);
        p = sc_new<int>();
        q = sc_new<sc_uint<4>>();
        
        SC_CTHREAD(local_ref_reset, clk.pos());
        async_reset_signal_is(rstn, 0);
        
        SC_CTHREAD(local_const_ref, clk.pos());
        async_reset_signal_is(rstn, 0);
        
        SC_CTHREAD(local_const_ref_arr, clk.pos());
        async_reset_signal_is(rstn, 0);
        
        SC_CTHREAD(local_ref1, clk.pos());
        async_reset_signal_is(rstn, 0);

        SC_CTHREAD(local_ref2, clk.pos());
        async_reset_signal_is(rstn, 0);
    
        SC_CTHREAD(local_ptr1, clk.pos());
        async_reset_signal_is(rstn, 0);

        SC_CTHREAD(local_record1, clk.pos());
        async_reset_signal_is(rstn, 0);
       
        SC_CTHREAD(local_record_ref, clk.pos());
        async_reset_signal_is(rstn, 0);
        
        SC_CTHREAD(local_record_ref_reset, clk.pos());
        async_reset_signal_is(rstn, 0);
    }
    
    // Local reference to variable declared in reset
    sc_signal<int> s0;
    void local_ref_reset()
    {
        int i = 42;         // reg   
        wait();
        
        while (true) {
            int& r = i;
            int j = i+1;    // comb
            s0 = j;
            wait();
        }
    }
    
    // Constant reference initialization 
    void local_const_ref()
    {
        int i = 42;         // reg   
        wait();
        
        while (true) {
            const int& cr = i+1; // comb
            int j = cr;          
            const int& cr2 = j;  // not declared
            s0 = cr2;
            const sc_int<34>& cr3 = j;  // comb
            s0 = cr3;
            const sc_biguint<34>& cr4 = 42;  // comb
            s0 = cr4.to_int();
            wait();
        }
    }
    
    sc_signal<int> s00;
    void local_const_ref_arr()
    {
        int i[2] = {42, 43};         // reg   
        wait();
        
        while (true) {
            sc_uint<12> x[2] = {44, 45};
            const int& cra = i[0];  // reg, not used, ignore for now 
            s00 = cra;          
            const int& cra2 = i[1]+1;  // comb
            s00 = cra2;
            const sc_uint<12>& cra3 = x[1];  // not declared
            s00 = cra3;
            const sc_uint<12>& cra4 = x[1]+1;  // comb
            s00 = cra4;
            wait();
        }
    }
    
    // Local reference in reset
    sc_signal<int> s1;
    void local_ref1()
    {
        int i = 42;         // comb
        int& r1 = i;
        int j = r1+1;       // reg
        wait();
        
        while (true) {
            int& r2 = i;
            s1 = j;
            wait();
        }
    }
    
    // Local reference in code
    sc_signal<int> s2;
    void local_ref2()
    {
        int i = 42;         // reg
        wait();
        
        while (true) {
            int& r1 = i;
            int j = i+1;   // reg
            int& r2 = j;
            s2 = j;
            wait();
            
            s2 = r2+1;
        }
    }
    
// --------------------------------------------------------------------------
    // Pointers
    
    sc_signal<int> s3;
    void local_ptr1()
    {
        const int A = 42;
        int i = 43;       
        int* lp = p;            // reg
        *p = 0;
        wait();
        
        while (true) {
            const int B = *cp;
            int* llp = lp;
            s3 = *lp + A + B;
            wait();
            
            *q = s2.read();     // comb
            if (*q) {
                (*q)--;
            }
            (*lp)++;
            s3 = *q;
        }
    }
    
    
// --------------------------------------------------------------------------
    // Records
    
    struct Simple {
        bool a;
        int b;
        Simple() = default;
        Simple(int par) : a(false), b(par) {}
    };
    
    // Record defined and constructor parameter used
    sc_signal<int> s5;
    void local_record1()
    {
        int i = 42;         // reg   
        wait();
        
        while (true) {
            Simple rec(i);  // reg
            wait();
            
            s5 = rec.b;
        }
    }
    
    // Record reference
    sc_signal<int> s6;
    void local_record_ref()
    {
        wait();
        
        while (true) {
            Simple rec1;     // comb
            Simple rec2;     // reg
            Simple& rr1 = rec1; 
            Simple& rr2 = rec2; 
            s6 = rr1.b;
            wait();
            s6 = rr2.b;
        }
    }
    
    // Reset record reference
    sc_signal<int> s7;
    void local_record_ref_reset()
    {
        Simple rec1;        // reg
        Simple rec2(42);    // reg
        wait();
        
        while (true) {
            Simple& rr1 = rec1; 
            Simple& rr2 = rec2; 
            s7 = rr1.b;
            wait();
            s7 = rr2.b + 1;
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

