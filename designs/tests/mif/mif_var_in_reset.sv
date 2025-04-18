//==============================================================================
//
// The code is generated by Intel Compiler for SystemC, version 1.6.6
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
logic nrst;
logic [3:0] minst_s[2];
logic minst_clk[2];
logic minst_nrst[2];

// Assignments generated for C++ channel arrays
assign minst_clk[0] = clk;
assign minst_clk[1] = clk;
assign minst_nrst[0] = nrst;
assign minst_nrst[1] = nrst;

//------------------------------------------------------------------------------
// Clocked THREAD: minst_threadProc (test_var_in_reset.cpp:31:5) 

// Thread-local variables
logic [3:0] minst_s_next[2];
logic signed [31:0] b;
logic signed [31:0] b_next;

// Next-state combinational logic
always_comb begin : minst_threadProc_comb     // test_var_in_reset.cpp:31:5
    minst_threadProc_func;
end
function void minst_threadProc_func;
    logic a;
    logic [3:0] c;
    b_next = b;
    minst_s_next[0] = minst_s[0];
    minst_s_next[0] = minst_s[0] + b_next;
endfunction

// Synchronous register update
always_ff @(posedge minst_clk[0] or negedge minst_nrst[0]) 
begin : minst_threadProc_ff
    if ( ~minst_nrst[0] ) begin
        logic a;
        logic [3:0] c;
        a = 1;
        b <= 42;
        c = b >>> 4;
        minst_s[0] <= c + 1;
    end
    else begin
        minst_s[0] <= minst_s_next[0];
        b <= b_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: minst_threadProc0 (test_var_in_reset.cpp:31:5) 

// Thread-local variables
logic signed [31:0] b0;
logic signed [31:0] b_next0;

// Next-state combinational logic
always_comb begin : minst_threadProc0_comb     // test_var_in_reset.cpp:31:5
    minst_threadProc0_func;
end
function void minst_threadProc0_func;
    logic a;
    logic [3:0] c;
    b_next0 = b0;
    minst_s_next[1] = minst_s[1];
    minst_s_next[1] = minst_s[1] + b_next0;
endfunction

// Synchronous register update
always_ff @(posedge minst_clk[1] or negedge minst_nrst[1]) 
begin : minst_threadProc0_ff
    if ( ~minst_nrst[1] ) begin
        logic a;
        logic [3:0] c;
        a = 1;
        b0 <= 42;
        c = b0 >>> 4;
        minst_s[1] <= c + 1;
    end
    else begin
        minst_s[1] <= minst_s_next[1];
        b0 <= b_next0;
    end
end

endmodule


