//==============================================================================
//
// The code is generated by Intel Compiler for SystemC, version 1.6.14
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
logic clk;
logic nrst;


//------------------------------------------------------------------------------
// Child module instances

A a_mod
(
  .clk(clk),
  .nrst(nrst),
  .a(a),
  .b(b)
);

endmodule



//==============================================================================
//
// Module: A (test_cthread_read_defined_unknown.cpp:262:5)
//
module A // "b_mod.a_mod"
(
    input logic clk,
    input logic nrst,
    input logic a,
    output logic b
);

// Variables generated for SystemC signals
logic pca[3];
logic ppca[3][2];

//------------------------------------------------------------------------------
// Method process: read_pointer (test_cthread_read_defined_unknown.cpp:78:5) 

// Process-local variables
logic signed [31:0] pi;

always_comb 
begin : read_pointer     // test_cthread_read_defined_unknown.cpp:78:5
    integer i;
    i = pi;
end

//------------------------------------------------------------------------------
// Method process: read_pointer_array_unknown1 (test_cthread_read_defined_unknown.cpp:84:5) 

// Process-local variables
logic signed [31:0] pa[3];
logic [2:0] ppa[3][2];

always_comb 
begin : read_pointer_array_unknown1     // test_cthread_read_defined_unknown.cpp:84:5
    integer i;
    logic [2:0] x;
    logic b_1;
    i = pa[a];
    x = ppa[a][0];
    b_1 = pca[a];
    b_1 = ppca[a][0];
end

//------------------------------------------------------------------------------
// Method process: read_pointer_array_unknown2 (test_cthread_read_defined_unknown.cpp:104:5) 

// Process-local variables
logic [2:0] ppd[3][2];

always_comb 
begin : read_pointer_array_unknown2     // test_cthread_read_defined_unknown.cpp:104:5
    logic [2:0] x;
    logic b_1;
    x = ppd[1][a];
    b_1 = pca[a];
    b_1 = ppca[1][a];
end

//------------------------------------------------------------------------------
// Method process: read_pointer_array_unknown3 (test_cthread_read_defined_unknown.cpp:119:5) 

// Process-local variables
logic [2:0] ppb[3][2];

always_comb 
begin : read_pointer_array_unknown3     // test_cthread_read_defined_unknown.cpp:119:5
    logic [2:0] x;
    logic b_1;
    x = ppb[0][a];
    b_1 = pca[a];
    b_1 = ppca[0][a];
end

//------------------------------------------------------------------------------
// Method process: define_pointer_array_unknown1 (test_cthread_read_defined_unknown.cpp:134:5) 

// Process-local variables
logic signed [31:0] pb[3];
logic [2:0] ppc[3][2];

always_comb 
begin : define_pointer_array_unknown1     // test_cthread_read_defined_unknown.cpp:134:5
    logic [2:0] x;
    pb[a] = 1;
    x = 0;
    ppc[a][0] = x;
    ppc[a][a] = x;
    pca[a] = 1;
    ppca[a][0] = 1;
    ppca[1][a] = 1;
end

//------------------------------------------------------------------------------
// Method process: read_array_unknown1 (test_cthread_read_defined_unknown.cpp:157:5) 

always_comb 
begin : read_array_unknown1     // test_cthread_read_defined_unknown.cpp:157:5
    integer arr[2];
    integer arr_[2][3];
    integer i;
    i = arr[a];
    i = arr_[a][0];
end

//------------------------------------------------------------------------------
// Method process: read_array_unknown2 (test_cthread_read_defined_unknown.cpp:169:5) 

always_comb 
begin : read_array_unknown2     // test_cthread_read_defined_unknown.cpp:169:5
    integer arr[2];
    integer arr_[2][3];
    integer i;
    integer j;
    i = arr[a];
    j = arr_[1][a];
end

//------------------------------------------------------------------------------
// Method process: read_array_unknown3 (test_cthread_read_defined_unknown.cpp:180:5) 

always_comb 
begin : read_array_unknown3     // test_cthread_read_defined_unknown.cpp:180:5
    integer arr[2];
    integer arr_[2][3];
    arr[a]++;
    arr_[a][0]++;
end

//------------------------------------------------------------------------------
// Method process: read_array_unknown4 (test_cthread_read_defined_unknown.cpp:191:5) 

always_comb 
begin : read_array_unknown4     // test_cthread_read_defined_unknown.cpp:191:5
    integer arr[2];
    integer arr_[2][3];
    arr[a] = arr[a] - 1;
    arr_[1][a] = arr_[1][a] - 1;
end

//------------------------------------------------------------------------------
// Method process: read_array_unknown_sc_type (test_cthread_read_defined_unknown.cpp:202:5) 

always_comb 
begin : read_array_unknown_sc_type     // test_cthread_read_defined_unknown.cpp:202:5
    logic [2:0] arr1[2];
    integer i;
    logic [2:0] arr2[2];
    integer j;
    logic [2:0] arr3[2];
    logic [2:0] arr4[2];
    arr1[0] = 0; arr1[1] = 0;
    i = arr1[a];
    arr2[0] = 0; arr2[1] = 0;
    j = arr2[a];
    arr3[0] = 0; arr3[1] = 0;
    arr3[a]++;
    arr4[0] = 0; arr4[1] = 0;
    arr4[a] = arr4[a] + 1;
end

//------------------------------------------------------------------------------
// Method process: define_array_unknown1 (test_cthread_read_defined_unknown.cpp:223:5) 

always_comb 
begin : define_array_unknown1     // test_cthread_read_defined_unknown.cpp:223:5
    integer arr[2];
    arr[a] = 0;
end

//------------------------------------------------------------------------------
// Method process: define_array_unknown2 (test_cthread_read_defined_unknown.cpp:232:5) 

always_comb 
begin : define_array_unknown2     // test_cthread_read_defined_unknown.cpp:232:5
    integer arr[2][2];
    arr[a][1] = 0;
    arr[0][a] = 0;
end

//------------------------------------------------------------------------------
// Method process: define_array_unknown_sc_type (test_cthread_read_defined_unknown.cpp:242:5) 

always_comb 
begin : define_array_unknown_sc_type     // test_cthread_read_defined_unknown.cpp:242:5
    logic signed [1:0] arr[2];
    arr[0] = 0; arr[1] = 0;
    arr[a] = 0;
end

endmodule


