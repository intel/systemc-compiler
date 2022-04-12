//==============================================================================
//
// The code is generated by Intel Compiler for SystemC, version 1.4.18
// see more information at https://github.com/intel/systemc-compiler
//
//==============================================================================

//==============================================================================
//
// Module: top ()
//
module top // "top_inst"
(
    input logic clk,
    input logic arstn1,
    input logic arstn2,
    input logic sreset1,
    input logic sreset2,
    input logic sreset3
);

// Variables generated for SystemC signals
logic signed [31:0] in;
logic signed [31:0] out;
logic signed [31:0] out2;

//------------------------------------------------------------------------------
// Clocked THREAD: test_thread1 (test_cthread_mult_resets.cpp:40:5) 

// Thread-local variables
logic signed [31:0] out_next;

// Next-state combinational logic
always_comb begin : test_thread1_comb     // test_cthread_mult_resets.cpp:40:5
    test_thread1_func;
end
function void test_thread1_func;
    out_next = out;
    out_next = 1;
endfunction

// Synchronous register update
always_ff @(posedge clk /*sync sreset1*/ or posedge arstn2 or negedge arstn1) 
begin : test_thread1_ff
    if ( sreset1 || arstn2 || ~arstn1 ) begin
        out <= 0;
    end
    else begin
        out <= out_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: test_thread2 (test_cthread_mult_resets.cpp:50:5) 

// Thread-local variables
logic signed [31:0] i;
logic signed [31:0] i_next;
logic signed [31:0] out2_next;
logic test_thread2_PROC_STATE;
logic test_thread2_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : test_thread2_comb     // test_cthread_mult_resets.cpp:50:5
    test_thread2_func;
end
function void test_thread2_func;
    i_next = i;
    out2_next = out2;
    test_thread2_PROC_STATE_next = test_thread2_PROC_STATE;
    
    case (test_thread2_PROC_STATE)
        0: begin
            out2_next = i_next++;
            test_thread2_PROC_STATE_next = 1; return;    // test_cthread_mult_resets.cpp:57:13;
        end
        1: begin
            if (|in)
            begin
                i_next--;
            end
            out2_next = i_next++;
            test_thread2_PROC_STATE_next = 1; return;    // test_cthread_mult_resets.cpp:57:13;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk /*sync sreset3*/ /*sync sreset2*/ /*sync sreset1*/ or negedge arstn1) 
begin : test_thread2_ff
    if ( sreset3 || sreset2 || sreset1 || ~arstn1 ) begin
        i <= 0;
        test_thread2_PROC_STATE <= 0;    // test_cthread_mult_resets.cpp:53:9;
    end
    else begin
        i <= i_next;
        out2 <= out2_next;
        test_thread2_PROC_STATE <= test_thread2_PROC_STATE_next;
    end
end

endmodule


