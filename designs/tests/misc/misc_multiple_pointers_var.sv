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


//------------------------------------------------------------------------------
// Child module instances

A a_mod
(

);

endmodule



//==============================================================================
//
// Module: A (test_multiple_pointers_var.cpp:48:5)
//
module A // "b_mod.a_mod"
(
);

// Variables generated for SystemC signals
logic signed [31:0] s;
logic signed [31:0] t0;

//------------------------------------------------------------------------------
// Method process: meth (test_multiple_pointers_var.cpp:38:5) 

// Process-local variables
logic a;

always_comb 
begin : meth     // test_multiple_pointers_var.cpp:38:5
    a = |s;
    a = |(s + 1);
    t0 = a;
end

endmodule


