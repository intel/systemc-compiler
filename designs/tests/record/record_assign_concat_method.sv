//==============================================================================
//
// The code is generated by Intel Compiler for SystemC, version 1.6.6
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
logic inp_a;
logic outp_b;
logic [31:0] inp_c;
logic [31:0] outp_d;
logic [31:0] outp_t1;
logic [31:0] outp_t2;
logic [31:0] outp_t3;


//------------------------------------------------------------------------------
// Child module instances

A a_mod
(
  .inp_a(inp_a),
  .inp_c(inp_c),
  .outp_b(outp_b),
  .outp_d(outp_d),
  .outp_t1(outp_t1),
  .outp_t2(outp_t2),
  .outp_t3(outp_t3)
);

endmodule



//==============================================================================
//
// Module: A (test_assign_concat_method.cpp:110:5)
//
module A // "b_mod.a_mod"
(
    input logic inp_a,
    input logic [31:0] inp_c,
    output logic outp_b,
    output logic [31:0] outp_d,
    output logic [31:0] outp_t1,
    output logic [31:0] outp_t2,
    output logic [31:0] outp_t3
);

// Variables generated for SystemC signals

//------------------------------------------------------------------------------
// Method process: record_local_var1 (test_assign_concat_method.cpp:63:5) 

always_comb 
begin : record_local_var1     // test_assign_concat_method.cpp:63:5
    integer r_x;
    logic [31:0] r_y;
    r_y = 1;
    // Call Rec1() begin
    r_x = 1;
    // Call Rec1() end
    r_x = r_y + 2 + inp_c;
    outp_t2 = r_x;
    outp_t3 = r_y;
end

//------------------------------------------------------------------------------
// Method process: record_assign1 (test_assign_concat_method.cpp:75:5) 

always_comb 
begin : record_assign1     // test_assign_concat_method.cpp:75:5
    logic r_a;
    integer r_b;
    logic s_a;
    integer s_b;
    s_a = r_a; s_b = r_b;
    r_a = s_a; r_b = s_b;
    outp_b = r_a;
    outp_d = r_b;
end

//------------------------------------------------------------------------------
// Method process: record_assign2 (test_assign_concat_method.cpp:84:5) 

always_comb 
begin : record_assign2     // test_assign_concat_method.cpp:84:5
    logic r_a;
    integer r_b;
    logic t_c;
    logic [2:0] t_d;
    t_c = 0;
    t_d = 0;
    t_d = {1'(r_a), 2'(r_b)};
end

//------------------------------------------------------------------------------
// Method process: record_assign3 (test_assign_concat_method.cpp:91:5) 

always_comb 
begin : record_assign3     // test_assign_concat_method.cpp:91:5
    logic t_c;
    logic [2:0] t_d;
    t_c = 0;
    t_d = 0;
    t_c = 1;
    t_d = 4;
    outp_t1 = {t_c, t_d};
end

endmodule


