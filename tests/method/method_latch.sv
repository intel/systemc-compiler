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


//------------------------------------------------------------------------------
// Child module instances

A a_mod
(
  .a(a),
  .b(b)
);

endmodule



//==============================================================================
//
// Module: A (test_latch.cpp:74:5)
//
module A // "b_mod.a_mod"
(
    input logic a,
    output logic b
);

//------------------------------------------------------------------------------
// Method process: latch1 (test_latch.cpp:33:5) 

always_comb 
begin : latch1     // test_latch.cpp:33:5
    if (a)
    begin
        b = 1;
    end else begin
        b = 0;
    end
end

//------------------------------------------------------------------------------
// Method process: latch2 (test_latch.cpp:45:5) 

always_latch 
begin : latch2     // test_latch.cpp:45:5
    if (a)
    begin
        b = 1;
    end
end

//------------------------------------------------------------------------------
// Method process: no_latch1 (test_latch.cpp:54:5) 

always_comb 
begin : no_latch1     // test_latch.cpp:54:5
    b = 1;
end

//------------------------------------------------------------------------------
// Method process: no_latch2 (test_latch.cpp:61:5) 

always_comb 
begin : no_latch2     // test_latch.cpp:61:5
    b = 1;
end

endmodule


