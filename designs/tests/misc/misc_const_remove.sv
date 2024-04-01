//==============================================================================
//
// The code is generated by Intel Compiler for SystemC, version 1.6.2
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
logic nrst;
logic [3:0] s;
logic signed [31:0] t;
logic signed [3:0] s1;
logic signed [3:0] s1a;
logic signed [63:0] s2;
logic signed [3:0] s3;

// Local parameters generated for C++ constants
localparam logic signed [31:0] C1 = 42;
localparam logic signed [31:0] C2 = 43;
localparam logic [31:0] C3 = 42;
localparam logic [31:0] C4 = 42;
localparam logic signed [31:0] D1 = 43;
localparam logic [31:0] D2 = 43;
localparam logic [31:0] D3 = 43;
localparam logic signed [31:0] N1 = -2'sd1;
localparam logic signed [31:0] NARR[3] = '{ -2'sd1, -3'sd2, -3'sd3 };
localparam logic [31:0] ARR[3] = '{ 1, 2, 3 };
localparam logic [31:0] ARR1 = 2;
localparam logic [2:0] L = 1;
localparam logic [3:0] marr[3] = '{ 4, 5, 6 };
localparam logic [2:0] S = 6;
localparam logic signed [3:0] XC = 3;
localparam logic signed [31:0] TC0 = 42;
localparam logic grec1_a = 0;
localparam logic [3:0] grec2_b = 5;
localparam logic signed [31:0] M = 3;
localparam logic [31:0] MUX_RATE = 4;

//------------------------------------------------------------------------------
// Method process: const_range1 (test_const_remove.cpp:119:5) 

always_comb 
begin : const_range1     // test_const_remove.cpp:119:5
    logic [11:0] R;
    logic [11:0] RR;
    logic [11:0] T;
    logic [3:0] larr[3];
    integer l;
    R = 7;
    RR = 7;
    T = 8;
    larr[0] = 1; larr[1] = 2; larr[2] = 3;
    l = S[1];
    l = R[1];
    l = RR[2 : 1];
    l = T;
    l = L[2 : 1];
    l = larr[2][2 : 1];
    l = marr[0][1];
    l = larr[s][2 : 1] + marr[s][1];
end

//------------------------------------------------------------------------------
// Method process: const_range2 (test_const_remove.cpp:143:5) 

always_comb 
begin : const_range2     // test_const_remove.cpp:143:5
    logic [2:0] val;
    integer l;
    val = 1;
    // Call cval() begin
    l = val[2 : 1];
    // Call cval() end
end

//------------------------------------------------------------------------------
// Clocked THREAD: multi_ref_call (test_const_remove.cpp:177:5) 

// Thread-local variables
logic signed [3:0] s1_next;

// Next-state combinational logic
always_comb begin : multi_ref_call_comb     // test_const_remove.cpp:177:5
    multi_ref_call_func;
end
function void multi_ref_call_func;
    logic signed [3:0] y;
    s1_next = s1;
    y = 41;
    // Call cref1() begin
    s1_next = y;
    // Call cref1() end
    // Call cref1() begin
    s1_next = XC;
    // Call cref1() end
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge nrst) 
begin : multi_ref_call_ff
    if ( ~nrst ) begin
    end
    else begin
        s1 <= s1_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: negative_cost_ref1 (test_const_remove.cpp:188:5) 

// Thread-local variables
logic signed [3:0] s1a_next;
logic negative_cost_ref1_PROC_STATE;
logic negative_cost_ref1_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : negative_cost_ref1_comb     // test_const_remove.cpp:188:5
    negative_cost_ref1_func;
end
function void negative_cost_ref1_func;
    logic signed [3:0] x;
    logic signed [3:0] par;
    s1a_next = s1a;
    negative_cost_ref1_PROC_STATE_next = negative_cost_ref1_PROC_STATE;
    
    case (negative_cost_ref1_PROC_STATE)
        0: begin
            x = -3'sd3;
            par = x + 1;
            // Call cref1a() begin
            s1a_next = par;
            // Call cref1a() end
            negative_cost_ref1_PROC_STATE_next = 1; return;    // test_const_remove.cpp:194:13;
        end
        1: begin
            par = N1;
            // Call cref1a() begin
            s1a_next = par;
            // Call cref1a() end
            x = -3'sd3;
            par = x + 1;
            // Call cref1a() begin
            s1a_next = par;
            // Call cref1a() end
            negative_cost_ref1_PROC_STATE_next = 1; return;    // test_const_remove.cpp:194:13;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge nrst) 
begin : negative_cost_ref1_ff
    if ( ~nrst ) begin
        negative_cost_ref1_PROC_STATE <= 0;    // test_const_remove.cpp:190:9;
    end
    else begin
        s1a <= s1a_next;
        negative_cost_ref1_PROC_STATE <= negative_cost_ref1_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: negative_cost_ref2 (test_const_remove.cpp:199:5) 

// Thread-local variables
logic signed [63:0] s2_next;
logic [1:0] negative_cost_ref2_PROC_STATE;
logic [1:0] negative_cost_ref2_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : negative_cost_ref2_comb     // test_const_remove.cpp:199:5
    negative_cost_ref2_func;
end
function void negative_cost_ref2_func;
    logic signed [63:0] par;
    s2_next = s2;
    negative_cost_ref2_PROC_STATE_next = negative_cost_ref2_PROC_STATE;
    
    case (negative_cost_ref2_PROC_STATE)
        0: begin
            par = -7'sd42;
            // Call cref2() begin
            s2_next = par + 1;
            // Call cref2() end
            par = -7'sd43;
            // Call cref2() begin
            s2_next = par + 1;
            // Call cref2() end
            negative_cost_ref2_PROC_STATE_next = 1; return;    // test_const_remove.cpp:205:13;
        end
        1: begin
            // Call cref2() begin
            s2_next = NARR[s] + 1;
            // Call cref2() end
            negative_cost_ref2_PROC_STATE_next = 0; return;    // test_const_remove.cpp:207:13;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge nrst) 
begin : negative_cost_ref2_ff
    if ( ~nrst ) begin
        negative_cost_ref2_PROC_STATE <= 0;    // test_const_remove.cpp:201:9;
    end
    else begin
        s2 <= s2_next;
        negative_cost_ref2_PROC_STATE <= negative_cost_ref2_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: negative_cost_ref3 (test_const_remove.cpp:211:5) 

// Thread-local variables
logic signed [3:0] l0;
logic signed [3:0] l_next;
logic signed [3:0] s3_next;
logic signed [3:0] par0;
logic signed [3:0] par_next;
logic signed [3:0] l1;
logic signed [3:0] l_next0;
logic [1:0] negative_cost_ref3_PROC_STATE;
logic [1:0] negative_cost_ref3_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : negative_cost_ref3_comb     // test_const_remove.cpp:211:5
    negative_cost_ref3_func;
end
function void negative_cost_ref3_func;
    logic signed [3:0] y;
    l_next = l0;
    l_next0 = l1;
    par_next = par0;
    s3_next = s3;
    negative_cost_ref3_PROC_STATE_next = negative_cost_ref3_PROC_STATE;
    
    case (negative_cost_ref3_PROC_STATE)
        0: begin
            y = ~4'd5;
            // Call cref3() begin
            l_next0 = 1;
            negative_cost_ref3_PROC_STATE_next = 1; return;    // test_const_remove.cpp:170:9;
            // Call cref3() end
        end
        1: begin
            // Call cref3() begin
            l_next0 = l_next0 + y;
            s3_next = l_next0;
            // Call cref3() end
            // Call cref3() begin
            l_next = 1;
            negative_cost_ref3_PROC_STATE_next = 2; return;    // test_const_remove.cpp:170:9;
            // Call cref3() end
        end
        2: begin
            // Call cref3() begin
            l_next = l_next + NARR[2];
            s3_next = l_next;
            // Call cref3() end
            y = ~4'd5;
            // Call cref3() begin
            l_next0 = 1;
            negative_cost_ref3_PROC_STATE_next = 1; return;    // test_const_remove.cpp:170:9;
            // Call cref3() end
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge nrst) 
begin : negative_cost_ref3_ff
    if ( ~nrst ) begin
        negative_cost_ref3_PROC_STATE <= 0;    // test_const_remove.cpp:213:9;
    end
    else begin
        l0 <= l_next;
        s3 <= s3_next;
        par0 <= par_next;
        l1 <= l_next0;
        negative_cost_ref3_PROC_STATE <= negative_cost_ref3_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: static_const_thread (test_const_remove.cpp:229:5) 

// Thread-local constants
logic signed [31:0] cs;
logic signed [31:0] c;
logic signed [31:0] sc;

// Next-state combinational logic
always_comb begin : static_const_thread_comb     // test_const_remove.cpp:229:5
    static_const_thread_func;
end
function void static_const_thread_func;
    logic [11:0] scu;
    integer k;
    scu = 52;
    k = c % cs;
    k = sc + scu;
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge nrst) 
begin : static_const_thread_ff
    if ( ~nrst ) begin
        c = 42;    // Call of ff()
        cs = 42 - 1;
        sc = 51;
    end
    else begin
    end
end

//------------------------------------------------------------------------------
// Method process: sig_init_method (test_const_remove.cpp:253:5) 

always_comb 
begin : sig_init_method     // test_const_remove.cpp:253:5
    integer LC0;
    integer LC1;
    logic [3:0] LC2;
    integer h;
    LC0 = 42;
    LC1 = t;
    LC2 = s;
    h = LC0 + LC1 + LC2;
end

//------------------------------------------------------------------------------
// Clocked THREAD: sig_init_thread (test_const_remove.cpp:263:5) 

// Thread-local constants
logic signed [31:0] TC1;
logic signed [31:0] TC2;
logic signed [31:0] TC3;

// Next-state combinational logic
always_comb begin : sig_init_thread_comb     // test_const_remove.cpp:263:5
    sig_init_thread_func;
end
function void sig_init_thread_func;
    logic [3:0] TC4;
    integer TC5;
    integer TC6;
    integer n;
    integer m;
    TC4 = s;
    TC5 = 42;    // Call of f()
    TC6 = 44;
    n = TC4 + TC5 + TC6;
    m = TC0 + TC1 + TC2 + TC3;
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge nrst) 
begin : sig_init_thread_ff
    if ( ~nrst ) begin
        TC1 = t;
        TC2 = 42;    // Call of f()
        TC3 = 43;
    end
    else begin
    end
end

//------------------------------------------------------------------------------
// Method process: no_sens_method (test_const_remove.cpp:281:5) 

integer L11;
integer a;
assign L11 = 45;
assign a = C2 + D1 + L11;

//------------------------------------------------------------------------------
// Method process: const_method (test_const_remove.cpp:286:5) 

always_comb 
begin : const_method     // test_const_remove.cpp:286:5
    integer L10;
    integer unsigned b;
    L10 = 45;
    b = C3 + D2 + L10 + M;
end

//------------------------------------------------------------------------------
// Method process: const_array_method (test_const_remove.cpp:291:5) 

always_comb 
begin : const_array_method     // test_const_remove.cpp:291:5
    integer unsigned sum;
    integer e;
    sum = 0;
    for (integer i = 0; i < 3; ++i)
    begin
        sum = sum + ARR[i];
    end
    e = ARR1;
end

//------------------------------------------------------------------------------
// Clocked THREAD: const_thread (test_const_remove.cpp:300:5) 

// Thread-local variables
logic const_thread_PROC_STATE;
logic const_thread_PROC_STATE_next;

// Thread-local constants
logic signed [31:0] L1;

// Next-state combinational logic
always_comb begin : const_thread_comb     // test_const_remove.cpp:300:5
    const_thread_func;
end
function void const_thread_func;
    integer L2;
    integer L3;
    integer c_1;
    integer L4;
    integer unsigned d;
    integer L5;
    const_thread_PROC_STATE_next = const_thread_PROC_STATE;
    
    case (const_thread_PROC_STATE)
        0: begin
            L4 = 44 + 2;
            d = C4 + L1 + L4 + D3;
            const_thread_PROC_STATE_next = 1; return;    // test_const_remove.cpp:310:13;
        end
        1: begin
            L5 = 44;
            L4 = 44 + 2;
            d = C4 + L1 + L4 + D3;
            const_thread_PROC_STATE_next = 1; return;    // test_const_remove.cpp:310:13;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge nrst) 
begin : const_thread_ff
    if ( ~nrst ) begin
        integer L2;
        integer L3;
        integer c_1;
        L1 = 44;
        L2 = 44;
        L3 = 44;
        c_1 = L2;
        const_thread_PROC_STATE <= 0;    // test_const_remove.cpp:305:9;
    end
    else begin
        const_thread_PROC_STATE <= const_thread_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Method process: const_local_array_method (test_const_remove.cpp:315:5) 

always_comb 
begin : const_local_array_method     // test_const_remove.cpp:315:5
    integer unsigned LARR[3];
    integer unsigned LLARR[3];
    integer unsigned LARR1;
    integer unsigned lsum;
    integer f;
    LARR[0] = 1; LARR[1] = 2; LARR[2] = 3;
    LLARR[0] = 1; LLARR[1] = 2; LLARR[2] = 3;
    LARR1 = LARR[1];
    lsum = 0;
    for (integer i = 0; i < 3; ++i)
    begin
        lsum = lsum + LARR[i];
    end
    f = LARR1 + LLARR[1];
end

//------------------------------------------------------------------------------
// Clocked THREAD: const_local_array_thread (test_const_remove.cpp:328:5) 

// Thread-local constants
logic [31:0] TARR[3];
logic [31:0] TARR1;

// Next-state combinational logic
always_comb begin : const_local_array_thread_comb     // test_const_remove.cpp:328:5
    const_local_array_thread_func;
end
function void const_local_array_thread_func;
    integer unsigned tsum;
    integer g;
    tsum = 0;
    for (integer i = 0; i < 3; ++i)
    begin
        tsum = tsum + TARR[i];
    end
    g = TARR1;
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge nrst) 
begin : const_local_array_thread_ff
    if ( ~nrst ) begin
        TARR[0] = 1; TARR[1] = 2; TARR[2] = 3;
        TARR1 = TARR[1];
    end
    else begin
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: const_ref_call_thread (test_const_remove.cpp:351:5) 

// Next-state combinational logic
always_comb begin : const_ref_call_thread_comb     // test_const_remove.cpp:351:5
    const_ref_call_thread_func;
end
function void const_ref_call_thread_func;
    integer aa;
    // Call g() begin
    aa = C1 + 1;
    // Call g() end
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge nrst) 
begin : const_ref_call_thread_ff
    if ( ~nrst ) begin
    end
    else begin
    end
end

//------------------------------------------------------------------------------
// Method process: const_record_method (test_const_remove.cpp:375:5) 

always_comb 
begin : const_record_method     // test_const_remove.cpp:375:5
    logic a_1;
    logic [3:0] b;
    logic rec_a;
    logic [3:0] rec_b;
    logic c_1;
    a_1 = 0; b = 1;
    rec_a = a_1;
    rec_b = b;
    // Call Simple() begin
    // Call Simple() end
    c_1 = rec_a || grec1_a;
end

//------------------------------------------------------------------------------
// Clocked THREAD: const_record_thread (test_const_remove.cpp:381:5) 

// Thread-local variables
logic [3:0] trec3_b;
logic [3:0] trec3_b_next;
logic const_record_thread_PROC_STATE;
logic const_record_thread_PROC_STATE_next;

// Thread-local constants
logic [3:0] trec1_b;

// Next-state combinational logic
always_comb begin : const_record_thread_comb     // test_const_remove.cpp:381:5
    const_record_thread_func;
end
function void const_record_thread_func;
    logic a_1;
    logic [3:0] b;
    logic trec1_a;
    logic c_1;
    logic trec2_a;
    logic [3:0] trec2_b;
    logic trec3_a;
    integer i;
    trec3_b_next = trec3_b;
    const_record_thread_PROC_STATE_next = const_record_thread_PROC_STATE;
    
    case (const_record_thread_PROC_STATE)
        0: begin
            a_1 = 1; b = 2;
            trec2_a = a_1;
            trec2_b = b;
            // Call Simple() begin
            // Call Simple() end
            a_1 = 1; b = 3;
            trec3_a = a_1;
            trec3_b_next = b;
            // Call Simple() begin
            // Call Simple() end
            i = trec1_b + trec2_b;
            const_record_thread_PROC_STATE_next = 1; return;    // test_const_remove.cpp:392:13;
        end
        1: begin
            i = trec3_b_next + grec2_b;
            a_1 = 1; b = 2;
            trec2_a = a_1;
            trec2_b = b;
            // Call Simple() begin
            // Call Simple() end
            a_1 = 1; b = 3;
            trec3_a = a_1;
            trec3_b_next = b;
            // Call Simple() begin
            // Call Simple() end
            i = trec1_b + trec2_b;
            const_record_thread_PROC_STATE_next = 1; return;    // test_const_remove.cpp:392:13;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge nrst) 
begin : const_record_thread_ff
    if ( ~nrst ) begin
        logic a_1;
        logic [3:0] b;
        logic trec1_a;
        logic c_1;
        a_1 = 0; b = 1;
        trec1_a = a_1;
        trec1_b = b;
        // Call Simple() begin
        // Call Simple() end
        c_1 = trec1_a;
        const_record_thread_PROC_STATE <= 0;    // test_const_remove.cpp:386:9;
    end
    else begin
        trec3_b <= trec3_b_next;
        const_record_thread_PROC_STATE <= const_record_thread_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: const_loc_thread (test_const_remove.cpp:397:5) 

// Thread-local variables
logic signed [31:0] CC;
logic signed [31:0] CC_next;
logic const_loc_thread_PROC_STATE;
logic const_loc_thread_PROC_STATE_next;

// Thread-local constants
logic signed [31:0] DD;

// Next-state combinational logic
always_comb begin : const_loc_thread_comb     // test_const_remove.cpp:397:5
    const_loc_thread_func;
end
function void const_loc_thread_func;
    integer EE;
    integer i;
    CC_next = CC;
    const_loc_thread_PROC_STATE_next = const_loc_thread_PROC_STATE;
    
    case (const_loc_thread_PROC_STATE)
        0: begin
            CC_next = 11;
            const_loc_thread_PROC_STATE_next = 1; return;    // test_const_remove.cpp:405:13;
        end
        1: begin
            i = CC_next + DD;
            CC_next = 11;
            const_loc_thread_PROC_STATE_next = 1; return;    // test_const_remove.cpp:405:13;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge nrst) 
begin : const_loc_thread_ff
    if ( ~nrst ) begin
        integer EE;
        EE = 22;
        DD = EE + 1;
        const_loc_thread_PROC_STATE <= 0;    // test_const_remove.cpp:401:9;
    end
    else begin
        CC <= CC_next;
        const_loc_thread_PROC_STATE <= const_loc_thread_PROC_STATE_next;
    end
end

`ifndef INTEL_SVA_OFF
sctAssertLine248 : assert property (
    @(posedge clk) 1 |-> s != MUX_RATE - 1 );
`endif // INTEL_SVA_OFF

endmodule


