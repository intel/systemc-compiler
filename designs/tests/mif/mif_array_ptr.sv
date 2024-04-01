//==============================================================================
//
// The code is generated by Intel Compiler for SystemC, version 1.6.2
// see more information at https://github.com/intel/systemc-compiler
//
//==============================================================================

//==============================================================================
//
// Module: Top ()
//
module Top // "top"
(
    input logic clk
);

// Variables generated for SystemC signals
logic [31:0] t;
logic [31:0] mif_s[2];
logic signed [31:0] mif_t0[2];
logic signed [31:0] t1;
logic signed [31:0] t2;
logic signed [31:0] t3;
logic signed [31:0] t4;

// Local parameters generated for C++ constants
localparam logic [3:0] R_a = 9;
localparam logic R_b = 1;
localparam logic signed [31:0] R_c = -3'sd2;
localparam logic [31:0] ca[2] = '{ 11, 12 };
localparam logic signed [31:0] c = 43;
localparam logic signed [31:0] pd = 44;
localparam logic [3:0] mif_CR_a[2] = '{ 9, 9 };
localparam logic signed [31:0] mif_CR_c[2] = '{ -3'sd2, -3'sd2 };

//------------------------------------------------------------------------------
// Method process: mif_var_rec (test_mif_array_ptr.cpp:32:5) 

// Process-local variables
logic [3:0] mif_r_a[2];
logic mif_r_b[2];
logic signed [31:0] mif_r_c[2];

always_comb 
begin : mif_var_rec     // test_mif_array_ptr.cpp:32:5
    integer l;
    mif_r_a[0] = mif_s[0] + 1;
    mif_r_b[0] = |(mif_r_a[0] * mif_CR_a[0]);
    l = |mif_s[0] ? mif_r_c[0] : 32'(mif_r_a[0]);
    mif_t0[0] = l;
end

//------------------------------------------------------------------------------
// Method process: mif_var_rec0 (test_mif_array_ptr.cpp:32:5) 

always_comb 
begin : mif_var_rec0     // test_mif_array_ptr.cpp:32:5
    integer l;
    mif_r_a[1] = mif_s[1] + 1;
    mif_r_b[1] = |(mif_r_a[1] * mif_CR_a[1]);
    l = |mif_s[1] ? mif_r_c[1] : 32'(mif_r_a[1]);
    mif_t0[1] = l;
end

//------------------------------------------------------------------------------
// Method process: const_ptr (test_mif_array_ptr.cpp:76:5) 

// Process-local variables
logic signed [31:0] i;

always_comb 
begin : const_ptr     // test_mif_array_ptr.cpp:76:5
    integer l;
    i = 41;
    l = i;
    l = c;
    l = pd;
    t1 = l;
end

//------------------------------------------------------------------------------
// Method process: const_ptr_arr (test_mif_array_ptr.cpp:86:5) 

always_comb 
begin : const_ptr_arr     // test_mif_array_ptr.cpp:86:5
    integer unsigned j;
    integer unsigned lu;
    j = t;
    lu = ca[j];
    t2 = lu;
end

//------------------------------------------------------------------------------
// Method process: const_rec (test_mif_array_ptr.cpp:94:5) 

always_comb 
begin : const_rec     // test_mif_array_ptr.cpp:94:5
    integer k;
    k = R_c;
    k = 32'(R_a);
    t3 = k;
end

//------------------------------------------------------------------------------
// Method process: mif_rec (test_mif_array_ptr.cpp:101:5) 

always_comb 
begin : mif_rec     // test_mif_array_ptr.cpp:101:5
    integer k;
    integer unsigned ii;
    ii = t;
    k = mif_CR_c[0];
    k = 32'(mif_CR_a[ii]);
    t4 = k;
end

endmodule


