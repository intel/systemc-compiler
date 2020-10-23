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
    }

    // @concat() in LHS with other read/write
    void concat_lhs1()
    {
        out = 0;
        sc_uint<3> x = 0;
        a = 3;
        wait();
        
        while (true) { 
            (x, a) = 42;       // Extra register for @x
            out.write(x + a);
            wait();
        }
    }
    
    // @concat() in LHS with other read/write
    void concat_lhs1a()
    {
        out = 0;
        sc_uint<3> x;
        sc_uint<2> y;
        wait();
        
        while (true) { 
            (x, y) = 42;    // Extra register for @x
            x = y + 1; 
            out.write(x + y);
            wait();
        }
    }
    
    // No write to @b except via @concat()
    void concat_lhs2()
    {
        out = 0;
        sc_uint<3> x;
        wait();
        
        while (true) { 
            (b, x) = 0;     // Extra register for @x
            out = b;
            wait();
        }
    }
    
    // No read from @c except via @concat()
    void concat_lhs3()
    {
        c = 0;
        sc_uint<3> x;
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
        sc_uint<3> x;
        wait();
        
        while (true) { 
            x = 1;
            concat(d,x) = in.read();
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
