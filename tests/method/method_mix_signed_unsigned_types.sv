//==============================================================================
//
// The code is generated by Intel Compiler for SystemC, version 1.4.18
// see more information at https://github.com/intel/systemc-compiler
//
//==============================================================================

//==============================================================================
//
// Module: B_top ()
//
module B_top // "b_mod"
(
);

// Variables generated for SystemC signals
logic a;
logic b;


//------------------------------------------------------------------------------
// Child module instances

A a_mod
(
  .a(a),
  .b(b)
);

endmodule



//==============================================================================
//
// Module: A (test_mix_signed_unsigned_types.cpp:95:5)
//
module A // "b_mod.a_mod"
(
    input logic a,
    output logic b
);

// Variables generated for SystemC signals

//------------------------------------------------------------------------------
// Method process: mix_sign_unsign (test_mix_signed_unsigned_types.cpp:56:5) 

always_comb 
begin : mix_sign_unsign     // test_mix_signed_unsigned_types.cpp:56:5
    integer iter;
    integer unsigned par1;
    integer par2;
    integer par3;
    integer unsigned A;
    integer B;
    integer C;
    integer par1_1;
    integer unsigned par2_1;
    integer par3_1;
    integer A_1;
    integer unsigned B_1;
    integer C_1;
    logic [63:0] par1_2;
    logic signed [63:0] par2_2;
    logic signed [63:0] par3_2;
    logic [63:0] A_2;
    logic signed [63:0] B_2;
    logic signed [63:0] C_2;
    logic signed [63:0] par1_3;
    logic [63:0] par2_3;
    logic signed [63:0] par3_3;
    logic signed [63:0] A_3;
    logic [63:0] B_3;
    logic signed [63:0] C_3;
    logic [3:0] par1_4;
    logic signed [3:0] par2_4;
    logic signed [7:0] par3_4;
    logic [3:0] A_4;
    logic signed [3:0] B_4;
    logic signed [7:0] C_4;
    logic signed [31:0] par1_5;
    logic [31:0] par2_5;
    logic signed [31:0] par3_5;
    logic signed [31:0] A_5;
    logic [31:0] B_5;
    logic signed [31:0] C_5;
    logic [31:0] par1_6;
    logic signed [30:0] par2_6;
    logic signed [31:0] par3_6;
    logic [31:0] A_6;
    logic signed [30:0] B_6;
    logic signed [31:0] C_6;
    logic signed [31:0] par1_7;
    logic [31:0] par2_7;
    logic signed [31:0] par3_7;
    logic signed [31:0] A_7;
    logic [31:0] B_7;
    logic signed [31:0] C_7;
    iter = 0;
    par1 = 2; par2 = -32'sd5; par3 = 2147483645;
    // Call mix_sign_unsign_fn() begin
    A = par1;
    B = par2;
    C = par2 / par1;
    // Call mix_sign_unsign_fn() end
    par1_1 = -32'sd2; par2_1 = 5; par3_1 = 0;
    // Call mix_sign_unsign_fn() begin
    A_1 = par1_1;
    B_1 = par2_1;
    C_1 = par2_1 / par1_1;
    // Call mix_sign_unsign_fn() end
    par1_2 = 2; par2_2 = -64'sd5; par3_2 = 63'h7FFFFFFFFFFFFFFD;
    // Call mix_sign_unsign_fn() begin
    A_2 = par1_2;
    B_2 = par2_2;
    C_2 = par2_2 / par1_2;
    // Call mix_sign_unsign_fn() end
    par1_3 = -64'sd2; par2_3 = 5; par3_3 = 0;
    // Call mix_sign_unsign_fn() begin
    A_3 = par1_3;
    B_3 = par2_3;
    C_3 = par2_3 / par1_3;
    // Call mix_sign_unsign_fn() end
    par1_4 = 4'd2; par2_4 = -4'sd5; par3_4 = -8'sd3;
    // Call mix_sign_unsign_fn() begin
    A_4 = par1_4;
    B_4 = par2_4;
    C_4 = 0;
    C_4 = par2_4 / par1_4;
    // Call mix_sign_unsign_fn() end
    par1_5 = -32'sd2; par2_5 = 32'd5; par3_5 = 32'd0;
    // Call mix_sign_unsign_fn() begin
    A_5 = par1_5;
    B_5 = par2_5;
    C_5 = 0;
    C_5 = par2_5 / par1_5;
    // Call mix_sign_unsign_fn() end
    par1_6 = 32'd2; par2_6 = -31'sd5; par3_6 = -32'sd2;
    // Call mix_sign_unsign_fn_assert() begin
    A_6 = par1_6;
    B_6 = par2_6;
    C_6 = 0;
    C_6 = par2_6 / signed'({1'b0, par1_6});
    // Call mix_sign_unsign_fn_assert() end
    par1_7 = -32'sd2; par2_7 = 32'd5; par3_7 = -32'sd2;
    // Call mix_sign_unsign_fn_assert() begin
    A_7 = par1_7;
    B_7 = par2_7;
    C_7 = 0;
    C_7 = signed'({1'b0, par2_7}) / par1_7;
    // Call mix_sign_unsign_fn_assert() end
end

endmodule


