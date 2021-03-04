/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>
#include <string>

// Two intrinsics examples
struct A : public sc_module {
    
    sc_in<bool> in;

    std::string __SC_TOOL_VERILOG_MOD__;
    
    SC_CTOR(A) :
        __SC_TOOL_VERILOG_MOD__(
    R"(
module A(input in);
   // Some verilog code for module A
endmodule
    )")
    {}
};

struct B : public sc_module {
    
    sc_in<bool> in;
    sc_out<sc_uint<2>> out;

    std::string __SC_TOOL_VERILOG_MOD__;
    
    SC_CTOR(B)
    {
        __SC_TOOL_VERILOG_MOD__ = R"(
module B(input in, 
         output out);
   // Some verilog code for module B
endmodule
    )";
    }
};    


SC_MODULE(testbench) 
{
    sc_signal<bool> s1;
    sc_signal<sc_uint<2>> s2;
    
    A a;
    B b;

    SC_CTOR(testbench) : a("a"), b("b") {
        a.in(s1);
        b.in(s1);
        b.out(s2);
    }
};

int sc_main(int argc, char **argv) {

    testbench tb_inst{"tb_inst"};
    sc_start();

    return 0;
}
