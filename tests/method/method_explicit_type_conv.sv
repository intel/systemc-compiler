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
// Module: A (test_explicit_type_conv.cpp:102:5)
//
module A // "b_mod.a_mod"
(
    input logic a,
    output logic b
);

// Variables generated for SystemC signals

//------------------------------------------------------------------------------
// Method process: type_conv (test_explicit_type_conv.cpp:45:5) 

always_comb 
begin : type_conv     // test_explicit_type_conv.cpp:45:5
    logic signed [31:0] par1;
    logic [31:0] par2;
    logic [31:0] par3;
    logic signed [31:0] A;
    logic [31:0] B;
    logic [31:0] C;
    logic signed [31:0] par1_1;
    logic [31:0] par2_1;
    logic [31:0] par3_1;
    logic signed [31:0] A_1;
    logic [31:0] B_1;
    logic [31:0] C_1;
    logic signed [31:0] par1_2;
    integer unsigned par2_2;
    integer unsigned par3_2;
    logic signed [31:0] A_2;
    integer unsigned B_2;
    integer unsigned C_2;
    logic signed [31:0] par1_3;
    logic [63:0] par2_3;
    logic [63:0] par3_3;
    logic signed [31:0] A_3;
    logic [63:0] B_3;
    logic [63:0] C_3;
    logic signed [31:0] par1_4;
    logic [15:0] par2_4;
    logic [15:0] par3_4;
    logic signed [31:0] A_4;
    logic [15:0] B_4;
    logic [15:0] C_4;
    integer par1_5;
    logic [31:0] par2_5;
    logic [31:0] par3_5;
    integer A_5;
    logic [31:0] B_5;
    logic [31:0] C_5;
    logic signed [63:0] par1_6;
    logic [31:0] par2_6;
    logic [31:0] par3_6;
    logic signed [63:0] A_6;
    logic [31:0] B_6;
    logic [31:0] C_6;
    logic signed [15:0] par1_7;
    logic [31:0] par2_7;
    logic [31:0] par3_7;
    logic signed [15:0] A_7;
    logic [31:0] B_7;
    logic [31:0] C_7;
    integer par1_8;
    logic [31:0] par2_8;
    logic [31:0] par3_8;
    integer A_8;
    logic [31:0] B_8;
    logic [31:0] C_8;
    logic signed [63:0] par1_9;
    logic [31:0] par2_9;
    logic [31:0] par3_9;
    logic signed [63:0] A_9;
    logic [31:0] B_9;
    logic [31:0] C_9;
    logic signed [15:0] par1_10;
    logic [31:0] par2_10;
    logic [31:0] par3_10;
    logic signed [15:0] A_10;
    logic [31:0] B_10;
    logic [31:0] C_10;
    logic [31:0] par1_11;
    logic signed [31:0] par2_11;
    logic signed [31:0] par3_11;
    logic [31:0] A_11;
    logic signed [31:0] B_11;
    logic signed [31:0] C_11;
    logic [31:0] par1_12;
    logic signed [31:0] par2_12;
    logic signed [31:0] par3_12;
    logic [31:0] A_12;
    logic signed [31:0] B_12;
    logic signed [31:0] C_12;
    logic [31:0] par1_13;
    integer par2_13;
    integer par3_13;
    logic [31:0] A_13;
    integer B_13;
    integer C_13;
    logic [31:0] par1_14;
    logic signed [63:0] par2_14;
    logic signed [63:0] par3_14;
    logic [31:0] A_14;
    logic signed [63:0] B_14;
    logic signed [63:0] C_14;
    logic [31:0] par1_15;
    logic signed [15:0] par2_15;
    logic signed [15:0] par3_15;
    logic [31:0] A_15;
    logic signed [15:0] B_15;
    logic signed [15:0] C_15;
    integer unsigned par1_16;
    logic [31:0] par2_16;
    logic [31:0] par3_16;
    integer unsigned A_16;
    logic [31:0] B_16;
    logic [31:0] C_16;
    logic [63:0] par1_17;
    logic [31:0] par2_17;
    logic [31:0] par3_17;
    logic [63:0] A_17;
    logic [31:0] B_17;
    logic [31:0] C_17;
    logic [15:0] par1_18;
    logic [31:0] par2_18;
    logic [31:0] par3_18;
    logic [15:0] A_18;
    logic [31:0] B_18;
    logic [31:0] C_18;
    integer unsigned par1_19;
    logic [31:0] par2_19;
    logic [31:0] par3_19;
    integer unsigned A_19;
    logic [31:0] B_19;
    logic [31:0] C_19;
    logic [63:0] par1_20;
    logic [31:0] par2_20;
    logic [31:0] par3_20;
    logic [63:0] A_20;
    logic [31:0] B_20;
    logic [31:0] C_20;
    logic [15:0] par1_21;
    logic [31:0] par2_21;
    logic [31:0] par3_21;
    logic [15:0] A_21;
    logic [31:0] B_21;
    logic [31:0] C_21;
    integer unsigned par1_22;
    logic signed [31:0] par2_22;
    logic signed [31:0] par3_22;
    integer unsigned A_22;
    logic signed [31:0] B_22;
    logic signed [31:0] C_22;
    logic [63:0] par1_23;
    logic signed [31:0] par2_23;
    logic signed [31:0] par3_23;
    logic [63:0] A_23;
    logic signed [31:0] B_23;
    logic signed [31:0] C_23;
    logic [15:0] par1_24;
    logic signed [31:0] par2_24;
    logic signed [31:0] par3_24;
    logic [15:0] A_24;
    logic signed [31:0] B_24;
    logic signed [31:0] C_24;
    logic [63:0] par1_25;
    logic signed [31:0] par2_25;
    logic signed [31:0] par3_25;
    logic [63:0] A_25;
    logic signed [31:0] B_25;
    logic signed [31:0] C_25;
    integer unsigned par1_26;
    logic signed [31:0] par2_26;
    logic signed [31:0] par3_26;
    integer unsigned A_26;
    logic signed [31:0] B_26;
    logic signed [31:0] C_26;
    logic [15:0] par1_27;
    logic signed [31:0] par2_27;
    logic signed [31:0] par3_27;
    logic [15:0] A_27;
    logic signed [31:0] B_27;
    logic signed [31:0] C_27;
    par1 = 32'd10; par2 = 32'd0; par3 = 32'd10;
    // Call type_conv_fn() begin
    A = par1;
    B = par2;
    C = 0;
    B = 32'(A);
    C = B;
    // Call type_conv_fn() end
    par1_1 = 32'd21312; par2_1 = 32'd0; par3_1 = 32'd21312;
    // Call type_conv_fn() begin
    A_1 = par1_1;
    B_1 = par2_1;
    C_1 = 0;
    B_1 = 32'(A_1);
    C_1 = B_1;
    // Call type_conv_fn() end
    par1_2 = 32'd10; par2_2 = 0; par3_2 = 10;
    // Call type_conv_fn() begin
    A_2 = par1_2;
    B_2 = par2_2;
    B_2 = 32'(A_2);
    C_2 = B_2;
    // Call type_conv_fn() end
    par1_3 = 32'd10; par2_3 = 0; par3_3 = 10;
    // Call type_conv_fn() begin
    A_3 = par1_3;
    B_3 = par2_3;
    B_3 = 64'(A_3);
    C_3 = B_3;
    // Call type_conv_fn() end
    par1_4 = 32'd10; par2_4 = 0; par3_4 = 10;
    // Call type_conv_fn() begin
    A_4 = par1_4;
    B_4 = par2_4;
    B_4 = 16'(A_4);
    C_4 = B_4;
    // Call type_conv_fn() end
    par1_5 = 10; par2_5 = 32'd0; par3_5 = 32'd10;
    // Call type_conv_fn() begin
    A_5 = par1_5;
    B_5 = par2_5;
    C_5 = 0;
    B_5 = 32'(A_5);
    C_5 = B_5;
    // Call type_conv_fn() end
    par1_6 = 10; par2_6 = 32'd0; par3_6 = 32'd10;
    // Call type_conv_fn() begin
    A_6 = par1_6;
    B_6 = par2_6;
    C_6 = 0;
    B_6 = 32'(A_6);
    C_6 = B_6;
    // Call type_conv_fn() end
    par1_7 = 10; par2_7 = 32'd0; par3_7 = 32'd10;
    // Call type_conv_fn() begin
    A_7 = par1_7;
    B_7 = par2_7;
    C_7 = 0;
    B_7 = 32'(A_7);
    C_7 = B_7;
    // Call type_conv_fn() end
    par1_8 = 10; par2_8 = 32'd0; par3_8 = 32'd10;
    // Call type_conv_fn() begin
    A_8 = par1_8;
    B_8 = par2_8;
    C_8 = 0;
    B_8 = 32'(A_8);
    C_8 = B_8;
    // Call type_conv_fn() end
    par1_9 = 10; par2_9 = 32'd0; par3_9 = 32'd10;
    // Call type_conv_fn() begin
    A_9 = par1_9;
    B_9 = par2_9;
    C_9 = 0;
    B_9 = 32'(A_9);
    C_9 = B_9;
    // Call type_conv_fn() end
    par1_10 = 10; par2_10 = 32'd0; par3_10 = 32'd10;
    // Call type_conv_fn() begin
    A_10 = par1_10;
    B_10 = par2_10;
    C_10 = 0;
    B_10 = 32'(A_10);
    C_10 = B_10;
    // Call type_conv_fn() end
    par1_11 = 32'd10; par2_11 = 32'd0; par3_11 = 32'd10;
    // Call type_conv_fn() begin
    A_11 = par1_11;
    B_11 = par2_11;
    C_11 = 0;
    B_11 = 32'(A_11);
    C_11 = B_11;
    // Call type_conv_fn() end
    par1_12 = 32'd21312; par2_12 = 32'd0; par3_12 = 32'd21312;
    // Call type_conv_fn() begin
    A_12 = par1_12;
    B_12 = par2_12;
    C_12 = 0;
    B_12 = 32'(A_12);
    C_12 = B_12;
    // Call type_conv_fn() end
    par1_13 = 32'd10; par2_13 = 0; par3_13 = 10;
    // Call type_conv_fn() begin
    A_13 = par1_13;
    B_13 = par2_13;
    B_13 = 32'(A_13);
    C_13 = B_13;
    // Call type_conv_fn() end
    par1_14 = 32'd10; par2_14 = 0; par3_14 = 10;
    // Call type_conv_fn() begin
    A_14 = par1_14;
    B_14 = par2_14;
    B_14 = 64'(A_14);
    C_14 = B_14;
    // Call type_conv_fn() end
    par1_15 = 32'd10; par2_15 = 0; par3_15 = 10;
    // Call type_conv_fn() begin
    A_15 = par1_15;
    B_15 = par2_15;
    B_15 = 16'(A_15);
    C_15 = B_15;
    // Call type_conv_fn() end
    par1_16 = 10; par2_16 = 32'd0; par3_16 = 32'd10;
    // Call type_conv_fn() begin
    A_16 = par1_16;
    B_16 = par2_16;
    C_16 = 0;
    B_16 = 32'(A_16);
    C_16 = B_16;
    // Call type_conv_fn() end
    par1_17 = 10; par2_17 = 32'd0; par3_17 = 32'd10;
    // Call type_conv_fn() begin
    A_17 = par1_17;
    B_17 = par2_17;
    C_17 = 0;
    B_17 = 32'(A_17);
    C_17 = B_17;
    // Call type_conv_fn() end
    par1_18 = 10; par2_18 = 32'd0; par3_18 = 32'd10;
    // Call type_conv_fn() begin
    A_18 = par1_18;
    B_18 = par2_18;
    C_18 = 0;
    B_18 = 32'(A_18);
    C_18 = B_18;
    // Call type_conv_fn() end
    par1_19 = 10; par2_19 = 32'd0; par3_19 = 32'd10;
    // Call type_conv_fn() begin
    A_19 = par1_19;
    B_19 = par2_19;
    C_19 = 0;
    B_19 = 32'(A_19);
    C_19 = B_19;
    // Call type_conv_fn() end
    par1_20 = 10; par2_20 = 32'd0; par3_20 = 32'd10;
    // Call type_conv_fn() begin
    A_20 = par1_20;
    B_20 = par2_20;
    C_20 = 0;
    B_20 = 32'(A_20);
    C_20 = B_20;
    // Call type_conv_fn() end
    par1_21 = 10; par2_21 = 32'd0; par3_21 = 32'd10;
    // Call type_conv_fn() begin
    A_21 = par1_21;
    B_21 = par2_21;
    C_21 = 0;
    B_21 = 32'(A_21);
    C_21 = B_21;
    // Call type_conv_fn() end
    par1_22 = 10; par2_22 = 32'd0; par3_22 = 32'd10;
    // Call type_conv_fn() begin
    A_22 = par1_22;
    B_22 = par2_22;
    C_22 = 0;
    B_22 = 32'(A_22);
    C_22 = B_22;
    // Call type_conv_fn() end
    par1_23 = 10; par2_23 = 32'd0; par3_23 = 32'd10;
    // Call type_conv_fn() begin
    A_23 = par1_23;
    B_23 = par2_23;
    C_23 = 0;
    B_23 = 32'(A_23);
    C_23 = B_23;
    // Call type_conv_fn() end
    par1_24 = 10; par2_24 = 32'd0; par3_24 = 32'd10;
    // Call type_conv_fn() begin
    A_24 = par1_24;
    B_24 = par2_24;
    C_24 = 0;
    B_24 = 32'(A_24);
    C_24 = B_24;
    // Call type_conv_fn() end
    par1_25 = 10; par2_25 = 32'd0; par3_25 = 32'd10;
    // Call type_conv_fn() begin
    A_25 = par1_25;
    B_25 = par2_25;
    C_25 = 0;
    B_25 = 32'(A_25);
    C_25 = B_25;
    // Call type_conv_fn() end
    par1_26 = 10; par2_26 = 32'd0; par3_26 = 32'd10;
    // Call type_conv_fn() begin
    A_26 = par1_26;
    B_26 = par2_26;
    C_26 = 0;
    B_26 = 32'(A_26);
    C_26 = B_26;
    // Call type_conv_fn() end
    par1_27 = 10; par2_27 = 32'd0; par3_27 = 32'd10;
    // Call type_conv_fn() begin
    A_27 = par1_27;
    B_27 = par2_27;
    C_27 = 0;
    B_27 = 32'(A_27);
    C_27 = B_27;
    // Call type_conv_fn() end
end

endmodule


