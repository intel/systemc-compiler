//==============================================================================
//
// The code is generated by Intel Compiler for SystemC, version 1.4.18
// see more information at https://github.com/intel/systemc-compiler
//
//==============================================================================

//==============================================================================
//
// Module: test_reset ()
//
module test_reset // "t_inst"
(
    input logic clk,
    input logic a0reset,
    input logic a1reset,
    input logic a2reset,
    input logic s0reset,
    input logic s1reset,
    input logic s2reset,
    input logic [2:0] a_reg_in,
    input logic [2:0] s_reg_in,
    output logic [2:0] data0_reg_o,
    output logic [2:0] data1_reg_o,
    output logic [2:0] data2_reg_o,
    output logic [2:0] data3_reg_o,
    output logic [2:0] data4_reg_o,
    output logic [2:0] data5_reg_o,
    output logic [2:0] data6_reg_o,
    output logic [2:0] data7_reg_o,
    output logic [2:0] data8_reg_o,
    output logic [2:0] data9_reg_o,
    output logic [2:0] data10_reg_o,
    output logic [2:0] data11_reg_o,
    output logic [2:0] data12_reg_o,
    output logic [2:0] data13_reg_o
);

//------------------------------------------------------------------------------
// Clocked THREAD: async_rst (test_cthread_sync_async_reset.cpp:113:5) 

// Thread-local variables
logic [2:0] data0_reg_o_next;

// Next-state combinational logic
always_comb begin : async_rst_comb     // test_cthread_sync_async_reset.cpp:113:5
    async_rst_func;
end
function void async_rst_func;
    data0_reg_o_next = data0_reg_o;
    data0_reg_o_next = a_reg_in;
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge a0reset) 
begin : async_rst_ff
    if ( ~a0reset ) begin
        data0_reg_o <= 0;
    end
    else begin
        data0_reg_o <= data0_reg_o_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: async_rst_active_high (test_cthread_sync_async_reset.cpp:124:5) 

// Thread-local variables
logic [2:0] data1_reg_o_next;

// Next-state combinational logic
always_comb begin : async_rst_active_high_comb     // test_cthread_sync_async_reset.cpp:124:5
    async_rst_active_high_func;
end
function void async_rst_active_high_func;
    data1_reg_o_next = data1_reg_o;
    data1_reg_o_next = a_reg_in;
endfunction

// Synchronous register update
always_ff @(posedge clk or posedge a0reset) 
begin : async_rst_active_high_ff
    if ( a0reset ) begin
        data1_reg_o <= 0;
    end
    else begin
        data1_reg_o <= data1_reg_o_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: sync_rst (test_cthread_sync_async_reset.cpp:135:5) 

// Thread-local variables
logic [2:0] data2_reg_o_next;

// Next-state combinational logic
always_comb begin : sync_rst_comb     // test_cthread_sync_async_reset.cpp:135:5
    sync_rst_func;
end
function void sync_rst_func;
    data2_reg_o_next = data2_reg_o;
    data2_reg_o_next = a_reg_in;
endfunction

// Synchronous register update
always_ff @(posedge clk /*sync s0reset*/) 
begin : sync_rst_ff
    if ( s0reset ) begin
        data2_reg_o <= 0;
    end
    else begin
        data2_reg_o <= data2_reg_o_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: sync_rst_active_low (test_cthread_sync_async_reset.cpp:146:5) 

// Thread-local variables
logic [2:0] data3_reg_o_next;

// Next-state combinational logic
always_comb begin : sync_rst_active_low_comb     // test_cthread_sync_async_reset.cpp:146:5
    sync_rst_active_low_func;
end
function void sync_rst_active_low_func;
    data3_reg_o_next = data3_reg_o;
    data3_reg_o_next = a_reg_in;
endfunction

// Synchronous register update
always_ff @(posedge clk /*sync s0reset*/) 
begin : sync_rst_active_low_ff
    if ( ~s0reset ) begin
        data3_reg_o <= 0;
    end
    else begin
        data3_reg_o <= data3_reg_o_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: sync_rst_multi (test_cthread_sync_async_reset.cpp:157:5) 

// Thread-local variables
logic [2:0] data4_reg_o_next;

// Next-state combinational logic
always_comb begin : sync_rst_multi_comb     // test_cthread_sync_async_reset.cpp:157:5
    sync_rst_multi_func;
end
function void sync_rst_multi_func;
    data4_reg_o_next = data4_reg_o;
    data4_reg_o_next = a_reg_in;
endfunction

// Synchronous register update
always_ff @(posedge clk /*sync s1reset*/ /*sync s0reset*/) 
begin : sync_rst_multi_ff
    if ( s1reset || s0reset ) begin
        data4_reg_o <= 0;
    end
    else begin
        data4_reg_o <= data4_reg_o_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: async_rst_multi (test_cthread_sync_async_reset.cpp:168:5) 

// Thread-local variables
logic [2:0] data5_reg_o_next;

// Next-state combinational logic
always_comb begin : async_rst_multi_comb     // test_cthread_sync_async_reset.cpp:168:5
    async_rst_multi_func;
end
function void async_rst_multi_func;
    data5_reg_o_next = data5_reg_o;
    data5_reg_o_next = a_reg_in;
endfunction

// Synchronous register update
always_ff @(posedge clk or posedge a1reset or posedge a0reset) 
begin : async_rst_multi_ff
    if ( a1reset || a0reset ) begin
        data5_reg_o <= 0;
    end
    else begin
        data5_reg_o <= data5_reg_o_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: sync_rst_multi_active_low (test_cthread_sync_async_reset.cpp:179:5) 

// Thread-local variables
logic [2:0] data6_reg_o_next;

// Next-state combinational logic
always_comb begin : sync_rst_multi_active_low_comb     // test_cthread_sync_async_reset.cpp:179:5
    sync_rst_multi_active_low_func;
end
function void sync_rst_multi_active_low_func;
    data6_reg_o_next = data6_reg_o;
    data6_reg_o_next = a_reg_in;
endfunction

// Synchronous register update
always_ff @(posedge clk /*sync s1reset*/ /*sync s0reset*/) 
begin : sync_rst_multi_active_low_ff
    if ( ~s1reset || ~s0reset ) begin
        data6_reg_o <= 0;
    end
    else begin
        data6_reg_o <= data6_reg_o_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: async_rst_multi_active_low (test_cthread_sync_async_reset.cpp:190:5) 

// Thread-local variables
logic [2:0] data7_reg_o_next;

// Next-state combinational logic
always_comb begin : async_rst_multi_active_low_comb     // test_cthread_sync_async_reset.cpp:190:5
    async_rst_multi_active_low_func;
end
function void async_rst_multi_active_low_func;
    data7_reg_o_next = data7_reg_o;
    data7_reg_o_next = a_reg_in;
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge a1reset or negedge a0reset) 
begin : async_rst_multi_active_low_ff
    if ( ~a1reset || ~a0reset ) begin
        data7_reg_o <= 0;
    end
    else begin
        data7_reg_o <= data7_reg_o_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: sync_rst_multi_active_low_high (test_cthread_sync_async_reset.cpp:201:5) 

// Thread-local variables
logic [2:0] data8_reg_o_next;

// Next-state combinational logic
always_comb begin : sync_rst_multi_active_low_high_comb     // test_cthread_sync_async_reset.cpp:201:5
    sync_rst_multi_active_low_high_func;
end
function void sync_rst_multi_active_low_high_func;
    data8_reg_o_next = data8_reg_o;
    data8_reg_o_next = a_reg_in;
endfunction

// Synchronous register update
always_ff @(posedge clk /*sync s1reset*/ /*sync s0reset*/) 
begin : sync_rst_multi_active_low_high_ff
    if ( s1reset || ~s0reset ) begin
        data8_reg_o <= 0;
    end
    else begin
        data8_reg_o <= data8_reg_o_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: sync_rst_multi_active_high_low (test_cthread_sync_async_reset.cpp:212:5) 

// Thread-local variables
logic [2:0] data9_reg_o_next;

// Next-state combinational logic
always_comb begin : sync_rst_multi_active_high_low_comb     // test_cthread_sync_async_reset.cpp:212:5
    sync_rst_multi_active_high_low_func;
end
function void sync_rst_multi_active_high_low_func;
    data9_reg_o_next = data9_reg_o;
    data9_reg_o_next = a_reg_in;
endfunction

// Synchronous register update
always_ff @(posedge clk /*sync s1reset*/ /*sync s0reset*/) 
begin : sync_rst_multi_active_high_low_ff
    if ( ~s1reset || s0reset ) begin
        data9_reg_o <= 0;
    end
    else begin
        data9_reg_o <= data9_reg_o_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: sync_rst_multi_async_single_sync (test_cthread_sync_async_reset.cpp:223:5) 

// Thread-local variables
logic [2:0] data10_reg_o_next;

// Next-state combinational logic
always_comb begin : sync_rst_multi_async_single_sync_comb     // test_cthread_sync_async_reset.cpp:223:5
    sync_rst_multi_async_single_sync_func;
end
function void sync_rst_multi_async_single_sync_func;
    data10_reg_o_next = data10_reg_o;
    data10_reg_o_next = a_reg_in;
endfunction

// Synchronous register update
always_ff @(posedge clk /*sync s0reset*/ or posedge a1reset or posedge a0reset) 
begin : sync_rst_multi_async_single_sync_ff
    if ( ~s0reset || a1reset || a0reset ) begin
        data10_reg_o <= 0;
    end
    else begin
        data10_reg_o <= data10_reg_o_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: sync_rst_multi_sync_single_async (test_cthread_sync_async_reset.cpp:234:5) 

// Thread-local variables
logic [2:0] data11_reg_o_next;

// Next-state combinational logic
always_comb begin : sync_rst_multi_sync_single_async_comb     // test_cthread_sync_async_reset.cpp:234:5
    sync_rst_multi_sync_single_async_func;
end
function void sync_rst_multi_sync_single_async_func;
    data11_reg_o_next = data11_reg_o;
    data11_reg_o_next = a_reg_in;
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge a0reset /*sync s1reset*/ /*sync s0reset*/) 
begin : sync_rst_multi_sync_single_async_ff
    if ( ~a0reset || ~s1reset || s0reset ) begin
        data11_reg_o <= 0;
    end
    else begin
        data11_reg_o <= data11_reg_o_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: sync_rst_multi_sync_multi_async (test_cthread_sync_async_reset.cpp:245:4) 

// Thread-local variables
logic [2:0] data12_reg_o_next;

// Next-state combinational logic
always_comb begin : sync_rst_multi_sync_multi_async_comb     // test_cthread_sync_async_reset.cpp:245:4
    sync_rst_multi_sync_multi_async_func;
end
function void sync_rst_multi_sync_multi_async_func;
    data12_reg_o_next = data12_reg_o;
    data12_reg_o_next = a_reg_in;
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge a2reset or negedge a1reset or posedge a0reset /*sync s1reset*/ /*sync s0reset*/) 
begin : sync_rst_multi_sync_multi_async_ff
    if ( ~a2reset || ~a1reset || a0reset || ~s1reset || s0reset ) begin
        data12_reg_o <= 0;
    end
    else begin
        data12_reg_o <= data12_reg_o_next;
    end
end

endmodule


