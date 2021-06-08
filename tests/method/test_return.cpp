/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Single and multiple returns in function, including return in if and switch
SC_MODULE(test) 
{
    sc_signal<sc_uint<3>> s;
    sc_signal<bool> t;

    bool* arrp[10];
    
    SC_CTOR(test) 
    {
        for (int i = 0; i < 10; i++) {
            arrp[i] = sc_new<bool>(); 
        }
        
        
        SC_METHOD(return_method1);
        sensitive << s;

        SC_METHOD(return_func1);
        sensitive << s;

        SC_METHOD(return_func2);
        sensitive << s;

        SC_METHOD(return_func3);
        sensitive << s;

        SC_METHOD(return_func4);
        sensitive << t << s;
        
        SC_METHOD(return_empty);
        sensitive << t << s;

        SC_METHOD(return_switch1);
        sensitive << s;
        
        SC_METHOD(return_switch2);
        sensitive << s;
    }

    // Return from method function
    void return_method1() {
        int x = 0;
        if (s.read()) {
            return;
        } else {
            x = 1;
        }
    }
    
// ---------------------------------------------------------------------------    
    
    // Normal return from function
    sc_uint<4> f1() {
        sc_uint<4> res = s.read();
        return res;
    }
    
    // Function call in return
    bool f2() {
        return (s.read() == 1 || f1());
    }
    
    void return_func1() 
    {
        auto a = f1();
        bool b = f2();
    }
    
    // Return parameter by value/reference return 
    sc_int<8> f3(sc_int<8> par1, sc_int<8>& par2, const sc_int<8>& par3) 
    {
        if (s.read() == 1) {
            return par1;
        } else 
        if (s.read() == 2) {
            if (s.read() == 3) {
                if (s.read() == 4) {
                    return 1;
                } else {
                    return 2;
                }
            } else 
                return par2;
        } else {
            return par3;
        }
    }
    
   
    void return_func2() 
    {
        sc_int<8> val1, val2;
        int c = f3(val1, val2, val1);
    }
        
    sc_uint<8> f4(sc_uint<8> par1, sc_uint<8>& par2, const sc_uint<8>& par3) 
    {
        int k = 0;
        if (s.read() == 1) {
            return par1;
        } else 
        if (s.read() == 2) {
            k = 1;
            return (par3+1);
        } else 
        if (s.read() == 3) {
            k = 2;
            return par2;
        } else {
            return par3;
        }
    }
    
    void return_func3() 
    {
        sc_uint<8> val3, val4;
        unsigned d = f4(val3, val3, val4);
    }
    
    // Assert before return and conditional in return
    bool f5() {
        if (s.read()) {
            assert (s.read() == 1);
            return t.read();
        } else
        if (t.read()) {
            assert (false);
            return t.read();
        } else 
            return (s.read() != 0 ? 1 : t.read());
    }

    void return_func4() 
    {
        auto i = f5();
    }

    
    // Empty return
    void f6(bool par) {
        int k;
        if (s.read()) {
            k = 0;
            return;
        } else
        if (t.read()) {
            k = 1;
            if (par) {
                k = 2;
            }
            return;
        } else 
            k = 3;
    }

    void return_empty() 
    {
        f6(t.read());
    }

    
// ---------------------------------------------------------------------------    
    
    // @return from switch
    unsigned sw1() 
    {
        switch (s.read()) {
            case 1 : return 2;
            case 2 : return 3;
            default: return 0;
        }
    }
    
    void return_switch1() {
        unsigned i = sw1();
    }
    
    unsigned sw2(sc_uint<8> par1, sc_uint<8>& par2) 
    {
        int k = 0;
        switch (par1) {
            case 1 : return par1;
            case 2 : k = 1; return par2;
            case 3 :;
            default: k = 2; return par1+par2;
        }
    }
    
    void return_switch2() {
        sc_uint<8> val1 = s.read();
        sc_uint<8>  val2;
        auto i = sw2(val1+1, val2);
    }

};

int sc_main(int argc, char **argv) {
    test t_inst{"t_inst"};
    sc_start();
    return 0;
}

