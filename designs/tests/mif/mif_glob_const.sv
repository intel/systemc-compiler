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

// Variables generated for SystemC signals
logic signed [31:0] minst_t1;
logic signed [31:0] t0;

// Local parameters generated for C++ constants
localparam logic B = 1;
localparam logic [31:0] C = 2;
localparam logic [31:0] E = 3;

//------------------------------------------------------------------------------
// Method process: minst_methProc (test_glob_const.cpp:54:5) 

always_comb 
begin : minst_methProc     // test_glob_const.cpp:54:5
    integer unsigned i;
    i = C;
    minst_t1 = i;
end

//------------------------------------------------------------------------------
// Method process: topProc (test_glob_const.cpp:82:5) 

always_comb 
begin : topProc     // test_glob_const.cpp:82:5
    integer unsigned i;
    i = E;
    t0 = i;
end


//------------------------------------------------------------------------------
// Child module instances

mod m
(

);

endmodule



//==============================================================================
//
// Module: mod (test_glob_const.cpp:74:5)
//
module mod // "top.m"
(
);

// Variables generated for SystemC signals
logic signed [31:0] minst_t0;

// Local parameters generated for C++ constants
localparam logic D = 0;
localparam logic [31:0] A = 1;

//------------------------------------------------------------------------------
// Method process: minst_methProc (test_glob_const.cpp:35:5) 

always_comb 
begin : minst_methProc     // test_glob_const.cpp:35:5
    integer unsigned i;
    i = A;
    minst_t0 = i;
end

endmodule


