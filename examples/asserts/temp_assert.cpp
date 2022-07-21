/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

//
// SystemC temporal assertion SCT_PROPERTY and SCT_ASSERT.
//

#include "sct_assert.h"
#include "systemc.h"

class TempAssert : public sc_module 
{
public:

    sc_in<bool>         clk{"clk"};
    sc_in<bool>         rstn{"rstn"};
    
    sc_in<sc_uint<4>>   a{"a"};
    sc_out<sc_uint<4>>  b{"b"};
    sc_signal<bool>     s;
    sc_signal<bool>     s_d;
    sc_signal<bool>     s_d2;
    sc_uint<8>          m;

    const unsigned N = 3;

    SC_CTOR(TempAssert)
    {
        // Assertions in clock threads
        SC_CTHREAD(sct_assert_sig, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(sct_assert_var, clk.pos());
        async_reset_signal_is(rstn, false);
    }
    
    // Assertions in module scope, clock event required
    SCT_ASSERT(s, SCT_TIME(0), s, clk);  
    SCT_ASSERT(s, SCT_TIME(N+1), s_d, clk.pos());
    // Assertion can be disabled when reset active (reset is active low)
    SCT_ASSERT(rstn && (s || s_d), SCT_TIME(1,2), s_d2, clk.neg());

    // Assertion in clocked process
    void sct_assert_sig() 
    {
        s = 0; s_d = 0; s_d2 = 0;
        // Assertion in reset section works during reset if not disabled
        SCT_ASSERT_THREAD(s, SCT_TIME(1), s_d, clk.pos());
        // Assertion can be disabled when reset active (reset is active low)
        SCT_ASSERT_THREAD(rstn && s, SCT_TIME(1), s_d, clk.pos());
        
        wait();

        // Assertion after reset section does not work under reset 
        SCT_ASSERT_THREAD(s || s_d, SCT_TIME(1,N), s_d2, clk.pos());
        
        while (true) {
            s_d = s; s_d2 = s_d;
            s = !s;
            
            wait();
        }
    }
    
    // Assertions with member/local variables
    void sct_assert_var()
    {
        int i = 0; m = 1;
        wait();

        // Assertion in process can use member and local variables 
        SCT_ASSERT_THREAD(m != i, SCT_TIME(1), s, clk.pos());

        while (true) {
            m = i+1; i++;
            wait();
        }
    }
};


SC_MODULE(Tb) 
{
    sc_in<bool> clk{"clk"};
    sc_signal<bool> rstn{"rstn"};
    sc_signal<sc_uint<4>>  s{"s"};

    TempAssert assert_mod{"assert_mod"};
            
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