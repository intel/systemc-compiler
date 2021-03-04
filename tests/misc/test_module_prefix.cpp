/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>
#include <string>

// Intrinsic modified in module constructor
struct I : public sc_module {
    
    sc_in<bool> in;

    std::string __SC_TOOL_VERILOG_MOD__;
    
    SC_CTOR(I) :
        __SC_TOOL_VERILOG_MOD__(
    R"(
module I(input in);
   // Some verilog code
endmodule
    )")
    {}
    
};

// Module prefix tool option test
struct A : public sc_module 
{
    sc_in<bool> s{"s"};
    
    SC_HAS_PROCESS(A);
    
    explicit A(const sc_module_name& name)
    {
        SC_METHOD(proc);
        sensitive << s;
    }
    
    void proc() {
        int i = 0;
        if (s) {
            i = 1;
        }
    }

};

class B : public sc_module 
{
public:
    sc_signal<bool> s{"s"};

    A  a1;
    A* a2;
    I i;
    
    explicit B(const sc_module_name& name) : a1("a1"), i("i")
    {
        a2 = new A("a2");
        
        a1.s(s);
        a2->s(s);
        i.in(s);
    }

};

struct Top : public sc_module 
{
    A aa{"aa"};
    B bb;
    
    sc_signal<bool> s{"s"};
    
    SC_CTOR(Top) : 
        bb("bb")
    {
        aa.s(s);
    }
};

int sc_main(int argc, char **argv) 
{
    Top top{"top"};
    sc_start();

    return 0;
}

