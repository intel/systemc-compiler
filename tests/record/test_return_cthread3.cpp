#include "sct_assert.h"
#include "systemc.h"
#include <iostream>

// Record (structure/class) returned from module function
struct SinCosTuple 
{
    int sin;
    int cos;
};

struct SinCos : public sc_module, public sc_interface
{
    const sc_uint<3> c = 5;
    explicit SinCos(const sc_module_name& name) : sc_module(name) 
    {}

    SinCosTuple convert_sin_cos() {
        SinCosTuple res;
        res.sin = c+1;
        return res;
    }
};

class A : public sc_module {
public:
    sc_in_clk clk;
    sc_signal<bool> rstn;
    
    SinCos scr;
    
    SC_CTOR(A) : scr("scr") 
    {
        SC_CTHREAD(record_return, clk.pos());
        async_reset_signal_is(rstn, false);
    }
    
    void record_return() {
        
        wait();
        
        while (true) {
            SinCosTuple r = scr.convert_sin_cos();
            wait();
            
            int b = r.sin + r.cos;
        }
    }
};

int sc_main(int argc, char *argv[]) 
{
    sc_clock clk("clk", 1, SC_NS);
    A a{"a"};
    a.clk(clk);
    
    sc_start();
    return 0;
}
