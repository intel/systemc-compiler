//==============================================================================
//
// The code is generated by Intel Compiler for SystemC, version 1.6.2
// see more information at https://github.com/intel/systemc-compiler
//
//==============================================================================

//==============================================================================
//
// Module: top ()
//
module top // "top_inst"
(
    input logic clk
);

// Variables generated for SystemC signals
logic arstn;
logic signed [31:0] in;
logic signed [31:0] out;
logic signed [31:0] s;
logic signed [31:0] tt1;
logic signed [31:0] tt2;
logic signed [31:0] tt3;
logic signed [31:0] t0;
logic signed [31:0] t1;
logic signed [31:0] t2;
logic signed [31:0] t3;
logic signed [31:0] s1;
logic signed [31:0] s2;
logic signed [31:0] s3;

//------------------------------------------------------------------------------
// Clocked THREAD: while_with_wait0 (test_while.cpp:68:5) 

// Thread-local variables
logic signed [31:0] i;
logic signed [31:0] i_next;
logic while_with_wait0_PROC_STATE;
logic while_with_wait0_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : while_with_wait0_comb     // test_while.cpp:68:5
    while_with_wait0_func;
end
function void while_with_wait0_func;
    i_next = i;
    while_with_wait0_PROC_STATE_next = while_with_wait0_PROC_STATE;
    
    case (while_with_wait0_PROC_STATE)
        0: begin
            i_next = 0;
            while_with_wait0_PROC_STATE_next = 1; return;    // test_while.cpp:76:17;
        end
        1: begin
            i_next++;
            if (i_next < 3)
            begin
                while_with_wait0_PROC_STATE_next = 1; return;    // test_while.cpp:76:17;
            end
            i_next = 0;
            while_with_wait0_PROC_STATE_next = 1; return;    // test_while.cpp:76:17;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge arstn) 
begin : while_with_wait0_ff
    if ( ~arstn ) begin
        while_with_wait0_PROC_STATE <= 0;    // test_while.cpp:70:9;
    end
    else begin
        i <= i_next;
        while_with_wait0_PROC_STATE <= while_with_wait0_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: while_with_wait0a (test_while.cpp:83:5) 

// Thread-local variables
logic signed [31:0] i0;
logic signed [31:0] i_next0;
logic [1:0] while_with_wait0a_PROC_STATE;
logic [1:0] while_with_wait0a_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : while_with_wait0a_comb     // test_while.cpp:83:5
    while_with_wait0a_func;
end
function void while_with_wait0a_func;
    i_next0 = i0;
    while_with_wait0a_PROC_STATE_next = while_with_wait0a_PROC_STATE;
    
    case (while_with_wait0a_PROC_STATE)
        0: begin
            if (i_next0 < 3)
            begin
                while_with_wait0a_PROC_STATE_next = 1; return;    // test_while.cpp:90:17;
            end
            while_with_wait0a_PROC_STATE_next = 0; return;    // test_while.cpp:93:13;
        end
        1: begin
            i_next0++;
            if (i_next0 < 3)
            begin
                while_with_wait0a_PROC_STATE_next = 1; return;    // test_while.cpp:90:17;
            end
            while_with_wait0a_PROC_STATE_next = 0; return;    // test_while.cpp:93:13;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge arstn) 
begin : while_with_wait0a_ff
    if ( ~arstn ) begin
        i0 <= 0;
        while_with_wait0a_PROC_STATE <= 0;    // test_while.cpp:86:9;
    end
    else begin
        i0 <= i_next0;
        while_with_wait0a_PROC_STATE <= while_with_wait0a_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: while_with_wait0b (test_while.cpp:97:5) 

// Thread-local variables
logic signed [31:0] i1;
logic signed [31:0] i_next1;
logic [1:0] while_with_wait0b_PROC_STATE;
logic [1:0] while_with_wait0b_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : while_with_wait0b_comb     // test_while.cpp:97:5
    while_with_wait0b_func;
end
function void while_with_wait0b_func;
    i_next1 = i1;
    while_with_wait0b_PROC_STATE_next = while_with_wait0b_PROC_STATE;
    
    case (while_with_wait0b_PROC_STATE)
        0: begin
            if (i_next1 < s)
            begin
                while_with_wait0b_PROC_STATE_next = 1; return;    // test_while.cpp:104:17;
            end
            while_with_wait0b_PROC_STATE_next = 0; return;    // test_while.cpp:107:13;
        end
        1: begin
            i_next1++;
            if (i_next1 < s)
            begin
                while_with_wait0b_PROC_STATE_next = 1; return;    // test_while.cpp:104:17;
            end
            while_with_wait0b_PROC_STATE_next = 0; return;    // test_while.cpp:107:13;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge arstn) 
begin : while_with_wait0b_ff
    if ( ~arstn ) begin
        i1 <= 0;
        while_with_wait0b_PROC_STATE <= 0;    // test_while.cpp:100:9;
    end
    else begin
        i1 <= i_next1;
        while_with_wait0b_PROC_STATE <= while_with_wait0b_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: while_with_wait1 (test_while.cpp:112:5) 

// Thread-local variables
logic signed [31:0] out_next;
logic signed [31:0] i2;
logic signed [31:0] i_next2;
logic [1:0] while_with_wait1_PROC_STATE;
logic [1:0] while_with_wait1_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : while_with_wait1_comb     // test_while.cpp:112:5
    while_with_wait1_func;
end
function void while_with_wait1_func;
    i_next2 = i2;
    out_next = out;
    while_with_wait1_PROC_STATE_next = while_with_wait1_PROC_STATE;
    
    case (while_with_wait1_PROC_STATE)
        0: begin
            i_next2 = 0;
            i_next2++;
            out_next = 1;
            while_with_wait1_PROC_STATE_next = 1; return;    // test_while.cpp:123:17;
        end
        1: begin
            if (i_next2 < 3)
            begin
                i_next2++;
                out_next = 1;
                while_with_wait1_PROC_STATE_next = 1; return;    // test_while.cpp:123:17;
            end
            out_next = 2;
            while_with_wait1_PROC_STATE_next = 0; return;    // test_while.cpp:126:13;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge arstn) 
begin : while_with_wait1_ff
    if ( ~arstn ) begin
        out <= 0;
        while_with_wait1_PROC_STATE <= 0;    // test_while.cpp:115:9;
    end
    else begin
        out <= out_next;
        i2 <= i_next2;
        while_with_wait1_PROC_STATE <= while_with_wait1_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: while_with_wait2 (test_while.cpp:132:5) 

// Thread-local variables
logic signed [31:0] tt1_next;
logic signed [31:0] i3;
logic signed [31:0] i_next3;
logic [1:0] while_with_wait2_PROC_STATE;
logic [1:0] while_with_wait2_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : while_with_wait2_comb     // test_while.cpp:132:5
    while_with_wait2_func;
end
function void while_with_wait2_func;
    i_next3 = i3;
    tt1_next = tt1;
    while_with_wait2_PROC_STATE_next = while_with_wait2_PROC_STATE;
    
    case (while_with_wait2_PROC_STATE)
        0: begin
            i_next3 = 0;
            i_next3++;
            tt1_next = 1;
            while_with_wait2_PROC_STATE_next = 1; return;    // test_while.cpp:143:17;
        end
        1: begin
            if (in > 1)
            begin
                tt1_next = 2;
                while_with_wait2_PROC_STATE_next = 2; return;    // test_while.cpp:147:21;
            end
            if (i_next3 < 3)
            begin
                i_next3++;
                tt1_next = 1;
                while_with_wait2_PROC_STATE_next = 1; return;    // test_while.cpp:143:17;
            end
            tt1_next = 3;
            while_with_wait2_PROC_STATE_next = 0; return;    // test_while.cpp:151:13;
        end
        2: begin
            if (i_next3 < 3)
            begin
                i_next3++;
                tt1_next = 1;
                while_with_wait2_PROC_STATE_next = 1; return;    // test_while.cpp:143:17;
            end
            tt1_next = 3;
            while_with_wait2_PROC_STATE_next = 0; return;    // test_while.cpp:151:13;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge arstn) 
begin : while_with_wait2_ff
    if ( ~arstn ) begin
        tt1 <= 0;
        while_with_wait2_PROC_STATE <= 0;    // test_while.cpp:135:9;
    end
    else begin
        tt1 <= tt1_next;
        i3 <= i_next3;
        while_with_wait2_PROC_STATE <= while_with_wait2_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: while_with_for (test_while.cpp:157:5) 

// Thread-local variables
logic signed [31:0] tt2_next;
logic signed [31:0] j;
logic signed [31:0] j_next;
logic signed [31:0] i4;
logic signed [31:0] i_next4;
logic [1:0] while_with_for_PROC_STATE;
logic [1:0] while_with_for_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : while_with_for_comb     // test_while.cpp:157:5
    while_with_for_func;
end
function void while_with_for_func;
    i_next4 = i4;
    j_next = j;
    tt2_next = tt2;
    while_with_for_PROC_STATE_next = while_with_for_PROC_STATE;
    
    case (while_with_for_PROC_STATE)
        0: begin
            i_next4 = 0;
            i_next4++;
            tt2_next = 1;
            j_next = 0;
            if (in > 1)
            begin
                tt2_next = j_next;
            end
            while_with_for_PROC_STATE_next = 1; return;    // test_while.cpp:173:21;
        end
        1: begin
            j_next++;
            if (j_next < 2)
            begin
                if (in > 1)
                begin
                    tt2_next = j_next;
                end
                while_with_for_PROC_STATE_next = 1; return;    // test_while.cpp:173:21;
            end
            if (i_next4 < 3)
            begin
                i_next4++;
                tt2_next = 1;
                j_next = 0;
                if (in > 1)
                begin
                    tt2_next = j_next;
                end
                while_with_for_PROC_STATE_next = 1; return;    // test_while.cpp:173:21;
            end
            tt2_next = 3;
            while_with_for_PROC_STATE_next = 0; return;    // test_while.cpp:177:13;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge arstn) 
begin : while_with_for_ff
    if ( ~arstn ) begin
        tt2 <= 0;
        while_with_for_PROC_STATE <= 0;    // test_while.cpp:160:9;
    end
    else begin
        tt2 <= tt2_next;
        j <= j_next;
        i4 <= i_next4;
        while_with_for_PROC_STATE <= while_with_for_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: while_with_signal_cond (test_while.cpp:183:5) 

// Thread-local variables
logic signed [31:0] tt3_next;
logic [1:0] while_with_signal_cond_PROC_STATE;
logic [1:0] while_with_signal_cond_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : while_with_signal_cond_comb     // test_while.cpp:183:5
    while_with_signal_cond_func;
end
function void while_with_signal_cond_func;
    tt3_next = tt3;
    while_with_signal_cond_PROC_STATE_next = while_with_signal_cond_PROC_STATE;
    
    case (while_with_signal_cond_PROC_STATE)
        0: begin
            if (|in)
            begin
                tt3_next = 1;
                while_with_signal_cond_PROC_STATE_next = 0; return;    // test_while.cpp:192:17;
            end
            tt3_next = 2;
            while_with_signal_cond_PROC_STATE_next = 0; return;    // test_while.cpp:196:13;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge arstn) 
begin : while_with_signal_cond_ff
    if ( ~arstn ) begin
        tt3 <= 0;
        while_with_signal_cond_PROC_STATE <= 0;    // test_while.cpp:186:9;
    end
    else begin
        tt3 <= tt3_next;
        while_with_signal_cond_PROC_STATE <= while_with_signal_cond_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: while_with_binary_oper (test_while.cpp:202:5) 

// Thread-local variables
logic b1;
logic b1_next;
logic b2;
logic b2_next;
logic signed [31:0] k;
logic signed [31:0] k_next;
logic signed [31:0] t0_next;
logic [1:0] while_with_binary_oper_PROC_STATE;
logic [1:0] while_with_binary_oper_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : while_with_binary_oper_comb     // test_while.cpp:202:5
    while_with_binary_oper_func;
end
function void while_with_binary_oper_func;
    b1_next = b1;
    b2_next = b2;
    k_next = k;
    t0_next = t0;
    while_with_binary_oper_PROC_STATE_next = while_with_binary_oper_PROC_STATE;
    
    case (while_with_binary_oper_PROC_STATE)
        0: begin
            if (b1_next || b2_next)
            begin
                k_next = 1;
                while_with_binary_oper_PROC_STATE_next = 1; return;    // test_while.cpp:211:17;
            end
            t0_next = k_next;
            while_with_binary_oper_PROC_STATE_next = 0; return;    // test_while.cpp:215:13;
        end
        1: begin
            k_next = 2;
            if (b1_next || b2_next)
            begin
                k_next = 1;
                while_with_binary_oper_PROC_STATE_next = 1; return;    // test_while.cpp:211:17;
            end
            t0_next = k_next;
            while_with_binary_oper_PROC_STATE_next = 0; return;    // test_while.cpp:215:13;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge arstn) 
begin : while_with_binary_oper_ff
    if ( ~arstn ) begin
        k <= 0;
        while_with_binary_oper_PROC_STATE <= 0;    // test_while.cpp:206:9;
    end
    else begin
        b1 <= b1_next;
        b2 <= b2_next;
        k <= k_next;
        t0 <= t0_next;
        while_with_binary_oper_PROC_STATE <= while_with_binary_oper_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: while_with_binary_oper1 (test_while.cpp:220:5) 

// Thread-local variables
logic b10;
logic b1_next0;
logic signed [31:0] k0;
logic signed [31:0] k_next0;
logic signed [31:0] t1_next;
logic [1:0] while_with_binary_oper1_PROC_STATE;
logic [1:0] while_with_binary_oper1_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : while_with_binary_oper1_comb     // test_while.cpp:220:5
    while_with_binary_oper1_func;
end
function void while_with_binary_oper1_func;
    b1_next0 = b10;
    k_next0 = k0;
    t1_next = t1;
    while_with_binary_oper1_PROC_STATE_next = while_with_binary_oper1_PROC_STATE;
    
    case (while_with_binary_oper1_PROC_STATE)
        0: begin
            if (b1_next0 && |s)
            begin
                k_next0 = 1;
                while_with_binary_oper1_PROC_STATE_next = 1; return;    // test_while.cpp:229:17;
            end
            t1_next = k_next0;
            while_with_binary_oper1_PROC_STATE_next = 0; return;    // test_while.cpp:233:13;
        end
        1: begin
            k_next0 = 2;
            if (b1_next0 && |s)
            begin
                k_next0 = 1;
                while_with_binary_oper1_PROC_STATE_next = 1; return;    // test_while.cpp:229:17;
            end
            t1_next = k_next0;
            while_with_binary_oper1_PROC_STATE_next = 0; return;    // test_while.cpp:233:13;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge arstn) 
begin : while_with_binary_oper1_ff
    if ( ~arstn ) begin
        k0 <= 0;
        while_with_binary_oper1_PROC_STATE <= 0;    // test_while.cpp:224:9;
    end
    else begin
        b10 <= b1_next0;
        k0 <= k_next0;
        t1 <= t1_next;
        while_with_binary_oper1_PROC_STATE <= while_with_binary_oper1_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: while_with_binary_oper2 (test_while.cpp:239:5) 

// Thread-local variables
logic b11;
logic b1_next1;
logic b20;
logic b2_next0;
logic b3;
logic b3_next;
logic signed [31:0] k1;
logic signed [31:0] k_next1;
logic signed [31:0] t2_next;
logic [1:0] while_with_binary_oper2_PROC_STATE;
logic [1:0] while_with_binary_oper2_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : while_with_binary_oper2_comb     // test_while.cpp:239:5
    while_with_binary_oper2_func;
end
function void while_with_binary_oper2_func;
    b1_next1 = b11;
    b2_next0 = b20;
    b3_next = b3;
    k_next1 = k1;
    t2_next = t2;
    while_with_binary_oper2_PROC_STATE_next = while_with_binary_oper2_PROC_STATE;
    
    case (while_with_binary_oper2_PROC_STATE)
        0: begin
            if ((b1_next1 || b2_next0) && b3_next)
            begin
                k_next1 = 1;
                while_with_binary_oper2_PROC_STATE_next = 1; return;    // test_while.cpp:248:17;
            end
            t2_next = k_next1;
            while_with_binary_oper2_PROC_STATE_next = 0; return;    // test_while.cpp:252:13;
        end
        1: begin
            k_next1 = 2;
            if ((b1_next1 || b2_next0) && b3_next)
            begin
                k_next1 = 1;
                while_with_binary_oper2_PROC_STATE_next = 1; return;    // test_while.cpp:248:17;
            end
            t2_next = k_next1;
            while_with_binary_oper2_PROC_STATE_next = 0; return;    // test_while.cpp:252:13;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge arstn) 
begin : while_with_binary_oper2_ff
    if ( ~arstn ) begin
        k1 <= 0;
        while_with_binary_oper2_PROC_STATE <= 0;    // test_while.cpp:243:9;
    end
    else begin
        b11 <= b1_next1;
        b20 <= b2_next0;
        b3 <= b3_next;
        k1 <= k_next1;
        t2 <= t2_next;
        while_with_binary_oper2_PROC_STATE <= while_with_binary_oper2_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: while_with_binary_oper3 (test_while.cpp:257:5) 

// Thread-local variables
logic b12;
logic b1_next2;
logic b30;
logic b3_next0;
logic signed [31:0] k2;
logic signed [31:0] k_next2;
logic signed [31:0] t3_next;
logic [1:0] while_with_binary_oper3_PROC_STATE;
logic [1:0] while_with_binary_oper3_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : while_with_binary_oper3_comb     // test_while.cpp:257:5
    while_with_binary_oper3_func;
end
function void while_with_binary_oper3_func;
    b1_next2 = b12;
    b3_next0 = b30;
    k_next2 = k2;
    t3_next = t3;
    while_with_binary_oper3_PROC_STATE_next = while_with_binary_oper3_PROC_STATE;
    
    case (while_with_binary_oper3_PROC_STATE)
        0: begin
            if ((b1_next2 && |s) || b3_next0)
            begin
                k_next2 = 1;
                while_with_binary_oper3_PROC_STATE_next = 1; return;    // test_while.cpp:266:17;
            end
            t3_next = k_next2;
            while_with_binary_oper3_PROC_STATE_next = 0; return;    // test_while.cpp:270:13;
        end
        1: begin
            k_next2 = 2;
            if ((b1_next2 && |s) || b3_next0)
            begin
                k_next2 = 1;
                while_with_binary_oper3_PROC_STATE_next = 1; return;    // test_while.cpp:266:17;
            end
            t3_next = k_next2;
            while_with_binary_oper3_PROC_STATE_next = 0; return;    // test_while.cpp:270:13;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge arstn) 
begin : while_with_binary_oper3_ff
    if ( ~arstn ) begin
        k2 <= 0;
        while_with_binary_oper3_PROC_STATE <= 0;    // test_while.cpp:261:9;
    end
    else begin
        b12 <= b1_next2;
        b30 <= b3_next0;
        k2 <= k_next2;
        t3 <= t3_next;
        while_with_binary_oper3_PROC_STATE <= while_with_binary_oper3_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: while_continue1 (test_while.cpp:292:5) 

// Thread-local variables
logic signed [31:0] s2_next;
logic [1:0] while_continue1_PROC_STATE;
logic [1:0] while_continue1_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : while_continue1_comb     // test_while.cpp:292:5
    while_continue1_func;
end
function void while_continue1_func;
    s2_next = s2;
    while_continue1_PROC_STATE_next = while_continue1_PROC_STATE;
    
    case (while_continue1_PROC_STATE)
        0: begin
            if (!(|s))
            begin
                while_continue1_PROC_STATE_next = 1; return;    // test_while.cpp:300:17;
            end
            if (|s)
            begin
                while_continue1_PROC_STATE_next = 2; return;    // test_while.cpp:305:17;
            end
            while_continue1_PROC_STATE_next = 0; return;    // test_while.cpp:311:13;
        end
        1: begin
            s2_next = 2;
            if (!(|s))
            begin
                while_continue1_PROC_STATE_next = 1; return;    // test_while.cpp:300:17;
            end
            if (|s)
            begin
                while_continue1_PROC_STATE_next = 2; return;    // test_while.cpp:305:17;
            end
            while_continue1_PROC_STATE_next = 0; return;    // test_while.cpp:311:13;
        end
        2: begin
            if (|s1)
            begin
                s2_next = 1;
                // continue begin
                if (|s)
                begin
                    while_continue1_PROC_STATE_next = 2; return;    // test_while.cpp:305:17;
                end
                while_continue1_PROC_STATE_next = 0; return;    // test_while.cpp:311:13;
                // continue end
            end
            if (|s)
            begin
                while_continue1_PROC_STATE_next = 2; return;    // test_while.cpp:305:17;
            end
            while_continue1_PROC_STATE_next = 0; return;    // test_while.cpp:311:13;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge arstn) 
begin : while_continue1_ff
    if ( ~arstn ) begin
        s2 <= 0;
        while_continue1_PROC_STATE <= 0;    // test_while.cpp:295:9;
    end
    else begin
        s2 <= s2_next;
        while_continue1_PROC_STATE <= while_continue1_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: while_break1 (test_while.cpp:316:5) 

// Thread-local variables
logic signed [31:0] s3_next;
logic [2:0] while_break1_PROC_STATE;
logic [2:0] while_break1_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : while_break1_comb     // test_while.cpp:316:5
    while_break1_func;
end
function void while_break1_func;
    s3_next = s3;
    while_break1_PROC_STATE_next = while_break1_PROC_STATE;
    
    case (while_break1_PROC_STATE)
        0: begin
            s3_next = 0;
            while_break1_PROC_STATE_next = 1; return;    // test_while.cpp:324:17;
        end
        1: begin
            if (|s1)
            begin
                // break begin
                if (|s1)
                begin
                    if (|s2)
                    begin
                        while_break1_PROC_STATE_next = 2; return;    // test_while.cpp:332:21;
                    end
                    if (s1 > s2)
                    begin
                        while_break1_PROC_STATE_next = 3; return;    // test_while.cpp:335:21;
                    end
                    s3_next = 2;
                    while_break1_PROC_STATE_next = 4; return;    // test_while.cpp:339:17;
                end
                while_break1_PROC_STATE_next = 0; return;    // test_while.cpp:342:13;
                // break end
            end
            s3_next = 1;
            while_break1_PROC_STATE_next = 1; return;    // test_while.cpp:324:17;
        end
        2: begin
            if (|s2)
            begin
                while_break1_PROC_STATE_next = 2; return;    // test_while.cpp:332:21;
            end
            if (s1 > s2)
            begin
                while_break1_PROC_STATE_next = 3; return;    // test_while.cpp:335:21;
            end
            s3_next = 2;
            while_break1_PROC_STATE_next = 4; return;    // test_while.cpp:339:17;
        end
        3: begin
            // break begin
            while_break1_PROC_STATE_next = 0; return;    // test_while.cpp:342:13;
            // break end
        end
        4: begin
            if (|s1)
            begin
                if (|s2)
                begin
                    while_break1_PROC_STATE_next = 2; return;    // test_while.cpp:332:21;
                end
                if (s1 > s2)
                begin
                    while_break1_PROC_STATE_next = 3; return;    // test_while.cpp:335:21;
                end
                s3_next = 2;
                while_break1_PROC_STATE_next = 4; return;    // test_while.cpp:339:17;
            end
            while_break1_PROC_STATE_next = 0; return;    // test_while.cpp:342:13;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge arstn) 
begin : while_break1_ff
    if ( ~arstn ) begin
        while_break1_PROC_STATE <= 0;    // test_while.cpp:318:9;
    end
    else begin
        s3 <= s3_next;
        while_break1_PROC_STATE <= while_break1_PROC_STATE_next;
    end
end

endmodule


