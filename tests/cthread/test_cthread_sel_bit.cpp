/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include <systemc.h>

// Bit access of SC type in LHS and RHS
class top : sc_module
{
public:
    sc_in_clk clk;
    sc_signal<bool>     arstn{"arstn", 1};
    sc_signal<int>      in{"in"};
    sc_signal<bool>     in2{"in2"};
    sc_signal<int>      out{"out"};

    sc_uint<3>  a;
    sc_uint<4>  b;
    sc_uint<5>  c;
    sc_uint<6>  d;
    sc_uint<7>  e;
    
    sc_signal<sc_uint<4>> s;
    sc_signal<bool> sb;
    
    SC_HAS_PROCESS(top);
    top(sc_module_name)
    {
        SC_CTHREAD(bit_select_use_def, clk.pos());
        async_reset_signal_is(arstn, false);
        
        SC_CTHREAD(bit_select_use_def, clk.pos());
        async_reset_signal_is(arstn, false);

        SC_CTHREAD(bit_select_lhs1, clk.pos());
        async_reset_signal_is(arstn, false);

        SC_CTHREAD(bit_select_lhs1a, clk.pos());
        async_reset_signal_is(arstn, false);

        SC_CTHREAD(bit_select_lhs2, clk.pos());
        async_reset_signal_is(arstn, false);
 
        SC_CTHREAD(bit_select_lhs3, clk.pos());
        async_reset_signal_is(arstn, false);

        SC_CTHREAD(bit_select_lhs4, clk.pos());
        async_reset_signal_is(arstn, false);

        SC_CTHREAD(bit_select_lhs4a, clk.pos());
        async_reset_signal_is(arstn, false);
          
        SC_CTHREAD(bit_select_logic, clk.pos());
        async_reset_signal_is(arstn, false);          

        SC_CTHREAD(bit_select_comp_logic, clk.pos());
        async_reset_signal_is(arstn, false);
        
        SC_CTHREAD(bit_select_lhs_misc, clk.pos());
        async_reset_signal_is(arstn, false);
    }

    // Write some bits does not lead to defined, values is unknown
    void bit_select_use_def()
    {
        sc_uint<5> z;
        wait();
        
        while (true) { 
            z[1] = 1;
            sct_assert_defined(z, true);
            wait();
        }
    }

    // @bit() in LHS with other read/write
    void bit_select_lhs1()
    {
        out = 0;
        sc_uint<3> x = 0;
        a = 3;
        wait();
        
        while (true) { 
            x.bit(1) = 1;       // Extra register for @x
            a[2] = x[1];
            
            out.write(x[1] + a);
            wait();
        }
    }
    
    // @bit() in LHS with other read/write
    void bit_select_lhs1a()
    {
        out = 0;
        sc_uint<3> x;
        wait();
        
        while (true) { 
            x[0] = 1; x[1] = 0; 
            x[2] = x[1] != in2;
            out = (x[2] == in2) ? x.bit(1) + 1 : x[0]*2;
            wait();
        }
    }
    
    // No write to @b except via @bit()
    void bit_select_lhs2()
    {
        out = 0;
        wait();
        
        while (true) { 
            b.bit(2) = 1;       
            out = b;
            wait();
        }
    }
    
    // No read from @c except via @bit()
    void bit_select_lhs3()
    {
        c = 0;
        wait();
        
        while (true) { 
            c = 3;
            c.bit(2) = 1;       
            wait();
        }
    }    
    
    // No read/write to @d except via @bit()
    void bit_select_lhs4()
    {
        wait();
        
        while (true) { 
            d.bit(0) = 1;
            wait();
        }
    }
    
     // No read/write to @d except via @bit()
    void bit_select_lhs4a()
    {
        out = 1;
        wait();
        
        while (true) {
            out = e.bit(2);
            wait();
        }
    }
    
    // @bit() used in logic expression
    void bit_select_logic()
    {
        int j = s.read();
        sc_uint<7> x = 0;
        wait();
        
        while (true) { 
            int k = 0;
            x.bit(j) = 1;
            if (x[j]) k = 1;
            if (x.bit(j+1)) k = 2;
            if (x[1] || j == 1) {
                if (x[j] && j == 2) {
                    k = 3;
                }
                k = 4;
            }
            
            bool b = x[1] || x[2] && x[3] || !x[4];
            b = x[1] || true && b && !(false || x[5] || x[6]);
            wait();
        }
    }
   
    // @bit() used in complex logic expression with &&/||
    void bit_select_comp_logic()
    {
        int j = s.read();
        sc_uint<3> x = 0;
        x[1] = sb.read();
        wait();
        
        while (true) { 
            int k = 0;
            if (true && x[1]) k = 1;
            if (true || x[2]) k = 2;
            
            bool b = true && x[1];
            b = true || x[2];
            
            wait();
        }
    }
    
    // Various usages of bit()
    void bit_select_lhs_misc()
    {
        sc_uint<3> x = 0;
        wait();
        
        while (true) { 
            x = in.read();
            
            if (x.bit(1)) {
                x.bit(2) = x.bit(0);
            }
            
            for (int i = 0; i < 3; i++) {
                x.bit(i) = i % 2;
            }
            
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

