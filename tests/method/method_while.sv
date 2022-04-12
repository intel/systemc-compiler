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
// Module: A (test_while.cpp:189:5)
//
module A // "b_mod.a_mod"
(
    input logic a,
    output logic b,
    output logic c
);

// Variables generated for SystemC signals
logic s;

// Local parameters generated for C++ constants
localparam logic signed [31:0] m = 11;

//------------------------------------------------------------------------------
// Method process: while_stmt_empty (test_while.cpp:44:5) 

always_comb 
begin : while_stmt_empty     // test_while.cpp:44:5
    integer i;
    i = 0;
    while (i < 2)
    begin
        i++;
    end
end

//------------------------------------------------------------------------------
// Method process: while_stmt1 (test_while.cpp:53:5) 

always_comb 
begin : while_stmt1     // test_while.cpp:53:5
    integer k_1;
    integer i;
    integer k_2;
    k_1 = 0;
    i = 0;
    while (i < 2)
    begin
        k_2 = k_2 + 1;
        i++;
    end
end

//------------------------------------------------------------------------------
// Method process: while_stmt2 (test_while.cpp:64:5) 

always_comb 
begin : while_stmt2     // test_while.cpp:64:5
    integer j;
    integer i;
    j = 1;
    i = 0;
    while (i < 3)
    begin
        i++;
        if (m > 0)
        begin
            j = 2;
        end else begin
            j = 3;
        end
    end
    j = 4;
end

//------------------------------------------------------------------------------
// Method process: while_stmt3 (test_while.cpp:81:5) 

always_comb 
begin : while_stmt3     // test_while.cpp:81:5
    integer i;
    integer j;
    integer k_1;
    i = 0;
    j = 1;
    k_1 = 0;
    if (s)
    begin
        j = 2;
    end else begin
        j = 3;
    end
    while (i < 2)
    begin
        k_1++;
        i++;
    end
    j = 4;
end

//------------------------------------------------------------------------------
// Method process: while_stmt4 (test_while.cpp:97:5) 

always_comb 
begin : while_stmt4     // test_while.cpp:97:5
    integer i;
    integer j;
    integer k_1;
    i = 0;
    j = 1;
    k_1 = 0;
    if (s)
    begin
        j = 2;
    end
    while (i < 2)
    begin
        k_1++;
        i++;
    end
    j = 3;
end

//------------------------------------------------------------------------------
// Method process: while_stmt5 (test_while.cpp:112:5) 

always_comb 
begin : while_stmt5     // test_while.cpp:112:5
    integer k_1;
    integer i;
    integer j;
    k_1 = 0;
    i = 0;
    while (i < 2)
    begin
        j = 0;
        i++;
        while (j < 3)
        begin
            k_1 = k_1 + 1;
            j++;
        end
    end
end

//------------------------------------------------------------------------------
// Method process: while_stmt6 (test_while.cpp:127:5) 

always_comb 
begin : while_stmt6     // test_while.cpp:127:5
    integer k_1;
    integer i;
    k_1 = 0;
    i = 0;
    if (s)
    begin
        while (i < 2)
        begin
            k_1 = k_1 + 1;
            i++;
        end
    end else begin
        k_1 = 3;
    end
end

//------------------------------------------------------------------------------
// Method process: while_stmt7 (test_while.cpp:141:5) 

always_comb 
begin : while_stmt7     // test_while.cpp:141:5
    integer k_1;
    integer n_1;
    integer mm;
    integer i;
    k_1 = 0;
    n_1 = 0;
    mm = 0;
    i = 0;
    while (i < 3)
    begin
        k_1 = k_1 + i;
        i++;
        n_1 = mm++;
    end
end

//------------------------------------------------------------------------------
// Method process: while_const (test_while.cpp:154:5) 

always_comb 
begin : while_const     // test_while.cpp:154:5
    integer k_1;
    k_1 = 0;
    b = |(k_1 + 1);
    k_1 = 10;
    b = |(k_1 + 2);
end

//------------------------------------------------------------------------------
// Method process: while_sc_type (test_while.cpp:168:5) 

// Process-local variables
logic signed [31:0] k;

always_comb 
begin : while_sc_type     // test_while.cpp:168:5
    logic [2:0] i;
    k = 1;
    i = 0;
    while (i < 2)
    begin
        k = k + 2;
        i++;
    end
end

endmodule


