//==============================================================================
//
// The code is generated by Intel Compiler for SystemC, version 1.6.2
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
// Module: Child (test_var_multiple_use2.cpp:62:5)
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
logic signed [31:0] out1;
logic signed [31:0] s1;

// Local parameters generated for C++ constants
localparam logic [2:0] vv = 0;

//------------------------------------------------------------------------------
// Method process: methA (test_var_multiple_use2.cpp:38:5) 

// Process-local variables
logic [2:0] v;

always_comb 
begin : methA     // test_var_multiple_use2.cpp:38:5
    v = in;
    s1 = 2 - v;
    out1 = 1 + in;
end

//------------------------------------------------------------------------------
// Clocked THREAD: thrA (test_var_multiple_use2.cpp:44:5) 

// Thread-local variables
logic signed [31:0] i;
logic signed [31:0] i_next;
logic signed [31:0] s_next;
logic [1:0] thrA_PROC_STATE;
logic [1:0] thrA_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : thrA_comb     // test_var_multiple_use2.cpp:44:5
    thrA_func;
end
function void thrA_func;
    i_next = i;
    s_next = s;
    thrA_PROC_STATE_next = thrA_PROC_STATE;
    
    case (thrA_PROC_STATE)
        0: begin
            i_next = vv;
            thrA_PROC_STATE_next = 1; return;    // test_var_multiple_use2.cpp:50:13;
        end
        1: begin
            s_next = i_next + in;
            thrA_PROC_STATE_next = 0; return;    // test_var_multiple_use2.cpp:52:13;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or posedge rst) 
begin : thrA_ff
    if ( rst ) begin
        out <= 0;
        thrA_PROC_STATE <= 0;    // test_var_multiple_use2.cpp:46:9;
    end
    else begin
        i <= i_next;
        s <= s_next;
        thrA_PROC_STATE <= thrA_PROC_STATE_next;
    end
end

endmodule


