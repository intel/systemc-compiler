/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Test for concat() access in LHS
class top : sc_module
{
public:
    sc_clock clk{"clk", 10, SC_NS};
    sc_signal<bool>     arstn{"arstn", 1};
    sc_signal<int>      in{"in"};
    sc_signal<bool>     in2{"in2"};
    sc_signal<int>      out{"out"};
    sc_signal<int>      out1{"out1"};
    sc_signal<int>      out2{"out2"};
    sc_signal<int>      out3{"out3"};

    sc_uint<3>  a;
    sc_uint<4>  b;
    sc_uint<5>  c;
    sc_uint<6>  d;
    sc_uint<7>  e;
    
    SC_HAS_PROCESS(top);
    top(sc_module_name)
    {
        SC_THREAD(concat_lhs1);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(concat_lhs1a);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(concat_lhs2);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(concat_lhs3);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(concat_lhs4);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
        
        SC_THREAD(concat_lhs_rhs);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(concat_lhs_rhs2);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
        
        SC_THREAD(concat_lhs_rhs3);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
        
        SC_THREAD(concat_lhs_multi);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
        
        SC_THREAD(concat_lhs_range);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
        
        SC_THREAD(concat_lhs_cast);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
    }
    
    // @concat() in LHS with other read/write
    void concat_lhs1()
    {
        out = 0;
        sc_uint<3> x = 0;       // extra registers for @a and @x
        a = 3;
        wait();
        
        while (true) { 
            (x, a) = 42;       
            out.write(x + a);
            wait();
        }
    }
    
    // @concat() in LHS with other read/write
    void concat_lhs1a()
    {
        out1 = 0;
        sc_uint<3> x;           // extra registers for @y and @x
        sc_uint<2> y;
        wait();
        
        while (true) { 
            (x, y) = 42;    
            x = y + 1; 
            out1.write(x + y);
            wait();
        }
    }
    
    // No write to @b except via @concat()
    void concat_lhs2()
    {
        out2 = 0;
        sc_uint<3> x;       // extra registers for @b and @x
        sc_uint<4> l;
        (x, l) = 42;
        wait();
        
        while (true) { 
            (b, x) = l;     // register for @l
            out2 = b;
            wait();
        }
    }
    
    // No read from @c except via @concat()
    void concat_lhs3()
    {
        c = 0;
        sc_uint<3> x;       // extra registers for @x
        wait();
        
        while (true) { 
            c = 3;
            concat(x, c) = 1;       
            wait();
        }
    }    
    
    // No read/write to @d except via @concat()
    void concat_lhs4()
    {
        sc_uint<3> x;       // extra registers for @d
        wait();
        
        while (true) { 
            x = 1;
            concat(d,x) = in.read();
            wait();
        }
    }
    
    // Registers for concat variables
    void concat_lhs_rhs()
    {
        sc_uint<3> x = 4;
        wait();
        
        while (true) { 
            sc_uint<3> y;
            (x, y) = (x, y);            // register for @x      
            wait();
        }
    }
    
    void concat_lhs_rhs2()
    {
        sc_uint<4> x = 4;               // extra registers for @x
        sc_uint<3> y = 5;
        wait();
        
        while (true) { 
            (x, y) = y;                 // register for @y
            wait();
        }
    }
        
    void concat_lhs_rhs3()
    {
        sc_uint<3> x;
        wait();
        
        while (true) { 
            sc_uint<3> y;
            sc_uint<6> z;
            z = (x, sc_uint<3>(0));     // register for @x
            wait();
            (x, y) = (x, y);            // register for @y
        }
    }
    
    // Multi-assign with concat
    void concat_lhs_multi()
    {
        sc_uint<3> xx;          // extra registers for @xx and @zz
        sc_uint<3> zz;
        wait();
        
        while (true) { 
            sc_uint<4> yy;
            sc_uint<4> tt;
            (xx, yy) = (zz, tt) = 42;
            wait();
            yy = tt;  // make @tt regoster
        }
    }
    
    // bit, range in concat in LHS
    void concat_lhs_range()
    {
        sc_uint<3> r1 = 0;
        sc_uint<3> r2 = 3;
        wait();
        
        while (true) {
            sc_uint<3> r3;
            (r1.range(1,0), r3) = 42;   // register for @r1
            (r3, r2.bit(1)) = 12;       // register for @r2
            out3.write(r1+r2);
            wait();
        }
    }
    
    void concat_lhs_cast()
    {
        sc_uint<3> t1 = 0;              // registers for all
        sc_uint<3> t2 = 3;
        sc_uint<3> t3 = 2;
        sc_uint<3> t4;
        wait();
        
        while (true) {
            (t1, t2) = (unsigned)(t1,t2);
            (t3, t4) = (sc_uint<5>)(t3,t4);
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

