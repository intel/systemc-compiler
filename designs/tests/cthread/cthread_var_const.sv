//==============================================================================
//
// The code is generated by Intel Compiler for SystemC, version 1.6.14
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
logic rstn;
logic signed [31:0] in;
logic [2:0] s;
logic signed [31:0] r0;
logic signed [31:0] r1;
logic signed [31:0] t0;
logic [2:0] s2;
logic [2:0] s3;
logic signed [31:0] t1;
logic [2:0] s4;

// Local parameters generated for C++ constants
localparam logic [31:0] B = 3;
localparam logic [31:0] A = 2;

//------------------------------------------------------------------------------
// Method process: const_init_meth (test_cthread_var_const.cpp:48:5) 

always_comb 
begin : const_init_meth     // test_cthread_var_const.cpp:48:5
    integer i;
    integer unsigned M1;
    logic [15:0] M2;
    if (|s)
    begin
        i = s;
        M1 = i + 42;
        r0 = M1;
    end
    M2 = 0;
    r0 = M2 + A + B;
end

//------------------------------------------------------------------------------
// Clocked THREAD: const_init_thread (test_cthread_var_const.cpp:60:5) 

// Thread-local variables
logic signed [31:0] r1_next;
logic [31:0] N1;
logic [31:0] N1_next;
logic [31:0] R1;
logic [31:0] R1_next;
logic const_init_thread_PROC_STATE;
logic const_init_thread_PROC_STATE_next;

// Thread-local constants
logic [31:0] R2;

// Next-state combinational logic
always_comb begin : const_init_thread_comb     // test_cthread_var_const.cpp:60:5
    const_init_thread_func;
end
function void const_init_thread_func;
    integer i;
    integer unsigned R1_1;
    N1_next = N1;
    R1_next = R1;
    r1_next = r1;
    const_init_thread_PROC_STATE_next = const_init_thread_PROC_STATE;
    
    case (const_init_thread_PROC_STATE)
        0: begin
            N1_next = 43;
            R1_next = 44;
            const_init_thread_PROC_STATE_next = 1; return;    // test_cthread_var_const.cpp:74:13;
        end
        1: begin
            r1_next = N1_next + R2 + R1_next + A + B;
            N1_next = 43;
            R1_next = 44;
            const_init_thread_PROC_STATE_next = 1; return;    // test_cthread_var_const.cpp:74:13;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge rstn) 
begin : const_init_thread_ff
    if ( ~rstn ) begin
        integer i;
        integer unsigned R1_1;
        if (|s)
        begin
            i = 41;
            R1_1 = i + 1;
            r1 <= R1_1;
        end
        R2 = 42;
        r1 <= R2 + B;
        const_init_thread_PROC_STATE <= 0;    // test_cthread_var_const.cpp:69:9;
    end
    else begin
        r1 <= r1_next;
        N1 <= N1_next;
        R1 <= R1_next;
        const_init_thread_PROC_STATE <= const_init_thread_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: local_rnd1 (test_cthread_var_const.cpp:83:5) 

// Thread-local variables
logic [2:0] D;
logic [2:0] D_next;
logic signed [31:0] i0;
logic signed [31:0] i_next;
logic signed [31:0] t0_next;
logic local_rnd1_PROC_STATE;
logic local_rnd1_PROC_STATE_next;

// Thread-local constants
logic signed [31:0] C;

// Next-state combinational logic
always_comb begin : local_rnd1_comb     // test_cthread_var_const.cpp:83:5
    local_rnd1_func;
end
function void local_rnd1_func;
    D_next = D;
    i_next = i0;
    t0_next = t0;
    local_rnd1_PROC_STATE_next = local_rnd1_PROC_STATE;
    
    case (local_rnd1_PROC_STATE)
        0: begin
            D_next = s;
            local_rnd1_PROC_STATE_next = 1; return;    // test_cthread_var_const.cpp:90:13;
        end
        1: begin
            if (|C)
            begin
                i_next = D_next;
            end
            t0_next = i_next;
            D_next = s;
            local_rnd1_PROC_STATE_next = 1; return;    // test_cthread_var_const.cpp:90:13;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge rstn) 
begin : local_rnd1_ff
    if ( ~rstn ) begin
        i0 <= 0;
        C = in;
        local_rnd1_PROC_STATE <= 0;    // test_cthread_var_const.cpp:86:9;
    end
    else begin
        D <= D_next;
        i0 <= i_next;
        t0 <= t0_next;
        local_rnd1_PROC_STATE <= local_rnd1_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: local_rnd2 (test_cthread_var_const.cpp:97:5) 

// Thread-local variables
logic signed [31:0] i1;
logic signed [31:0] i_next0;
logic [2:0] G;
logic [2:0] G_next;
logic [2:0] s2_next;
logic [1:0] local_rnd2_PROC_STATE;
logic [1:0] local_rnd2_PROC_STATE_next;

// Thread-local constants
logic signed [31:0] ARR[3];

// Next-state combinational logic
always_comb begin : local_rnd2_comb     // test_cthread_var_const.cpp:97:5
    local_rnd2_func;
end
function void local_rnd2_func;
    G_next = G;
    i_next0 = i1;
    s2_next = s2;
    local_rnd2_PROC_STATE_next = local_rnd2_PROC_STATE;
    
    case (local_rnd2_PROC_STATE)
        0: begin
            G_next = ARR[2];
            local_rnd2_PROC_STATE_next = 1; return;    // test_cthread_var_const.cpp:104:13;
        end
        1: begin
            if (|s)
            begin
                i_next0 = ARR[s];
            end
            local_rnd2_PROC_STATE_next = 2; return;    // test_cthread_var_const.cpp:107:13;
        end
        2: begin
            s2_next = G_next + i_next0;
            G_next = ARR[2];
            local_rnd2_PROC_STATE_next = 1; return;    // test_cthread_var_const.cpp:104:13;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge rstn) 
begin : local_rnd2_ff
    if ( ~rstn ) begin
        i1 <= 0;
        ARR[0] = 1; ARR[1] = 2; ARR[2] = in;
        local_rnd2_PROC_STATE <= 0;    // test_cthread_var_const.cpp:100:9;
    end
    else begin
        i1 <= i_next0;
        G <= G_next;
        s2 <= s2_next;
        local_rnd2_PROC_STATE <= local_rnd2_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: local_rnd3 (test_cthread_var_const.cpp:115:5) 

// Thread-local variables
logic signed [31:0] i2;
logic signed [31:0] i_next1;
logic signed [3:0] Y;
logic signed [3:0] Y_next;
logic [3:0] X;
logic [3:0] X_next;
logic signed [31:0] j;
logic signed [31:0] j_next;
logic [2:0] s3_next;
logic [2:0] local_rnd3_PROC_STATE;
logic [2:0] local_rnd3_PROC_STATE_next;

// Thread-local constants
logic [3:0] Z;

// Next-state combinational logic
always_comb begin : local_rnd3_comb     // test_cthread_var_const.cpp:115:5
    local_rnd3_func;
end
function void local_rnd3_func;
    X_next = X;
    Y_next = Y;
    i_next1 = i2;
    j_next = j;
    s3_next = s3;
    local_rnd3_PROC_STATE_next = local_rnd3_PROC_STATE;
    
    case (local_rnd3_PROC_STATE)
        0: begin
            i_next1 = Z + 1;
            local_rnd3_PROC_STATE_next = 1; return;    // test_cthread_var_const.cpp:122:9;
        end
        1: begin
            if (|s)
            begin
                Y_next = i_next1;
                local_rnd3_PROC_STATE_next = 2; return;    // test_cthread_var_const.cpp:127:17;
            end
            local_rnd3_PROC_STATE_next = 1; return;    // test_cthread_var_const.cpp:135:13;
        end
        2: begin
            j_next = 0;
            X_next = Y_next + j_next;
            local_rnd3_PROC_STATE_next = 3; return;    // test_cthread_var_const.cpp:131:21;
        end
        3: begin
            s3_next = X_next;
            ++j_next;
            if (j_next < 3)
            begin
                X_next = Y_next + j_next;
                local_rnd3_PROC_STATE_next = 3; return;    // test_cthread_var_const.cpp:131:21;
            end
            local_rnd3_PROC_STATE_next = 1; return;    // test_cthread_var_const.cpp:135:13;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge rstn) 
begin : local_rnd3_ff
    if ( ~rstn ) begin
        i2 <= 0;
        Z = in;
        local_rnd3_PROC_STATE <= 0;    // test_cthread_var_const.cpp:119:13;
    end
    else begin
        i2 <= i_next1;
        Y <= Y_next;
        X <= X_next;
        j <= j_next;
        s3 <= s3_next;
        local_rnd3_PROC_STATE <= local_rnd3_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: local_def_read1 (test_cthread_var_const.cpp:141:5) 

// Thread-local variables
logic signed [31:0] t1_next;

// Next-state combinational logic
always_comb begin : local_def_read1_comb     // test_cthread_var_const.cpp:141:5
    local_def_read1_func;
end
function void local_def_read1_func;
    integer E;
    integer i;
    logic [2:0] F;
    t1_next = t1;
    F = s;
    i = F;
    t1_next = i;
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge rstn) 
begin : local_def_read1_ff
    if ( ~rstn ) begin
        integer E;
        integer i;
        E = in;
        i = E;
        t1 <= i;
    end
    else begin
        t1 <= t1_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: local_def_read2 (test_cthread_var_const.cpp:156:5) 

// Thread-local variables
logic [2:0] L;
logic [2:0] L_next;
logic [2:0] s4_next;
logic local_def_read2_PROC_STATE;
logic local_def_read2_PROC_STATE_next;

// Thread-local constants
logic [2:0] K;

// Next-state combinational logic
always_comb begin : local_def_read2_comb     // test_cthread_var_const.cpp:156:5
    local_def_read2_func;
end
function void local_def_read2_func;
    logic [2:0] ARRA[3];
    integer ARRB[3];
    L_next = L;
    s4_next = s4;
    local_def_read2_PROC_STATE_next = local_def_read2_PROC_STATE;
    
    case (local_def_read2_PROC_STATE)
        0: begin
            ARRB[0] = in + 1; ARRB[1] = in + 2; ARRB[2] = in + 3;
            L_next = ARRB[K + s];
            local_def_read2_PROC_STATE_next = 1; return;    // test_cthread_var_const.cpp:165:13;
        end
        1: begin
            s4_next = L_next;
            ARRB[0] = in + 1; ARRB[1] = in + 2; ARRB[2] = in + 3;
            L_next = ARRB[K + s];
            local_def_read2_PROC_STATE_next = 1; return;    // test_cthread_var_const.cpp:165:13;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge rstn) 
begin : local_def_read2_ff
    if ( ~rstn ) begin
        logic [2:0] ARRA[3];
        ARRA[0] = 1; ARRA[1] = 2; ARRA[2] = 3;
        K = ARRA[0];
        local_def_read2_PROC_STATE <= 0;    // test_cthread_var_const.cpp:159:9;
    end
    else begin
        L <= L_next;
        s4 <= s4_next;
        local_def_read2_PROC_STATE <= local_def_read2_PROC_STATE_next;
    end
end

endmodule


