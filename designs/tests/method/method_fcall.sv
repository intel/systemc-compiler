//==============================================================================
//
// The code is generated by Intel Compiler for SystemC, version 1.6.8
// see more information at https://github.com/intel/systemc-compiler
//
//==============================================================================

//==============================================================================
//
// Module: A ()
//
module A // "a_mod"
(
    input logic a,
    output logic b,
    output logic c,
    output logic p,
    input logic [2:0] ip
);

// Variables generated for SystemC signals
logic [1:0] s1[2];
logic [2:0] sp;
logic [1:0] struct_c_out_port;
logic [1:0] struct_c2_out_port;

// Assignments generated for C++ channel arrays
assign s1[0] = struct_c_out_port;
assign s1[1] = struct_c2_out_port;

//------------------------------------------------------------------------------
// Method process: std_funcs (test_fcall.cpp:432:5) 

always_comb 
begin : std_funcs     // test_fcall.cpp:432:5
end

//------------------------------------------------------------------------------
// Method process: read_channel_in_cout (test_fcall.cpp:119:5) 

always_comb 
begin : read_channel_in_cout     // test_fcall.cpp:119:5
end

//------------------------------------------------------------------------------
// Method process: func_call_params (test_fcall.cpp:255:5) 

// Process-local variables
logic signed [31:0] m;

always_comb 
begin : func_call_params     // test_fcall.cpp:255:5
    integer j;
    integer arr_arg[3];
    logic [2:0] arr_arg2[2];
    // Call f1() begin
    m = m + 1;
    // Call f1() end
    j = 2;
    // Call f4() begin
    j = j + 1;
    // Call f4() end
    arr_arg[0] = 1; arr_arg[1] = 2; arr_arg[2] = 3;
    j = 6;    // Call of f5()
    arr_arg2[0] = 1; arr_arg2[1] = 2;
    j = 3;
end

//------------------------------------------------------------------------------
// Method process: child_record_call (test_fcall.cpp:280:5) 

always_comb 
begin : child_record_call     // test_fcall.cpp:280:5
    logic [2:0] x;
    logic [2:0] y;
    x = 1;
    y = 2;
end

//------------------------------------------------------------------------------
// Method process: port_access_call (test_fcall.cpp:299:5) 

always_comb 
begin : port_access_call     // test_fcall.cpp:299:5
    logic TMP_0;
    logic [1:0] val;
    // Call f6() begin
    b = !a;
    // Call f6() end
    // Call f7() begin
    c = a;
    TMP_0 = !a;
    // Call f7() end
    val = 3;
    // Call f3() begin
    struct_c_out_port = val;
    // Call f3() end
end

//------------------------------------------------------------------------------
// Method process: included_func_calls (test_fcall.cpp:312:5) 

always_comb 
begin : included_func_calls     // test_fcall.cpp:312:5
    logic [3:0] TMP_0;
    logic [2:0] val;
    logic TMP_3;
    integer unsigned TMP_4;
    integer unsigned val_1;
    val = 2;
    // Call f8() begin
    // Call f8_() begin
    TMP_3 = a;
    // Call f8_() end
    if (TMP_3)
    begin
        TMP_0 = 0;
    end else begin
        TMP_0 = a ? val + 1 : val - 1;
    end
    // Call f8() end
    val_1 = 2;
    // Call f9() begin
    // Call f9_() begin
    val_1++;
    // Call f9_() end
    // Call f9_() begin
    val_1++;
    // Call f9_() end
    TMP_4 = val_1;
    // Call f9() end
end

//------------------------------------------------------------------------------
// Method process: multiple_returns (test_fcall.cpp:321:5) 

always_comb 
begin : multiple_returns     // test_fcall.cpp:321:5
    integer j;
    j = 1;
    j = 2;    // Call of f11()
end

//------------------------------------------------------------------------------
// Method process: return_in_loop (test_fcall.cpp:331:5) 

always_comb 
begin : return_in_loop     // test_fcall.cpp:331:5
    integer unsigned TMP_0;
    integer unsigned val;
    for (integer i = 0; i < 3; i++)
    begin
        val = i;
        // Call f11() begin
        case (val)
        1 : begin
            TMP_0 = 1;
        end
        2 : begin
            TMP_0 = 2;
        end
        default : begin
            TMP_0 = 3;
        end
        endcase
        // Call f11() end
    end
end

//------------------------------------------------------------------------------
// Method process: glob_func_call (test_fcall.cpp:237:5) 

always_comb 
begin : glob_func_call     // test_fcall.cpp:237:5
    integer i;
    i = 1;    // Call of globFunc2()
end

//------------------------------------------------------------------------------
// Method process: static_func_call (test_fcall.cpp:246:5) 

always_comb 
begin : static_func_call     // test_fcall.cpp:246:5
    integer i;
    i = 1;    // Call of staticFunc2()
end

//------------------------------------------------------------------------------
// Method process: included_funcs1 (test_fcall.cpp:357:5) 

always_comb 
begin : included_funcs1     // test_fcall.cpp:357:5
end

//------------------------------------------------------------------------------
// Method process: included_funcs2 (test_fcall.cpp:366:5) 

always_comb 
begin : included_funcs2     // test_fcall.cpp:366:5
end

//------------------------------------------------------------------------------
// Method process: func_double_used (test_fcall.cpp:377:5) 

always_comb 
begin : func_double_used     // test_fcall.cpp:377:5
end

//------------------------------------------------------------------------------
// Method process: func_chan_pointer (test_fcall.cpp:394:5) 

always_comb 
begin : func_chan_pointer     // test_fcall.cpp:394:5
    integer TMP_0;
    integer i;
    integer TMP_1;
    integer i_1;
    integer TMP_2;
    integer i_2;
    integer TMP_3;
    integer i_3;
    // Call f_ch_ptr() begin
    i = sp;
    TMP_0 = i;
    // Call f_ch_ptr() end
    // Call f_ch_ptr2() begin
    i_1 = sp;
    TMP_1 = i_1;
    // Call f_ch_ptr2() end
    // Call f_ch_ptr() begin
    i_2 = p;
    TMP_2 = i_2;
    // Call f_ch_ptr() end
    // Call f_ch_ptr() begin
    i_3 = ip;
    TMP_3 = i_3;
    // Call f_ch_ptr() end
end

//------------------------------------------------------------------------------
// Method process: func_var_pointer (test_fcall.cpp:417:5) 

// Process-local variables
logic [2:0] s;

always_comb 
begin : func_var_pointer     // test_fcall.cpp:417:5
end

//------------------------------------------------------------------------------
// Method process: special_funcs (test_fcall.cpp:424:5) 

always_comb 
begin : special_funcs     // test_fcall.cpp:424:5
    integer y;
    y = 0;
end

endmodule


