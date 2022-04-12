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
logic c;


//------------------------------------------------------------------------------
// Child module instances

A a_mod
(
  .a(a),
  .b(b),
  .c(c)
);

endmodule



//==============================================================================
//
// Module: A (test_if.cpp:384:5)
//
module A // "b_mod.a_mod"
(
    input logic a,
    output logic b,
    output logic c
);

// Variables generated for SystemC signals
logic s;
logic s1;
logic s2;

// Local parameters generated for C++ constants
localparam logic signed [31:0] n = 11;
localparam logic READ_BUFFER_ENABLE = 1;

//------------------------------------------------------------------------------
// Method process: if_const_and_signal (test_if.cpp:64:5) 

always_comb 
begin : if_const_and_signal     // test_if.cpp:64:5
    integer i;
    i = 0;
end

//------------------------------------------------------------------------------
// Method process: if_empty1 (test_if.cpp:84:5) 

always_comb 
begin : if_empty1     // test_if.cpp:84:5
    integer i;
    if (n > 0)
    begin
        i = 3;
    end
    i = 5;
end

//------------------------------------------------------------------------------
// Method process: if_empty2 (test_if.cpp:95:5) 

always_comb 
begin : if_empty2     // test_if.cpp:95:5
    integer i;
    integer n_1;
    n_1 = s;
    if (n_1 > 0)
    begin
    end else begin
        i = 3;
    end
    i = 5;
end

//------------------------------------------------------------------------------
// Method process: if_empty3 (test_if.cpp:106:5) 

always_comb 
begin : if_empty3     // test_if.cpp:106:5
    integer n_1;
    n_1 = s;
    b = 1;
end

//------------------------------------------------------------------------------
// Method process: if_empty4 (test_if.cpp:116:5) 

// Process-local variables
logic signed [31:0] m;

always_comb 
begin : if_empty4     // test_if.cpp:116:5
    integer n_1;
    n_1 = s;
    if (n_1 > 0)
    begin
        if (n_1 > 1)
        begin
            m = 1;
        end
    end
    if (n_1 > 2)
    begin
    end else begin
        m = 2;
    end
    m = 3;
end

//------------------------------------------------------------------------------
// Method process: if_stmt1 (test_if.cpp:135:5) 

always_comb 
begin : if_stmt1     // test_if.cpp:135:5
    integer i;
    integer j_1;
    j_1 = a;
    if (j_1 > 0)
    begin
        i = 1;
    end else begin
        i = 2;
    end
    b = |i;
end

//------------------------------------------------------------------------------
// Method process: if_stmt2 (test_if.cpp:148:5) 

always_comb 
begin : if_stmt2     // test_if.cpp:148:5
    integer i;
    integer m_1;
    integer n_1;
    m_1 = s;
    n_1 = s;
    if (m_1 > 0)
    begin
        i = 1;
        if (n_1 > 0)
        begin
            i = 3;
        end
        i = 5;
    end else begin
        i = 2;
        if (n_1 > 1)
        begin
            i = 4;
        end
    end
    i = 0;
end

//------------------------------------------------------------------------------
// Method process: if_stmt2a (test_if.cpp:169:5) 

always_comb 
begin : if_stmt2a     // test_if.cpp:169:5
    integer k_1;
    integer m_1;
    integer n_1;
    k_1 = 0;
    m_1 = s;
    n_1 = s;
    if (m_1 > 0)
    begin
        k_1 = 1;
        if (n_1 > 0)
        begin
            k_1 = 2;
        end else begin
            k_1 = 3;
        end
    end else begin
        if (n_1 > 1)
        begin
            k_1 = 4;
        end
    end
    k_1 = 6;
end

//------------------------------------------------------------------------------
// Method process: if_stmt3 (test_if.cpp:193:5) 

always_comb 
begin : if_stmt3     // test_if.cpp:193:5
    integer i;
    integer n_1;
    integer k_1;
    n_1 = s;
    k_1 = s;
    if (a)
    begin
        i = 1;
        if (n_1 > a)
        begin
            if (k_1 == a)
            begin
                i++;
            end else begin
                i--;
            end
            i = 3;
        end
        i = 5;
    end
    b = |i;
end

//------------------------------------------------------------------------------
// Method process: if_stmt4 (test_if.cpp:214:5) 

always_comb 
begin : if_stmt4     // test_if.cpp:214:5
    integer i;
    if (a)
    begin
        if (b)
        begin
            i = 1;
        end
    end
    b = |i;
end

//------------------------------------------------------------------------------
// Method process: if_stmt5 (test_if.cpp:226:5) 

always_comb 
begin : if_stmt5     // test_if.cpp:226:5
    integer i;
    integer m_1;
    integer k_1;
    integer n_1;
    m_1 = s;
    k_1 = s;
    n_1 = s;
    if (m_1 > 0)
    begin
        i = 1;
        i = 2;
    end
    if (k_1 < 0)
    begin
        i = 3;
        if (n_1 > 0)
        begin
            i = 4;
        end
    end
    i = 5;
    if (k_1 == 0)
    begin
        i = 6;
    end
end

//------------------------------------------------------------------------------
// Method process: if_compl_cond1 (test_if.cpp:249:5) 

always_comb 
begin : if_compl_cond1     // test_if.cpp:249:5
    integer i;
    integer k_1;
    integer m_1;
    k_1 = s;
    m_1 = s;
    if (m_1 == 1 || k_1 == 1)
    begin
        i = 1;
    end else begin
        i = 2;
    end
    if (m_1 > 1 || k_1 < 1)
    begin
        i = 1;
    end
    if (m_1 != k_1 || k_1 != 1)
    begin
    end else begin
        i = 2;
    end
    b = |i;
end

//------------------------------------------------------------------------------
// Method process: if_compl_cond2 (test_if.cpp:273:5) 

always_comb 
begin : if_compl_cond2     // test_if.cpp:273:5
    integer i;
    integer k_1;
    integer m_1;
    k_1 = s;
    m_1 = s;
    if (m_1 == 1 && k_1 == 1)
    begin
        i = 1;
    end else begin
        i = 2;
    end
    if (m_1 > 1 && k_1 < 1)
    begin
        i = 1;
    end
    if (m_1 != k_1 && k_1 != 1)
    begin
    end else begin
        i = 2;
    end
    b = |i;
end

//------------------------------------------------------------------------------
// Method process: if_compl_cond3 (test_if.cpp:297:5) 

always_comb 
begin : if_compl_cond3     // test_if.cpp:297:5
    integer i;
    integer k_1;
    integer m_1;
    k_1 = s;
    m_1 = s;
    if (m_1 == 1 && k_1 == 2 || k_1 < m_1)
    begin
        i = 1;
    end else begin
        i = 2;
    end
    if (m_1 == 1 || k_1 == 2 && k_1 < m_1)
    begin
        i = 1;
    end
    if (m_1 == 1 && k_1 == 1 || m_1 == 2 && k_1 == 2)
    begin
    end else begin
        i = 2;
    end
    b = |i;
end

//------------------------------------------------------------------------------
// Method process: if_compl_cond4 (test_if.cpp:321:5) 

always_comb 
begin : if_compl_cond4     // test_if.cpp:321:5
    integer i;
    integer k_1;
    integer m_1;
    k_1 = s;
    m_1 = s;
    if (m_1 == 1 && (k_1 == 2 || k_1 < m_1))
    begin
        i = 1;
    end else begin
        i = 2;
    end
    if ((m_1 == 1 || k_1 == 2) && k_1 < m_1)
    begin
        i = 1;
    end
    if (m_1 == 1 && (k_1 == 1 || m_1 == 2) && k_1 == 2)
    begin
    end else begin
        i = 2;
    end
    b = |i;
end

//------------------------------------------------------------------------------
// Method process: if_const (test_if.cpp:345:5) 

always_comb 
begin : if_const     // test_if.cpp:345:5
    integer i;
    integer k_1;
    integer m_1;
    k_1 = s;
    m_1 = s;
    i = m_1;
    i = m_1;
    i = k_1;
    i = m_1;
end

endmodule


