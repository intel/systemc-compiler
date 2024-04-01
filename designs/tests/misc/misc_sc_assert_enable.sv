//==============================================================================
//
// The code is generated by Intel Compiler for SystemC, version 1.6.2
// see more information at https://github.com/intel/systemc-compiler
//
//==============================================================================

//==============================================================================
//
// Module: A ()
//
module A // "test_top.a_mod"
(
    input logic clk,
    input logic rstn,
    input logic [3:0] a,
    output logic [3:0] b
);

// Variables generated for SystemC signals
logic signed [31:0] t0;
logic signed [31:0] t1;
logic signed [31:0] t2;

//------------------------------------------------------------------------------
// Method process: assert_test (test_sc_assert_enable.cpp:36:5) 

always_comb 
begin : assert_test     // test_sc_assert_enable.cpp:36:5
    integer i;
    i = 0;
    t0 = i;
end

//------------------------------------------------------------------------------
// Method process: assert_in_loop_test (test_sc_assert_enable.cpp:44:5) 

always_comb 
begin : assert_in_loop_test     // test_sc_assert_enable.cpp:44:5
    t1 = 0;
    for (integer unsigned i = 0; i < 3; ++i)
    begin
        if (|a)
        begin
            t1 = 1;
        end
    end
end

//------------------------------------------------------------------------------
// Method process: assert_in_if_test (test_sc_assert_enable.cpp:56:5) 

always_comb 
begin : assert_in_if_test     // test_sc_assert_enable.cpp:56:5
    t2 = 0;
    case (a)
    0 : begin
        t2 = 1;
    end
    default : begin
        t2 = 2;
    end
    endcase
    if (|a)
    begin
        t2 = 2;
    end
end

endmodule

