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
    input logic clk
);

// Variables generated for SystemC signals
logic ar1_nrst[2];
logic signed [31:0] ar1_t0[2];
logic br1_nrst[2];
logic signed [31:0] br1_t1[2];
logic ar2_nrst[2];
logic signed [31:0] ar2_t0[2];
logic br2_nrst[2];
logic signed [31:0] br2_t1[2];
logic ar3_nrst[2];
logic signed [31:0] ar3_t0[2];
logic br3_nrst[2];
logic signed [31:0] br3_t1[2];
logic ar1_clk[2];
logic br1_clk[2];
logic ar2_clk[2];
logic br2_clk[2];
logic ar3_clk[2];
logic br3_clk[2];

// Local parameters generated for C++ constants
localparam logic [31:0] ar1_V[2] = '{ 1, 2 };
localparam logic [31:0] ar1_C[2] = '{ 1, 2 };
localparam logic [31:0] br1_V[2] = '{ 1, 2 };
localparam logic [31:0] ar2_V[2] = '{ 1, 2 };
localparam logic [31:0] ar2_C[2] = '{ 1, 2 };
localparam logic [31:0] br2_V[2] = '{ 1, 2 };
localparam logic [31:0] ar3_V[2] = '{ 1, 2 };
localparam logic [31:0] ar3_C[2] = '{ 0, 0 };
localparam logic [31:0] br3_V[2] = '{ 1, 2 };

// Assignments generated for C++ channel arrays
assign ar1_clk[0] = clk;
assign ar1_clk[1] = clk;
assign br1_clk[0] = clk;
assign br1_clk[1] = clk;
assign ar2_clk[0] = clk;
assign ar2_clk[1] = clk;
assign br2_clk[0] = clk;
assign br2_clk[1] = clk;
assign ar3_clk[0] = clk;
assign ar3_clk[1] = clk;
assign br3_clk[0] = clk;
assign br3_clk[1] = clk;

//------------------------------------------------------------------------------
// Clocked THREAD: ar1_proc (test_mif_array_differ.cpp:34:5) 

// Thread-local variables
logic signed [31:0] ar1_t0_next[2];

// Modular interfaces array generate block
generate 
for (genvar sct_i = 0; sct_i != 2; sct_i = sct_i + 1) 
begin : ar1_proc_sct_i 

    // Next-state combinational logic
    always_comb begin : ar1_proc_comb     // test_mif_array_differ.cpp:34:5
        ar1_proc_func;
    end
    function void ar1_proc_func;
        integer unsigned l;
        ar1_t0_next[sct_i] = ar1_t0[sct_i];
        l = ar1_V[sct_i] + ar1_C[sct_i];
        ar1_t0_next[sct_i] = l;
    endfunction

    // Synchronous register update
    always_ff @(posedge ar1_clk[sct_i] or negedge ar1_nrst[sct_i]) 
    begin : ar1_proc_ff
        if ( ~ar1_nrst[sct_i] ) begin
            integer unsigned l;
            l = ar1_V[sct_i];
        end
        else begin
            ar1_t0[sct_i] <= ar1_t0_next[sct_i];
        end
    end

end
endgenerate

//------------------------------------------------------------------------------
// Clocked THREAD: br1_proc (test_mif_array_differ.cpp:63:5) 

// Thread-local variables
logic signed [31:0] br1_t1_next[2];
logic proc_PROC_STATE[2];
logic proc_PROC_STATE_next[2];

// Modular interfaces array generate block
generate 
for (genvar sct_i = 0; sct_i != 2; sct_i = sct_i + 1) 
begin : br1_proc_sct_i 

    // Next-state combinational logic
    always_comb begin : br1_proc_comb     // test_mif_array_differ.cpp:63:5
        br1_proc_func;
    end
    function void br1_proc_func;
        integer unsigned l;
        br1_t1_next[sct_i] = br1_t1[sct_i];
        proc_PROC_STATE_next[sct_i] = proc_PROC_STATE[sct_i];
    
        case (proc_PROC_STATE[sct_i])
            0: begin
                l = br1_V[sct_i];
                proc_PROC_STATE_next[sct_i] = 1; return;    // test_mif_array_differ.cpp:68:13;
            end
            1: begin
                l = br1_V[sct_i] + 1;
                br1_t1_next[sct_i] = l;
                l = br1_V[sct_i];
                proc_PROC_STATE_next[sct_i] = 1; return;    // test_mif_array_differ.cpp:68:13;
            end
        endcase
    endfunction

    // Synchronous register update
    always_ff @(posedge br1_clk[sct_i] or negedge br1_nrst[sct_i]) 
    begin : br1_proc_ff
        if ( ~br1_nrst[sct_i] ) begin
            proc_PROC_STATE[sct_i] <= 0;    // test_mif_array_differ.cpp:65:9;
        end
        else begin
            br1_t1[sct_i] <= br1_t1_next[sct_i];
            proc_PROC_STATE[sct_i] <= proc_PROC_STATE_next[sct_i];
        end
    end

end
endgenerate

//------------------------------------------------------------------------------
// Clocked THREAD: ar2_proc (test_mif_array_differ.cpp:34:5) 

// Thread-local variables
logic signed [31:0] ar2_t0_next[2];

// Modular interfaces array generate block
generate 
for (genvar sct_i = 0; sct_i != 2; sct_i = sct_i + 1) 
begin : ar2_proc_sct_i 

    // Next-state combinational logic
    always_comb begin : ar2_proc_comb     // test_mif_array_differ.cpp:34:5
        ar2_proc_func;
    end
    function void ar2_proc_func;
        integer unsigned l;
        ar2_t0_next[sct_i] = ar2_t0[sct_i];
        l = ar2_V[sct_i] + ar2_C[sct_i];
        ar2_t0_next[sct_i] = l;
    endfunction

    // Synchronous register update
    always_ff @(posedge ar2_clk[sct_i] or negedge ar2_nrst[sct_i]) 
    begin : ar2_proc_ff
        if ( ~ar2_nrst[sct_i] ) begin
            integer unsigned l;
            l = ar2_V[sct_i];
        end
        else begin
            ar2_t0[sct_i] <= ar2_t0_next[sct_i];
        end
    end

end
endgenerate

//------------------------------------------------------------------------------
// Clocked THREAD: br2_proc (test_mif_array_differ.cpp:63:5) 

// Thread-local variables
logic signed [31:0] br2_t1_next[2];
logic proc_PROC_STATE0[2];
logic proc_PROC_STATE_next0[2];

// Modular interfaces array generate block
generate 
for (genvar sct_i = 0; sct_i != 2; sct_i = sct_i + 1) 
begin : br2_proc_sct_i 

    // Next-state combinational logic
    always_comb begin : br2_proc_comb     // test_mif_array_differ.cpp:63:5
        br2_proc_func;
    end
    function void br2_proc_func;
        integer unsigned l;
        br2_t1_next[sct_i] = br2_t1[sct_i];
        proc_PROC_STATE_next0[sct_i] = proc_PROC_STATE0[sct_i];
    
        case (proc_PROC_STATE0[sct_i])
            0: begin
                l = br2_V[sct_i];
                proc_PROC_STATE_next0[sct_i] = 1; return;    // test_mif_array_differ.cpp:68:13;
            end
            1: begin
                l = br2_V[sct_i] + 1;
                br2_t1_next[sct_i] = l;
                l = br2_V[sct_i];
                proc_PROC_STATE_next0[sct_i] = 1; return;    // test_mif_array_differ.cpp:68:13;
            end
        endcase
    endfunction

    // Synchronous register update
    always_ff @(posedge br2_clk[sct_i] or negedge br2_nrst[sct_i]) 
    begin : br2_proc_ff
        if ( ~br2_nrst[sct_i] ) begin
            proc_PROC_STATE0[sct_i] <= 0;    // test_mif_array_differ.cpp:65:9;
        end
        else begin
            br2_t1[sct_i] <= br2_t1_next[sct_i];
            proc_PROC_STATE0[sct_i] <= proc_PROC_STATE_next0[sct_i];
        end
    end

end
endgenerate

//------------------------------------------------------------------------------
// Clocked THREAD: ar3_proc (test_mif_array_differ.cpp:34:5) 

// Thread-local variables
logic signed [31:0] ar3_t0_next[2];

// Modular interfaces array generate block
generate 
for (genvar sct_i = 0; sct_i != 2; sct_i = sct_i + 1) 
begin : ar3_proc_sct_i 

    // Next-state combinational logic
    always_comb begin : ar3_proc_comb     // test_mif_array_differ.cpp:34:5
        ar3_proc_func;
    end
    function void ar3_proc_func;
        integer unsigned l;
        ar3_t0_next[sct_i] = ar3_t0[sct_i];
        l = ar3_V[sct_i] + ar3_C[sct_i];
        ar3_t0_next[sct_i] = l;
    endfunction

    // Synchronous register update
    always_ff @(posedge ar3_clk[sct_i] or negedge ar3_nrst[sct_i]) 
    begin : ar3_proc_ff
        if ( ~ar3_nrst[sct_i] ) begin
            integer unsigned l;
            l = ar3_V[sct_i];
        end
        else begin
            ar3_t0[sct_i] <= ar3_t0_next[sct_i];
        end
    end

end
endgenerate

//------------------------------------------------------------------------------
// Clocked THREAD: br3_proc (test_mif_array_differ.cpp:63:5) 

// Thread-local variables
logic signed [31:0] br3_t1_next[2];
logic proc_PROC_STATE1[2];
logic proc_PROC_STATE_next1[2];

// Modular interfaces array generate block
generate 
for (genvar sct_i = 0; sct_i != 2; sct_i = sct_i + 1) 
begin : br3_proc_sct_i 

    // Next-state combinational logic
    always_comb begin : br3_proc_comb     // test_mif_array_differ.cpp:63:5
        br3_proc_func;
    end
    function void br3_proc_func;
        integer unsigned l;
        br3_t1_next[sct_i] = br3_t1[sct_i];
        proc_PROC_STATE_next1[sct_i] = proc_PROC_STATE1[sct_i];
    
        case (proc_PROC_STATE1[sct_i])
            0: begin
                l = br3_V[sct_i];
                proc_PROC_STATE_next1[sct_i] = 1; return;    // test_mif_array_differ.cpp:68:13;
            end
            1: begin
                l = br3_V[sct_i] + 1;
                br3_t1_next[sct_i] = l;
                l = br3_V[sct_i];
                proc_PROC_STATE_next1[sct_i] = 1; return;    // test_mif_array_differ.cpp:68:13;
            end
        endcase
    endfunction

    // Synchronous register update
    always_ff @(posedge br3_clk[sct_i] or negedge br3_nrst[sct_i]) 
    begin : br3_proc_ff
        if ( ~br3_nrst[sct_i] ) begin
            proc_PROC_STATE1[sct_i] <= 0;    // test_mif_array_differ.cpp:65:9;
        end
        else begin
            br3_t1[sct_i] <= br3_t1_next[sct_i];
            proc_PROC_STATE1[sct_i] <= proc_PROC_STATE_next1[sct_i];
        end
    end

end
endgenerate

endmodule


