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
    input logic clk
);


//------------------------------------------------------------------------------
// Child module instances

A a_mod
(
  .clk(clk)
);

endmodule



//==============================================================================
//
// Module: A (test_array_method.cpp:322:5)
//
module A // "b_mod.a_mod"
(
    input logic clk
);

// Variables generated for SystemC signals
logic sig;
logic signed [31:0] t0;
logic signed [31:0] t1;
logic signed [31:0] t2;
logic signed [31:0] t3;
logic signed [31:0] t4;
logic signed [31:0] t5;
logic signed [31:0] t6;
logic signed [31:0] t7;
logic signed [31:0] t8;

//------------------------------------------------------------------------------
// Method process: rec_loc_arr0 (test_array_method.cpp:94:5) 

always_comb 
begin : rec_loc_arr0     // test_array_method.cpp:94:5
    logic ap_a[2];
    integer ap_b_1[2];
    integer i;
    t0 = 0;
    ap_a[0] = 1;
    ap_b_1[1] = 12;
    ap_b_1[0] = ap_b_1[1] + 1;
    if (ap_a[1])
    begin
        i = ap_b_1[0] >>> ap_b_1[1];
        t0 = i;
    end
end

//------------------------------------------------------------------------------
// Method process: rec_loc_arr1 (test_array_method.cpp:112:5) 

always_comb 
begin : rec_loc_arr1     // test_array_method.cpp:112:5
    logic ar_a[2];
    integer ar_b[2];
    integer i;
    i = sig;
    ar_a[i] = 0;
    ar_b[i + 1] = ar_b[i] - 1;
end

//------------------------------------------------------------------------------
// Method process: rec_loc_arr2 (test_array_method.cpp:122:5) 

always_comb 
begin : rec_loc_arr2     // test_array_method.cpp:122:5
    logic ar_a[2];
    integer ar_b[2];
    logic arr_a[2][3];
    integer arr_b[2][3];
    integer c;
    ar_a[1] = 0;
    arr_a[1][2] = !ar_a[1];
    c = arr_b[1][0] + ar_b[0];
    t1 = c;
end

//------------------------------------------------------------------------------
// Method process: rec_loc_arr3 (test_array_method.cpp:143:5) 

always_comb 
begin : rec_loc_arr3     // test_array_method.cpp:143:5
    logic ar_a[2];
    integer ar_b[2];
    integer k;
    for (integer i = 0; i < 2; i++)
    begin
        ar_a[i] = |(i % 2);
        ar_b[i] = i;
    end
    k = 0;
    for (integer i_1 = 0; i_1 < 2; i_1++)
    begin
        k = k + (ar_a[i_1] ? ar_b[i_1] : 0);
    end
    t2 = k;
end

//------------------------------------------------------------------------------
// Method process: rec_arr_elem_assign (test_array_method.cpp:157:5) 

always_comb 
begin : rec_arr_elem_assign     // test_array_method.cpp:157:5
    logic ar_a[3];
    integer ar_b[3];
    logic br_a[2];
    integer br_b[2];
    ar_a[1] = ar_a[2]; ar_b[1] = ar_b[2];
    br_a[1] = ar_a[0]; br_b[1] = ar_b[0];
end

//------------------------------------------------------------------------------
// Method process: rec_arr_elem_const_val1 (test_array_method.cpp:175:5) 

always_comb 
begin : rec_arr_elem_const_val1     // test_array_method.cpp:175:5
    logic sr_a;
    integer sr_b;
    logic par_a;
    integer par_b;
    integer i;
    sr_b = sig;
    par_a = sr_a; par_b = sr_b;
    // Call ff1() begin
    i = par_a + par_b;
    t3 = i;
    // Call ff1() end
end

//------------------------------------------------------------------------------
// Method process: rec_arr_elem_const_val2 (test_array_method.cpp:182:5) 

always_comb 
begin : rec_arr_elem_const_val2     // test_array_method.cpp:182:5
    logic sr_a;
    integer sr_b;
    logic par_a;
    integer par_b;
    integer i;
    par_a = sr_a; par_b = sr_b;
    // Call ff1() begin
    i = par_a + par_b;
    t3 = i;
    // Call ff1() end
end

//------------------------------------------------------------------------------
// Method process: rec_arr_elem_const_val3 (test_array_method.cpp:188:5) 

always_comb 
begin : rec_arr_elem_const_val3     // test_array_method.cpp:188:5
    logic crra_a[3];
    integer crra_b[3];
    logic par_a;
    integer par_b;
    integer i;
    par_a = crra_a[1]; par_b = crra_b[1];
    // Call ff1() begin
    i = par_a + par_b;
    t3 = i;
    // Call ff1() end
end

//------------------------------------------------------------------------------
// Method process: rec_arr_elem_const_val4 (test_array_method.cpp:194:5) 

always_comb 
begin : rec_arr_elem_const_val4     // test_array_method.cpp:194:5
    logic crrb_a[3];
    integer crrb_b[3];
    integer i;
    logic par_a;
    integer par_b;
    integer i_1;
    i = sig;
    par_a = crrb_a[i]; par_b = crrb_b[i];
    // Call ff1() begin
    i_1 = par_a + par_b;
    t3 = i_1;
    // Call ff1() end
end

//------------------------------------------------------------------------------
// Method process: rec_arr_elem_func_param_val (test_array_method.cpp:228:5) 

always_comb 
begin : rec_arr_elem_func_param_val     // test_array_method.cpp:228:5
    logic ar_a[3];
    integer ar_b[3];
    logic par_a;
    integer par_b;
    integer i;
    par_a = ar_a[1]; par_b = ar_b[1];
    // Call f1() begin
    i = par_b;
    t4 = i;
    // Call f1() end
end

//------------------------------------------------------------------------------
// Method process: rec_arr_elem_func_param_val2 (test_array_method.cpp:234:5) 

always_comb 
begin : rec_arr_elem_func_param_val2     // test_array_method.cpp:234:5
    logic ar_a[3][2];
    integer ar_b[3][2];
    logic par_a;
    integer par_b;
    integer i;
    par_a = ar_a[2][1]; par_b = ar_b[2][1];
    // Call f1() begin
    i = par_b;
    t4 = i;
    // Call f1() end
end

//------------------------------------------------------------------------------
// Method process: rec_arr_elem_func_param_ref (test_array_method.cpp:240:5) 

always_comb 
begin : rec_arr_elem_func_param_ref     // test_array_method.cpp:240:5
    logic ar_a[3];
    integer ar_b[3];
    integer i;
    // Call f2() begin
    i = ar_b[1];
    t5 = i;
    // Call f2() end
end

//------------------------------------------------------------------------------
// Method process: rec_arr_elem_func_param_ref2 (test_array_method.cpp:246:5) 

always_comb 
begin : rec_arr_elem_func_param_ref2     // test_array_method.cpp:246:5
    logic ar_a[3][2];
    integer ar_b[3][2];
    integer i;
    // Call f2() begin
    i = ar_b[2][1];
    t5 = i;
    // Call f2() end
end

//------------------------------------------------------------------------------
// Method process: rec_arr_elem_field_func_param_val (test_array_method.cpp:252:5) 

always_comb 
begin : rec_arr_elem_field_func_param_val     // test_array_method.cpp:252:5
    logic ar_a[3];
    integer ar_b[3];
    integer par1;
    logic par2;
    integer i;
    par1 = ar_b[2]; par2 = ar_a[1];
    // Call f3() begin
    i = par2 ? par1 : 1;
    t6 = i;
    // Call f3() end
end

//------------------------------------------------------------------------------
// Method process: rec_arr_elem_field_func_param_ref (test_array_method.cpp:258:5) 

always_comb 
begin : rec_arr_elem_field_func_param_ref     // test_array_method.cpp:258:5
    logic ar_a[3];
    integer ar_b[3];
    integer i;
    // Call f4() begin
    i = ar_a[1] ? ar_b[2] : 1;
    t7 = i;
    // Call f4() end
end

//------------------------------------------------------------------------------
// Method process: rec_arr_func_param (test_array_method.cpp:309:5) 

always_comb 
begin : rec_arr_func_param     // test_array_method.cpp:309:5
    logic ar_a[2];
    integer ar_b[2];
    ar_b[0] = 0;
end

//------------------------------------------------------------------------------
// Method process: rec_arr_elem_func_param_cref1 (test_array_method.cpp:272:5) 

always_comb 
begin : rec_arr_elem_func_param_cref1     // test_array_method.cpp:272:5
    integer indx;
    logic cvr_a[3];
    integer cvr_b[3];
    integer res;
    logic cwr_a[3];
    integer cwr_b[3];
    indx = 0;
    // Call cref_sum() begin
    res = cvr_a[1] + cvr_b[1];
    t8 = res;
    // Call cref_sum() end
    indx = sig;
    // Call cref_sum() begin
    res = cwr_a[indx] + cwr_b[indx];
    t8 = res;
    // Call cref_sum() end
end

//------------------------------------------------------------------------------
// Method process: rec_arr_elem_func_param_cref2 (test_array_method.cpp:283:5) 

always_comb 
begin : rec_arr_elem_func_param_cref2     // test_array_method.cpp:283:5
    logic cvrr_a[3];
    integer cvrr_b[3];
    integer res;
    t8 = 0;
    if (sig)
    begin
        // Call cref_sum() begin
        res = cvrr_a[2] + cvrr_b[2];
        t8 = res;
        // Call cref_sum() end
    end
end

//------------------------------------------------------------------------------
// Method process: rec_arr_elem_func_param_cref3 (test_array_method.cpp:293:5) 

always_comb 
begin : rec_arr_elem_func_param_cref3     // test_array_method.cpp:293:5
    integer indx;
    logic cwrr_a[3];
    integer cwrr_b[3];
    integer res;
    indx = 0;
    indx = sig;
    // Call cref_sum() begin
    res = cwrr_a[indx] + cwrr_b[indx];
    t8 = res;
    // Call cref_sum() end
end

endmodule


