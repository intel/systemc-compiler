//==============================================================================
//
// The code is generated by Intel Compiler for SystemC, version 1.6.14
// see more information at https://github.com/intel/systemc-compiler
//
//==============================================================================

//==============================================================================
//
// Module: top ()
//
module top // "top_inst"
(
);

// Variables generated for SystemC signals
logic [7:0] arr[4096];
logic [7:0] out;

//------------------------------------------------------------------------------
// Method process: proc (test_large_array.cpp:24:5) 

always_comb 
begin : proc     // test_large_array.cpp:24:5
    out = arr[0];
end

endmodule


