/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include <systemc.h>

// Range access of SC type in LHS and RHS
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
        SC_CTHREAD(range_select_use_def, clk.pos());
        async_reset_signal_is(arstn, false);

        SC_CTHREAD(range_select_lhs1, clk.pos());
        async_reset_signal_is(arstn, false);

        SC_CTHREAD(range_select_lhs1a, clk.pos());
        async_reset_signal_is(arstn, false);

        SC_CTHREAD(range_select_lhs2, clk.pos());
        async_reset_signal_is(arstn, false);
 
        SC_CTHREAD(range_select_lhs3, clk.pos());
        async_reset_signal_is(arstn, false);

        SC_CTHREAD(range_select_lhs4, clk.pos());
        async_reset_signal_is(arstn, false);

        SC_CTHREAD(range_select_lhs4a, clk.pos());
        async_reset_signal_is(arstn, false);
        
        SC_CTHREAD(range_select_logic, clk.pos());
        async_reset_signal_is(arstn, false);          

        SC_CTHREAD(range_select_comp_logic, clk.pos());
        async_reset_signal_is(arstn, false);

        SC_CTHREAD(range_select_arithm, clk.pos());
        async_reset_signal_is(arstn, false);

        SC_CTHREAD(range_select_lhs_misc, clk.pos());
        async_reset_signal_is(arstn, false);
    }

      // Write some bits does not lead to defined, values is unknown
    void range_select_use_def()
    {
        sc_uint<5> z;
        wait();
        
        while (true) { 
            z(2,1) = 1;
            sct_assert_defined(z, true);

            z(4,0) = 1;
            sct_assert_defined(z, true);
            wait();
        }
    }
    
    // @range() in LHS with other read/write
    void range_select_lhs1()
    {
        out = 0;
        sc_uint<3> x = 0;
        a = 3;
        wait();
        
        while (true) { 
            x.range(1,0) = 2;       // Extra register for @x
            a(2,1) = x.range(1,0);
            
            out.write(x.range(1,0) + a);
            wait();
        }
    }
    
    // @range() in LHS with other read/write
    void range_select_lhs1a()
    {
        out = 0;
        sc_uint<3> x;
        wait();
        
        while (true) { 
            x(1,0) = 1; x(2,0) = 5; 
            x(2,1) = x(1,0) << 1;
            out = (x(2,1) == in.read()) ? x.range(2,0) + 1 : x(1,1)*2;
            wait();
        }
    }
    
    // No write to @b except via @range()
    void range_select_lhs2()
    {
        out = 0;
        wait();
        
        while (true) { 
            b.range(2,1) = 1;       
            out = b;
            wait();
        }
    }
    
    // No read from @c except via @range()
    void range_select_lhs3()
    {
        c = 0;
        wait();
        
        while (true) { 
            c = 3;
            c.range(3,0) = 1;       
            wait();
        }
    }    
    
    // No read/write to @d except via @range()
    void range_select_lhs4()
    {
        wait();
        
        while (true) { 
            d.range(3,2) = 1;
            wait();
        }
    }
    
     // No read/write to @d except via @range()
    void range_select_lhs4a()
    {
        out = 1;
        wait();
        
        while (true) {
            out = e.range(2,0);
            wait();
        }
    }
    
     // @range() used in logic expression
    void range_select_logic()
    {
        int j = s.read();
        sc_uint<7> x = 0;
        wait();
        
        while (true) { 
            int k = 0;
            x.range(j,j-1) = 1;
            if (x(j, j-1)) k = 1;
            if (x.range(1,0) == 3) k = 2;
            if (x(1,1) || j == 1) {
                if (x(j+2,j) && j == 2) {
                    k = 3;
                }
                k = 4;
            }
            
            bool b = x(1,0) || x(2,1) == 1 && x(3,0) > 0;
            b = x(1,0) || true && b && !(false || x(3,2) || x(3,1) == x(2,0));
            wait();
        }
    }
   
    // @range() used in complex logic expression with &&/||
    void range_select_comp_logic()
    {
        int j = s.read();
        sc_uint<10> x = 0;
        x(5,0) = s.read();
        wait();
        
        while (true) { 
            int k = 0;
            if (true && x(1,0)) k = 1;
            if (true || x(2,0) == 1) k = 2;
            if (false && x(3,3)) k = 3;
            if (false || x(4,2)) k = 4;
            if (false || true && x(5,1) || false) k = 5;
            if (false || true && x(6,1) || true) k = 6;
            wait();
        }
    }
    
    // @range() used in arithmetic expression
    void range_select_arithm()
    {
        int j = s.read();
        sc_uint<20> y = j+1;
        sc_biguint<40> z = j+2;
        wait();
        
        while (true) { 
            sc_uint<16> yy = y.range(2+j,j) + 1;
            yy = y.range(3,0) * yy;
            yy = y.range(10,7) / y.range(2,1) + y.range(5,4);
            yy = y.range(12,10) - y.range(14,10) >> y.range(7,4);

            sc_uint<16> zz;
            zz = y.range(3,0) % z.range(3,0);
            zz = z.range(10,7) / y.range(2,1) + z.range(5,4);
            zz = z.range(14,10) >> j;
            zz = (z.range(14,10) * 3)>> z.range(5,0);
            zz = z.range(30,10) % z.range(5,0);
            wait();
        }
    }
    
    // Various usages of range()
    void range_select_lhs_misc()
    {
        sc_uint<3> x = 0;
        wait();
        
        while (true) { 
            x = in.read();
            
            if (x.range(1,1)) {
                x.range(2,1) = x.range(1,0);
            }
            
            for (int i = 0; i < 3; i++) {
                x.range(i+1, i) = i % 2;
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

