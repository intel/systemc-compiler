/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>


const unsigned A = 1;
const bool B = true;

namespace NS {
    const unsigned C = 2;
    const bool D = false;
};

const unsigned E = 3;


// Global constant and constant in name space used in MIF
struct mif : public sc_module, public sc_interface
{
    sc_signal<sc_uint<4>>     s;
    
    SC_HAS_PROCESS(mif);
    
    explicit mif(const sc_module_name& name) : sc_module(name)
    {
        SC_METHOD(methProc); sensitive << s;
    }
    
    void methProc() {
        unsigned i = !NS::D ? A : 0;
    }
};

// Global constant and constant in name space used in MIF
struct mif2 : public sc_module, public sc_interface
{
    sc_signal<sc_uint<4>>     s;
    
    SC_HAS_PROCESS(mif2);
    
    explicit mif2(const sc_module_name& name) : sc_module(name)
    {
        SC_METHOD(methProc); sensitive << s;
    }
    
    void methProc() {
        unsigned i = B ? NS::C : 0;
    }
};

SC_MODULE(mod) 
{
    mif                 minst{"minst"};
    SC_CTOR(mod)
    {}
};

SC_MODULE(Top) 
{
    sc_in_clk           clk{"clk"};
    sc_signal<bool>     nrst;
    
    sc_signal<sc_uint<4>>   t;
    mif2                minst{"minst"};
    mod                 m{"m"};

    SC_CTOR(Top)
    {
        SC_METHOD(topProc); sensitive << t;
    }
    
    void topProc() {
        unsigned i = E;
    }
};

int sc_main(int argc, char **argv) 
{
    sc_clock clk {"clk", sc_time(1, SC_NS)};
    Top top{"top"};
    top.clk(clk);
    
    sc_start();

    return 0;
}
