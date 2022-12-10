/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

//
// SVC tool example. SystemC immediate assertion with sct_assert.
//

#include "sct_assert.h"
#include "systemc.h"

class ImmAssert : public sc_module 
{
public:

    sc_in<bool>         clk{"clk"};
    sc_in<bool>         rstn{"rstn"};
    
    sc_in<sc_uint<4>>   a{"a"};
    sc_out<sc_uint<4>>  b{"b"};
    sc_signal<bool>     s;
    sc_signal<bool>     s_d;
    sc_uint<8>          m;

    SC_CTOR(ImmAssert)
    {
        SC_METHOD(sct_assert_method); 
        sensitive << a << b;

        SC_CTHREAD(sct_assert_thread, clk);
        async_reset_signal_is(rstn, false);
    }
    
    // Immediate assertions in method
    void sct_assert_method() 
    {
        sc_uint<4> i = a;
        sct_assert(i == a);

        sct_assert(a != b, "Out equal to input"); 
    }
    
    // Simple immediate assertion test
    void sct_assert_thread() 
    {
        m = 0; b = 0;
        s_d = 0; s = 0;
        wait();
        
        while (true) {
            m++; b = m;
            sct_assert(m != 0);

            s = !s;
            s_d = s;
            wait();

            sct_assert(s != s_d, "Delayed value equal to current");
        }
    }
};


SC_MODULE(Tb) 
{
    sc_in<bool> clk{"clk"};
    sc_signal<bool> rstn{"rstn"};
    sc_signal<sc_uint<4>>  s{"s"};

    ImmAssert assert_mod{"assert_mod"};
            
    SC_CTOR(Tb) {

        assert_mod.clk(clk);
        assert_mod.rstn(rstn);
        assert_mod.a(s);
        assert_mod.b(s);

        SC_CTHREAD(testProc, clk.pos());
    }
    
    void testProc() {
        rstn = 0;
        s = 0;
        wait();
        
        s = 1; 
        rstn = 1;
        wait(10);
        
        cout << "Tests passed" << endl;
        sc_stop();
    }
};

int sc_main(int argc, char **argv) 
{
    sc_clock clk{"clk", 1, SC_NS};

    Tb tb_mod{"tb_mod"};
    tb_mod.clk(clk);
    
    sc_start();

    return 0;
}