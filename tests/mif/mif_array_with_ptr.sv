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
logic [3:0] minst_s[2];
logic signed [31:0] minst_p[2];
logic signed [31:0] minst_pp[2];
logic minst_clk[2];
logic minst_rst[2];

// Assignments generated for C++ channel arrays
assign minst_clk[0] = clk;
assign minst_clk[1] = clk;
assign minst_rst[0] = rst;
assign minst_rst[1] = rst;

//------------------------------------------------------------------------------
// Method process: minst_mif_meth (test_mif_array_with_ptr.cpp:42:5) 

// Process-local variables
logic [3:0] minst_v[2];
logic [3:0] minst_vp[2];

always_comb 
begin : minst_mif_meth     // test_mif_array_with_ptr.cpp:42:5
    minst_v[0] = minst_s[0];
    minst_vp[0] = 3;
end

//------------------------------------------------------------------------------
// Clocked THREAD: minst_mif_thread (test_mif_array_with_ptr.cpp:47:5) 

// Thread-local variables
logic signed [31:0] minst_p_next[2];

// Next-state combinational logic
always_comb begin : minst_mif_thread_comb     // test_mif_array_with_ptr.cpp:47:5
    minst_mif_thread_func;
end
function void minst_mif_thread_func;
    minst_p_next[0] = minst_p[0];
    minst_p_next[0] = 1;
endfunction

// Synchronous register update
always_ff @(posedge minst_clk[0] or posedge minst_rst[0]) 
begin : minst_mif_thread_ff
    if ( minst_rst[0] ) begin
        minst_p[0] <= 0;
    end
    else begin
        minst_p[0] <= minst_p_next[0];
    end
end

//------------------------------------------------------------------------------
// Method process: minst_mif_meth0 (test_mif_array_with_ptr.cpp:42:5) 

always_comb 
begin : minst_mif_meth0     // test_mif_array_with_ptr.cpp:42:5
    minst_v[1] = minst_s[1];
    minst_vp[1] = 3;
end

//------------------------------------------------------------------------------
// Clocked THREAD: minst_mif_thread0 (test_mif_array_with_ptr.cpp:47:5) 

// Next-state combinational logic
always_comb begin : minst_mif_thread0_comb     // test_mif_array_with_ptr.cpp:47:5
    minst_mif_thread0_func;
end
function void minst_mif_thread0_func;
    minst_p_next[1] = minst_p[1];
    minst_p_next[1] = 1;
endfunction

// Synchronous register update
always_ff @(posedge minst_clk[1] or posedge minst_rst[1]) 
begin : minst_mif_thread0_ff
    if ( minst_rst[1] ) begin
        minst_p[1] <= 0;
    end
    else begin
        minst_p[1] <= minst_p_next[1];
    end
end

//------------------------------------------------------------------------------
// Method process: top_method (test_mif_array_with_ptr.cpp:80:5) 

// Process-local variables
logic [4:0] minst_vvp[2];
logic [3:0] minst_vv[2];

always_comb 
begin : top_method     // test_mif_array_with_ptr.cpp:80:5
    integer i;
    logic [3:0] a;
    i = t;
    minst_s[1] = 1;
    minst_p[1] = 2;
    minst_p[1] = 3;
    minst_p[1] = 4;
    minst_vvp[1] = 5;
    a = minst_s[i];
    minst_s[i] = minst_vv[i + 1] + a;
end

//------------------------------------------------------------------------------
// Clocked THREAD: top_thread (test_mif_array_with_ptr.cpp:96:5) 

// Thread-local variables
logic signed [31:0] minst_pp_next[2];

// Next-state combinational logic
always_comb begin : top_thread_comb     // test_mif_array_with_ptr.cpp:96:5
    top_thread_func;
end
function void top_thread_func;
    minst_pp_next = minst_pp;
    minst_pp_next[1] = 1;
endfunction

// Synchronous register update
always_ff @(posedge clk or posedge rst) 
begin : top_thread_ff
    if ( rst ) begin
        minst_pp[1] <= 0;
    end
    else begin
        minst_pp <= minst_pp_next;
    end
end

endmodule


