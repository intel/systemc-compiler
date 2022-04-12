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
// Module: A (test_bool.cpp:255:5)
//
module A // "b_mod.a_mod"
(
    input logic a,
    output logic b,
    output logic c
);

// Variables generated for SystemC signals
logic s;
logic ps;

//------------------------------------------------------------------------------
// Method process: bool_arithmetic (test_bool.cpp:60:5) 

always_comb 
begin : bool_arithmetic     // test_bool.cpp:60:5
    integer res;
    logic b_1;
    integer unsigned u;
    integer i;
    logic [15:0] ux;
    logic signed [15:0] ix;
    logic [15:0] ub;
    logic signed [15:0] ib;
    b_1 = 1;
    u = 42;
    i = -7'sd42;
    ux = 43;
    ix = -7'sd43;
    ub = 44;
    ib = -7'sd44;
    res = b_1 + b_1 * b_1;
    `ifndef INTEL_SVA_OFF
        assert (res == 2) else $error("Assertion failed at test_bool.cpp:58:24>");
    `endif // INTEL_SVA_OFF
    res = b_1 + 1;
    `ifndef INTEL_SVA_OFF
        assert (res == 2) else $error("Assertion failed at test_bool.cpp:58:24>");
    `endif // INTEL_SVA_OFF
    res = u + b_1;
    `ifndef INTEL_SVA_OFF
        assert (res == 43) else $error("Assertion failed at test_bool.cpp:58:24>");
    `endif // INTEL_SVA_OFF
    res = i + b_1;
    `ifndef INTEL_SVA_OFF
        assert (res == -7'sd41) else $error("Assertion failed at test_bool.cpp:58:24>");
    `endif // INTEL_SVA_OFF
    res = ux + b_1;
    `ifndef INTEL_SVA_OFF
        assert (res == 44) else $error("Assertion failed at test_bool.cpp:58:24>");
    `endif // INTEL_SVA_OFF
    res = ix + b_1;
    `ifndef INTEL_SVA_OFF
        assert (res == -7'sd42) else $error("Assertion failed at test_bool.cpp:58:24>");
    `endif // INTEL_SVA_OFF
    res = 64'(ub) + b_1;
    `ifndef INTEL_SVA_OFF
        assert (res == 45) else $error("Assertion failed at test_bool.cpp:58:24>");
    `endif // INTEL_SVA_OFF
    res = 64'(ib) + b_1;
    `ifndef INTEL_SVA_OFF
        assert (res == -7'sd43) else $error("Assertion failed at test_bool.cpp:58:24>");
    `endif // INTEL_SVA_OFF
    res = -b_1 + i;
    `ifndef INTEL_SVA_OFF
        assert (res == -7'sd43) else $error("Assertion failed at test_bool.cpp:58:24>");
    `endif // INTEL_SVA_OFF
end

//------------------------------------------------------------------------------
// Method process: test_bool_to_bool (test_bool.cpp:94:5) 

always_comb 
begin : test_bool_to_bool     // test_bool.cpp:94:5
    logic b1;
    logic b2;
    b1 = a;
    b1 = a;
    b2 = b1;
    b2 = b1;
    b = b1;
    b = b1;
    b = a;
    b = a;
    b = a;
    b = a;
    s = b;
    s = b1;
    b = s;
    b = s;
    ps = b1;
    ps = b;
    b = ps;
    b = ps;
end

//------------------------------------------------------------------------------
// Method process: test_bool_unary (test_bool.cpp:119:5) 

always_comb 
begin : test_bool_unary     // test_bool.cpp:119:5
    integer unsigned i;
    logic b_1;
    i = 2;
    b_1 = |i;
    b_1 = |(-i);
    b_1 = |(i++);
    b_1 = |(--i);
end

//------------------------------------------------------------------------------
// Method process: test_sc_to_bool (test_bool.cpp:129:5) 

// Process-local variables
logic [4:0] px;

always_comb 
begin : test_sc_to_bool     // test_bool.cpp:129:5
    logic b1;
    logic [2:0] x;
    x = 0;
    b1 = x[1];
    b1 = |x[2 : 1];
    b1 = |x;
    b1 = px[1];
    b1 = |px[2 : 1];
    b1 = |px;
    b1 = |(px + x);
    s = x[1];
    s = |x[2 : 1];
    s = |x;
    ps = x[1];
    ps = |x[2 : 1];
    ps = |x;
end

//------------------------------------------------------------------------------
// Method process: test_ptr_comp (test_bool.cpp:153:5) 

// Process-local variables
logic signed [31:0] p1;

always_comb 
begin : test_ptr_comp     // test_bool.cpp:153:5
    logic b_1;
    b_1 = 1;
    b_1 = 0;
    b_1 = 0;
    b_1 = 1;
    b_1 = 0;
    b_1 = 1;
    b_1 = 0;
    b_1 = 1;
    b_1 = 1;
    b_1 = 0;
    b_1 = 1;
    b_1 = 0;
end

//------------------------------------------------------------------------------
// Method process: test_int_comp (test_bool.cpp:173:5) 

// Process-local variables
logic signed [31:0] p2;

always_comb 
begin : test_int_comp     // test_bool.cpp:173:5
    logic b1;
    integer i;
    logic [3:0] x;
    x = 0;
    b1 = i == 0;
    b1 = x > 1;
    b1 = i != x;
    b = i < 3;
    b = x >= i;
    s = x == i;
    s = p2 > 0;
    ps = x != 0;
    ps = i == p2;
end

//------------------------------------------------------------------------------
// Method process: test_sc_comp (test_bool.cpp:193:5) 

always_comb 
begin : test_sc_comp     // test_bool.cpp:193:5
    logic b1;
    logic signed [3:0] x;
    logic signed [11:0] ux;
    logic b2;
    x = 0;
    ux = 0;
    b2 = b1 == ux;
    b2 = b1 == x;
end

//------------------------------------------------------------------------------
// Method process: test_bool1 (test_bool.cpp:203:5) 

// Process-local variables
logic signed [31:0] m;

always_comb 
begin : test_bool1     // test_bool.cpp:203:5
    integer unsigned i;
    logic b1;
    i = 2;
    b1 = 1;
    b1 = |i;
    b1 = |i;
    b1 = |i;
    b1 = |(i + 1);
    b1 = |(|i + 1);
    b1 = |(a ? m : m == i);
    b1 = |m ? |(++m) : m > 0;
end

//------------------------------------------------------------------------------
// Method process: test_bool2 (test_bool.cpp:217:5) 

always_comb 
begin : test_bool2     // test_bool.cpp:217:5
    logic [3:0] x;
    logic b_1;
    x = 6;
    b_1 = x[2];
    b_1 = |x[3 : 1];
    b_1 = |x;
    b_1 = !(|x);
    b_1 = !x[2];
    b_1 = !(|x[3 : 1]);
    b_1 = |(x + x[3 : 1]);
end

//------------------------------------------------------------------------------
// Method process: test_bool4 (test_bool.cpp:231:5) 

// Process-local variables
logic signed [31:0] p;

always_comb 
begin : test_bool4     // test_bool.cpp:231:5
    integer unsigned i;
    i = 1;
    i = 2;
    i = 3;
    i = 4;
end

endmodule


