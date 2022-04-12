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
    input logic clk,
    input logic rst,
    input logic signed [31:0] in,
    output logic signed [31:0] out
);

// Variables generated for SystemC signals
logic a;
logic [3:0] b;
logic signed [31:0] c;
logic signed [32:0] d;
logic [31:0] e;
logic [3:0] f;
logic [3:0] g;
logic [3:0] h;
logic [3:0] arr[3];
logic signed [31:0] r0;
logic signed [31:0] s1;
logic signed [31:0] r5;
logic signed [31:0] s6;
logic signed [31:0] s7;
logic signed [31:0] s8;
logic signed [31:0] s9;

//------------------------------------------------------------------------------
// Clocked THREAD: var_init_reset (test_cthread_var_reg.cpp:73:5) 

// Thread-local variables
logic signed [31:0] vi;
logic signed [31:0] vi_next;
logic signed [31:0] mi;
logic signed [31:0] mi_next;
logic signed [31:0] r0_next;

// Thread-local constants
logic signed [31:0] ci;

// Next-state combinational logic
always_comb begin : var_init_reset_comb     // test_cthread_var_reg.cpp:73:5
    var_init_reset_func;
end
function void var_init_reset_func;
    mi_next = mi;
    r0_next = r0;
    vi_next = vi;
    r0_next = mi_next + vi_next + ci;
endfunction

// Synchronous register update
always_ff @(posedge clk or posedge rst) 
begin : var_init_reset_ff
    if ( rst ) begin
        ci = 42;
        vi <= 43;
        mi <= 44;
    end
    else begin
        vi <= vi_next;
        mi <= mi_next;
        r0 <= r0_next;
    end
end

//------------------------------------------------------------------------------
// Method process: channels0 (test_cthread_var_reg.cpp:85:5) 

always_comb 
begin : channels0     // test_cthread_var_reg.cpp:85:5
    logic x;
    logic [3:0] y;
    x = a;
    y = b + c;
    h = y;
end

//------------------------------------------------------------------------------
// Clocked THREAD: channels1 (test_cthread_var_reg.cpp:93:5) 

// Thread-local variables
logic [3:0] b_next;
logic signed [31:0] out_next;
logic signed [31:0] c_next;
logic signed [32:0] d_next;
logic [31:0] e_next;
logic channels1_PROC_STATE;
logic channels1_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : channels1_comb     // test_cthread_var_reg.cpp:93:5
    channels1_func;
end
function void channels1_func;
    b_next = b;
    c_next = c;
    d_next = d;
    e_next = e;
    out_next = out;
    channels1_PROC_STATE_next = channels1_PROC_STATE;
    
    case (channels1_PROC_STATE)
        0: begin
            b_next = 42;
            c_next = 43;
            d_next = 44;
            out_next = in;
            e_next = 0;
            channels1_PROC_STATE_next = 1; return;    // test_cthread_var_reg.cpp:106:13;
        end
        1: begin
            e_next = 1;
            c_next = 32'(d) + e;
            b_next = 42;
            c_next = 43;
            d_next = 44;
            out_next = in;
            e_next = 0;
            channels1_PROC_STATE_next = 1; return;    // test_cthread_var_reg.cpp:106:13;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or posedge rst) 
begin : channels1_ff
    if ( rst ) begin
        a <= 1;
        b <= 0;
        out <= 0;
        channels1_PROC_STATE <= 0;    // test_cthread_var_reg.cpp:98:9;
    end
    else begin
        b <= b_next;
        out <= out_next;
        c <= c_next;
        d <= d_next;
        e <= e_next;
        channels1_PROC_STATE <= channels1_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: channels2 (test_cthread_var_reg.cpp:114:5) 

// Thread-local variables
logic [3:0] f_next;
logic [3:0] g_next;
logic [3:0] arr_next[3];

// Next-state combinational logic
always_comb begin : channels2_comb     // test_cthread_var_reg.cpp:114:5
    channels2_func;
end
function void channels2_func;
    arr_next = arr;
    f_next = f;
    g_next = g;
    if (a)
    begin
        g_next = f;
    end
    f_next = arr[1];
    arr_next[b] = in;
endfunction

// Synchronous register update
always_ff @(posedge clk or posedge rst) 
begin : channels2_ff
    if ( rst ) begin
        f <= 0;
        g <= 0;
        arr[0] <= 1;
        for (integer i = 1; i < 3; ++i)
        begin
            arr[i] <= i;
        end
    end
    else begin
        f <= f_next;
        g <= g_next;
        arr <= arr_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: variables1 (test_cthread_var_reg.cpp:141:5) 

// Thread-local variables
logic [3:0] n;
logic [3:0] n_next;
logic [31:0] m;
logic [31:0] m_next;
logic k;
logic k_next;
logic [3:0] z;
logic [3:0] z_next;
logic [3:0] p;
logic variables1_PROC_STATE;
logic variables1_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : variables1_comb     // test_cthread_var_reg.cpp:141:5
    variables1_func;
end
function void variables1_func;
    integer t;
    k_next = k;
    m_next = m;
    n_next = n;
    z_next = z;
    variables1_PROC_STATE_next = variables1_PROC_STATE;
    
    case (variables1_PROC_STATE)
        0: begin
            z_next = b + n_next;
            t = m_next;
            p = n_next;
            variables1_PROC_STATE_next = 1; return;    // test_cthread_var_reg.cpp:152:13;
        end
        1: begin
            if (a)
            begin
                z_next = 0;
            end
            n_next = z_next + 0;
            m_next++;
            z_next = b + n_next;
            t = m_next;
            p = n_next;
            variables1_PROC_STATE_next = 1; return;    // test_cthread_var_reg.cpp:152:13;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or posedge rst) 
begin : variables1_ff
    if ( rst ) begin
        logic [3:0] x;
        x = 1;
        k <= 0;
        m <= 0;
        variables1_PROC_STATE <= 0;    // test_cthread_var_reg.cpp:145:9;
    end
    else begin
        n <= n_next;
        m <= m_next;
        k <= k_next;
        z <= z_next;
        variables1_PROC_STATE <= variables1_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: variables2 (test_cthread_var_reg.cpp:164:5) 

// Thread-local variables
logic signed [31:0] s1_next;
logic arrv[3];
logic arrv_next[3];
logic [1:0] r;
logic [1:0] r_next;
logic [3:0] w;
logic [3:0] w_next;
logic [63:0] v;
logic [63:0] v_next;
logic [1:0] variables2_PROC_STATE;
logic [1:0] variables2_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : variables2_comb     // test_cthread_var_reg.cpp:164:5
    variables2_func;
end
function void variables2_func;
    arrv_next = arrv;
    r_next = r;
    s1_next = s1;
    v_next = v;
    w_next = w;
    variables2_PROC_STATE_next = variables2_PROC_STATE;
    
    case (variables2_PROC_STATE)
        0: begin
            if (|b)
            begin
                v_next = r_next >>> arrv_next[b];
                if (|c)
                begin
                    s1_next = w_next + v_next;
                    variables2_PROC_STATE_next = 1; return;    // test_cthread_var_reg.cpp:177:21;
                end
                s1_next = v_next;
            end
            variables2_PROC_STATE_next = 2; return;    // test_cthread_var_reg.cpp:182:13;
        end
        1: begin
            if (|c)
            begin
                s1_next = w_next + v_next;
                variables2_PROC_STATE_next = 1; return;    // test_cthread_var_reg.cpp:177:21;
            end
            s1_next = v_next;
            variables2_PROC_STATE_next = 2; return;    // test_cthread_var_reg.cpp:182:13;
        end
        2: begin
            if (|b)
            begin
                v_next = r_next >>> arrv_next[b];
                if (|c)
                begin
                    s1_next = w_next + v_next;
                    variables2_PROC_STATE_next = 1; return;    // test_cthread_var_reg.cpp:177:21;
                end
                s1_next = v_next;
            end
            variables2_PROC_STATE_next = 2; return;    // test_cthread_var_reg.cpp:182:13;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or posedge rst) 
begin : variables2_ff
    if ( rst ) begin
        arrv[0] <= 1;
        r <= a ? arrv[0] : 1'd0;
        w <= 0;
        s1 <= 0;
        variables2_PROC_STATE <= 0;    // test_cthread_var_reg.cpp:169:9;
    end
    else begin
        s1 <= s1_next;
        arrv <= arrv_next;
        r <= r_next;
        w <= w_next;
        v <= v_next;
        variables2_PROC_STATE <= variables2_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: variables_in_scope1 (test_cthread_var_reg.cpp:187:5) 

// Thread-local variables
logic [15:0] lu;
logic [15:0] lu_next;
logic [1:0] variables_in_scope1_PROC_STATE;
logic [1:0] variables_in_scope1_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : variables_in_scope1_comb     // test_cthread_var_reg.cpp:187:5
    variables_in_scope1_func;
end
function void variables_in_scope1_func;
    lu_next = lu;
    variables_in_scope1_PROC_STATE_next = variables_in_scope1_PROC_STATE;
    
    case (variables_in_scope1_PROC_STATE)
        0: begin
            if (a)
            begin
                lu_next = b;
                if (lu_next != c)
                begin
                    lu_next--;
                    variables_in_scope1_PROC_STATE_next = 1; return;    // test_cthread_var_reg.cpp:197:21;
                end
            end
            variables_in_scope1_PROC_STATE_next = 2; return;    // test_cthread_var_reg.cpp:201:13;
        end
        1: begin
            if (lu_next != c)
            begin
                lu_next--;
                variables_in_scope1_PROC_STATE_next = 1; return;    // test_cthread_var_reg.cpp:197:21;
            end
            variables_in_scope1_PROC_STATE_next = 2; return;    // test_cthread_var_reg.cpp:201:13;
        end
        2: begin
            if (a)
            begin
                lu_next = b;
                if (lu_next != c)
                begin
                    lu_next--;
                    variables_in_scope1_PROC_STATE_next = 1; return;    // test_cthread_var_reg.cpp:197:21;
                end
            end
            variables_in_scope1_PROC_STATE_next = 2; return;    // test_cthread_var_reg.cpp:201:13;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or posedge rst) 
begin : variables_in_scope1_ff
    if ( rst ) begin
        variables_in_scope1_PROC_STATE <= 0;    // test_cthread_var_reg.cpp:188:9;
    end
    else begin
        lu <= lu_next;
        variables_in_scope1_PROC_STATE <= variables_in_scope1_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: variables_in_scope2 (test_cthread_var_reg.cpp:206:5) 

// Thread-local variables
logic signed [31:0] i0;
logic signed [31:0] i_next;
logic signed [31:0] r5_next;
logic signed [7:0] lbi;
logic signed [7:0] lbi_next;
logic [1:0] variables_in_scope2_PROC_STATE;
logic [1:0] variables_in_scope2_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : variables_in_scope2_comb     // test_cthread_var_reg.cpp:206:5
    variables_in_scope2_func;
end
function void variables_in_scope2_func;
    integer li;
    i_next = i0;
    lbi_next = lbi;
    r5_next = r5;
    variables_in_scope2_PROC_STATE_next = variables_in_scope2_PROC_STATE;
    
    case (variables_in_scope2_PROC_STATE)
        0: begin
            i_next = 0;
            lbi_next = 8'(b);
            if (a)
            begin
                li = i_next + 1;
                r5_next = li;
                variables_in_scope2_PROC_STATE_next = 1; return;    // test_cthread_var_reg.cpp:216:21;
            end
            variables_in_scope2_PROC_STATE_next = 2; return;    // test_cthread_var_reg.cpp:220:17;
        end
        1: begin
            r5_next = i_next + 32'(lbi_next);
            variables_in_scope2_PROC_STATE_next = 2; return;    // test_cthread_var_reg.cpp:220:17;
        end
        2: begin
            ++i_next;
            if (i_next < 10)
            begin
                lbi_next = 8'(b);
                if (a)
                begin
                    li = i_next + 1;
                    r5_next = li;
                    variables_in_scope2_PROC_STATE_next = 1; return;    // test_cthread_var_reg.cpp:216:21;
                end
                variables_in_scope2_PROC_STATE_next = 2; return;    // test_cthread_var_reg.cpp:220:17;
            end
            variables_in_scope2_PROC_STATE_next = 3; return;    // test_cthread_var_reg.cpp:223:13;
        end
        3: begin
            i_next = 0;
            lbi_next = 8'(b);
            if (a)
            begin
                li = i_next + 1;
                r5_next = li;
                variables_in_scope2_PROC_STATE_next = 1; return;    // test_cthread_var_reg.cpp:216:21;
            end
            variables_in_scope2_PROC_STATE_next = 2; return;    // test_cthread_var_reg.cpp:220:17;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or posedge rst) 
begin : variables_in_scope2_ff
    if ( rst ) begin
        variables_in_scope2_PROC_STATE <= 0;    // test_cthread_var_reg.cpp:207:9;
    end
    else begin
        i0 <= i_next;
        r5 <= r5_next;
        lbi <= lbi_next;
        variables_in_scope2_PROC_STATE <= variables_in_scope2_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: local_array1 (test_cthread_var_reg.cpp:232:5) 

// Thread-local variables
logic signed [31:0] i1;
logic signed [31:0] i_next0;
logic arr1[3];
logic arr1_next[3];
logic signed [31:0] s6_next;

// Next-state combinational logic
always_comb begin : local_array1_comb     // test_cthread_var_reg.cpp:232:5
    local_array1_func;
end
function void local_array1_func;
    integer j;
    logic [3:0] arr4[2];
    arr1_next = arr1;
    i_next0 = i1;
    s6_next = s6;
    j = 43;
    arr4[0] = i_next0; arr4[1] = j + 1;
    s6_next = arr1_next[0];
endfunction

// Synchronous register update
always_ff @(posedge clk or posedge rst) 
begin : local_array1_ff
    if ( rst ) begin
        integer k_1;
        logic [3:0] arr2[3];
        integer arr3[3];
        i1 <= 42;
        k_1 = 42;
        arr2[0] = 0; arr2[1] = 0; arr2[2] = 0;
        arr3[0] = 1; arr3[1] = 2; arr3[2] = k_1;
    end
    else begin
        i1 <= i_next0;
        arr1 <= arr1_next;
        s6 <= s6_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: local_array2 (test_cthread_var_reg.cpp:251:5) 

// Thread-local variables
logic signed [31:0] arr30[3];
logic signed [31:0] arr3_next[3];
logic [3:0] arr40[2];
logic [3:0] arr4_next[2];
logic [3:0] arr20[3];
logic [3:0] arr2_next[3];
logic signed [31:0] s7_next;
logic local_array2_PROC_STATE;
logic local_array2_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : local_array2_comb     // test_cthread_var_reg.cpp:251:5
    local_array2_func;
end
function void local_array2_func;
    arr2_next = arr20;
    arr3_next = arr30;
    arr4_next = arr40;
    s7_next = s7;
    local_array2_PROC_STATE_next = local_array2_PROC_STATE;
    
    case (local_array2_PROC_STATE)
        0: begin
            arr4_next[0] = arr3_next[0]; arr4_next[1] = arr3_next[1];
            local_array2_PROC_STATE_next = 1; return;    // test_cthread_var_reg.cpp:261:13;
        end
        1: begin
            s7_next = arr2_next[1] + arr4_next[1];
            arr4_next[0] = arr3_next[0]; arr4_next[1] = arr3_next[1];
            local_array2_PROC_STATE_next = 1; return;    // test_cthread_var_reg.cpp:261:13;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or posedge rst) 
begin : local_array2_ff
    if ( rst ) begin
        logic arr1_1[3];
        arr1_1[0] = 1;
        arr20[0] <= 0; arr20[1] <= 0; arr20[2] <= 0;
        arr30[0] <= 1; arr30[1] <= 2; arr30[2] <= arr1_1[0];
        local_array2_PROC_STATE <= 0;    // test_cthread_var_reg.cpp:257:9;
    end
    else begin
        arr30 <= arr3_next;
        arr40 <= arr4_next;
        arr20 <= arr2_next;
        s7 <= s7_next;
        local_array2_PROC_STATE <= local_array2_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: member_array1 (test_cthread_var_reg.cpp:272:5) 

// Thread-local variables
logic signed [31:0] s8_next;
logic signed [31:0] marr3[3];
logic signed [31:0] marr3_next[3];
logic [3:0] marr2[20];
logic [3:0] marr2_next[20];
logic marr1[3];

// Next-state combinational logic
always_comb begin : member_array1_comb     // test_cthread_var_reg.cpp:272:5
    member_array1_func;
end
function void member_array1_func;
    marr2_next = marr2;
    marr3_next = marr3;
    s8_next = s8;
    s8_next = marr2_next[1] + marr3_next[1];
    marr3_next[s7] = s6;
endfunction

// Synchronous register update
always_ff @(posedge clk or posedge rst) 
begin : member_array1_ff
    if ( rst ) begin
        logic marr1[3];
        marr1[0] = 1;
        for (integer i = 0; i < 20; ++i)
        begin
            marr2[i] <= i;
        end
        s8 <= marr1[0];
    end
    else begin
        s8 <= s8_next;
        marr3 <= marr3_next;
        marr2 <= marr2_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: local_record_array1 (test_cthread_var_reg.cpp:299:5) 

// Thread-local variables
logic signed [31:0] rarr1_b[3];
logic signed [31:0] rarr1_b_next[3];
logic signed [31:0] rarr0_b[3];
logic signed [31:0] rarr0_b_next[3];
logic signed [31:0] s9_next;

// Next-state combinational logic
always_comb begin : local_record_array1_comb     // test_cthread_var_reg.cpp:299:5
    local_record_array1_func;
end
function void local_record_array1_func;
    logic rarr1_a[3];
    logic rarr2_a[2];
    integer rarr2_b[2];
    rarr0_b_next = rarr0_b;
    rarr1_b_next = rarr1_b;
    s9_next = s9;
    rarr2_b[1] = 42;
    s9_next = rarr0_b_next[0] + rarr1_b_next[0] + rarr2_b[1];
    rarr1_b_next[s8] = 43;
endfunction

// Synchronous register update
always_ff @(posedge clk or posedge rst) 
begin : local_record_array1_ff
    if ( rst ) begin
        logic rarr0_a[3];
        logic rarr1_a[3];
        rarr0_a[0] = 1;
        rarr0_b[0] <= 0;
    end
    else begin
        rarr1_b <= rarr1_b_next;
        rarr0_b <= rarr0_b_next;
        s9 <= s9_next;
    end
end

endmodule


