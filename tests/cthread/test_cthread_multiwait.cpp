//
// Created by Huzaifa Rampurawala
//

#include <systemc.h>

// Reset section in clocked thread
class top : sc_module
{
public:
    sc_clock clk{"clk", 10, SC_NS};
    sc_signal<bool> arstn{"arstn", 1};
    sc_signal<sc_uint<31>> a{"a"};
    sc_signal<sc_uint<31>> b{"b"};


    SC_HAS_PROCESS(top);
    top(sc_module_name)
    {
        SC_CTHREAD(comb_init_in_reset, clk);
        async_reset_signal_is(arstn, false);
    }

    // Combinational variables in reset
    void comb_init_in_reset() 
    {
        a = 0;
        b = 0;
        wait();
        a=1;
        wait();
        a=3;
        b = 1;
        wait();
        a=4;
        wait();
        a=5;
        wait();
        while (true) {
            a=a.read()+1;
            wait();
            b=a.read()+b.read();
            wait();
;        }
    }
    
};

int sc_main(int argc, char *argv[])
{
    top top_inst{"top_inst"};
    sc_start(100, SC_NS);
    return 0;
}
