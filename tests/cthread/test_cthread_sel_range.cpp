#include <systemc.h>

// Test for range access in LHS
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
        SC_THREAD(range_select_lhs1);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(range_select_lhs1a);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(range_select_lhs2);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
 
        SC_THREAD(range_select_lhs3);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(range_select_lhs4);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(range_select_lhs4a);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(range_select_lhs_misc);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
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
    top top_inst{"top_inst"};
    
    sc_start(100, SC_NS);
    return 0;
}
