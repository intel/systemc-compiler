//==============================================================================
//
// The code is generated by Intel Compiler for SystemC, version 1.4.18
// see more information at https://github.com/intel/systemc-compiler
//
//==============================================================================

//==============================================================================
//
// Module: top ()
//
module top // "top_inst"
(
);

// Variables generated for SystemC signals
logic clk;
logic arstn;
logic signed [31:0] out;
logic signed [31:0] in;
logic signed [31:0] s1;
logic signed [31:0] s2;
logic signed [31:0] s3;
logic signed [31:0] s4;
logic signed [31:0] s5;
logic signed [31:0] s6;

//------------------------------------------------------------------------------
// Clocked THREAD: dowhile_forever (test_do_while_always1.cpp:52:5) 

// Thread-local variables
logic signed [31:0] out_next;
logic signed [31:0] i;
logic signed [31:0] i_next;
logic dowhile_forever_PROC_STATE;
logic dowhile_forever_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : dowhile_forever_comb     // test_do_while_always1.cpp:52:5
    dowhile_forever_func;
end
function void dowhile_forever_func;
    i_next = i;
    out_next = out;
    dowhile_forever_PROC_STATE_next = dowhile_forever_PROC_STATE;
    
    case (dowhile_forever_PROC_STATE)
        0: begin
            i_next = 0;
            out_next = 1;
            dowhile_forever_PROC_STATE_next = 1; return;    // test_do_while_always1.cpp:62:17;
        end
        1: begin
            i_next++;
            if (i_next < 3)
            begin
                out_next = 1;
                dowhile_forever_PROC_STATE_next = 1; return;    // test_do_while_always1.cpp:62:17;
            end
            out_next = 2;
            i_next = 0;
            out_next = 1;
            dowhile_forever_PROC_STATE_next = 1; return;    // test_do_while_always1.cpp:62:17;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge arstn) 
begin : dowhile_forever_ff
    if ( ~arstn ) begin
        out <= 0;
        dowhile_forever_PROC_STATE <= 0;    // test_do_while_always1.cpp:55:9;
    end
    else begin
        out <= out_next;
        i <= i_next;
        dowhile_forever_PROC_STATE <= dowhile_forever_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: for_ever (test_do_while_always1.cpp:73:5) 

// Thread-local variables
logic signed [31:0] s1_next;
logic signed [31:0] j;
logic signed [31:0] j_next;
logic signed [31:0] i0;
logic signed [31:0] i_next0;
logic [1:0] for_ever_PROC_STATE;
logic [1:0] for_ever_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : for_ever_comb     // test_do_while_always1.cpp:73:5
    for_ever_func;
end
function void for_ever_func;
    i_next0 = i0;
    j_next = j;
    s1_next = s1;
    for_ever_PROC_STATE_next = for_ever_PROC_STATE;
    
    case (for_ever_PROC_STATE)
        0: begin
            i_next0 = 0;
            i_next0++;
            s1_next = 1;
            j_next = 0;
            if (in > 1)
            begin
                s1_next = j_next;
            end
            for_ever_PROC_STATE_next = 1; return;    // test_do_while_always1.cpp:89:21;
        end
        1: begin
            j_next++;
            if (j_next < 2)
            begin
                if (in > 1)
                begin
                    s1_next = j_next;
                end
                for_ever_PROC_STATE_next = 1; return;    // test_do_while_always1.cpp:89:21;
            end
            if (i_next0 < 3)
            begin
                i_next0++;
                s1_next = 1;
                j_next = 0;
                if (in > 1)
                begin
                    s1_next = j_next;
                end
                for_ever_PROC_STATE_next = 1; return;    // test_do_while_always1.cpp:89:21;
            end
            s1_next = 3;
            for_ever_PROC_STATE_next = 2; return;    // test_do_while_always1.cpp:93:13;
        end
        2: begin
            i_next0 = 0;
            i_next0++;
            s1_next = 1;
            j_next = 0;
            if (in > 1)
            begin
                s1_next = j_next;
            end
            for_ever_PROC_STATE_next = 1; return;    // test_do_while_always1.cpp:89:21;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge arstn) 
begin : for_ever_ff
    if ( ~arstn ) begin
        s1 <= 0;
        for_ever_PROC_STATE <= 0;    // test_do_while_always1.cpp:76:9;
    end
    else begin
        s1 <= s1_next;
        j <= j_next;
        i0 <= i_next0;
        for_ever_PROC_STATE <= for_ever_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: dowhile_break1 (test_do_while_always1.cpp:98:5) 

// Thread-local variables
logic signed [31:0] s2_next;
logic signed [31:0] i1;
logic signed [31:0] i_next1;
logic [1:0] dowhile_break1_PROC_STATE;
logic [1:0] dowhile_break1_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : dowhile_break1_comb     // test_do_while_always1.cpp:98:5
    dowhile_break1_func;
end
function void dowhile_break1_func;
    i_next1 = i1;
    s2_next = s2;
    dowhile_break1_PROC_STATE_next = dowhile_break1_PROC_STATE;
    
    case (dowhile_break1_PROC_STATE)
        0: begin
            if (i_next1 > 3)
            begin
                dowhile_break1_PROC_STATE_next = 2; return;    // test_do_while_always1.cpp:108:21;
            end
            i_next1++;
            s2_next = i_next1;
            dowhile_break1_PROC_STATE_next = 1; return;    // test_do_while_always1.cpp:114:17;
        end
        1: begin
            if (i_next1 > 3)
            begin
                dowhile_break1_PROC_STATE_next = 2; return;    // test_do_while_always1.cpp:108:21;
            end
            i_next1++;
            s2_next = i_next1;
            dowhile_break1_PROC_STATE_next = 1; return;    // test_do_while_always1.cpp:114:17;
        end
        2: begin
            // break begin
            dowhile_break1_PROC_STATE_next = 3; return;    // test_do_while_always1.cpp:118:13;
            // break end
        end
        3: begin
            if (i_next1 > 3)
            begin
                dowhile_break1_PROC_STATE_next = 2; return;    // test_do_while_always1.cpp:108:21;
            end
            i_next1++;
            s2_next = i_next1;
            dowhile_break1_PROC_STATE_next = 1; return;    // test_do_while_always1.cpp:114:17;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge arstn) 
begin : dowhile_break1_ff
    if ( ~arstn ) begin
        s2 <= 0;
        i1 <= 0;
        dowhile_break1_PROC_STATE <= 0;    // test_do_while_always1.cpp:102:9;
    end
    else begin
        s2 <= s2_next;
        i1 <= i_next1;
        dowhile_break1_PROC_STATE <= dowhile_break1_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: dowhile_break2 (test_do_while_always1.cpp:123:5) 

// Thread-local variables
logic signed [31:0] s3_next;
logic [1:0] dowhile_break2_PROC_STATE;
logic [1:0] dowhile_break2_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : dowhile_break2_comb     // test_do_while_always1.cpp:123:5
    dowhile_break2_func;
end
function void dowhile_break2_func;
    s3_next = s3;
    dowhile_break2_PROC_STATE_next = dowhile_break2_PROC_STATE;
    
    case (dowhile_break2_PROC_STATE)
        0: begin
            s3_next = s2;
            dowhile_break2_PROC_STATE_next = 1; return;    // test_do_while_always1.cpp:132:17;
        end
        1: begin
            if (|s2)
            begin
                // break begin
                dowhile_break2_PROC_STATE_next = 2; return;    // test_do_while_always1.cpp:143:13;
                // break end
            end else begin
                // continue begin
                s3_next = s2;
                dowhile_break2_PROC_STATE_next = 1; return;    // test_do_while_always1.cpp:132:17;
                // continue end
            end
        end
        2: begin
            s3_next = s2;
            dowhile_break2_PROC_STATE_next = 1; return;    // test_do_while_always1.cpp:132:17;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge arstn) 
begin : dowhile_break2_ff
    if ( ~arstn ) begin
        s3 <= 0;
        dowhile_break2_PROC_STATE <= 0;    // test_do_while_always1.cpp:126:9;
    end
    else begin
        s3 <= s3_next;
        dowhile_break2_PROC_STATE <= dowhile_break2_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: dowhile_break3 (test_do_while_always1.cpp:148:5) 

// Thread-local variables
logic signed [31:0] s4_next;
logic [2:0] dowhile_break3_PROC_STATE;
logic [2:0] dowhile_break3_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : dowhile_break3_comb     // test_do_while_always1.cpp:148:5
    dowhile_break3_func;
end
function void dowhile_break3_func;
    s4_next = s4;
    dowhile_break3_PROC_STATE_next = dowhile_break3_PROC_STATE;
    
    case (dowhile_break3_PROC_STATE)
        0: begin
            s4_next = s1;
            dowhile_break3_PROC_STATE_next = 1; return;    // test_do_while_always1.cpp:157:17;
        end
        1: begin
            if (|s1)
            begin
                // break begin
                if (|s2)
                begin
                    s4_next = 1;
                    // break begin
                    dowhile_break3_PROC_STATE_next = 4; return;    // test_do_while_always1.cpp:176:13;
                    // break end
                end
                s4_next = s2;
                dowhile_break3_PROC_STATE_next = 3; return;    // test_do_while_always1.cpp:172:17;
                // break end
            end
            dowhile_break3_PROC_STATE_next = 2; return;    // test_do_while_always1.cpp:162:17;
        end
        2: begin
            s4_next = s1;
            dowhile_break3_PROC_STATE_next = 1; return;    // test_do_while_always1.cpp:157:17;
        end
        3: begin
            if (|s2)
            begin
                s4_next = 1;
                // break begin
                dowhile_break3_PROC_STATE_next = 4; return;    // test_do_while_always1.cpp:176:13;
                // break end
            end
            s4_next = s2;
            dowhile_break3_PROC_STATE_next = 3; return;    // test_do_while_always1.cpp:172:17;
        end
        4: begin
            s4_next = s1;
            dowhile_break3_PROC_STATE_next = 1; return;    // test_do_while_always1.cpp:157:17;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge arstn) 
begin : dowhile_break3_ff
    if ( ~arstn ) begin
        s4 <= 0;
        dowhile_break3_PROC_STATE <= 0;    // test_do_while_always1.cpp:151:9;
    end
    else begin
        s4 <= s4_next;
        dowhile_break3_PROC_STATE <= dowhile_break3_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: dowhile_continue1 (test_do_while_always1.cpp:182:5) 

// Thread-local variables
logic signed [31:0] s5_next;
logic signed [31:0] i2;
logic signed [31:0] i_next2;
logic [1:0] dowhile_continue1_PROC_STATE;
logic [1:0] dowhile_continue1_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : dowhile_continue1_comb     // test_do_while_always1.cpp:182:5
    dowhile_continue1_func;
end
function void dowhile_continue1_func;
    i_next2 = i2;
    s5_next = s5;
    dowhile_continue1_PROC_STATE_next = dowhile_continue1_PROC_STATE;
    
    case (dowhile_continue1_PROC_STATE)
        0: begin
            i_next2--;
            dowhile_continue1_PROC_STATE_next = 1; return;    // test_do_while_always1.cpp:193:21;
        end
        1: begin
            if (i_next2 > s3)
            begin
                // continue begin
                if (|s4)
                begin
                    i_next2--;
                    dowhile_continue1_PROC_STATE_next = 1; return;    // test_do_while_always1.cpp:193:21;
                end
                i_next2 = s1;
                if (|s2)
                begin
                    s5_next = 1;
                    // continue begin
                    dowhile_continue1_PROC_STATE_next = 3; return;    // test_do_while_always1.cpp:208:13;
                    // continue end
                end
                dowhile_continue1_PROC_STATE_next = 2; return;    // test_do_while_always1.cpp:204:17;
                // continue end
            end
            if (|s4)
            begin
                i_next2--;
                dowhile_continue1_PROC_STATE_next = 1; return;    // test_do_while_always1.cpp:193:21;
            end
            i_next2 = s1;
            if (|s2)
            begin
                s5_next = 1;
                // continue begin
                dowhile_continue1_PROC_STATE_next = 3; return;    // test_do_while_always1.cpp:208:13;
                // continue end
            end
            dowhile_continue1_PROC_STATE_next = 2; return;    // test_do_while_always1.cpp:204:17;
        end
        2: begin
            dowhile_continue1_PROC_STATE_next = 3; return;    // test_do_while_always1.cpp:208:13;
        end
        3: begin
            i_next2--;
            dowhile_continue1_PROC_STATE_next = 1; return;    // test_do_while_always1.cpp:193:21;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge arstn) 
begin : dowhile_continue1_ff
    if ( ~arstn ) begin
        s5 <= 0;
        i2 <= 42;
        dowhile_continue1_PROC_STATE <= 0;    // test_do_while_always1.cpp:186:9;
    end
    else begin
        s5 <= s5_next;
        i2 <= i_next2;
        dowhile_continue1_PROC_STATE <= dowhile_continue1_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: dowhile_continue2 (test_do_while_always1.cpp:213:5) 

// Thread-local variables
logic signed [31:0] s6_next;
logic signed [31:0] i3;
logic signed [31:0] i_next3;
logic [1:0] dowhile_continue2_PROC_STATE;
logic [1:0] dowhile_continue2_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : dowhile_continue2_comb     // test_do_while_always1.cpp:213:5
    dowhile_continue2_func;
end
function void dowhile_continue2_func;
    i_next3 = i3;
    s6_next = s6;
    dowhile_continue2_PROC_STATE_next = dowhile_continue2_PROC_STATE;
    
    case (dowhile_continue2_PROC_STATE)
        0: begin
            i_next3--;
            dowhile_continue2_PROC_STATE_next = 1; return;    // test_do_while_always1.cpp:224:21;
        end
        1: begin
            if (i_next3 > s3)
            begin
                // continue begin
                i_next3 = s1;
                if (|s2)
                begin
                    s6_next = 1;
                    // continue begin
                    if (|s2)
                    begin
                        i_next3--;
                        dowhile_continue2_PROC_STATE_next = 1; return;    // test_do_while_always1.cpp:224:21;
                    end
                    dowhile_continue2_PROC_STATE_next = 3; return;    // test_do_while_always1.cpp:238:13;
                    // continue end
                end
                dowhile_continue2_PROC_STATE_next = 2; return;    // test_do_while_always1.cpp:234:17;
                // continue end
            end
            i_next3 = s1;
            if (|s2)
            begin
                s6_next = 1;
                // continue begin
                if (|s2)
                begin
                    i_next3--;
                    dowhile_continue2_PROC_STATE_next = 1; return;    // test_do_while_always1.cpp:224:21;
                end
                dowhile_continue2_PROC_STATE_next = 3; return;    // test_do_while_always1.cpp:238:13;
                // continue end
            end
            dowhile_continue2_PROC_STATE_next = 2; return;    // test_do_while_always1.cpp:234:17;
        end
        2: begin
            if (|s2)
            begin
                i_next3--;
                dowhile_continue2_PROC_STATE_next = 1; return;    // test_do_while_always1.cpp:224:21;
            end
            dowhile_continue2_PROC_STATE_next = 3; return;    // test_do_while_always1.cpp:238:13;
        end
        3: begin
            i_next3--;
            dowhile_continue2_PROC_STATE_next = 1; return;    // test_do_while_always1.cpp:224:21;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge arstn) 
begin : dowhile_continue2_ff
    if ( ~arstn ) begin
        s6 <= 0;
        i3 <= 42;
        dowhile_continue2_PROC_STATE <= 0;    // test_do_while_always1.cpp:217:9;
    end
    else begin
        s6 <= s6_next;
        i3 <= i_next3;
        dowhile_continue2_PROC_STATE <= dowhile_continue2_PROC_STATE_next;
    end
end

endmodule


