//==============================================================================
//
// The code is generated by Intel Compiler for SystemC, version 1.4.18
// see more information at https://github.com/intel/systemc-compiler
//
//==============================================================================

//==============================================================================
//
// Module: testbench ()
//
module testbench // "tb_inst"
(
);

// Variables generated for SystemC signals
logic s1;
logic [1:0] s2;


//------------------------------------------------------------------------------
// Child module instances

A a
(
  .in(s1)
);

B b
(
  .in(s1),
  .out(s2)
);

endmodule



//==============================================================================
//
// Verilog intrinsic for module: A (test_verilog_intrinsic2.cpp:52:5)
//
module A(input in);
   // Some verilog code for module A
endmodule
    

//==============================================================================
//
// Verilog intrinsic for module: B (test_verilog_intrinsic2.cpp:53:5)
//
module B(input in, 
         output out);
   // Some verilog code for module B
endmodule
    
