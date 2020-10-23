/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

//
// SVC tool. Verilog intrinsic example.
//

#include <systemc.h>
#include <string>

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
    input  logic        rstn,
    input  logic [31:0] din,
    output logic [31:0] dout
);


always_ff @(posedge clk or negedge rstn) begin
    if (~rstn) begin
        dout <= 0;
    end else
    begin
        dout <= din;
    end
end

endmodule
)";

    sc_in<bool> clk{"clk"};
    sc_in<bool> rstn{"rstn"};

    sc_in<sc_uint<32>>  din{"din"};
    sc_out<sc_uint<32>> dout{"dout"};

    SC_CTOR(my_register) {
        SC_METHOD(test_method);
        sensitive << clk.pos() << rstn.neg();
    }

    void test_method () {
        if (!rstn.read()) {
            dout = 0;
        } else {
            dout = din;
        }
    }
};


SC_MODULE(Top) 
{
    sc_in<bool> clk{"clk"};
    sc_signal<bool> rstn{"rstn"};

    sc_signal<sc_uint<32>>  din{"din"};
    sc_signal<sc_uint<32>>  dout{"dout"};

    empty_intrinsic empty1{"empty1"};
    my_register reg0{"reg0"};

    SC_CTOR(Top) {

        empty1.clk(clk);
        reg0.clk(clk);

        reg0.rstn(rstn);

        reg0.din(din);
        reg0.dout(dout);
    }
};

int sc_main(int argc, char **argv) 
{
    sc_clock clk{"clk", 1, SC_NS};
    Top top{"top"};
    top.clk(clk);
    
    sc_start();

    return 0;
}