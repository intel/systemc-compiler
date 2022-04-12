//==============================================================================
//
// The code is generated by Intel Compiler for SystemC, version 1.4.18
// see more information at https://github.com/intel/systemc-compiler
//
//==============================================================================

//==============================================================================
//
// Module: B_top ()
//
module B_top // "b_mod"
(
);

// Variables generated for SystemC signals
logic a;
logic b;
logic clk;
logic nrst;


//------------------------------------------------------------------------------
// Child module instances

A a_mod
(
  .clk(clk),
  .nrst(nrst),
  .a(a),
  .b(b)
);

endmodule



//==============================================================================
//
// Module: A (test_const_prop_loop.cpp:329:5)
//
module A // "b_mod.a_mod"
(
    input logic clk,
    input logic nrst,
    input logic a,
    output logic b
);

// Variables generated for SystemC signals
logic signed [31:0] s;

//------------------------------------------------------------------------------
// Method process: loc_var_loop (test_const_prop_loop.cpp:60:5) 

always_comb 
begin : loc_var_loop     // test_const_prop_loop.cpp:60:5
    integer unsigned slot;
    slot = 0;
    for (integer i = 0; i < s; i++)
    begin
        slot = (slot == 2) ? 0 : slot + 1;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: threadProc (test_const_prop_loop.cpp:67:5) 

// Thread-local variables
logic [31:0] slot0;
logic [31:0] slot_next;

// Next-state combinational logic
always_comb begin : threadProc_comb     // test_const_prop_loop.cpp:67:5
    threadProc_func;
end
function void threadProc_func;
    slot_next = slot0;
    slot_next = (slot_next == 2) ? 0 : slot_next + 1;
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge nrst) 
begin : threadProc_ff
    if ( ~nrst ) begin
        slot0 <= 0;
    end
    else begin
        slot0 <= slot_next;
    end
end

//------------------------------------------------------------------------------
// Method process: unknown_cond_loop (test_const_prop_loop.cpp:80:5) 

always_comb 
begin : unknown_cond_loop     // test_const_prop_loop.cpp:80:5
    integer m_1;
    m_1 = 0;
    for (integer i = 0; i < s; i++)
    begin
        m_1++;
    end
end

//------------------------------------------------------------------------------
// Method process: unstable_loop1 (test_const_prop_loop.cpp:90:5) 

always_comb 
begin : unstable_loop1     // test_const_prop_loop.cpp:90:5
    integer k_1;
    k_1 = 0;
    for (integer i = 0; i < 100; i++)
    begin
        if (i == 99)
        begin
            k_1 = 1;
        end
    end
end

//------------------------------------------------------------------------------
// Method process: unstable_loop2 (test_const_prop_loop.cpp:99:5) 

always_comb 
begin : unstable_loop2     // test_const_prop_loop.cpp:99:5
    integer k_1;
    integer m_1;
    logic b_1;
    integer l;
    k_1 = 0;
    m_1 = 0;
    b_1 = 0;
    for (integer i = 0; i < 100; i++)
    begin
        b_1 = !b_1;
        if (b_1)
        begin
            k_1 = 1;
        end
    end
    if (b_1)
    begin
        m_1 = 1;
    end
    l = 1;
end

//------------------------------------------------------------------------------
// Method process: unstable_loop3 (test_const_prop_loop.cpp:117:5) 

always_comb 
begin : unstable_loop3     // test_const_prop_loop.cpp:117:5
    logic l1;
    logic l2;
    logic l3;
    l1 = 0;
    l2 = 0;
    l3 = 0;
    for (integer i = 0; i < 1026; i++)
    begin
        if (l2)
        begin
            l3 = 1;
        end
        if (l1)
        begin
            l2 = 1;
        end
        if (i > 20)
        begin
            l1 = 1;
        end
    end
end

//------------------------------------------------------------------------------
// Method process: unstable_loop3a (test_const_prop_loop.cpp:131:5) 

always_comb 
begin : unstable_loop3a     // test_const_prop_loop.cpp:131:5
    logic first;
    integer unsigned slot;
    first = 1;
    slot = 0;
    for (integer i = 0; i < 1026; i++)
    begin
        slot = (slot == 1 && !first) ? 0 : slot + 1;
        first = 0;
    end
end

//------------------------------------------------------------------------------
// Method process: unstable_loop4 (test_const_prop_loop.cpp:141:5) 

always_comb 
begin : unstable_loop4     // test_const_prop_loop.cpp:141:5
    logic l1;
    logic l2;
    logic l3;
    l1 = 0;
    l2 = 0;
    l3 = 0;
    for (integer i = 0; i < 4; i++)
    begin
        if (i == 3)
        begin
            l1 = 1;
        end
    end
end

//------------------------------------------------------------------------------
// Method process: unstable_loop4a (test_const_prop_loop.cpp:155:5) 

always_comb 
begin : unstable_loop4a     // test_const_prop_loop.cpp:155:5
    logic l1;
    logic l2;
    logic l3;
    l1 = 0;
    l2 = 0;
    l3 = 0;
    for (integer i = 0; i < 5; i++)
    begin
        if (l1)
        begin
            l2 = 1;
        end
        if (i == 3)
        begin
            l1 = 1;
        end
    end
end

//------------------------------------------------------------------------------
// Method process: unstable_loop5 (test_const_prop_loop.cpp:171:5) 

always_comb 
begin : unstable_loop5     // test_const_prop_loop.cpp:171:5
    logic l1;
    logic l2;
    logic l3;
    l1 = 0;
    l2 = 0;
    l3 = 0;
    for (integer i = 0; i < 10; i++)
    begin
        if (i == 9)
        begin
            l1 = 1;
        end
    end
end

//------------------------------------------------------------------------------
// Method process: unstable_loop6 (test_const_prop_loop.cpp:185:5) 

always_comb 
begin : unstable_loop6     // test_const_prop_loop.cpp:185:5
    logic l1;
    logic l2;
    logic l3;
    l1 = 0;
    l2 = 0;
    l3 = 0;
    for (integer i = 0; i < 11; i++)
    begin
        if (l2)
        begin
            l3 = 1;
        end
        if (l1)
        begin
            l2 = 1;
        end
        if (i == 10)
        begin
            l1 = 1;
        end
    end
end

//------------------------------------------------------------------------------
// Method process: simple_for1 (test_const_prop_loop.cpp:201:5) 

always_comb 
begin : simple_for1     // test_const_prop_loop.cpp:201:5
    integer m_1;
    m_1 = 0;
    for (integer i = 0; i < 2; i++)
    begin
        m_1++;
    end
end

//------------------------------------------------------------------------------
// Method process: simple_for1a (test_const_prop_loop.cpp:211:5) 

always_comb 
begin : simple_for1a     // test_const_prop_loop.cpp:211:5
    integer m_1;
    integer l;
    m_1 = 0;
    for (integer i = 0; i < 100; i++)
    begin
        m_1++;
    end
    if (|m_1)
    begin
        l = 1;
    end
end

//------------------------------------------------------------------------------
// Method process: simple_for1b (test_const_prop_loop.cpp:221:5) 

always_comb 
begin : simple_for1b     // test_const_prop_loop.cpp:221:5
    logic b_1;
    integer l;
    b_1 = 0;
    for (integer i = 0; i < 100; i++)
    begin
        b_1 = !b_1;
    end
    if (b_1)
    begin
        l = 1;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: simple_for2 (test_const_prop_loop.cpp:231:5) 

// Thread-local variables
logic signed [31:0] i0;
logic signed [31:0] i_next;
logic signed [31:0] k0;
logic signed [31:0] k_next;
logic signed [31:0] m0;
logic signed [31:0] m_next;
logic [1:0] simple_for2_PROC_STATE;
logic [1:0] simple_for2_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : simple_for2_comb     // test_const_prop_loop.cpp:231:5
    simple_for2_func;
end
function void simple_for2_func;
    integer l;
    i_next = i0;
    k_next = k0;
    m_next = m0;
    simple_for2_PROC_STATE_next = simple_for2_PROC_STATE;
    
    case (simple_for2_PROC_STATE)
        0: begin
            m_next = 0;
            k_next = a;
            i_next = 0;
            if (i_next < k_next)
            begin
                m_next = 1;
                simple_for2_PROC_STATE_next = 1; return;    // test_const_prop_loop.cpp:240:17;
            end
            if (|m_next)
            begin
                l = 1;
            end
            simple_for2_PROC_STATE_next = 2; return;    // test_const_prop_loop.cpp:244:13;
        end
        1: begin
            i_next++;
            if (i_next < k_next)
            begin
                m_next = 1;
                simple_for2_PROC_STATE_next = 1; return;    // test_const_prop_loop.cpp:240:17;
            end
            if (|m_next)
            begin
                l = 1;
            end
            simple_for2_PROC_STATE_next = 2; return;    // test_const_prop_loop.cpp:244:13;
        end
        2: begin
            m_next = 0;
            k_next = a;
            i_next = 0;
            if (i_next < k_next)
            begin
                m_next = 1;
                simple_for2_PROC_STATE_next = 1; return;    // test_const_prop_loop.cpp:240:17;
            end
            if (|m_next)
            begin
                l = 1;
            end
            simple_for2_PROC_STATE_next = 2; return;    // test_const_prop_loop.cpp:244:13;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge nrst) 
begin : simple_for2_ff
    if ( ~nrst ) begin
        simple_for2_PROC_STATE <= 0;    // test_const_prop_loop.cpp:232:9;
    end
    else begin
        i0 <= i_next;
        k0 <= k_next;
        m0 <= m_next;
        simple_for2_PROC_STATE <= simple_for2_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Method process: simple_for3 (test_const_prop_loop.cpp:248:5) 

always_comb 
begin : simple_for3     // test_const_prop_loop.cpp:248:5
    integer m_1;
    integer n_1;
    m_1 = 0;
    n_1 = 0;
    for (integer i = 0; i < 2; i++)
    begin
        m_1++;
        for (integer j = 0; j < m_1; j++)
        begin
            n_1++;
        end
    end
end

//------------------------------------------------------------------------------
// Method process: simple_for4 (test_const_prop_loop.cpp:261:5) 

always_comb 
begin : simple_for4     // test_const_prop_loop.cpp:261:5
    integer m_1;
    integer n_1;
    m_1 = 0;
    n_1 = 0;
    for (integer i = 0; i < 2; i++)
    begin
        m_1++;
        for (integer j = 0; j < 3; j++)
        begin
            n_1++;
        end
    end
end

//------------------------------------------------------------------------------
// Method process: continue_in_for1 (test_const_prop_loop.cpp:275:5) 

always_comb 
begin : continue_in_for1     // test_const_prop_loop.cpp:275:5
    integer m_1;
    m_1 = 0;
    for (integer i = 0; i < 2; i++)
    begin
        if (a)
        begin
            continue;
        end
        m_1 = 1;
    end
end

//------------------------------------------------------------------------------
// Method process: continue_in_for2 (test_const_prop_loop.cpp:287:5) 

always_comb 
begin : continue_in_for2     // test_const_prop_loop.cpp:287:5
    integer m_1;
    m_1 = 0;
    for (integer i = 0; i < 2; i++)
    begin
        if (a)
        begin
            continue;
        end else begin
            m_1 = 1;
        end
    end
end

//------------------------------------------------------------------------------
// Method process: continue_in_for3 (test_const_prop_loop.cpp:300:5) 

always_comb 
begin : continue_in_for3     // test_const_prop_loop.cpp:300:5
    integer m_1;
    m_1 = 0;
    for (integer i = 0; i < 5; i++)
    begin
        if (i < 3)
        begin
            continue;
        end
        m_1++;
    end
end

//------------------------------------------------------------------------------
// Method process: dowhile_loop (test_const_prop_loop.cpp:311:5) 

always_comb 
begin : dowhile_loop     // test_const_prop_loop.cpp:311:5
    integer i;
    i = 0;
    do
    begin
        i++;
    end
    while (i < 3);
end

endmodule


