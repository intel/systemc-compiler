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


//------------------------------------------------------------------------------
// Child module instances

A a_mod
(

);

endmodule



//==============================================================================
//
// Module: A (test_switch_enum.cpp:77:5)
//
module A // "b_mod.a_mod"
(
);

// Variables generated for SystemC signals
logic [31:0] s;

//------------------------------------------------------------------------------
// Method process: switch_enum1 (test_switch_enum.cpp:28:5) 

always_comb 
begin : switch_enum1     // test_switch_enum.cpp:28:5
    integer a;
    integer b;
    a = s;
    case (a)
    2 : begin
        b = 1;
    end
    4 : begin
        b = 2;
    end
    endcase
    b = 0;
end

//------------------------------------------------------------------------------
// Method process: switch_enum2 (test_switch_enum.cpp:47:5) 

always_comb 
begin : switch_enum2     // test_switch_enum.cpp:47:5
    logic [1:0] a;
    integer b;
    a = 2'(s);
    case (a)
    0 : begin
        b = 1;
    end
    1 : begin
        b = 2;
    end
    2 : begin
        b = 3;
    end
    endcase
    b = 0;
end

endmodule


