#include <systemc.h>

// Test for bit access in LHS
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
        SC_THREAD(bit_select_lhs1);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(bit_select_lhs1a);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(bit_select_lhs2);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
 
        SC_THREAD(bit_select_lhs3);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(bit_select_lhs4);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(bit_select_lhs4a);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(bit_select_lhs_misc);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
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
    top top_inst{"top_inst"};
    
    sc_start(100, SC_NS);
    return 0;
}
