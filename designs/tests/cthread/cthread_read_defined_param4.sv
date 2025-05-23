//==============================================================================
//
// The code is generated by Intel Compiler for SystemC, version 1.6.17
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
logic clk;
logic nrst;


//------------------------------------------------------------------------------
// Child module instances

A a_mod
(
  .clk(clk),
  .nrst(nrst)
);

endmodule



//==============================================================================
//
// Module: A (test_cthread_read_defined_param4.cpp:338:5)
//
module A // "b_mod.a_mod"
(
    input logic clk,
    input logic nrst
);

// Variables generated for SystemC signals
logic signed [31:0] s;
logic pc;
logic signed [31:0] parr[3];
logic signed [31:0] parr_[3];

// Local parameters generated for C++ constants
localparam logic [31:0] E = 1;
localparam logic [31:0] ARR[3] = '{ 1, 2, 3 };
localparam logic signed [31:0] ARRI[3] = '{ 2, 3, 4 };

//------------------------------------------------------------------------------
// Method process: const_ref_call (test_cthread_read_defined_param4.cpp:100:5) 

always_comb 
begin : const_ref_call     // test_cthread_read_defined_param4.cpp:100:5
    integer unsigned j;
    integer unsigned TMP_1;
    integer unsigned l;
    j = 0;
    j = 1;    // Call of cmref()
    // Call cmref() begin
    l = ARR[s];
    TMP_1 = l;
    // Call cmref() end
    j = TMP_1 + 2;
end

//------------------------------------------------------------------------------
// Clocked THREAD: const_ref_call1 (test_cthread_read_defined_param4.cpp:113:5) 

// Thread-local variables
logic [31:0] par;
logic [31:0] par_next;
logic [1:0] const_ref_call1_PROC_STATE;
logic [1:0] const_ref_call1_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : const_ref_call1_comb     // test_cthread_read_defined_param4.cpp:113:5
    const_ref_call1_func;
end
function void const_ref_call1_func;
    integer unsigned j;
    integer unsigned TMP_0;
    integer unsigned l;
    par_next = par;
    const_ref_call1_PROC_STATE_next = const_ref_call1_PROC_STATE;
    
    case (const_ref_call1_PROC_STATE)
        0: begin
            // Call cref() begin
            const_ref_call1_PROC_STATE_next = 1; return;    // test_cthread_read_defined_param4.cpp:108:9;
            // Call cref() end
        end
        1: begin
            // Call cref() begin
            l = ARR[2];
            TMP_0 = l;
            // Call cref() end
            j = TMP_0;
            const_ref_call1_PROC_STATE_next = 0; return;    // test_cthread_read_defined_param4.cpp:119:13;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge nrst) 
begin : const_ref_call1_ff
    if ( ~nrst ) begin
        integer unsigned j;
        j = 0;
        const_ref_call1_PROC_STATE <= 0;    // test_cthread_read_defined_param4.cpp:115:9;
    end
    else begin
        par <= par_next;
        const_ref_call1_PROC_STATE <= const_ref_call1_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: const_ref_call2 (test_cthread_read_defined_param4.cpp:123:5) 

// Thread-local variables
logic [31:0] par0;
logic [31:0] par_next0;
logic [1:0] const_ref_call2_PROC_STATE;
logic [1:0] const_ref_call2_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : const_ref_call2_comb     // test_cthread_read_defined_param4.cpp:123:5
    const_ref_call2_func;
end
function void const_ref_call2_func;
    integer unsigned j;
    integer unsigned TMP_0;
    integer unsigned l;
    par_next0 = par0;
    const_ref_call2_PROC_STATE_next = const_ref_call2_PROC_STATE;
    
    case (const_ref_call2_PROC_STATE)
        0: begin
            // Call cref() begin
            const_ref_call2_PROC_STATE_next = 1; return;    // test_cthread_read_defined_param4.cpp:108:9;
            // Call cref() end
        end
        1: begin
            // Call cref() begin
            l = ARR[s];
            TMP_0 = l;
            // Call cref() end
            j = TMP_0;
            const_ref_call2_PROC_STATE_next = 0; return;    // test_cthread_read_defined_param4.cpp:129:13;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge nrst) 
begin : const_ref_call2_ff
    if ( ~nrst ) begin
        integer unsigned j;
        j = 0;
        const_ref_call2_PROC_STATE <= 0;    // test_cthread_read_defined_param4.cpp:125:9;
    end
    else begin
        par0 <= par_next0;
        const_ref_call2_PROC_STATE <= const_ref_call2_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: const_ref_call3 (test_cthread_read_defined_param4.cpp:133:5) 

// Thread-local variables
logic [31:0] arrc[3];
logic [31:0] arrc_next[3];
logic [1:0] const_ref_call3_PROC_STATE;
logic [1:0] const_ref_call3_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : const_ref_call3_comb     // test_cthread_read_defined_param4.cpp:133:5
    const_ref_call3_func;
end
function void const_ref_call3_func;
    integer unsigned j;
    integer unsigned TMP_0;
    integer unsigned l;
    arrc_next = arrc;
    const_ref_call3_PROC_STATE_next = const_ref_call3_PROC_STATE;
    
    case (const_ref_call3_PROC_STATE)
        0: begin
            for (integer i = 0; i < 3; i++)
            begin
                arrc_next[i] = i;
            end
            // Call cref() begin
            const_ref_call3_PROC_STATE_next = 1; return;    // test_cthread_read_defined_param4.cpp:108:9;
            // Call cref() end
        end
        1: begin
            // Call cref() begin
            l = arrc_next[2];
            TMP_0 = l;
            // Call cref() end
            j = TMP_0;
            const_ref_call3_PROC_STATE_next = 0; return;    // test_cthread_read_defined_param4.cpp:142:13;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge nrst) 
begin : const_ref_call3_ff
    if ( ~nrst ) begin
        integer unsigned j;
        j = 0;
        const_ref_call3_PROC_STATE <= 0;    // test_cthread_read_defined_param4.cpp:135:9;
    end
    else begin
        arrc <= arrc_next;
        const_ref_call3_PROC_STATE <= const_ref_call3_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: const_ref_call4 (test_cthread_read_defined_param4.cpp:147:5) 

// Thread-local variables
logic [31:0] arrcm[3];
logic [31:0] arrcm_next[3];
logic [1:0] const_ref_call4_PROC_STATE;
logic [1:0] const_ref_call4_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : const_ref_call4_comb     // test_cthread_read_defined_param4.cpp:147:5
    const_ref_call4_func;
end
function void const_ref_call4_func;
    integer unsigned j;
    integer unsigned TMP_0;
    integer unsigned l;
    arrcm_next = arrcm;
    const_ref_call4_PROC_STATE_next = const_ref_call4_PROC_STATE;
    
    case (const_ref_call4_PROC_STATE)
        0: begin
            for (integer i = 0; i < 3; i++)
            begin
                arrcm_next[i] = i;
            end
            // Call cref() begin
            const_ref_call4_PROC_STATE_next = 1; return;    // test_cthread_read_defined_param4.cpp:108:9;
            // Call cref() end
        end
        1: begin
            // Call cref() begin
            l = arrcm_next[s + 1];
            TMP_0 = l;
            // Call cref() end
            j = TMP_0;
            const_ref_call4_PROC_STATE_next = 0; return;    // test_cthread_read_defined_param4.cpp:155:13;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge nrst) 
begin : const_ref_call4_ff
    if ( ~nrst ) begin
        integer unsigned j;
        j = 0;
        const_ref_call4_PROC_STATE <= 0;    // test_cthread_read_defined_param4.cpp:149:9;
    end
    else begin
        arrcm <= arrcm_next;
        const_ref_call4_PROC_STATE <= const_ref_call4_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: array_thread1 (test_cthread_read_defined_param4.cpp:169:5) 

// Thread-local variables
logic signed [31:0] larr[3];
logic signed [31:0] larr_next[3];
logic [1:0] array_thread1_PROC_STATE;
logic [1:0] array_thread1_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : array_thread1_comb     // test_cthread_read_defined_param4.cpp:169:5
    array_thread1_func;
end
function void array_thread1_func;
    integer j;
    integer TMP_0;
    integer l;
    larr_next = larr;
    array_thread1_PROC_STATE_next = array_thread1_PROC_STATE;
    
    case (array_thread1_PROC_STATE)
        0: begin
            // Call arr_wait1() begin
            array_thread1_PROC_STATE_next = 1; return;    // test_cthread_read_defined_param4.cpp:163:9;
            // Call arr_wait1() end
        end
        1: begin
            // Call arr_wait1() begin
            larr_next[s] = 2;
            l = larr_next[s];
            TMP_0 = l;
            // Call arr_wait1() end
            j = TMP_0;
            array_thread1_PROC_STATE_next = 0; return;    // test_cthread_read_defined_param4.cpp:176:13;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge nrst) 
begin : array_thread1_ff
    if ( ~nrst ) begin
        integer j;
        larr[0] <= 1; larr[1] <= 2; larr[2] <= 3;
        array_thread1_PROC_STATE <= 0;    // test_cthread_read_defined_param4.cpp:172:9;
    end
    else begin
        larr <= larr_next;
        array_thread1_PROC_STATE <= array_thread1_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: array_thread2 (test_cthread_read_defined_param4.cpp:181:5) 

// Thread-local variables
logic signed [31:0] marr[3];
logic signed [31:0] marr_next[3];
logic [1:0] array_thread2_PROC_STATE;
logic [1:0] array_thread2_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : array_thread2_comb     // test_cthread_read_defined_param4.cpp:181:5
    array_thread2_func;
end
function void array_thread2_func;
    integer j;
    integer TMP_0;
    integer l;
    marr_next = marr;
    array_thread2_PROC_STATE_next = array_thread2_PROC_STATE;
    
    case (array_thread2_PROC_STATE)
        0: begin
            // Call arr_wait1() begin
            array_thread2_PROC_STATE_next = 1; return;    // test_cthread_read_defined_param4.cpp:163:9;
            // Call arr_wait1() end
        end
        1: begin
            // Call arr_wait1() begin
            marr_next[s] = 2;
            l = marr_next[s];
            TMP_0 = l;
            // Call arr_wait1() end
            j = TMP_0;
            array_thread2_PROC_STATE_next = 0; return;    // test_cthread_read_defined_param4.cpp:187:13;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge nrst) 
begin : array_thread2_ff
    if ( ~nrst ) begin
        integer j;
        array_thread2_PROC_STATE <= 0;    // test_cthread_read_defined_param4.cpp:183:9;
    end
    else begin
        marr <= marr_next;
        array_thread2_PROC_STATE <= array_thread2_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: array_thread3 (test_cthread_read_defined_param4.cpp:200:5) 

// Thread-local variables
logic [1:0] array_thread3_PROC_STATE;
logic [1:0] array_thread3_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : array_thread3_comb     // test_cthread_read_defined_param4.cpp:200:5
    array_thread3_func;
end
function void array_thread3_func;
    integer j;
    integer TMP_0;
    integer l;
    array_thread3_PROC_STATE_next = array_thread3_PROC_STATE;
    
    case (array_thread3_PROC_STATE)
        0: begin
            // Call arr_wait2() begin
            array_thread3_PROC_STATE_next = 1; return;    // test_cthread_read_defined_param4.cpp:195:9;
            // Call arr_wait2() end
        end
        1: begin
            // Call arr_wait2() begin
            l = ARRI[s] + ARRI[1];
            TMP_0 = l;
            // Call arr_wait2() end
            j = TMP_0;
            array_thread3_PROC_STATE_next = 0; return;    // test_cthread_read_defined_param4.cpp:206:13;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge nrst) 
begin : array_thread3_ff
    if ( ~nrst ) begin
        integer j;
        array_thread3_PROC_STATE <= 0;    // test_cthread_read_defined_param4.cpp:202:9;
    end
    else begin
        array_thread3_PROC_STATE <= array_thread3_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: array_thread4 (test_cthread_read_defined_param4.cpp:210:5) 

// Thread-local variables
logic signed [31:0] llarr[3];
logic signed [31:0] llarr_next[3];
logic [1:0] array_thread4_PROC_STATE;
logic [1:0] array_thread4_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : array_thread4_comb     // test_cthread_read_defined_param4.cpp:210:5
    array_thread4_func;
end
function void array_thread4_func;
    integer j;
    integer TMP_0;
    integer l;
    llarr_next = llarr;
    array_thread4_PROC_STATE_next = array_thread4_PROC_STATE;
    
    case (array_thread4_PROC_STATE)
        0: begin
            // Call arr_wait2() begin
            array_thread4_PROC_STATE_next = 1; return;    // test_cthread_read_defined_param4.cpp:195:9;
            // Call arr_wait2() end
        end
        1: begin
            // Call arr_wait2() begin
            l = llarr_next[s] + llarr_next[1];
            TMP_0 = l;
            // Call arr_wait2() end
            j = TMP_0;
            array_thread4_PROC_STATE_next = 0; return;    // test_cthread_read_defined_param4.cpp:217:13;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge nrst) 
begin : array_thread4_ff
    if ( ~nrst ) begin
        integer j;
        llarr[0] <= 1; llarr[1] <= 2; llarr[2] <= 3;
        array_thread4_PROC_STATE <= 0;    // test_cthread_read_defined_param4.cpp:213:9;
    end
    else begin
        llarr <= llarr_next;
        array_thread4_PROC_STATE <= array_thread4_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: array_thread5 (test_cthread_read_defined_param4.cpp:222:5) 

// Thread-local variables
logic signed [31:0] mmarr[3];
logic signed [31:0] mmarr_next[3];
logic [1:0] array_thread5_PROC_STATE;
logic [1:0] array_thread5_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : array_thread5_comb     // test_cthread_read_defined_param4.cpp:222:5
    array_thread5_func;
end
function void array_thread5_func;
    integer j;
    integer TMP_0;
    integer l;
    mmarr_next = mmarr;
    array_thread5_PROC_STATE_next = array_thread5_PROC_STATE;
    
    case (array_thread5_PROC_STATE)
        0: begin
            // Call arr_wait2() begin
            array_thread5_PROC_STATE_next = 1; return;    // test_cthread_read_defined_param4.cpp:195:9;
            // Call arr_wait2() end
        end
        1: begin
            // Call arr_wait2() begin
            l = mmarr_next[s] + mmarr_next[1];
            TMP_0 = l;
            // Call arr_wait2() end
            j = TMP_0;
            array_thread5_PROC_STATE_next = 2; return;    // test_cthread_read_defined_param4.cpp:229:13;
        end
        2: begin
            for (integer i_1 = 0; i_1 < 3; i_1++)
            begin
                mmarr_next[i_1] = i_1;
            end
            // Call arr_wait2() begin
            array_thread5_PROC_STATE_next = 1; return;    // test_cthread_read_defined_param4.cpp:195:9;
            // Call arr_wait2() end
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge nrst) 
begin : array_thread5_ff
    if ( ~nrst ) begin
        integer j;
        for (integer i = 0; i < 3; i++)
        begin
            mmarr[i] <= 0;
        end
        array_thread5_PROC_STATE <= 0;    // test_cthread_read_defined_param4.cpp:225:9;
    end
    else begin
        mmarr <= mmarr_next;
        array_thread5_PROC_STATE <= array_thread5_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: chan_array_thread1 (test_cthread_read_defined_param4.cpp:246:5) 

// Thread-local variables
logic signed [31:0] parr_next[3];
logic [1:0] chan_array_thread1_PROC_STATE;
logic [1:0] chan_array_thread1_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : chan_array_thread1_comb     // test_cthread_read_defined_param4.cpp:246:5
    chan_array_thread1_func;
end
function void chan_array_thread1_func;
    integer i;
    integer TMP_0;
    integer l;
    parr_next = parr;
    chan_array_thread1_PROC_STATE_next = chan_array_thread1_PROC_STATE;
    
    case (chan_array_thread1_PROC_STATE)
        0: begin
            // Call chan_arr_wait1() begin
            chan_array_thread1_PROC_STATE_next = 1; return;    // test_cthread_read_defined_param4.cpp:240:9;
            // Call chan_arr_wait1() end
        end
        1: begin
            // Call chan_arr_wait1() begin
            parr_next[1] = 42;
            l = parr[s];
            TMP_0 = l;
            // Call chan_arr_wait1() end
            i = TMP_0;
            chan_array_thread1_PROC_STATE_next = 0; return;    // test_cthread_read_defined_param4.cpp:252:13;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge nrst) 
begin : chan_array_thread1_ff
    if ( ~nrst ) begin
        integer i;
        chan_array_thread1_PROC_STATE <= 0;    // test_cthread_read_defined_param4.cpp:248:9;
    end
    else begin
        parr <= parr_next;
        chan_array_thread1_PROC_STATE <= chan_array_thread1_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: chan_array_thread2 (test_cthread_read_defined_param4.cpp:263:5) 

// Thread-local variables
logic signed [31:0] parr__next[3];
logic [1:0] chan_array_thread2_PROC_STATE;
logic [1:0] chan_array_thread2_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : chan_array_thread2_comb     // test_cthread_read_defined_param4.cpp:263:5
    chan_array_thread2_func;
end
function void chan_array_thread2_func;
    integer i;
    integer TMP_0;
    integer l;
    parr__next = parr_;
    chan_array_thread2_PROC_STATE_next = chan_array_thread2_PROC_STATE;
    
    case (chan_array_thread2_PROC_STATE)
        0: begin
            // Call chan_arr_wait2() begin
            chan_array_thread2_PROC_STATE_next = 1; return;    // test_cthread_read_defined_param4.cpp:257:9;
            // Call chan_arr_wait2() end
        end
        1: begin
            // Call chan_arr_wait2() begin
            parr__next[1] = 42;
            l = parr_[s];
            TMP_0 = l;
            // Call chan_arr_wait2() end
            i = TMP_0;
            chan_array_thread2_PROC_STATE_next = 0; return;    // test_cthread_read_defined_param4.cpp:269:13;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge nrst) 
begin : chan_array_thread2_ff
    if ( ~nrst ) begin
        integer i;
        chan_array_thread2_PROC_STATE <= 0;    // test_cthread_read_defined_param4.cpp:265:9;
    end
    else begin
        parr_ <= parr__next;
        chan_array_thread2_PROC_STATE <= chan_array_thread2_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: ptr_thread1 (test_cthread_read_defined_param4.cpp:284:5) 

// Thread-local variables
logic [2:0] pk;
logic [2:0] pk_next;
logic [1:0] ptr_thread1_PROC_STATE;
logic [1:0] ptr_thread1_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : ptr_thread1_comb     // test_cthread_read_defined_param4.cpp:284:5
    ptr_thread1_func;
end
function void ptr_thread1_func;
    integer i;
    logic [3:0] TMP_0;
    integer l;
    pk_next = pk;
    ptr_thread1_PROC_STATE_next = ptr_thread1_PROC_STATE;
    
    case (ptr_thread1_PROC_STATE)
        0: begin
            pk_next = 1;
            // Call fptr2() begin
            ptr_thread1_PROC_STATE_next = 1; return;    // test_cthread_read_defined_param4.cpp:278:9;
            // Call fptr2() end
        end
        1: begin
            // Call fptr2() begin
            l = pk_next;
            TMP_0 = l;
            // Call fptr2() end
            ptr_thread1_PROC_STATE_next = 0; return;    // test_cthread_read_defined_param4.cpp:291:13;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge nrst) 
begin : ptr_thread1_ff
    if ( ~nrst ) begin
        integer i;
        i = 0;
        ptr_thread1_PROC_STATE <= 0;    // test_cthread_read_defined_param4.cpp:286:9;
    end
    else begin
        pk <= pk_next;
        ptr_thread1_PROC_STATE <= ptr_thread1_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: ptr_thread2 (test_cthread_read_defined_param4.cpp:296:5) 

// Thread-local variables
logic [2:0] pl;
logic [2:0] pl_next;
logic [1:0] ptr_thread2_PROC_STATE;
logic [1:0] ptr_thread2_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : ptr_thread2_comb     // test_cthread_read_defined_param4.cpp:296:5
    ptr_thread2_func;
end
function void ptr_thread2_func;
    integer i;
    logic [3:0] TMP_0;
    integer l;
    pl_next = pl;
    ptr_thread2_PROC_STATE_next = ptr_thread2_PROC_STATE;
    
    case (ptr_thread2_PROC_STATE)
        0: begin
            // Call fptr2() begin
            ptr_thread2_PROC_STATE_next = 1; return;    // test_cthread_read_defined_param4.cpp:278:9;
            // Call fptr2() end
        end
        1: begin
            // Call fptr2() begin
            l = pl_next;
            TMP_0 = l;
            // Call fptr2() end
            ptr_thread2_PROC_STATE_next = 0; return;    // test_cthread_read_defined_param4.cpp:303:13;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge nrst) 
begin : ptr_thread2_ff
    if ( ~nrst ) begin
        integer i;
        i = 0;
        pl <= 1;
        ptr_thread2_PROC_STATE <= 0;    // test_cthread_read_defined_param4.cpp:299:9;
    end
    else begin
        pl <= pl_next;
        ptr_thread2_PROC_STATE <= ptr_thread2_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: ptr_ch_thread1 (test_cthread_read_defined_param4.cpp:316:5) 

// Thread-local variables
logic pc_next;
logic ll;
logic ll_next;
logic [1:0] ptr_ch_thread1_PROC_STATE;
logic [1:0] ptr_ch_thread1_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : ptr_ch_thread1_comb     // test_cthread_read_defined_param4.cpp:316:5
    ptr_ch_thread1_func;
end
function void ptr_ch_thread1_func;
    logic TMP_0;
    logic d;
    ll_next = ll;
    pc_next = pc;
    ptr_ch_thread1_PROC_STATE_next = ptr_ch_thread1_PROC_STATE;
    
    case (ptr_ch_thread1_PROC_STATE)
        0: begin
            // Call fch_ptr() begin
            ptr_ch_thread1_PROC_STATE_next = 1; return;    // test_cthread_read_defined_param4.cpp:309:9;
            // Call fch_ptr() end
        end
        1: begin
            // Call fch_ptr() begin
            ll_next = pc;
            ptr_ch_thread1_PROC_STATE_next = 2; return;    // test_cthread_read_defined_param4.cpp:311:9;
            // Call fch_ptr() end
        end
        2: begin
            // Call fch_ptr() begin
            pc_next = !ll_next;
            TMP_0 = ll_next;
            // Call fch_ptr() end
            d = TMP_0;
            ptr_ch_thread1_PROC_STATE_next = 0; return;    // test_cthread_read_defined_param4.cpp:323:13;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge nrst) 
begin : ptr_ch_thread1_ff
    if ( ~nrst ) begin
        pc <= 0;
        ptr_ch_thread1_PROC_STATE <= 0;    // test_cthread_read_defined_param4.cpp:319:9;
    end
    else begin
        pc <= pc_next;
        ll <= ll_next;
        ptr_ch_thread1_PROC_STATE <= ptr_ch_thread1_PROC_STATE_next;
    end
end

endmodule


