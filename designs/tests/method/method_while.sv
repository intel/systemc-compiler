//==============================================================================
//
// The code is generated by Intel Compiler for SystemC, version 1.5.13
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
// Module: A (test_while.cpp:199:5)
//
module A // "b_mod.a_mod"
(
    input logic a,
    output logic b,
    output logic c
);

// Variables generated for SystemC signals
logic s;
logic signed [31:0] t0;
logic signed [31:0] t1;
logic signed [31:0] t2;
logic signed [31:0] t3;
logic signed [31:0] t4;

// Local parameters generated for C++ constants
localparam logic signed [31:0] m = 11;

//------------------------------------------------------------------------------
// Method process: while_stmt_empty (test_while.cpp:45:5) 

always_comb 
begin : while_stmt_empty     // test_while.cpp:45:5
    integer i;
    i = 0;
    while (i < 2)
    begin
        i++;
    end
    t0 = i;
end

//------------------------------------------------------------------------------
// Method process: while_stmt1 (test_while.cpp:56:5) 

always_comb 
begin : while_stmt1     // test_while.cpp:56:5
    integer i;
    integer k_1;
    i = 0;
    while (i < 2)
    begin
        k_1 = k_1 + 1;
        i++;
    end
    t1 = i;
end

//------------------------------------------------------------------------------
// Method process: while_stmt2 (test_while.cpp:69:5) 

always_comb 
begin : while_stmt2     // test_while.cpp:69:5
    integer j;
    integer i;
    j = 1;
    i = 0;
    while (i < 3)
    begin
        i++;
        j = 2;
    end
    j = 4;
    t2 = j;
end

//------------------------------------------------------------------------------
// Method process: while_stmt3 (test_while.cpp:88:5) 

always_comb 
begin : while_stmt3     // test_while.cpp:88:5
    integer i;
    integer j;
    i = 0;
    j = 1;
    if (s)
    begin
        j = 2;
    end else begin
        j = 3;
    end
    while (i < 2)
    begin
        i++;
    end
    j = 4;
    t3 = j;
end

//------------------------------------------------------------------------------
// Method process: while_stmt4 (test_while.cpp:106:5) 

always_comb 
begin : while_stmt4     // test_while.cpp:106:5
    integer i;
    integer j;
    i = 0;
    j = 1;
    if (s)
    begin
        j = 2;
    end
    while (i < 2)
    begin
        i++;
    end
    j = 3;
    t4 = j;
end

//------------------------------------------------------------------------------
// Method process: while_stmt5 (test_while.cpp:122:5) 

always_comb 
begin : while_stmt5     // test_while.cpp:122:5
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
// Method process: while_stmt6 (test_while.cpp:137:5) 

always_comb 
begin : while_stmt6     // test_while.cpp:137:5
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
// Method process: while_stmt7 (test_while.cpp:151:5) 

always_comb 
begin : while_stmt7     // test_while.cpp:151:5
    integer k_1;
    integer i;
    k_1 = 0;
    i = 0;
    while (i < 3)
    begin
        k_1 = k_1 + i;
        i++;
    end
end

//------------------------------------------------------------------------------
// Method process: while_const (test_while.cpp:164:5) 

always_comb 
begin : while_const     // test_while.cpp:164:5
    integer k_1;
    k_1 = 0;
    b = |(k_1 + 1);
    k_1 = 10;
    b = |(k_1 + 2);
end

//------------------------------------------------------------------------------
// Method process: while_sc_type (test_while.cpp:178:5) 

// Process-local variables
logic signed [31:0] k;

always_comb 
begin : while_sc_type     // test_while.cpp:178:5
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


