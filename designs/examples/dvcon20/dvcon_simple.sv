//==============================================================================
//
// The code is generated by Intel Compiler for SystemC, version 1.4.38
// see more information at https://github.com/intel/systemc-compiler
//
//==============================================================================

//==============================================================================
//
// Module: Dut ()
//
module Dut // "tb.dut"
(
    input logic clk,
    input logic nrst,
    input logic [31:0] a,
    input logic [31:0] b,
    output logic [31:0] sum,
    output logic crr
);

// Variables generated for SystemC signals
logic enbl[4];
logic actv[4][32];

// Local parameters generated for C++ constants
localparam logic [7:0] N = 4;

//------------------------------------------------------------------------------
// Clocked THREAD: proc (test_simple.cpp:34:5) 

// Thread-local variables
logic [31:0] sum_next;
logic [4:0] res;
logic [4:0] res_next;
logic crr_next;
logic proc_PROC_STATE;
logic proc_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : proc_comb     // test_simple.cpp:34:5
    proc_func;
end
function void proc_func;
    crr_next = crr;
    res_next = res;
    sum_next = sum;
    proc_PROC_STATE_next = proc_PROC_STATE;
    
    case (proc_PROC_STATE)
        0: begin
            res_next = a + b;
            proc_PROC_STATE_next = 1; return;    // test_simple.cpp:40:13;
        end
        1: begin
            sum_next = res_next;
            crr_next = res_next[N];
            res_next = a + b;
            proc_PROC_STATE_next = 1; return;    // test_simple.cpp:40:13;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge nrst) 
begin : proc_ff
    if ( ~nrst ) begin
        sum <= 0;
        proc_PROC_STATE <= 0;    // test_simple.cpp:36:9;
    end
    else begin
        sum <= sum_next;
        res <= res_next;
        crr <= crr_next;
        proc_PROC_STATE <= proc_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: thread_proc (test_simple.cpp:67:1) 

// Next-state combinational logic
always_comb begin : thread_proc_comb     // test_simple.cpp:67:1
    thread_proc_func;
end
function void thread_proc_func;
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge nrst) 
begin : thread_proc_ff
    if ( ~nrst ) begin

    `ifndef INTEL_SVA_OFF
        for (integer i = 0; i < signed'({1'b0, N}); ++i) begin
            sctAssertLine70 : assert property ( enbl[i] |=> !enbl[i] );
        end
        for (integer i = 0; i < signed'({1'b0, N}); ++i) begin
            for (integer j = 0; j < 32; ++j) begin
                sctAssertLine72 : assert property ( actv[i][j] |-> ##2 actv[i][32 - j - 1] );
            end
        end
    `endif // INTEL_SVA_OFF
    end
    else begin

    `ifndef INTEL_SVA_OFF
        for (integer i = 0; i < signed'({1'b0, N}); ++i) begin
            sctAssertLine70 : assert property ( enbl[i] |=> !enbl[i] );
        end
        for (integer i = 0; i < signed'({1'b0, N}); ++i) begin
            for (integer j = 0; j < 32; ++j) begin
                sctAssertLine72 : assert property ( actv[i][j] |-> ##2 actv[i][32 - j - 1] );
            end
        end
    `endif // INTEL_SVA_OFF
    end
end

`ifndef INTEL_SVA_OFF
sctAssertLine32 : assert property (
    @(posedge clk) |a |-> ##[10:30] |b );
`endif // INTEL_SVA_OFF

endmodule

