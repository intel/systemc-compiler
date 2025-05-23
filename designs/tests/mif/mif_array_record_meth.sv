//==============================================================================
//
// The code is generated by Intel Compiler for SystemC, version 1.6.14
// see more information at https://github.com/intel/systemc-compiler
//
//==============================================================================

//==============================================================================
//
// Module: Top ()
//
module Top // "top"
(
);

// Variables generated for SystemC signals
logic minst_s[2];
logic signed [31:0] minst_t0[2];
logic signed [31:0] minst_t1[2];

//------------------------------------------------------------------------------
// Method process: minst_memRecMeth (test_array_record_meth.cpp:70:5) 

// Process-local variables
logic [3:0] minst_r_b[2][3];
logic signed [31:0] minst_r_rec_c[2];
logic minst_rr_a[2];
logic [3:0] minst_rr_b[2][3];
logic signed [31:0] minst_rr_rec_arr_c[2][2];
logic signed [31:0] minst_f_rec_arr_c[2][2];
logic signed [31:0] minst_f_rec_oth_arr_c[2][2];

// Modular interfaces array generate block
generate 
for (genvar sct_i = 0; sct_i != 2; sct_i = sct_i + 1) 
begin : minst_memRecMeth_sct_i 

    always_comb 
    begin : minst_memRecMeth     // test_array_record_meth.cpp:70:5
        integer i;
        minst_r_b[sct_i][0] = 1;
        minst_r_rec_c[sct_i] = 2;
        minst_rr_a[sct_i] = minst_s[sct_i];
        minst_rr_b[sct_i][2] = 3;
        minst_rr_rec_arr_c[sct_i][1] = 4;
        minst_f_rec_arr_c[sct_i][1] = 5;
        minst_f_rec_oth_arr_c[sct_i][1] = 6;
        i = minst_f_rec_arr_c[sct_i][0] + minst_rr_rec_arr_c[sct_i][0] + minst_r_b[sct_i][1];
        minst_t0[sct_i] = i;
    end

end
endgenerate

//------------------------------------------------------------------------------
// Method process: minst_memRecArrMeth (test_array_record_meth.cpp:91:5) 

// Process-local variables
logic minst_w_a[2][2];
logic [3:0] minst_w_b[2][2][3];
logic signed [31:0] minst_w_rec_c[2][2];
logic signed [31:0] minst_w_rec_arr_c[2][2][2];
logic signed [31:0] minst_ww_rec_arr_c[2][3][2];
logic signed [31:0] minst_ww_rec_oth_arr_c[2][3][2];

// Modular interfaces array generate block
generate 
for (genvar sct_i = 0; sct_i != 2; sct_i = sct_i + 1) 
begin : minst_memRecArrMeth_sct_i 

    always_comb 
    begin : minst_memRecArrMeth     // test_array_record_meth.cpp:91:5
        integer sum;
        logic [3:0] x;
        minst_w_a[sct_i][0] = minst_s[sct_i];
        minst_w_b[sct_i][0][1] = 10;
        minst_w_rec_c[sct_i][1] = 20;
        minst_w_rec_arr_c[sct_i][0][1] = 30;
        sum = 0;
        for (integer i = 0; i < 2; ++i)
        begin
            minst_ww_rec_arr_c[sct_i][0][i] = i;
            sum = sum + minst_ww_rec_oth_arr_c[sct_i][0][i];
        end
        x = minst_w_rec_arr_c[sct_i][0][1] + minst_ww_rec_oth_arr_c[sct_i][0][0];
        minst_t1[sct_i] = x + sum;
    end

end
endgenerate

endmodule


