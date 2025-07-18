//==============================================================================
//
// The code is generated by Intel Compiler for SystemC, version 1.6.20
// see more information at https://github.com/intel/systemc-compiler
//
//==============================================================================

//==============================================================================
//
// Module: test_enum ()
//
module test_enum // "tenum"
(
);

// Variables generated for SystemC signals
logic signed [31:0] t0;

// Local parameters generated for C++ constants
localparam logic [1:0] nm = 3;
localparam logic [2:0] mode_const = '0;
localparam logic [1:0] color_const = 2;

//------------------------------------------------------------------------------
// Method process: test_method (test_enum.cpp:29:5) 

// Process-local variables
logic [1:0] color;

always_comb 
begin : test_method     // test_enum.cpp:29:5
    logic [1:0] xcolor;
    logic [2:0] xmode;
    integer icolor;
    logic [1:0] dir;
    integer x;
    color = 1;
    xcolor = color_const;
    xmode = mode_const;
    icolor = 3;
    xcolor = 3;
    dir = 1;
    x = 0;
    x = 1;
    t0 = icolor + signed'({1'b0, xcolor}) + signed'({1'b0, xmode});
end

endmodule


