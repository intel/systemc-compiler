//==============================================================================
//
// The code is generated by Intel Compiler for SystemC, version 1.4.18
// see more information at https://github.com/intel/systemc-compiler
//
//==============================================================================

//==============================================================================
//
// Module: A ()
//
module A // "modA"
(
);

// Variables generated for SystemC signals
logic [31:0] s;

// Local parameters generated for C++ constants
localparam logic b = 1;
localparam logic signed [31:0] i = 12;
localparam logic [15:0] x = 48;
localparam logic signed [63:0] y = -7'sd36;
localparam logic [63:0] uy = 64'h8000000000000000;
localparam logic [63:0] AA = 64'h8000000000000000;
localparam logic [63:0] BB = 64'h8000000000000000;
localparam logic [31:0] arr[3] = '{ 1, 2, 3 };
localparam logic [15:0] z = 11;
localparam logic [63:0] N = 3;

//------------------------------------------------------------------------------
// Method process: constInit (test_const_init.cpp:61:5) 

always_comb 
begin : constInit     // test_const_init.cpp:61:5
    logic [63:0] bu;
    s = i + x;
    bu = uy + AA + BB;
end

//------------------------------------------------------------------------------
// Method process: unknownConstInit (test_const_init.cpp:85:5) 

always_comb 
begin : unknownConstInit     // test_const_init.cpp:85:5
    logic c1;
    integer c2;
    logic [15:0] c3;
    logic signed [64:0] c4;
    logic d1;
    integer unsigned d2;
    logic [19:0] d3;
    logic [99:0] d4;
    c1 = s == 42;
    c2 = s;
    c3 = 16'(s);
    c4 = 65'(s);
    d1 = c2 == c3;
    d2 = d1 ? 32'(c3) : 32'(c4);
    d3 = 20'(d2);
    d4 = 100'(c4 + 1);
end

//------------------------------------------------------------------------------
// Method process: unknownConstFunc (test_const_init.cpp:126:5) 

always_comb 
begin : unknownConstFunc     // test_const_init.cpp:126:5
    integer TMP_0;
    integer e1;
    logic [79:0] TMP_2;
    integer i_1;
    integer TMP_4;
    logic [79:0] e2;
    logic signed [21:0] TMP_6;
    integer i_2;
    logic signed [21:0] e3;
    logic [15:0] TMP_8;
    logic signed [21:0] i_3;
    logic [15:0] e4;
    // Call f1() begin
    TMP_0 = s;
    // Call f1() end
    e1 = TMP_0;
    i_1 = e1;
    // Call f2() begin
    // Call f1() begin
    TMP_4 = s;
    // Call f1() end
    TMP_2 = 80'(TMP_4);
    // Call f2() end
    e2 = TMP_2;
    i_2 = s;
    // Call f3() begin
    TMP_6 = 22'(i_2 + 1);
    // Call f3() end
    e3 = TMP_6;
    i_3 = e3;
    // Call f4() begin
    TMP_8 = 16'(i_3[15 : 0]);
    // Call f4() end
    e4 = TMP_8;
end

endmodule


