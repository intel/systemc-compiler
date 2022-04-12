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
    input logic clk
);

// Variables generated for SystemC signals
logic arstn;
logic signed [31:0] out;
logic signed [31:0] out2;
logic signed [31:0] out3;
logic signed [31:0] out4;
logic signed [31:0] out5;
logic signed [31:0] out6;
logic signed [31:0] in;
logic [3:0] s;

//------------------------------------------------------------------------------
// Clocked THREAD: dowhile_with_wait0_sc_int (test_do_while_other_types.cpp:63:5) 

// Thread-local variables
logic signed [31:0] out_next;
logic signed [10:0] i;
logic signed [10:0] i_next;
logic dowhile_with_wait0_sc_int_PROC_STATE;
logic dowhile_with_wait0_sc_int_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : dowhile_with_wait0_sc_int_comb     // test_do_while_other_types.cpp:63:5
    dowhile_with_wait0_sc_int_func;
end
function void dowhile_with_wait0_sc_int_func;
    i_next = i;
    out_next = out;
    dowhile_with_wait0_sc_int_PROC_STATE_next = dowhile_with_wait0_sc_int_PROC_STATE;
    
    case (dowhile_with_wait0_sc_int_PROC_STATE)
        0: begin
            i_next = 0;
            out_next = 1;
            dowhile_with_wait0_sc_int_PROC_STATE_next = 1; return;    // test_do_while_other_types.cpp:73:17;
        end
        1: begin
            i_next++;
            if (i_next < 3)
            begin
                out_next = 1;
                dowhile_with_wait0_sc_int_PROC_STATE_next = 1; return;    // test_do_while_other_types.cpp:73:17;
            end
            out_next = 2;
            i_next = 0;
            out_next = 1;
            dowhile_with_wait0_sc_int_PROC_STATE_next = 1; return;    // test_do_while_other_types.cpp:73:17;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge arstn) 
begin : dowhile_with_wait0_sc_int_ff
    if ( ~arstn ) begin
        out <= 0;
        dowhile_with_wait0_sc_int_PROC_STATE <= 0;    // test_do_while_other_types.cpp:66:9;
    end
    else begin
        out <= out_next;
        i <= i_next;
        dowhile_with_wait0_sc_int_PROC_STATE <= dowhile_with_wait0_sc_int_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: dowhile_with_wait1_sc_uint (test_do_while_other_types.cpp:82:5) 

// Thread-local variables
logic signed [31:0] out2_next;
logic [31:0] i0;
logic [31:0] i_next0;
logic [1:0] dowhile_with_wait1_sc_uint_PROC_STATE;
logic [1:0] dowhile_with_wait1_sc_uint_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : dowhile_with_wait1_sc_uint_comb     // test_do_while_other_types.cpp:82:5
    dowhile_with_wait1_sc_uint_func;
end
function void dowhile_with_wait1_sc_uint_func;
    i_next0 = i0;
    out2_next = out2;
    dowhile_with_wait1_sc_uint_PROC_STATE_next = dowhile_with_wait1_sc_uint_PROC_STATE;
    
    case (dowhile_with_wait1_sc_uint_PROC_STATE)
        0: begin
            i_next0 = 0;
            i_next0++;
            out2_next = 1;
            dowhile_with_wait1_sc_uint_PROC_STATE_next = 1; return;    // test_do_while_other_types.cpp:93:17;
        end
        1: begin
            if (i_next0 < 3)
            begin
                i_next0++;
                out2_next = 1;
                dowhile_with_wait1_sc_uint_PROC_STATE_next = 1; return;    // test_do_while_other_types.cpp:93:17;
            end
            out2_next = 2;
            dowhile_with_wait1_sc_uint_PROC_STATE_next = 2; return;    // test_do_while_other_types.cpp:96:13;
        end
        2: begin
            i_next0 = 0;
            i_next0++;
            out2_next = 1;
            dowhile_with_wait1_sc_uint_PROC_STATE_next = 1; return;    // test_do_while_other_types.cpp:93:17;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge arstn) 
begin : dowhile_with_wait1_sc_uint_ff
    if ( ~arstn ) begin
        out2 <= 0;
        dowhile_with_wait1_sc_uint_PROC_STATE <= 0;    // test_do_while_other_types.cpp:85:9;
    end
    else begin
        out2 <= out2_next;
        i0 <= i_next0;
        dowhile_with_wait1_sc_uint_PROC_STATE <= dowhile_with_wait1_sc_uint_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: dowhile_with_wait2_sc_bigint (test_do_while_other_types.cpp:101:5) 

// Thread-local variables
logic signed [31:0] out3_next;
logic signed [63:0] i1;
logic signed [63:0] i_next1;
logic [1:0] dowhile_with_wait2_sc_bigint_PROC_STATE;
logic [1:0] dowhile_with_wait2_sc_bigint_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : dowhile_with_wait2_sc_bigint_comb     // test_do_while_other_types.cpp:101:5
    dowhile_with_wait2_sc_bigint_func;
end
function void dowhile_with_wait2_sc_bigint_func;
    i_next1 = i1;
    out3_next = out3;
    dowhile_with_wait2_sc_bigint_PROC_STATE_next = dowhile_with_wait2_sc_bigint_PROC_STATE;
    
    case (dowhile_with_wait2_sc_bigint_PROC_STATE)
        0: begin
            i_next1 = 0;
            i_next1++;
            out3_next = 1;
            dowhile_with_wait2_sc_bigint_PROC_STATE_next = 1; return;    // test_do_while_other_types.cpp:112:17;
        end
        1: begin
            if (in > 1)
            begin
                out3_next = 2;
                dowhile_with_wait2_sc_bigint_PROC_STATE_next = 2; return;    // test_do_while_other_types.cpp:116:21;
            end
            if (i_next1 < 3)
            begin
                i_next1++;
                out3_next = 1;
                dowhile_with_wait2_sc_bigint_PROC_STATE_next = 1; return;    // test_do_while_other_types.cpp:112:17;
            end
            out3_next = 3;
            dowhile_with_wait2_sc_bigint_PROC_STATE_next = 3; return;    // test_do_while_other_types.cpp:120:13;
        end
        2: begin
            if (i_next1 < 3)
            begin
                i_next1++;
                out3_next = 1;
                dowhile_with_wait2_sc_bigint_PROC_STATE_next = 1; return;    // test_do_while_other_types.cpp:112:17;
            end
            out3_next = 3;
            dowhile_with_wait2_sc_bigint_PROC_STATE_next = 3; return;    // test_do_while_other_types.cpp:120:13;
        end
        3: begin
            i_next1 = 0;
            i_next1++;
            out3_next = 1;
            dowhile_with_wait2_sc_bigint_PROC_STATE_next = 1; return;    // test_do_while_other_types.cpp:112:17;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge arstn) 
begin : dowhile_with_wait2_sc_bigint_ff
    if ( ~arstn ) begin
        out3 <= 0;
        dowhile_with_wait2_sc_bigint_PROC_STATE <= 0;    // test_do_while_other_types.cpp:104:9;
    end
    else begin
        out3 <= out3_next;
        i1 <= i_next1;
        dowhile_with_wait2_sc_bigint_PROC_STATE <= dowhile_with_wait2_sc_bigint_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: dowhile_with_for_sc_biguint (test_do_while_other_types.cpp:125:5) 

// Thread-local variables
logic signed [31:0] out4_next;
logic signed [31:0] j;
logic signed [31:0] j_next;
logic [31:0] i2;
logic [31:0] i_next2;
logic [1:0] dowhile_with_for_sc_biguint_PROC_STATE;
logic [1:0] dowhile_with_for_sc_biguint_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : dowhile_with_for_sc_biguint_comb     // test_do_while_other_types.cpp:125:5
    dowhile_with_for_sc_biguint_func;
end
function void dowhile_with_for_sc_biguint_func;
    i_next2 = i2;
    j_next = j;
    out4_next = out4;
    dowhile_with_for_sc_biguint_PROC_STATE_next = dowhile_with_for_sc_biguint_PROC_STATE;
    
    case (dowhile_with_for_sc_biguint_PROC_STATE)
        0: begin
            i_next2 = 0;
            i_next2++;
            out4_next = 1;
            j_next = 0;
            if (in > 1)
            begin
                out4_next = j_next;
            end
            dowhile_with_for_sc_biguint_PROC_STATE_next = 1; return;    // test_do_while_other_types.cpp:141:21;
        end
        1: begin
            j_next++;
            if (j_next < 2)
            begin
                if (in > 1)
                begin
                    out4_next = j_next;
                end
                dowhile_with_for_sc_biguint_PROC_STATE_next = 1; return;    // test_do_while_other_types.cpp:141:21;
            end
            if (i_next2 < 3)
            begin
                i_next2++;
                out4_next = 1;
                j_next = 0;
                if (in > 1)
                begin
                    out4_next = j_next;
                end
                dowhile_with_for_sc_biguint_PROC_STATE_next = 1; return;    // test_do_while_other_types.cpp:141:21;
            end
            out4_next = 3;
            dowhile_with_for_sc_biguint_PROC_STATE_next = 2; return;    // test_do_while_other_types.cpp:145:13;
        end
        2: begin
            i_next2 = 0;
            i_next2++;
            out4_next = 1;
            j_next = 0;
            if (in > 1)
            begin
                out4_next = j_next;
            end
            dowhile_with_for_sc_biguint_PROC_STATE_next = 1; return;    // test_do_while_other_types.cpp:141:21;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge arstn) 
begin : dowhile_with_for_sc_biguint_ff
    if ( ~arstn ) begin
        out4 <= 0;
        dowhile_with_for_sc_biguint_PROC_STATE <= 0;    // test_do_while_other_types.cpp:128:9;
    end
    else begin
        out4 <= out4_next;
        j <= j_next;
        i2 <= i_next2;
        dowhile_with_for_sc_biguint_PROC_STATE <= dowhile_with_for_sc_biguint_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: dowhile_with_signal_cond (test_do_while_other_types.cpp:150:5) 

// Thread-local variables
logic signed [31:0] out5_next;
logic [1:0] dowhile_with_signal_cond_PROC_STATE;
logic [1:0] dowhile_with_signal_cond_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : dowhile_with_signal_cond_comb     // test_do_while_other_types.cpp:150:5
    dowhile_with_signal_cond_func;
end
function void dowhile_with_signal_cond_func;
    out5_next = out5;
    dowhile_with_signal_cond_PROC_STATE_next = dowhile_with_signal_cond_PROC_STATE;
    
    case (dowhile_with_signal_cond_PROC_STATE)
        0: begin
            out5_next = 1;
            dowhile_with_signal_cond_PROC_STATE_next = 1; return;    // test_do_while_other_types.cpp:159:17;
        end
        1: begin
            if (|in)
            begin
                out5_next = 1;
                dowhile_with_signal_cond_PROC_STATE_next = 1; return;    // test_do_while_other_types.cpp:159:17;
            end
            out5_next = 2;
            dowhile_with_signal_cond_PROC_STATE_next = 2; return;    // test_do_while_other_types.cpp:163:13;
        end
        2: begin
            out5_next = 1;
            dowhile_with_signal_cond_PROC_STATE_next = 1; return;    // test_do_while_other_types.cpp:159:17;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge arstn) 
begin : dowhile_with_signal_cond_ff
    if ( ~arstn ) begin
        out5 <= 0;
        dowhile_with_signal_cond_PROC_STATE <= 0;    // test_do_while_other_types.cpp:153:9;
    end
    else begin
        out5 <= out5_next;
        dowhile_with_signal_cond_PROC_STATE <= dowhile_with_signal_cond_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: complex0 (test_do_while_other_types.cpp:167:5) 

// Next-state combinational logic
always_comb begin : complex0_comb     // test_do_while_other_types.cpp:167:5
    complex0_func;
end
function void complex0_func;
    logic [3:0] i_1;
    i_1 = 0;
    do
    begin
        i_1++;
        if (i_1 < 3)
        begin
            continue;
        end
    end
    while (i_1 < 4);
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge arstn) 
begin : complex0_ff
    if ( ~arstn ) begin
    end
    else begin
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: complex1 (test_do_while_other_types.cpp:185:5) 

// Thread-local variables
logic [3:0] i3;
logic [3:0] i_next3;
logic [1:0] complex1_PROC_STATE;
logic [1:0] complex1_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : complex1_comb     // test_do_while_other_types.cpp:185:5
    complex1_func;
end
function void complex1_func;
    i_next3 = i3;
    complex1_PROC_STATE_next = complex1_PROC_STATE;
    
    case (complex1_PROC_STATE)
        0: begin
            i_next3 = 0;
            do
            begin
                i_next3++;
            end
            while (0);
            i_next3 = 0;
            i_next3++;
            if (|in)
            begin
                // break begin
                complex1_PROC_STATE_next = 2; return;    // test_do_while_other_types.cpp:206:13;
                // break end
            end
            complex1_PROC_STATE_next = 1; return;    // test_do_while_other_types.cpp:202:17;
        end
        1: begin
            if (i_next3 < 3)
            begin
                i_next3++;
                if (|in)
                begin
                    // break begin
                    complex1_PROC_STATE_next = 2; return;    // test_do_while_other_types.cpp:206:13;
                    // break end
                end
                complex1_PROC_STATE_next = 1; return;    // test_do_while_other_types.cpp:202:17;
            end
            complex1_PROC_STATE_next = 2; return;    // test_do_while_other_types.cpp:206:13;
        end
        2: begin
            i_next3 = 0;
            do
            begin
                i_next3++;
            end
            while (0);
            i_next3 = 0;
            i_next3++;
            if (|in)
            begin
                // break begin
                complex1_PROC_STATE_next = 2; return;    // test_do_while_other_types.cpp:206:13;
                // break end
            end
            complex1_PROC_STATE_next = 1; return;    // test_do_while_other_types.cpp:202:17;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge arstn) 
begin : complex1_ff
    if ( ~arstn ) begin
        complex1_PROC_STATE <= 0;    // test_do_while_other_types.cpp:187:9;
    end
    else begin
        i3 <= i_next3;
        complex1_PROC_STATE <= complex1_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: complex2 (test_do_while_other_types.cpp:210:5) 

// Thread-local variables
logic signed [31:0] out6_next;
logic [3:0] i4;
logic [3:0] i_next4;
logic [1:0] complex2_PROC_STATE;
logic [1:0] complex2_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : complex2_comb     // test_do_while_other_types.cpp:210:5
    complex2_func;
end
function void complex2_func;
    i_next4 = i4;
    out6_next = out6;
    complex2_PROC_STATE_next = complex2_PROC_STATE;
    
    case (complex2_PROC_STATE)
        0: begin
            i_next4 = 0;
            do
            begin
                i_next4++;
                i_next4++;
            end
            while (0);
            do
            begin
                i_next4++;
                out6_next = i_next4;
            end
            while (i_next4 < 5);
            i_next4 = 0;
            i_next4++;
            out6_next = i_next4;
            if (|in)
            begin
                // break begin
                complex2_PROC_STATE_next = 2; return;    // test_do_while_other_types.cpp:248:17;
                // break end
            end
            complex2_PROC_STATE_next = 1; return;    // test_do_while_other_types.cpp:242:17;
        end
        1: begin
            if (i_next4 < 3)
            begin
                i_next4++;
                out6_next = i_next4;
                if (|in)
                begin
                    // break begin
                    complex2_PROC_STATE_next = 2; return;    // test_do_while_other_types.cpp:248:17;
                    // break end
                end
                complex2_PROC_STATE_next = 1; return;    // test_do_while_other_types.cpp:242:17;
            end else begin
                complex2_PROC_STATE_next = 2; return;    // test_do_while_other_types.cpp:248:17;
            end
        end
        2: begin
            if (|in)
            begin
                complex2_PROC_STATE_next = 2; return;    // test_do_while_other_types.cpp:248:17;
            end
            i_next4 = 0;
            do
            begin
                i_next4++;
                i_next4++;
            end
            while (0);
            do
            begin
                i_next4++;
                out6_next = i_next4;
            end
            while (i_next4 < 5);
            i_next4 = 0;
            i_next4++;
            out6_next = i_next4;
            if (|in)
            begin
                // break begin
                complex2_PROC_STATE_next = 2; return;    // test_do_while_other_types.cpp:248:17;
                // break end
            end
            complex2_PROC_STATE_next = 1; return;    // test_do_while_other_types.cpp:242:17;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge arstn) 
begin : complex2_ff
    if ( ~arstn ) begin
        out6 <= 0;
        complex2_PROC_STATE <= 0;    // test_do_while_other_types.cpp:213:9;
    end
    else begin
        out6 <= out6_next;
        i4 <= i_next4;
        complex2_PROC_STATE <= complex2_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: break_in_if (test_do_while_other_types.cpp:255:5) 

// Thread-local variables
logic [3:0] i5;
logic [3:0] i_next5;
logic [1:0] break_in_if_PROC_STATE;
logic [1:0] break_in_if_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : break_in_if_comb     // test_do_while_other_types.cpp:255:5
    break_in_if_func;
end
function void break_in_if_func;
    integer k;
    i_next5 = i5;
    break_in_if_PROC_STATE_next = break_in_if_PROC_STATE;
    
    case (break_in_if_PROC_STATE)
        0: begin
            i_next5 = s;
            i_next5++;
            if (|in)
            begin
                if (|s)
                begin
                    // break begin
                    break_in_if_PROC_STATE_next = 2; return;    // test_do_while_other_types.cpp:275:13;
                    // break end
                end
            end else begin
                k = 1;
            end
            break_in_if_PROC_STATE_next = 1; return;    // test_do_while_other_types.cpp:270:17;
        end
        1: begin
            if (i_next5 < 3)
            begin
                i_next5++;
                if (|in)
                begin
                    if (|s)
                    begin
                        // break begin
                        break_in_if_PROC_STATE_next = 2; return;    // test_do_while_other_types.cpp:275:13;
                        // break end
                    end
                end else begin
                    k = 1;
                end
                break_in_if_PROC_STATE_next = 1; return;    // test_do_while_other_types.cpp:270:17;
            end
            break_in_if_PROC_STATE_next = 2; return;    // test_do_while_other_types.cpp:275:13;
        end
        2: begin
            i_next5 = s;
            i_next5++;
            if (|in)
            begin
                if (|s)
                begin
                    // break begin
                    break_in_if_PROC_STATE_next = 2; return;    // test_do_while_other_types.cpp:275:13;
                    // break end
                end
            end else begin
                k = 1;
            end
            break_in_if_PROC_STATE_next = 1; return;    // test_do_while_other_types.cpp:270:17;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge arstn) 
begin : break_in_if_ff
    if ( ~arstn ) begin
        integer k;
        k = 0;
        break_in_if_PROC_STATE <= 0;    // test_do_while_other_types.cpp:258:9;
    end
    else begin
        i5 <= i_next5;
        break_in_if_PROC_STATE <= break_in_if_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: break_in_if2 (test_do_while_other_types.cpp:279:5) 

// Thread-local variables
logic [3:0] i6;
logic [3:0] i_next6;
logic [1:0] break_in_if2_PROC_STATE;
logic [1:0] break_in_if2_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : break_in_if2_comb     // test_do_while_other_types.cpp:279:5
    break_in_if2_func;
end
function void break_in_if2_func;
    integer k;
    i_next6 = i6;
    break_in_if2_PROC_STATE_next = break_in_if2_PROC_STATE;
    
    case (break_in_if2_PROC_STATE)
        0: begin
            i_next6 = s;
            i_next6++;
            if (|in)
            begin
                if (|s)
                begin
                    // break begin
                    break_in_if2_PROC_STATE_next = 2; return;    // test_do_while_other_types.cpp:300:13;
                    // break end
                end else begin
                    k = 1;
                end
            end
            break_in_if2_PROC_STATE_next = 1; return;    // test_do_while_other_types.cpp:295:17;
        end
        1: begin
            if (i_next6 < 3)
            begin
                i_next6++;
                if (|in)
                begin
                    if (|s)
                    begin
                        // break begin
                        break_in_if2_PROC_STATE_next = 2; return;    // test_do_while_other_types.cpp:300:13;
                        // break end
                    end else begin
                        k = 1;
                    end
                end
                break_in_if2_PROC_STATE_next = 1; return;    // test_do_while_other_types.cpp:295:17;
            end
            break_in_if2_PROC_STATE_next = 2; return;    // test_do_while_other_types.cpp:300:13;
        end
        2: begin
            i_next6 = s;
            i_next6++;
            if (|in)
            begin
                if (|s)
                begin
                    // break begin
                    break_in_if2_PROC_STATE_next = 2; return;    // test_do_while_other_types.cpp:300:13;
                    // break end
                end else begin
                    k = 1;
                end
            end
            break_in_if2_PROC_STATE_next = 1; return;    // test_do_while_other_types.cpp:295:17;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge arstn) 
begin : break_in_if2_ff
    if ( ~arstn ) begin
        integer k;
        k = 0;
        break_in_if2_PROC_STATE <= 0;    // test_do_while_other_types.cpp:282:9;
    end
    else begin
        i6 <= i_next6;
        break_in_if2_PROC_STATE <= break_in_if2_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Method process: break_in_if2_m (test_do_while_other_types.cpp:304:5) 

always_comb 
begin : break_in_if2_m     // test_do_while_other_types.cpp:304:5
    integer k;
    integer i_1;
    i_1 = 0;
    do
    begin
        i_1++;
        if (|in)
        begin
            if (|s)
            begin
                break;
            end
        end
    end
    while (i_1 < 3);
end

endmodule


