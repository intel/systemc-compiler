//==============================================================================
//
// The code is generated by Intel Compiler for SystemC, version 1.4.18
// see more information at https://github.com/intel/systemc-compiler
//
//==============================================================================

//==============================================================================
//
// Module: A ()
//
module A // "a_mod"
(
    input logic clk
);

// Variables generated for SystemC signals
logic rst;
logic s;
logic t;
logic ta;
logic tb;
logic [7:0] r;
logic [7:0] ra;
logic [7:0] rb;

// Local parameters generated for C++ constants
localparam logic signed [31:0] b = 11;
localparam logic signed [7:0] d = 7;

//------------------------------------------------------------------------------
// Clocked THREAD: loc_var_reset (test_sig_access_reset.cpp:46:6) 

// Next-state combinational logic
always_comb begin : loc_var_reset_comb     // test_sig_access_reset.cpp:46:6
    loc_var_reset_func;
end
function void loc_var_reset_func;
    logic j1;
    integer j2;
    logic [3:0] j3;
    j1 = 0;
    j2 = 1;
    j3 = 2;
endfunction

// Synchronous register update
always_ff @(posedge clk or posedge rst) 
begin : loc_var_reset_ff
    if ( rst ) begin
        logic [3:0] j3;
        j3 = 0;
    end
    else begin
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: loc_var_body (test_sig_access_reset.cpp:60:5) 

// Next-state combinational logic
always_comb begin : loc_var_body_comb     // test_sig_access_reset.cpp:60:5
    loc_var_body_func;
end
function void loc_var_body_func;
endfunction

// Synchronous register update
always_ff @(posedge clk or posedge rst) 
begin : loc_var_body_ff
    if ( rst ) begin
        logic j1;
        integer j2;
        logic [3:0] j3;
        j1 = 1;
        j2 = 2;
        j3 = j2 + 1;
    end
    else begin
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: sig_thread (test_sig_access_reset.cpp:71:5) 

// Next-state combinational logic
always_comb begin : sig_thread_comb     // test_sig_access_reset.cpp:71:5
    sig_thread_func;
end
function void sig_thread_func;
endfunction

// Synchronous register update
always_ff @(posedge clk or posedge rst) 
begin : sig_thread_ff
    if ( rst ) begin
        s <= 1;
        t <= 0;
        r <= 0;
        ta <= 0;
        ra <= 0;
    end
    else begin
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: assert_sig (test_sig_access_reset.cpp:82:5) 

// Thread-local variables
logic tb_next;
logic [7:0] rb_next;
logic assert_sig_PROC_STATE;
logic assert_sig_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : assert_sig_comb     // test_sig_access_reset.cpp:82:5
    assert_sig_func;
end
function void assert_sig_func;
    rb_next = rb;
    tb_next = tb;
    assert_sig_PROC_STATE_next = assert_sig_PROC_STATE;
    
    case (assert_sig_PROC_STATE)
        0: begin
            assert_sig_PROC_STATE_next = 1; return;    // test_sig_access_reset.cpp:91:13;
        end
        1: begin
            assert_sig_PROC_STATE_next = 1; return;    // test_sig_access_reset.cpp:91:13;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or posedge rst) 
begin : assert_sig_ff
    if ( rst ) begin
        tb <= 0;
        rb <= 0;
        assert_sig_PROC_STATE <= 0;    // test_sig_access_reset.cpp:87:9;
    end
    else begin
        tb <= tb_next;
        rb <= rb_next;
        assert_sig_PROC_STATE <= assert_sig_PROC_STATE_next;

    `ifndef INTEL_SVA_OFF
        sctAssertLine88 : assert property ( ta |-> |ra );
    `endif // INTEL_SVA_OFF
    end

`ifndef INTEL_SVA_OFF
    sctAssertLine85 : assert property ( t |-> |r );
    sctAssertLine86 : assert property ( tb |-> |rb );
`endif // INTEL_SVA_OFF
end

//------------------------------------------------------------------------------
// Clocked THREAD: assert_global (test_sig_access_reset.cpp:102:5) 

// Thread-local variables
logic [7:0] c;
logic [7:0] c_next;
logic a;
logic a_next;
logic [7:0] mm;
logic assert_global_PROC_STATE;
logic assert_global_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : assert_global_comb     // test_sig_access_reset.cpp:102:5
    assert_global_func;
end
function void assert_global_func;
    a_next = a;
    c_next = c;
    assert_global_PROC_STATE_next = assert_global_PROC_STATE;
    
    case (assert_global_PROC_STATE)
        0: begin
            assert_global_PROC_STATE_next = 1; return;    // test_sig_access_reset.cpp:110:13;
        end
        1: begin
            assert_global_PROC_STATE_next = 1; return;    // test_sig_access_reset.cpp:110:13;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or posedge rst) 
begin : assert_global_ff
    if ( rst ) begin
        logic [7:0] mm;
        a <= 0;
        c <= 1;
        mm = 2;
        assert_global_PROC_STATE <= 0;    // test_sig_access_reset.cpp:106:9;
    end
    else begin
        c <= c_next;
        a <= a_next;
        assert_global_PROC_STATE <= assert_global_PROC_STATE_next;

    `ifndef INTEL_SVA_OFF
        sctAssertLine107 : assert property ( |b |-> d == 1 );
    `endif // INTEL_SVA_OFF
    end

`ifndef INTEL_SVA_OFF
    sctAssertLine105 : assert property ( a_next |-> |c_next );
`endif // INTEL_SVA_OFF
end

//------------------------------------------------------------------------------
// Clocked THREAD: assert_local (test_sig_access_reset.cpp:115:5) 

// Thread-local variables
logic signed [7:0] h;
logic signed [7:0] h_next;
logic e;
logic e_next;
logic signed [31:0] f;
logic signed [31:0] f_next;
logic [7:0] g;
logic [7:0] g_next;
logic assert_local_PROC_STATE;
logic assert_local_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : assert_local_comb     // test_sig_access_reset.cpp:115:5
    assert_local_func;
end
function void assert_local_func;
    e_next = e;
    f_next = f;
    g_next = g;
    h_next = h;
    assert_local_PROC_STATE_next = assert_local_PROC_STATE;
    
    case (assert_local_PROC_STATE)
        0: begin
            assert_local_PROC_STATE_next = 1; return;    // test_sig_access_reset.cpp:128:13;
        end
        1: begin
            assert_local_PROC_STATE_next = 1; return;    // test_sig_access_reset.cpp:128:13;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or posedge rst) 
begin : assert_local_ff
    if ( rst ) begin
        logic [7:0] nn;
        g <= 0;
        h <= 0;
        nn = 0;
        assert_local_PROC_STATE <= 0;    // test_sig_access_reset.cpp:124:9;
    end
    else begin
        h <= h_next;
        e <= e_next;
        f <= f_next;
        g <= g_next;
        assert_local_PROC_STATE <= assert_local_PROC_STATE_next;

    `ifndef INTEL_SVA_OFF
        sctAssertLine125 : assert property ( |f_next |-> |g_next );
    `endif // INTEL_SVA_OFF
    end

`ifndef INTEL_SVA_OFF
    sctAssertLine123 : assert property ( e_next |-> h_next == 1 );
`endif // INTEL_SVA_OFF
end

endmodule


