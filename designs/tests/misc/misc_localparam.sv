//==============================================================================
//
// The code is generated by Intel Compiler for SystemC, version 1.6.14
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


//------------------------------------------------------------------------------
// Child module instances

A a
(
  .clk(clk)
);

endmodule



//==============================================================================
//
// Module: A (test_localparam.cpp:120:6)
//
module A // "top.a"
(
    input logic clk
);

// Variables generated for SystemC signals
logic [31:0] s;
logic signed [31:0] t0;
logic signed [31:0] t1;
logic signed [31:0] t2;
logic t;

// Local parameters generated for C++ constants
localparam logic [31:0] C = 42;
localparam logic [15:0] S = 52;
localparam logic signed [47:0] T = -7'sd50;
localparam logic [31:0] V = 43;
localparam logic [31:0] W = 60;
localparam logic [31:0] W1 = 63;
localparam logic B = 1;
localparam logic D = 1;
localparam logic E = 1;
localparam logic F = 1;
localparam logic [31:0] CE = 41;

//------------------------------------------------------------------------------
// Method process: constProc1 (test_localparam.cpp:52:5) 

// Process-local variables
logic [15:0] S1;
logic signed [47:0] T1;

always_comb 
begin : constProc1     // test_localparam.cpp:52:5
    logic signed [63:0] l;
    S1 = 51;
    T1 = -7'sd51;
    l = S + T;
    l = S1 + T1;
    t0 = l;
end

//------------------------------------------------------------------------------
// Method process: constProc2 (test_localparam.cpp:63:5) 

// Process-local variables
logic [31:0] V1;

always_comb 
begin : constProc2     // test_localparam.cpp:63:5
    integer unsigned i;
    V1 = 44;
    i = CE;
    i = C;
    i = V + V1;
    t1 = i;
end

//------------------------------------------------------------------------------
// Method process: constProc3 (test_localparam.cpp:80:5) 

// Process-local variables
logic [31:0] ARR[3];

always_comb 
begin : constProc3     // test_localparam.cpp:80:5
    integer unsigned i;
    i = W;
    i = W1;
    i = ARR[0] + ARR[1] + ARR[2];
    t2 = i;
end

//------------------------------------------------------------------------------
// Method process: constProc4 (test_localparam.cpp:96:5) 

// Process-local variables
logic MF;
logic signed [31:0] ar[3];

always_comb 
begin : constProc4     // test_localparam.cpp:96:5
    t = 1;
    t = 1;
    t = 1;
    t = F;
    t = MF;
    if (|s)
    begin
        MF = 0;
    end
end

endmodule


