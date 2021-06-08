/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>
#include <string>

// Empty intrinsic and register examples

// Intrinsic without Verilog text
struct empty_intrinsic: sc_module {
    sc_in<bool> clk{"clk"};

    // Initialized to empty string
    std::string __SC_TOOL_VERILOG_MOD__ = "";

    SC_CTOR(empty_intrinsic) {}

};


// Intrinsic with Verilog text
struct my_register : sc_module {
std::string __SC_TOOL_VERILOG_MOD__ =
R"(
module my_register (
    input  logic        clk,
    input  logic        arstn,
    input  logic [31:0] din,
    output logic [31:0] dout
);


always_ff @(posedge clk or negedge arstn) begin
    if (~arstn) begin
        dout <= 0;
    end else
    begin
        dout <= din;
    end
end

endmodule
)";

    sc_in<bool> clk{"clk"};
    sc_in<bool> arstn{"arstn"};

    sc_in<sc_uint<32>>  din{"din"};
    sc_out<sc_uint<32>> dout{"dout"};

    SC_CTOR(my_register) {
        SC_METHOD(test_method);
        sensitive << clk.pos() << arstn.neg();
    }

    void test_method () {
        if (!arstn.read()) {
            dout = 0;
        } else {
            dout = din;
        }
    }

};


SC_MODULE(testbench) {

    sc_clock        clock_gen{"clock_gen", 10, SC_NS};
    sc_signal<bool> arstn{"arstn"};

    sc_signal<sc_uint<32>>  din{"din"};
    sc_signal<sc_uint<32>>  tmp0{"tmp0"};
    sc_signal<sc_uint<32>>  tmp1{"tmp1"};
    sc_signal<sc_uint<32>>  dout{"dout"};

    my_register reg0{"reg0"};
    my_register reg1{"reg1"};
    my_register reg2{"reg2"};

    empty_intrinsic empty1{"empty1"};
    empty_intrinsic empty2{"empty1"};

    SC_CTOR(testbench) {

        empty1.clk(clock_gen);
        empty2.clk(clock_gen);

        reg0.clk(clock_gen);
        reg1.clk(clock_gen);
        reg2.clk(clock_gen);

        reg0.arstn(arstn);
        reg1.arstn(arstn);
        reg2.arstn(arstn);

        reg0.din(din);
        reg0.dout(tmp0);
        reg1.din(tmp0);
        reg1.dout(tmp1);
        reg2.din(tmp1);
        reg2.dout(dout);

        SC_THREAD(test_thread);
        sensitive << clock_gen.posedge_event();
    }


    void test_thread() {
        while(1) wait();
    }

};

int sc_main(int argc, char **argv) {

    testbench tb_inst{"tb_inst"};
    sc_start();

    return 0;
}
