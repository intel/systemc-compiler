/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Single and multiple returns in function, including return in if and switch
struct A : sc_module, sc_interface
{
    sc_signal<sc_uint<8>> s;   
    SC_CTOR(A)
    {}
    
    sc_uint<8> read(unsigned addr) {
        return s.read() + addr;
    }
};

SC_MODULE(test) 
{
    sc_in_clk clk;
    sc_signal<bool> rst;
    sc_signal<sc_uint<4>> s;

    A* a_mif;
    bool* arrp[10];
    
    SC_CTOR(test) 
    {
        a_mif = new A("a_mif");
        
        for (int i = 0; i < 10; i++) {
            arrp[i] = sc_new<bool>(); 
        }
        
        SC_CTHREAD(return_func1, clk.pos());
        async_reset_signal_is(rst, 0);
        
        SC_CTHREAD(mif_method_call, clk.pos());
        async_reset_signal_is(rst, 0);
        
        SC_CTHREAD(return_func2, clk.pos());
        async_reset_signal_is(rst, 0);

        SC_CTHREAD(return_func3, clk.pos());
        async_reset_signal_is(rst, 0);

        SC_CTHREAD(return_func4, clk.pos());
        async_reset_signal_is(rst, 0);
    }
    
    // Return bit/range from function
    sc_uint<1> f1(unsigned bitIndx) {
        sc_uint<4> res = s.read();
        return (sc_uint<1>)res.bit(bitIndx-1);
    }
    
    sc_uint<4> f2(unsigned rangeLo) {
        sc_uint<16> res = s.read();
        return res.range(rangeLo+4, rangeLo);
    }
    
    sc_signal<int> s0;
    void return_func1() 
    {
        wait();
        while (true) 
        {
            int i = f1(s.read());
            i = f2(s.read()).bit(1);
            s0 = i;
            wait();
        }
    }
    
    
    // Call MIF method in return
    sc_uint<8> read(unsigned addr) 
    {
        return a_mif->read(addr);
    }
    
    sc_signal<int> s1;
    void mif_method_call() 
    {
        wait();
        while (true) 
        {
            s1 = read(s.read());
            wait();
        }
    }

    // Function with return called in do..while
    sc_signal<bool> t;
    bool f5(sc_uint<4> blockIndx)
    {
        return !(*this->arrp[blockIndx]) || t;
    }
    
    void ff5(sc_uint<4> blockIndx) 
    {
        bool A;
        do {
            wait();
            A = f5(blockIndx);
            
        } while(!A);
    }
    
    void return_func2() 
    {
        wait();
        while (true) {
            
            ff5(s.read());
            wait();
        }
    }
    
// ---------------------------------------------------------------------------    

    template <class T>
    T ff6(T& par) {
        if (par > s1.read()) {
            return ++par;
        } else {
            return par;
        }
    }
    
    // Return in IF
    sc_signal<int> s3;
    void return_func3() 
    {
        wait();
        while (true) {

            int i = 42;
            if (s.read()) {
                ff6(i);
            } else {
                i = ff6(i);
            }
            s3 = i;
            wait();
        }
    }
    
    
// ---------------------------------------------------------------------------    

    template <class T>
    T ff7(T par) {
        switch (par) {
            case 0: return 1;
            case 1: return s.read();
            default: return 0;
        }
    }
    
    // Return in switch
    sc_signal<int> s4;
    void return_func4() 
    {
        wait();
        while (true) {
            s4 = ff7(s.read());
            wait();
        }
    }
    
// ---------------------------------------------------------------------------    
    
};

int sc_main(int argc, char **argv) {
    sc_clock clk("clk", 1, SC_NS);
    test t_inst{"t_inst"};
    t_inst.clk(clk);
    sc_start();
    return 0;
}

