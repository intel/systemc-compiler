//==============================================================================
//
// The code is generated by Intel Compiler for SystemC, version 1.6.14
// see more information at https://github.com/intel/systemc-compiler
//
//==============================================================================

//==============================================================================
//
// Module: MyModule ()
//
module MyModule // "top_mod"
(
    input logic clk
);

// Variables generated for SystemC signals
logic nrst;
logic [7:0] s;
logic [7:0] t;
logic signed [31:0] rs_m;

//------------------------------------------------------------------------------
// Method process: zeroTypeProc (test_zero_width_func.cpp:111:5) 

always_comb 
begin : zeroTypeProc     // test_zero_width_func.cpp:111:5
    integer i;
    integer i_1;
    // Call f() begin
    i = 42;
    t = i;
    // Call f() end
    // Call g() begin
    i_1 = 43;
    // Call TRec() begin
    // Call TRec() end
    // Call g() end
end

//------------------------------------------------------------------------------
// Clocked THREAD: zeroTypeThrd (test_zero_width_func.cpp:118:5) 

// Thread-local variables
logic zeroTypeThrd_PROC_STATE;
logic zeroTypeThrd_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : zeroTypeThrd_comb     // test_zero_width_func.cpp:118:5
    zeroTypeThrd_func;
end
function void zeroTypeThrd_func;
    integer unsigned j;
    integer i;
    zeroTypeThrd_PROC_STATE_next = zeroTypeThrd_PROC_STATE;
    
    case (zeroTypeThrd_PROC_STATE)
        0: begin
            i = 0 + 0;
            zeroTypeThrd_PROC_STATE_next = 1; return;    // test_zero_width_func.cpp:128:13;
        end
        1: begin
            i = 0 + 0;
            i = 0 + 0;
            zeroTypeThrd_PROC_STATE_next = 1; return;    // test_zero_width_func.cpp:128:13;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge nrst) 
begin : zeroTypeThrd_ff
    if ( ~nrst ) begin
        integer unsigned j;
        j = 0 + 1;
        zeroTypeThrd_PROC_STATE <= 0;    // test_zero_width_func.cpp:122:9;
    end
    else begin
        zeroTypeThrd_PROC_STATE <= zeroTypeThrd_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Method process: recAssignProc (test_zero_width_func.cpp:137:5) 

always_comb 
begin : recAssignProc     // test_zero_width_func.cpp:137:5
    integer unsigned j;
    integer lr_m;
    integer larr_m[3];
    j = s;
    // Call Rec() begin
    // Call Rec() end
    larr_m[0] = 1;
    larr_m[1] = 2;
    larr_m[2] = 3;
    larr_m[j] = lr_m;
end

//------------------------------------------------------------------------------
// Method process: recRefProc (test_zero_width_func.cpp:157:5) 

always_comb 
begin : recRefProc     // test_zero_width_func.cpp:157:5
    integer unsigned j;
    integer larr_m[2];
    integer l_m;
    integer i;
    integer v_m;
    j = s;
    // Call Rec() begin
    // Call Rec() end
    l_m = larr_m[j];
    larr_m[j] = l_m;
    larr_m[j] = j;
    j = larr_m[j];
    j = larr_m[j] + l_m * larr_m[j];
    rs_m = larr_m[j];
    rs_m = larr_m[j];
    larr_m[j] = rs_m;
    larr_m[j] = rs_m;
    larr_m[j] = larr_m[j];
    larr_m[j + 1] = larr_m[j];
    // Call refArr() begin
    i = larr_m[j];
    v_m = larr_m[j];
    // Call refArr() end
    // Call refArr() begin
    i = larr_m[j];
    v_m = larr_m[j];
    // Call refArr() end
    // Call refArr() begin
    i = larr_m[j];
    v_m = larr_m[j];
    // Call refArr() end
end

//------------------------------------------------------------------------------
// Method process: recRefValue (test_zero_width_func.cpp:184:5) 

always_comb 
begin : recRefValue     // test_zero_width_func.cpp:184:5
    integer unsigned j;
    integer larr_m[2];
    integer l_m;
    j = s;
    // Call Rec() begin
    // Call Rec() end
    l_m = 42;
    l_m = larr_m[j];
end

//------------------------------------------------------------------------------
// Method process: recConstRefProc (test_zero_width_func.cpp:204:5) 

always_comb 
begin : recConstRefProc     // test_zero_width_func.cpp:204:5
    integer unsigned j;
    integer larr_m[2];
    integer l_m;
    integer i;
    integer v_m;
    j = s;
    // Call Rec() begin
    // Call Rec() end
    l_m = larr_m[j];
    j = larr_m[j];
    j = larr_m[j] + l_m * larr_m[j];
    rs_m = larr_m[j];
    rs_m = larr_m[j];
    // Call refConstArr() begin
    i = larr_m[j] + 1;
    v_m = larr_m[j];
    // Call refConstArr() end
    // Call refConstArr() begin
    i = larr_m[j] + 1;
    v_m = larr_m[j];
    // Call refConstArr() end
end

//------------------------------------------------------------------------------
// Method process: valParamProc (test_zero_width_func.cpp:242:5) 

always_comb 
begin : valParamProc     // test_zero_width_func.cpp:242:5
    integer unsigned k;
    integer a;
    integer par;
    integer l;
    integer i;
    logic [2:0] j;
    k = s;
    a = 11;
    par = a;
    // Call valParam() begin
    l = par;
    par = 1;
    // Call valParam() end
    par = 0;
    // Call valParam() begin
    l = par;
    par = 1;
    // Call valParam() end
    // Call valParamZero() begin
    i = 42 + 0;
    // Call valParamZero() end
    // Call valParamZero() begin
    i = 42 + 0;
    // Call valParamZero() end
    // Call valParamZero() begin
    i = 42 + 0;
    // Call valParamZero() end
    // Call valParamZero() begin
    i = 42 + 0;
    // Call valParamZero() end
    // Call valParamZero() begin
    i = 42 + 0;
    // Call valParamZero() end
    k = 0;
    j = 0;
    j = 0;
end

//------------------------------------------------------------------------------
// Method process: refParamProc (test_zero_width_func.cpp:279:5) 

always_comb 
begin : refParamProc     // test_zero_width_func.cpp:279:5
    integer unsigned k;
    integer a;
    integer i;
    integer j;
    k = s;
    a = 11;
    a = 42 + 0;
    a = 42 + 0;
    a = 42 + 0;
    // Call refParamZero() begin
    i = 42 + 0;
    // Call refParamZero() end
    // Call refParamZero() begin
    i = 42 + 0;
    // Call refParamZero() end
    // Call refParamZero() begin
    i = 42 + 0;
    // Call refParamZero() end
    // Call constRefParamZero() begin
    j = 42 + 0;
    // Call constRefParamZero() end
    // Call constRefParamZero() begin
    j = 42 + 0;
    // Call constRefParamZero() end
end

//------------------------------------------------------------------------------
// Method process: locRefProc (test_zero_width_func.cpp:304:5) 

always_comb 
begin : locRefProc     // test_zero_width_func.cpp:304:5
    integer unsigned k;
    integer l;
    k = s;
    l = 0;
    l = 0 + 1;
    l = 0 + 2;
end

//------------------------------------------------------------------------------
// Method process: someRefParamProc (test_zero_width_func.cpp:340:5) 

// Process-local variables
logic [31:0] mr_mu;

always_comb 
begin : someRefParamProc     // test_zero_width_func.cpp:340:5
    integer unsigned j;
    integer unsigned lr_mu;
    integer unsigned larr_mu[3];
    integer unsigned par1_mu;
    integer k;
    integer unsigned l_mu;
    integer unsigned par_mu;
    integer unsigned l_mu_1;
    integer unsigned TMP_0_mu;
    integer unsigned TMP_1_mu;
    integer unsigned TMP_2_mu;
    integer unsigned TMP_3_mu;
    integer unsigned TMP_4_mu;
    j = s;
    // Call Some() begin
    // Call Some() end
    par1_mu = lr_mu;
    // Call recParam() begin
    k = 0 + 0;
    l_mu = lr_mu;
    // Call recParam() end
    par1_mu = mr_mu;
    // Call recParam() begin
    k = 0 + 0;
    l_mu = larr_mu[j];
    // Call recParam() end
    par_mu = lr_mu;
    // Call recReturn() begin
    l_mu_1 = par_mu;
    TMP_0_mu = l_mu_1;
    // Call recReturn() end
    par_mu = mr_mu;
    // Call recReturn() begin
    l_mu_1 = par_mu;
    TMP_1_mu = l_mu_1;
    // Call recReturn() end
    par_mu = lr_mu;
    // Call recReturn() begin
    l_mu_1 = par_mu;
    TMP_2_mu = l_mu_1;
    // Call recReturn() end
    mr_mu = TMP_2_mu;
    par_mu = larr_mu[1];
    // Call recReturn() begin
    l_mu_1 = par_mu;
    TMP_3_mu = l_mu_1;
    // Call recReturn() end
    larr_mu[2] = TMP_3_mu;
    par_mu = mr_mu;
    // Call recReturn() begin
    l_mu_1 = par_mu;
    TMP_4_mu = l_mu_1;
    // Call recReturn() end
    larr_mu[j] = TMP_4_mu;
end

//------------------------------------------------------------------------------
// Method process: allRefParamProc (test_zero_width_func.cpp:355:5) 

always_comb 
begin : allRefParamProc     // test_zero_width_func.cpp:355:5
    integer unsigned j;
    integer k;
    j = s;
    // Call All() begin
    // Call All() end
    // Call recParam() begin
    k = 0 + 0;
    // Call recParam() end
    // Call recParam() begin
    k = 0 + 0;
    // Call recParam() end
    // Call recReturn() begin
    // Call recReturn() end
    // Call recReturn() begin
    // Call recReturn() end
end

//------------------------------------------------------------------------------
// Method process: recArrProc (test_zero_width_func.cpp:369:5) 

// Process-local variables
logic [31:0] sarr_mu[3];

always_comb 
begin : recArrProc     // test_zero_width_func.cpp:369:5
    integer unsigned j;
    integer unsigned lsarr_mu[3];
    integer unsigned par1_mu;
    integer k;
    integer unsigned l_mu;
    integer k_1;
    j = s;
    par1_mu = sarr_mu[1];
    // Call recParam() begin
    k = 0 + 0;
    l_mu = lsarr_mu[j];
    // Call recParam() end
    // Call recParam() begin
    k_1 = 0 + 0;
    // Call recParam() end
end

endmodule


