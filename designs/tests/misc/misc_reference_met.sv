//==============================================================================
//
// The code is generated by Intel Compiler for SystemC, version 1.6.14
// see more information at https://github.com/intel/systemc-compiler
//
//==============================================================================

//==============================================================================
//
// Module: test_referece_met ()
//
module test_referece_met // "tinst"
(
);

// Variables generated for SystemC signals
logic sig;
logic sigArray[2];
logic signed [31:0] t0;

//------------------------------------------------------------------------------
// Method process: test_method (test_reference_met.cpp:29:5) 

// Process-local variables
logic signed [31:0] x;

always_comb 
begin : test_method     // test_reference_met.cpp:29:5
    x = sig;
    x = 2;
    t0 = x;
    sigArray[0] = 1;
end

endmodule


