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
logic rstn;
logic [31:0] mif_b_mif_aa_a[2];
logic [31:0] mif_b_mif_aa_b[2];
logic [31:0] mif_b_mif_a_a[2][3];
logic [31:0] mif_b_mif_a_b[2][3];
logic [31:0] mif_b_c[2];
logic signed [31:0] mif_b_t1[2];
logic signed [31:0] mif_b_t2[2];
logic [15:0] s;
logic mif_b_clk[2];
logic mif_b_rstn[2];

// Assignments generated for C++ channel arrays
assign mif_b_clk[0] = clk;
assign mif_b_clk[1] = clk;
assign mif_b_rstn[0] = rstn;
assign mif_b_rstn[1] = rstn;

//------------------------------------------------------------------------------
// Method process: mif_b_methB (test_sc_vector_mif6.cpp:67:5) 

always_comb 
begin : mif_b_methB     // test_sc_vector_mif6.cpp:67:5
    mif_b_t1[0] = mif_b_mif_aa_a[0];
    mif_b_t2[0] = mif_b_mif_a_a[0][mif_b_c[0]];
end

//------------------------------------------------------------------------------
// Clocked THREAD: mif_b_threadB (test_sc_vector_mif6.cpp:72:5) 

// Thread-local variables
logic [31:0] mif_b_mif_aa_a_next[2];
logic [31:0] mif_b_mif_a_a_next[2][3];
logic [31:0] mif_b_mif_aa_m[2];
logic [31:0] mif_b_mif_aa_m_next[2];
logic [31:0] mif_b_mif_a_m[2][3];
logic [31:0] mif_b_mif_a_m_next[2][3];

// Next-state combinational logic
always_comb begin : mif_b_threadB_comb     // test_sc_vector_mif6.cpp:72:5
    mif_b_threadB_func;
end
function void mif_b_threadB_func;
    integer unsigned l;
    mif_b_mif_a_a_next[0] = mif_b_mif_a_a[0];
    mif_b_mif_a_m_next[0] = mif_b_mif_a_m[0];
    mif_b_mif_aa_a_next[0] = mif_b_mif_aa_a[0];
    mif_b_mif_aa_m_next[0] = mif_b_mif_aa_m[0];
    l = mif_b_mif_aa_a[0] + mif_b_mif_aa_m_next[0] + mif_b_mif_a_a[0][0] + mif_b_mif_a_m_next[0][0];
    mif_b_mif_aa_a_next[0] = 4;
    mif_b_mif_aa_m_next[0] = 4;
    mif_b_mif_a_a_next[0][mif_b_c[0]] = 4;
    mif_b_mif_a_m_next[0][mif_b_c[0]] = 4;
endfunction

// Synchronous register update
always_ff @(posedge mif_b_clk[0] or negedge mif_b_rstn[0]) 
begin : mif_b_threadB_ff
    if ( ~mif_b_rstn[0] ) begin
        mif_b_mif_aa_a[0] <= 3;
        mif_b_mif_aa_m[0] <= 3;
        mif_b_mif_a_a[0][0] <= 3;
        mif_b_mif_a_m[0][0] <= 3;
    end
    else begin
        mif_b_mif_aa_a[0] <= mif_b_mif_aa_a_next[0];
        mif_b_mif_a_a[0] <= mif_b_mif_a_a_next[0];
        mif_b_mif_aa_m[0] <= mif_b_mif_aa_m_next[0];
        mif_b_mif_a_m[0] <= mif_b_mif_a_m_next[0];
    end
end

//------------------------------------------------------------------------------
// Method process: mif_b_methB0 (test_sc_vector_mif6.cpp:67:5) 

always_comb 
begin : mif_b_methB0     // test_sc_vector_mif6.cpp:67:5
    mif_b_t1[1] = mif_b_mif_aa_a[1];
    mif_b_t2[1] = mif_b_mif_a_a[1][mif_b_c[1]];
end

//------------------------------------------------------------------------------
// Clocked THREAD: mif_b_threadB0 (test_sc_vector_mif6.cpp:72:5) 

// Next-state combinational logic
always_comb begin : mif_b_threadB0_comb     // test_sc_vector_mif6.cpp:72:5
    mif_b_threadB0_func;
end
function void mif_b_threadB0_func;
    integer unsigned l;
    mif_b_mif_a_a_next[1] = mif_b_mif_a_a[1];
    mif_b_mif_a_m_next[1] = mif_b_mif_a_m[1];
    mif_b_mif_aa_a_next[1] = mif_b_mif_aa_a[1];
    mif_b_mif_aa_m_next[1] = mif_b_mif_aa_m[1];
    l = mif_b_mif_aa_a[1] + mif_b_mif_aa_m_next[1] + mif_b_mif_a_a[1][0] + mif_b_mif_a_m_next[1][0];
    mif_b_mif_aa_a_next[1] = 4;
    mif_b_mif_aa_m_next[1] = 4;
    mif_b_mif_a_a_next[1][mif_b_c[1]] = 4;
    mif_b_mif_a_m_next[1][mif_b_c[1]] = 4;
endfunction

// Synchronous register update
always_ff @(posedge mif_b_clk[1] or negedge mif_b_rstn[1]) 
begin : mif_b_threadB0_ff
    if ( ~mif_b_rstn[1] ) begin
        mif_b_mif_aa_a[1] <= 3;
        mif_b_mif_aa_m[1] <= 3;
        mif_b_mif_a_a[1][0] <= 3;
        mif_b_mif_a_m[1][0] <= 3;
    end
    else begin
        mif_b_mif_aa_a[1] <= mif_b_mif_aa_a_next[1];
        mif_b_mif_a_a[1] <= mif_b_mif_a_a_next[1];
        mif_b_mif_aa_m[1] <= mif_b_mif_aa_m_next[1];
        mif_b_mif_a_m[1] <= mif_b_mif_a_m_next[1];
    end
end

//------------------------------------------------------------------------------
// Method process: mainMeth (test_sc_vector_mif6.cpp:117:6) 

// Process-local variables
logic [31:0] mif_b_k[2];

always_comb 
begin : mainMeth     // test_sc_vector_mif6.cpp:117:6
    mif_b_c[s] = 1;
    mif_b_k[s] = 1;
end

//------------------------------------------------------------------------------
// Clocked THREAD: mainThread (test_sc_vector_mif6.cpp:122:5) 

// Thread-local variables
logic [31:0] mif_b_mif_aa_b_next[2];
logic [31:0] mif_b_mif_a_n[2][3];
logic [31:0] mif_b_mif_a_n_next[2][3];
logic [31:0] mif_b_mif_aa_n[2];
logic [31:0] mif_b_mif_aa_n_next[2];
logic [31:0] mif_b_mif_a_b_next[2][3];

// Next-state combinational logic
always_comb begin : mainThread_comb     // test_sc_vector_mif6.cpp:122:5
    mainThread_func;
end
function void mainThread_func;
    integer unsigned l;
    mif_b_mif_a_b_next = mif_b_mif_a_b;
    mif_b_mif_a_n_next = mif_b_mif_a_n;
    mif_b_mif_aa_b_next = mif_b_mif_aa_b;
    mif_b_mif_aa_n_next = mif_b_mif_aa_n;
    l = mif_b_mif_aa_b[0] + mif_b_mif_aa_b[1] + mif_b_mif_a_b[1][0] + mif_b_mif_a_n_next[0][1];
    mif_b_mif_aa_b_next[s] = 1;
    mif_b_mif_aa_n_next[s] = 1;
    mif_b_mif_a_b_next[s + 1][s] = 2;
    mif_b_mif_a_n_next[s + 1][s] = mif_b_mif_aa_n_next[s];
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge rstn) 
begin : mainThread_ff
    if ( ~rstn ) begin
        mif_b_mif_aa_b[s] <= 1;
        mif_b_mif_aa_n[s] <= 1;
    end
    else begin
        mif_b_mif_aa_b <= mif_b_mif_aa_b_next;
        mif_b_mif_a_n <= mif_b_mif_a_n_next;
        mif_b_mif_aa_n <= mif_b_mif_aa_n_next;
        mif_b_mif_a_b <= mif_b_mif_a_b_next;
    end
end

endmodule


