#include <systemc.h>

// Simple modular interface with port
struct mod_if : public sc_module, sc_interface 
{
    sc_in_clk       clk{"clk"};
    sc_in<bool>     in;

    SC_CTOR(mod_if) 
    {}
};

SC_MODULE(Top) {

    sc_in_clk       clk{"clk"};
    mod_if          minst{"minst"};
    
    sc_signal<bool> s;

    SC_CTOR(Top) {
        minst.clk(clk);
        minst.in(s);
    }
};

int sc_main(int argc, char **argv) 
{
    sc_clock  clk("clk", sc_time(1, SC_NS));
    Top top{"top"};
    top.clk(clk);
    
    sc_start();

    return 0;
}