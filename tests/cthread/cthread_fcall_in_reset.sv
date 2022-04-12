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
logic nrst;
logic signed [31:0] s0;
logic signed [31:0] s1;
logic signed [31:0] s2;
logic signed [31:0] s3;
logic signed [31:0] s4;
logic signed [31:0] s5;

//------------------------------------------------------------------------------
// Clocked THREAD: top_thread0 (test_fcall_in_reset.cpp:51:5) 

// Next-state combinational logic
always_comb begin : top_thread0_comb     // test_fcall_in_reset.cpp:51:5
    top_thread0_func;
end
function void top_thread0_func;
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge nrst) 
begin : top_thread0_ff
    if ( ~nrst ) begin
        integer i;
        i = 505;    // Call of g()
        s0 <= i;
    end
    else begin
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: top_thread1 (test_fcall_in_reset.cpp:64:5) 

// Thread-local variables
logic signed [31:0] k;
logic signed [31:0] k_next;
logic signed [31:0] s1_next;

// Thread-local constants
logic signed [31:0] cs;
logic signed [31:0] c;

// Next-state combinational logic
always_comb begin : top_thread1_comb     // test_fcall_in_reset.cpp:64:5
    top_thread1_func;
end
function void top_thread1_func;
    integer j;
    k_next = k;
    s1_next = s1;
    j = c / cs + k_next;
    s1_next = j;
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge nrst) 
begin : top_thread1_ff
    if ( ~nrst ) begin
        c = 42;    // Call of f()
        cs = 42 - 1;
        k <= c % cs;
    end
    else begin
        k <= k_next;
        s1 <= s1_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: top_thread2 (test_fcall_in_reset.cpp:81:5) 

// Thread-local variables
logic signed [31:0] i0;
logic signed [31:0] i_next;
logic signed [31:0] j0;
logic signed [31:0] j_next;
logic signed [31:0] s2_next;
logic top_thread2_PROC_STATE;
logic top_thread2_PROC_STATE_next;

// Thread-local constants
logic signed [31:0] c0;
logic signed [31:0] cs0;

// Next-state combinational logic
always_comb begin : top_thread2_comb     // test_fcall_in_reset.cpp:81:5
    top_thread2_func;
end
function void top_thread2_func;
    i_next = i0;
    j_next = j0;
    s2_next = s2;
    top_thread2_PROC_STATE_next = top_thread2_PROC_STATE;
    
    case (top_thread2_PROC_STATE)
        0: begin
            j_next = c0 + i_next;
            top_thread2_PROC_STATE_next = 1; return;    // test_fcall_in_reset.cpp:92:13;
        end
        1: begin
            j_next = j_next + cs0;
            s2_next = j_next;
            j_next = c0 + i_next;
            top_thread2_PROC_STATE_next = 1; return;    // test_fcall_in_reset.cpp:92:13;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge nrst) 
begin : top_thread2_ff
    if ( ~nrst ) begin
        i0 <= 42;    // Call of f()
        c0 = 41 + i0;
        cs0 = 42;
        top_thread2_PROC_STATE <= 0;    // test_fcall_in_reset.cpp:87:9;
    end
    else begin
        i0 <= i_next;
        j0 <= j_next;
        s2 <= s2_next;
        top_thread2_PROC_STATE <= top_thread2_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: top_thread3 (test_fcall_in_reset.cpp:104:5) 

// Next-state combinational logic
always_comb begin : top_thread3_comb     // test_fcall_in_reset.cpp:104:5
    top_thread3_func;
end
function void top_thread3_func;
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge nrst) 
begin : top_thread3_ff
    if ( ~nrst ) begin
        integer i;
        i = 42;    // Call of f()
        // Call fr() begin
        i--;
        // Call fr() end
        s3 <= i;
    end
    else begin
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: top_thread4 (test_fcall_in_reset.cpp:126:5) 

// Thread-local variables
logic signed [31:0] i1;
logic signed [31:0] i_next0;
logic signed [31:0] l;
logic signed [31:0] l_next;
logic signed [31:0] s4_next;
logic [1:0] top_thread4_PROC_STATE;
logic [1:0] top_thread4_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : top_thread4_comb     // test_fcall_in_reset.cpp:126:5
    top_thread4_func;
end
function void top_thread4_func;
    integer TMP_0;
    i_next0 = i1;
    l_next = l;
    s4_next = s4;
    top_thread4_PROC_STATE_next = top_thread4_PROC_STATE;
    
    case (top_thread4_PROC_STATE)
        0: begin
            // Call fw() begin
            TMP_0 = l_next - i1;
            // Call fw() end
            s4_next = TMP_0;
            top_thread4_PROC_STATE_next = 1; return;    // test_fcall_in_reset.cpp:130:9;
        end
        1: begin
            top_thread4_PROC_STATE_next = 2; return;    // test_fcall_in_reset.cpp:134:13;
        end
        2: begin
            top_thread4_PROC_STATE_next = 2; return;    // test_fcall_in_reset.cpp:134:13;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge nrst) 
begin : top_thread4_ff
    if ( ~nrst ) begin
        integer TMP_0;
        i1 <= 10;
        // Call fw() begin
        l <= i1 + i1;
        top_thread4_PROC_STATE <= 0;    // test_fcall_in_reset.cpp:121:9;
        // Call fw() end
    end
    else begin
        i1 <= i_next0;
        l <= l_next;
        s4 <= s4_next;
        top_thread4_PROC_STATE <= top_thread4_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: top_thread5 (test_fcall_in_reset.cpp:139:5) 

// Thread-local variables
logic signed [31:0] par;
logic signed [31:0] par_next;
logic signed [31:0] l0;
logic signed [31:0] l_next0;
logic signed [31:0] s5_next;
logic [1:0] top_thread5_PROC_STATE;
logic [1:0] top_thread5_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : top_thread5_comb     // test_fcall_in_reset.cpp:139:5
    top_thread5_func;
end
function void top_thread5_func;
    integer TMP_0;
    l_next0 = l0;
    par_next = par;
    s5_next = s5;
    top_thread5_PROC_STATE_next = top_thread5_PROC_STATE;
    
    case (top_thread5_PROC_STATE)
        0: begin
            // Call fw() begin
            TMP_0 = l_next0 - par;
            // Call fw() end
            s5_next = TMP_0;
            top_thread5_PROC_STATE_next = 1; return;    // test_fcall_in_reset.cpp:142:9;
        end
        1: begin
            top_thread5_PROC_STATE_next = 2; return;    // test_fcall_in_reset.cpp:146:13;
        end
        2: begin
            top_thread5_PROC_STATE_next = 2; return;    // test_fcall_in_reset.cpp:146:13;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge nrst) 
begin : top_thread5_ff
    if ( ~nrst ) begin
        integer TMP_0;
        par = 11;
        // Call fw() begin
        l0 <= par + par;
        top_thread5_PROC_STATE <= 0;    // test_fcall_in_reset.cpp:121:9;
        // Call fw() end
    end
    else begin
        par <= par_next;
        l0 <= l_next0;
        s5 <= s5_next;
        top_thread5_PROC_STATE <= top_thread5_PROC_STATE_next;
    end
end

endmodule


