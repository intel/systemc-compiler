//==============================================================================
//
// The code is generated by Intel Compiler for SystemC, version 1.6.14
// see more information at https://github.com/intel/systemc-compiler
//
//==============================================================================

//==============================================================================
//
// Module: top ()
//
module top // "t"
(
);

// Variables generated for SystemC signals
logic clk;


//------------------------------------------------------------------------------
// Child module instances

derived d
(
  .clk(clk)
);

endmodule



//==============================================================================
//
// Module: derived (test_cthread_virtual_wait.cpp:55:5)
//
module derived // "t.d"
(
    input logic clk
);

// Variables generated for SystemC signals
logic nrst;
logic x;

//------------------------------------------------------------------------------
// Clocked THREAD: test_thread (test_cthread_virtual_wait.cpp:26:5) 

// Thread-local variables
logic x_next;
logic [1:0] test_thread_PROC_STATE;
logic [1:0] test_thread_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : test_thread_comb     // test_cthread_virtual_wait.cpp:26:5
    test_thread_func;
end
function void test_thread_func;
    x_next = x;
    test_thread_PROC_STATE_next = test_thread_PROC_STATE;
    
    case (test_thread_PROC_STATE)
        0: begin
            // Call virtual_wait() begin
            x_next = 0;
            test_thread_PROC_STATE_next = 1; return;    // test_cthread_virtual_wait.cpp:44:9;
            // Call virtual_wait() end
        end
        1: begin
            // Call virtual_wait() begin
            x_next = 1;
            test_thread_PROC_STATE_next = 0; return;    // test_cthread_virtual_wait.cpp:46:9;
            // Call virtual_wait() end
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or posedge nrst) 
begin : test_thread_ff
    if ( nrst ) begin
        test_thread_PROC_STATE <= 0;    // test_cthread_virtual_wait.cpp:27:9;
    end
    else begin
        x <= x_next;
        test_thread_PROC_STATE <= test_thread_PROC_STATE_next;
    end
end

endmodule


