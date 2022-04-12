//==============================================================================
//
// The code is generated by Intel Compiler for SystemC, version 1.4.18
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
logic minst_rst;
logic minst_s[2];
logic minst_r[2];
logic minst_clk;

// Assignments generated for C++ channel arrays
assign minst_clk = clk;

//------------------------------------------------------------------------------
// Method process: minst_meth (test_mif_with_chan_array.cpp:41:5) 

// Process-local variables
logic minst_v[2];

always_comb 
begin : minst_meth     // test_mif_with_chan_array.cpp:41:5
    logic b;
    minst_v[1] = 0;
    b = minst_s[1] || minst_r[1] || minst_v[1];
end

//------------------------------------------------------------------------------
// Clocked THREAD: minst_thread (test_mif_with_chan_array.cpp:46:5) 

// Thread-local variables
logic minst_vv[2];
logic minst_vv_next[2];
logic c;
logic c_next;
logic thread_PROC_STATE;
logic thread_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : minst_thread_comb     // test_mif_with_chan_array.cpp:46:5
    minst_thread_func;
end
function void minst_thread_func;
    logic d;
    c_next = c;
    minst_vv_next = minst_vv;
    thread_PROC_STATE_next = thread_PROC_STATE;
    
    case (thread_PROC_STATE)
        0: begin
            c_next = minst_s[1] || minst_r[1] || minst_vv_next[1];
            thread_PROC_STATE_next = 1; return;    // test_mif_with_chan_array.cpp:53:13;
        end
        1: begin
            d = c_next;
            c_next = minst_s[1] || minst_r[1] || minst_vv_next[1];
            thread_PROC_STATE_next = 1; return;    // test_mif_with_chan_array.cpp:53:13;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge minst_clk or posedge minst_rst) 
begin : minst_thread_ff
    if ( minst_rst ) begin
        minst_vv[1] <= 0;
        thread_PROC_STATE <= 0;    // test_mif_with_chan_array.cpp:49:9;
    end
    else begin
        minst_vv <= minst_vv_next;
        c <= c_next;
        thread_PROC_STATE <= thread_PROC_STATE_next;
    end
end

endmodule


