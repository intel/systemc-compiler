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
logic s1;
logic s2;


//------------------------------------------------------------------------------
// Child module instances

A a_mod
(
  .out(s2),
  .p(s1)
);

endmodule



//==============================================================================
//
// Module: A (test_pointers.cpp:194:5)
//
module A // "b_mod.a_mod"
(
    output logic out,
    output logic p
);

// Variables generated for SystemC signals
logic s;
logic signed [31:0] sp;
logic signed [11:0] parr2[4];
logic [41:0] parrp[5];
logic signed [31:0] c1;
logic [39:0] c2;
logic signed [79:0] c3;
logic signed [31:0] cp1;
logic [39:0] cp2;
logic signed [79:0] cp3;
logic signed [63:0] sl;

// Local parameters generated for C++ constants
localparam logic signed [31:0] k = 11;
localparam logic [31:0] N = 4;
localparam logic [31:0] M = 5;

//------------------------------------------------------------------------------
// Method process: this_pointer (test_pointers.cpp:93:5) 

// Process-local variables
logic signed [31:0] m;
logic [2:0] u;

always_comb 
begin : this_pointer     // test_pointers.cpp:93:5
    m = 1;
    s = |(2 + m);
    u = 4 + s;
    sp = 5 + u;
    m = 6 + m;
    s = |(7 + m);
    u = 8 + s;
    sp = 9 + u;
end

//------------------------------------------------------------------------------
// Method process: this_pointer2 (test_pointers.cpp:106:5) 

always_comb 
begin : this_pointer2     // test_pointers.cpp:106:5
    integer i;
    i = sp + sp;
end

//------------------------------------------------------------------------------
// Method process: pointer_decl_init (test_pointers.cpp:111:5) 

// Process-local variables
logic [2:0] n;
logic signed [31:0] q;
logic [2:0] u1;

always_comb 
begin : pointer_decl_init     // test_pointers.cpp:111:5
    integer i;
    i = q + q;
    i = k;
    i = u1 - u1 + n;
    n = i + 1;
    i = n;
end

//------------------------------------------------------------------------------
// Method process: pointer_if (test_pointers.cpp:127:5) 

// Process-local variables
logic [2:0] l;

always_comb 
begin : pointer_if     // test_pointers.cpp:127:5
    l = 2;
end

//------------------------------------------------------------------------------
// Method process: array_ptr1 (test_pointers.cpp:162:5) 

// Process-local variables
logic [11:0] parr1[4];

always_comb 
begin : array_ptr1     // test_pointers.cpp:162:5
    for (integer i = 0; i < N; i++)
    begin
        parr1[i] = i;
    end
    for (integer i_1 = 0; i_1 < N - 1; i_1++)
    begin
        parr2[i_1] = 32'(parr1[i_1]);
    end
    parr2[N - 1] = parr1[N - 2] + parr1[N - 1];
    parr2[sp] = 1;
end

//------------------------------------------------------------------------------
// Method process: array_ptr2 (test_pointers.cpp:176:5) 

always_comb 
begin : array_ptr2     // test_pointers.cpp:176:5
    for (integer i = 0; i < N; i++)
    begin
        parrp[i] = 42'(parr2[i]);
    end
    for (integer i_1 = N; i_1 < M; i_1++)
    begin
        parrp[i_1] = 0;
    end
    sl = 32'(parrp[0] + parrp[parr2[0]]);
end

//------------------------------------------------------------------------------
// Method process: channel_pointer (test_pointers.cpp:146:5) 

always_comb 
begin : channel_pointer     // test_pointers.cpp:146:5
    integer l_1;
    logic signed [80:0] x;
    logic signed [80:0] y;
    l_1 = c1;
    l_1 = c2;
    l_1 = l_1 * signed'({1'b0, c2});
    l_1 = l_1 + 32'(c3);
    x = signed'({1'b0, c2}) - c3;
    l_1 = cp1;
    l_1 = l_1 * signed'({1'b0, cp2});
    l_1 = l_1 + 32'(cp3);
    y = signed'({1'b0, cp2}) - cp3;
end

endmodule


