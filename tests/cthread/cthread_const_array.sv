//==============================================================================
//
// The code is generated by Intel Compiler for SystemC, version 1.4.18
// see more information at https://github.com/intel/systemc-compiler
//
//==============================================================================

//==============================================================================
//
// Module: test ()
//
module test // "test_inst"
(
);

// Variables generated for SystemC signals
logic clk;
logic rstn;
logic idx;
logic signed [31:0] outs;
logic signed [31:0] mouts;

// Local parameters generated for C++ constants
localparam logic signed [31:0] ARR7[2] = '{ 70, 71 };
localparam logic signed [31:0] ARR5[2] = '{ 50, 51 };
localparam logic signed [31:0] ARR6[2] = '{ 61, 62 };
localparam logic signed [31:0] ARR1[2] = '{ 10, 11 };
localparam logic signed [31:0] ARR2[2] = '{ 21, 22 };
localparam logic signed [31:0] ARR3[2] = '{ 30, 31 };
localparam logic signed [31:0] ARR4[2] = '{ 40, 41 };

//------------------------------------------------------------------------------
// Method process: test_method (test_cthread_const_array.cpp:36:5) 

always_comb 
begin : test_method     // test_cthread_const_array.cpp:36:5
    mouts = ARR1[idx];
    mouts = ARR2[idx];
    mouts = ARR3[idx];
    mouts = ARR4[idx];
    mouts = ARR5[idx];
    mouts = ARR6[idx];
    mouts = ARR7[idx];
end

//------------------------------------------------------------------------------
// Clocked THREAD: test_thread (test_cthread_const_array.cpp:50:5) 

// Thread-local variables
logic signed [31:0] outs_next;
logic test_thread_PROC_STATE;
logic test_thread_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : test_thread_comb     // test_cthread_const_array.cpp:50:5
    test_thread_func;
end
function void test_thread_func;
    outs_next = outs;
    test_thread_PROC_STATE_next = test_thread_PROC_STATE;
    
    case (test_thread_PROC_STATE)
        0: begin
            outs_next = ARR2[idx];
            outs_next = ARR3[idx];
            outs_next = ARR4[idx];
            outs_next = ARR5[idx];
            outs_next = ARR6[idx];
            outs_next = ARR7[idx];
            test_thread_PROC_STATE_next = 1; return;    // test_cthread_const_array.cpp:61:13;
        end
        1: begin
            outs_next = ARR3[idx];
            outs_next = ARR4[idx];
            outs_next = ARR5[idx];
            outs_next = ARR6[idx];
            outs_next = ARR7[idx];
            test_thread_PROC_STATE_next = 1; return;    // test_cthread_const_array.cpp:61:13;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge rstn) 
begin : test_thread_ff
    if ( ~rstn ) begin
        outs <= ARR1[idx];
        test_thread_PROC_STATE <= 0;    // test_cthread_const_array.cpp:52:9;
    end
    else begin
        outs <= outs_next;
        test_thread_PROC_STATE <= test_thread_PROC_STATE_next;
    end
end

endmodule


