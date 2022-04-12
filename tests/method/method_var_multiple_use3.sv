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
logic rst;
logic signed [31:0] t;


//------------------------------------------------------------------------------
// Child module instances

Child child
(
  .clk(clk),
  .rst(rst),
  .in(t),
  .out(t)
);

endmodule



//==============================================================================
//
// Module: Child (test_var_multiple_use3.cpp:62:5)
//
module Child // "top.child"
(
    input logic clk,
    input logic rst,
    input logic signed [31:0] in,
    output logic signed [31:0] out
);

// Variables generated for SystemC signals
logic signed [31:0] s;

// Local parameters generated for C++ constants
localparam logic signed [31:0] c = 42;

//------------------------------------------------------------------------------
// Method process: methA (test_var_multiple_use3.cpp:36:5) 

// Process-local variables
logic [2:0] v;

always_comb 
begin : methA     // test_var_multiple_use3.cpp:36:5
    v = in;
    s = 2 - v;
    if (|in)
    begin
        s = v + 1;
    end
    s = in;
end

//------------------------------------------------------------------------------
// Clocked THREAD: thrA (test_var_multiple_use3.cpp:45:5) 

// Thread-local variables
logic signed [31:0] out_next;
logic thrA_PROC_STATE;
logic thrA_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : thrA_comb     // test_var_multiple_use3.cpp:45:5
    thrA_func;
end
function void thrA_func;
    out_next = out;
    thrA_PROC_STATE_next = thrA_PROC_STATE;
    
    case (thrA_PROC_STATE)
        0: begin
            out_next = in;
            thrA_PROC_STATE_next = 1; return;    // test_var_multiple_use3.cpp:51:13;
        end
        1: begin
            out_next = c;
            out_next = in;
            thrA_PROC_STATE_next = 1; return;    // test_var_multiple_use3.cpp:51:13;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or posedge rst) 
begin : thrA_ff
    if ( rst ) begin
        out <= 0;
        thrA_PROC_STATE <= 0;    // test_var_multiple_use3.cpp:47:9;
    end
    else begin
        out <= out_next;
        thrA_PROC_STATE <= thrA_PROC_STATE_next;
    end
end

endmodule


