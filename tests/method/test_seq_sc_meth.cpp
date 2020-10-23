#include "systemc.h"
#include "sct_assert.h"
#include <iostream>

template <int N>
SC_MODULE (A) {
public:
    sc_in_clk clk{"clk"};
    sc_signal<bool> sync_rst{"sync_rst", 1};

    sc_in<int> a;
    sc_out<int> c;
    SC_CTOR(A) : a("a"), c("c") {
        SC_METHOD(dff);
        sensitive << clk.pos() << a << sync_rst;
     }
     void dff() {
         if (sync_rst == 0) {
             c = 0; // reset signals
         } else {
             // assign signals
             c = a;
         }
     }
};

class B_top : public sc_module
{
public:
    sc_signal<int>  a{"a"};
    sc_signal<int>  c{"c"};
    sc_signal<int>  ya{"ya"};
    sc_signal<int>  yc{"yc"};
    sc_in_clk clk{"clk"};

    A<7> x{"x"};
    A<4> y{"y"};

    SC_CTOR(B_top) {
        x.a(a);
        x.c(c);
        y.a(ya);
        y.c(yc);
        x.clk(clk);
        y.clk(clk);

    }
};

int sc_main(int argc, char* argv[])
{
    sc_clock clk{"clk", 1, SC_NS};

    B_top b_mod{"b_mod"};
    b_mod.clk(clk);

    sc_start();
    return 0;
}
