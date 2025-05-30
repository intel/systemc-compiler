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
// Module: A (test_cthread_read_defined_param3.cpp:411:5)
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
localparam logic [31:0] ARR[3] = '{ 1, 2, 3 };
localparam logic signed [31:0] ARRI[3] = '{ 2, 3, 4 };

//------------------------------------------------------------------------------
// Clocked THREAD: const_ref_call0 (test_cthread_read_defined_param3.cpp:101:5) 

// Next-state combinational logic
always_comb begin : const_ref_call0_comb     // test_cthread_read_defined_param3.cpp:101:5
    const_ref_call0_func;
end
function void const_ref_call0_func;
    integer unsigned j;
    logic val;
    logic l;
    val = 0;
    // Call cref0() begin
    l = 0;
    // Call cref0() end
    val = ARR[1] < ARR[2];
    // Call cref0() begin
    l = val;
    // Call cref0() end
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge nrst) 
begin : const_ref_call0_ff
    if ( ~nrst ) begin
        integer unsigned j;
        j = 0;
    end
    else begin
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: const_ref_call1 (test_cthread_read_defined_param3.cpp:118:5) 

// Thread-local variables
logic [31:0] l0;
logic [31:0] l_next;
logic [1:0] const_ref_call1_PROC_STATE;
logic [1:0] const_ref_call1_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : const_ref_call1_comb     // test_cthread_read_defined_param3.cpp:118:5
    const_ref_call1_func;
end
function void const_ref_call1_func;
    integer unsigned j;
    integer unsigned TMP_0;
    integer unsigned TMP_1;
    l_next = l0;
    const_ref_call1_PROC_STATE_next = const_ref_call1_PROC_STATE;
    
    case (const_ref_call1_PROC_STATE)
        0: begin
            // Call cref() begin
            l_next = ARR[1];
            const_ref_call1_PROC_STATE_next = 1; return;    // test_cthread_read_defined_param3.cpp:114:9;
            // Call cref() end
        end
        1: begin
            // Call cref() begin
            TMP_0 = l_next;
            // Call cref() end
            j = TMP_0;
            // Call cref() begin
            l_next = ARR[s];
            const_ref_call1_PROC_STATE_next = 2; return;    // test_cthread_read_defined_param3.cpp:114:9;
            // Call cref() end
        end
        2: begin
            // Call cref() begin
            TMP_1 = l_next;
            // Call cref() end
            j = TMP_1;
            const_ref_call1_PROC_STATE_next = 0; return;    // test_cthread_read_defined_param3.cpp:125:13;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge nrst) 
begin : const_ref_call1_ff
    if ( ~nrst ) begin
        integer unsigned j;
        j = 0;
        const_ref_call1_PROC_STATE <= 0;    // test_cthread_read_defined_param3.cpp:120:9;
    end
    else begin
        l0 <= l_next;
        const_ref_call1_PROC_STATE <= const_ref_call1_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: const_ref_call2 (test_cthread_read_defined_param3.cpp:129:5) 

// Thread-local variables
logic [31:0] l1;
logic [31:0] l_next0;
logic [1:0] const_ref_call2_PROC_STATE;
logic [1:0] const_ref_call2_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : const_ref_call2_comb     // test_cthread_read_defined_param3.cpp:129:5
    const_ref_call2_func;
end
function void const_ref_call2_func;
    integer unsigned j;
    integer unsigned arrc[3];
    integer unsigned TMP_0;
    l_next0 = l1;
    const_ref_call2_PROC_STATE_next = const_ref_call2_PROC_STATE;
    
    case (const_ref_call2_PROC_STATE)
        0: begin
            for (integer i = 0; i < 3; i++)
            begin
                arrc[i] = i;
            end
            // Call cref() begin
            l_next0 = arrc[2];
            const_ref_call2_PROC_STATE_next = 1; return;    // test_cthread_read_defined_param3.cpp:114:9;
            // Call cref() end
        end
        1: begin
            // Call cref() begin
            TMP_0 = l_next0;
            // Call cref() end
            j = TMP_0;
            const_ref_call2_PROC_STATE_next = 0; return;    // test_cthread_read_defined_param3.cpp:138:13;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge nrst) 
begin : const_ref_call2_ff
    if ( ~nrst ) begin
        integer unsigned j;
        j = 0;
        const_ref_call2_PROC_STATE <= 0;    // test_cthread_read_defined_param3.cpp:131:9;
    end
    else begin
        l1 <= l_next0;
        const_ref_call2_PROC_STATE <= const_ref_call2_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: const_ref_call3 (test_cthread_read_defined_param3.cpp:143:5) 

// Thread-local variables
logic [31:0] l2;
logic [31:0] l_next1;
logic [31:0] arrcm[3];
logic [1:0] const_ref_call3_PROC_STATE;
logic [1:0] const_ref_call3_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : const_ref_call3_comb     // test_cthread_read_defined_param3.cpp:143:5
    const_ref_call3_func;
end
function void const_ref_call3_func;
    integer unsigned j;
    integer unsigned TMP_0;
    l_next1 = l2;
    const_ref_call3_PROC_STATE_next = const_ref_call3_PROC_STATE;
    
    case (const_ref_call3_PROC_STATE)
        0: begin
            for (integer i = 0; i < 3; i++)
            begin
                arrcm[i] = i;
            end
            // Call cref() begin
            l_next1 = arrcm[2];
            const_ref_call3_PROC_STATE_next = 1; return;    // test_cthread_read_defined_param3.cpp:114:9;
            // Call cref() end
        end
        1: begin
            // Call cref() begin
            TMP_0 = l_next1;
            // Call cref() end
            j = TMP_0;
            const_ref_call3_PROC_STATE_next = 0; return;    // test_cthread_read_defined_param3.cpp:151:13;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge nrst) 
begin : const_ref_call3_ff
    if ( ~nrst ) begin
        integer unsigned j;
        j = 0;
        const_ref_call3_PROC_STATE <= 0;    // test_cthread_read_defined_param3.cpp:145:9;
    end
    else begin
        l2 <= l_next1;
        const_ref_call3_PROC_STATE <= const_ref_call3_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Method process: array_in_func (test_cthread_read_defined_param3.cpp:170:5) 

always_comb 
begin : array_in_func     // test_cthread_read_defined_param3.cpp:170:5
    integer arr[3];
    integer arr_[3];
    integer i;
    integer arr__[3];
    integer i_1;
    // Call arr_func1() begin
    arr[0] = 2;
    // Call arr_func1() end
    // Call arr_func2() begin
    i = arr_[1];
    // Call arr_func2() end
    // Call arr_func3() begin
    arr__[2] = 2;
    i_1 = arr__[s];
    // Call arr_func3() end
end

//------------------------------------------------------------------------------
// Clocked THREAD: array_thread1 (test_cthread_read_defined_param3.cpp:197:5) 

// Thread-local variables
logic signed [31:0] larr[3];
logic signed [31:0] larr_next[3];
logic signed [31:0] l3;
logic signed [31:0] l_next2;
logic [1:0] array_thread1_PROC_STATE;
logic [1:0] array_thread1_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : array_thread1_comb     // test_cthread_read_defined_param3.cpp:197:5
    array_thread1_func;
end
function void array_thread1_func;
    integer j;
    integer TMP_0;
    l_next2 = l3;
    larr_next = larr;
    array_thread1_PROC_STATE_next = array_thread1_PROC_STATE;
    
    case (array_thread1_PROC_STATE)
        0: begin
            // Call arr_wait1() begin
            larr_next[s] = 2;
            l_next2 = larr_next[s];
            array_thread1_PROC_STATE_next = 1; return;    // test_cthread_read_defined_param3.cpp:193:9;
            // Call arr_wait1() end
        end
        1: begin
            // Call arr_wait1() begin
            TMP_0 = l_next2;
            // Call arr_wait1() end
            j = TMP_0;
            array_thread1_PROC_STATE_next = 0; return;    // test_cthread_read_defined_param3.cpp:204:13;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge nrst) 
begin : array_thread1_ff
    if ( ~nrst ) begin
        integer j;
        larr[0] <= 1; larr[1] <= 2; larr[2] <= 3;
        array_thread1_PROC_STATE <= 0;    // test_cthread_read_defined_param3.cpp:200:9;
    end
    else begin
        larr <= larr_next;
        l3 <= l_next2;
        array_thread1_PROC_STATE <= array_thread1_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: array_thread2 (test_cthread_read_defined_param3.cpp:214:5) 

// Thread-local variables
logic signed [31:0] marr[3];
logic signed [31:0] marr_next[3];

// Next-state combinational logic
always_comb begin : array_thread2_comb     // test_cthread_read_defined_param3.cpp:214:5
    array_thread2_func;
end
function void array_thread2_func;
    integer l;
    marr_next = marr;
    // Call arr_call2() begin
    marr_next[1] = 2;
    l = marr_next[2];
    // Call arr_call2() end
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge nrst) 
begin : array_thread2_ff
    if ( ~nrst ) begin
    end
    else begin
        marr <= marr_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: array_thread3 (test_cthread_read_defined_param3.cpp:232:5) 

// Thread-local variables
logic signed [31:0] l4;
logic signed [31:0] l_next3;
logic [1:0] array_thread3_PROC_STATE;
logic [1:0] array_thread3_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : array_thread3_comb     // test_cthread_read_defined_param3.cpp:232:5
    array_thread3_func;
end
function void array_thread3_func;
    integer j;
    integer TMP_0;
    l_next3 = l4;
    array_thread3_PROC_STATE_next = array_thread3_PROC_STATE;
    
    case (array_thread3_PROC_STATE)
        0: begin
            // Call arr_wait3() begin
            l_next3 = ARRI[s] + ARRI[1];
            array_thread3_PROC_STATE_next = 1; return;    // test_cthread_read_defined_param3.cpp:228:9;
            // Call arr_wait3() end
        end
        1: begin
            // Call arr_wait3() begin
            TMP_0 = l_next3;
            // Call arr_wait3() end
            j = TMP_0;
            array_thread3_PROC_STATE_next = 0; return;    // test_cthread_read_defined_param3.cpp:238:13;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge nrst) 
begin : array_thread3_ff
    if ( ~nrst ) begin
        integer j;
        array_thread3_PROC_STATE <= 0;    // test_cthread_read_defined_param3.cpp:234:9;
    end
    else begin
        l4 <= l_next3;
        array_thread3_PROC_STATE <= array_thread3_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: array_thread4 (test_cthread_read_defined_param3.cpp:242:5) 

// Thread-local variables
logic signed [31:0] llarr[3];
logic signed [31:0] llarr_next[3];
logic signed [31:0] l5;
logic signed [31:0] l_next4;
logic [1:0] array_thread4_PROC_STATE;
logic [1:0] array_thread4_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : array_thread4_comb     // test_cthread_read_defined_param3.cpp:242:5
    array_thread4_func;
end
function void array_thread4_func;
    integer j;
    integer TMP_0;
    l_next4 = l5;
    llarr_next = llarr;
    array_thread4_PROC_STATE_next = array_thread4_PROC_STATE;
    
    case (array_thread4_PROC_STATE)
        0: begin
            // Call arr_wait3() begin
            l_next4 = llarr_next[s] + llarr_next[1];
            array_thread4_PROC_STATE_next = 1; return;    // test_cthread_read_defined_param3.cpp:228:9;
            // Call arr_wait3() end
        end
        1: begin
            // Call arr_wait3() begin
            TMP_0 = l_next4;
            // Call arr_wait3() end
            j = TMP_0;
            array_thread4_PROC_STATE_next = 0; return;    // test_cthread_read_defined_param3.cpp:249:13;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge nrst) 
begin : array_thread4_ff
    if ( ~nrst ) begin
        integer j;
        llarr[0] <= 1; llarr[1] <= 2; llarr[2] <= 3;
        array_thread4_PROC_STATE <= 0;    // test_cthread_read_defined_param3.cpp:245:9;
    end
    else begin
        llarr <= llarr_next;
        l5 <= l_next4;
        array_thread4_PROC_STATE <= array_thread4_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: array_thread5 (test_cthread_read_defined_param3.cpp:254:5) 

// Thread-local variables
logic signed [31:0] mmarr[3];
logic signed [31:0] mmarr_next[3];
logic signed [31:0] l6;
logic signed [31:0] l_next5;
logic [1:0] array_thread5_PROC_STATE;
logic [1:0] array_thread5_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : array_thread5_comb     // test_cthread_read_defined_param3.cpp:254:5
    array_thread5_func;
end
function void array_thread5_func;
    integer j;
    integer TMP_0;
    l_next5 = l6;
    mmarr_next = mmarr;
    array_thread5_PROC_STATE_next = array_thread5_PROC_STATE;
    
    case (array_thread5_PROC_STATE)
        0: begin
            // Call arr_wait3() begin
            l_next5 = mmarr_next[s] + mmarr_next[1];
            array_thread5_PROC_STATE_next = 1; return;    // test_cthread_read_defined_param3.cpp:228:9;
            // Call arr_wait3() end
        end
        1: begin
            // Call arr_wait3() begin
            TMP_0 = l_next5;
            // Call arr_wait3() end
            j = TMP_0;
            array_thread5_PROC_STATE_next = 2; return;    // test_cthread_read_defined_param3.cpp:261:13;
        end
        2: begin
            for (integer i_1 = 0; i_1 < 3; i_1++)
            begin
                mmarr_next[i_1] = i_1;
            end
            // Call arr_wait3() begin
            l_next5 = mmarr_next[s] + mmarr_next[1];
            array_thread5_PROC_STATE_next = 1; return;    // test_cthread_read_defined_param3.cpp:228:9;
            // Call arr_wait3() end
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
        array_thread5_PROC_STATE <= 0;    // test_cthread_read_defined_param3.cpp:257:9;
    end
    else begin
        mmarr <= mmarr_next;
        l6 <= l_next5;
        array_thread5_PROC_STATE <= array_thread5_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: chan_array_thread1 (test_cthread_read_defined_param3.cpp:278:5) 

// Thread-local variables
logic signed [31:0] parr_next[3];
logic signed [31:0] l7;
logic signed [31:0] l_next6;
logic [1:0] chan_array_thread1_PROC_STATE;
logic [1:0] chan_array_thread1_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : chan_array_thread1_comb     // test_cthread_read_defined_param3.cpp:278:5
    chan_array_thread1_func;
end
function void chan_array_thread1_func;
    integer i;
    integer TMP_0;
    l_next6 = l7;
    parr_next = parr;
    chan_array_thread1_PROC_STATE_next = chan_array_thread1_PROC_STATE;
    
    case (chan_array_thread1_PROC_STATE)
        0: begin
            // Call chan_arr_wait1() begin
            parr_next[1] = 42;
            l_next6 = parr[s] + parr[2];
            chan_array_thread1_PROC_STATE_next = 1; return;    // test_cthread_read_defined_param3.cpp:274:9;
            // Call chan_arr_wait1() end
        end
        1: begin
            // Call chan_arr_wait1() begin
            TMP_0 = l_next6;
            // Call chan_arr_wait1() end
            i = TMP_0;
            chan_array_thread1_PROC_STATE_next = 0; return;    // test_cthread_read_defined_param3.cpp:284:13;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge nrst) 
begin : chan_array_thread1_ff
    if ( ~nrst ) begin
        integer i;
        chan_array_thread1_PROC_STATE <= 0;    // test_cthread_read_defined_param3.cpp:280:9;
    end
    else begin
        parr <= parr_next;
        l7 <= l_next6;
        chan_array_thread1_PROC_STATE <= chan_array_thread1_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: chan_array_thread2 (test_cthread_read_defined_param3.cpp:295:5) 

// Thread-local variables
logic signed [31:0] parr__next[3];
logic signed [31:0] l8;
logic signed [31:0] l_next7;
logic [1:0] chan_array_thread2_PROC_STATE;
logic [1:0] chan_array_thread2_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : chan_array_thread2_comb     // test_cthread_read_defined_param3.cpp:295:5
    chan_array_thread2_func;
end
function void chan_array_thread2_func;
    integer i;
    integer TMP_0;
    l_next7 = l8;
    parr__next = parr_;
    chan_array_thread2_PROC_STATE_next = chan_array_thread2_PROC_STATE;
    
    case (chan_array_thread2_PROC_STATE)
        0: begin
            // Call chan_arr_wait2() begin
            parr__next[1] = 42;
            l_next7 = parr_[s] + parr_[1];
            chan_array_thread2_PROC_STATE_next = 1; return;    // test_cthread_read_defined_param3.cpp:291:9;
            // Call chan_arr_wait2() end
        end
        1: begin
            // Call chan_arr_wait2() begin
            TMP_0 = l_next7;
            // Call chan_arr_wait2() end
            i = TMP_0;
            chan_array_thread2_PROC_STATE_next = 0; return;    // test_cthread_read_defined_param3.cpp:301:13;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge nrst) 
begin : chan_array_thread2_ff
    if ( ~nrst ) begin
        integer i;
        chan_array_thread2_PROC_STATE <= 0;    // test_cthread_read_defined_param3.cpp:297:9;
    end
    else begin
        parr_ <= parr__next;
        l8 <= l_next7;
        chan_array_thread2_PROC_STATE <= chan_array_thread2_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Method process: array_2d_in_func (test_cthread_read_defined_param3.cpp:317:5) 

always_comb 
begin : array_2d_in_func     // test_cthread_read_defined_param3.cpp:317:5
    integer arr[3][2];
    integer arr_[3][2];
    integer i;
    // Call arr_2D_func1() begin
    arr[0][1] = 2;
    arr[s][s + 1] = 3;
    // Call arr_2D_func1() end
    // Call arr_2D_func2() begin
    i = arr_[1][0] + arr_[s][s + 1];
    // Call arr_2D_func2() end
end

//------------------------------------------------------------------------------
// Method process: ptr_func1 (test_cthread_read_defined_param3.cpp:336:5) 

// Process-local variables
logic [2:0] pj;
logic [2:0] pi;

always_comb 
begin : ptr_func1     // test_cthread_read_defined_param3.cpp:336:5
    pj = 1;
end

//------------------------------------------------------------------------------
// Clocked THREAD: ptr_thread1 (test_cthread_read_defined_param3.cpp:358:5) 

// Thread-local variables
logic signed [31:0] l9;
logic signed [31:0] l_next8;
logic [2:0] pk;
logic [1:0] ptr_thread1_PROC_STATE;
logic [1:0] ptr_thread1_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : ptr_thread1_comb     // test_cthread_read_defined_param3.cpp:358:5
    ptr_thread1_func;
end
function void ptr_thread1_func;
    integer i;
    logic [3:0] TMP_0;
    l_next8 = l9;
    ptr_thread1_PROC_STATE_next = ptr_thread1_PROC_STATE;
    
    case (ptr_thread1_PROC_STATE)
        0: begin
            pk = 1;
            // Call fptr2() begin
            l_next8 = pk;
            ptr_thread1_PROC_STATE_next = 1; return;    // test_cthread_read_defined_param3.cpp:353:9;
            // Call fptr2() end
        end
        1: begin
            // Call fptr2() begin
            TMP_0 = l_next8;
            // Call fptr2() end
            ptr_thread1_PROC_STATE_next = 0; return;    // test_cthread_read_defined_param3.cpp:365:13;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge nrst) 
begin : ptr_thread1_ff
    if ( ~nrst ) begin
        integer i;
        i = 0;
        ptr_thread1_PROC_STATE <= 0;    // test_cthread_read_defined_param3.cpp:360:9;
    end
    else begin
        l9 <= l_next8;
        ptr_thread1_PROC_STATE <= ptr_thread1_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: ptr_thread2 (test_cthread_read_defined_param3.cpp:370:5) 

// Thread-local variables
logic [2:0] pl;
logic [2:0] pl_next;
logic signed [31:0] l10;
logic signed [31:0] l_next9;
logic [1:0] ptr_thread2_PROC_STATE;
logic [1:0] ptr_thread2_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : ptr_thread2_comb     // test_cthread_read_defined_param3.cpp:370:5
    ptr_thread2_func;
end
function void ptr_thread2_func;
    integer i;
    logic [3:0] TMP_0;
    l_next9 = l10;
    pl_next = pl;
    ptr_thread2_PROC_STATE_next = ptr_thread2_PROC_STATE;
    
    case (ptr_thread2_PROC_STATE)
        0: begin
            // Call fptr2() begin
            l_next9 = pl_next;
            ptr_thread2_PROC_STATE_next = 1; return;    // test_cthread_read_defined_param3.cpp:353:9;
            // Call fptr2() end
        end
        1: begin
            // Call fptr2() begin
            TMP_0 = l_next9;
            // Call fptr2() end
            ptr_thread2_PROC_STATE_next = 0; return;    // test_cthread_read_defined_param3.cpp:377:13;
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
        ptr_thread2_PROC_STATE <= 0;    // test_cthread_read_defined_param3.cpp:373:9;
    end
    else begin
        pl <= pl_next;
        l10 <= l_next9;
        ptr_thread2_PROC_STATE <= ptr_thread2_PROC_STATE_next;
    end
end

//------------------------------------------------------------------------------
// Clocked THREAD: ptr_ch_thread1 (test_cthread_read_defined_param3.cpp:389:5) 

// Thread-local variables
logic pc_next;
logic ll;
logic ll_next;
logic [1:0] ptr_ch_thread1_PROC_STATE;
logic [1:0] ptr_ch_thread1_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : ptr_ch_thread1_comb     // test_cthread_read_defined_param3.cpp:389:5
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
            ll_next = pc;
            ptr_ch_thread1_PROC_STATE_next = 1; return;    // test_cthread_read_defined_param3.cpp:384:9;
            // Call fch_ptr() end
        end
        1: begin
            // Call fch_ptr() begin
            pc_next = !ll_next;
            TMP_0 = ll_next;
            // Call fch_ptr() end
            d = TMP_0;
            ptr_ch_thread1_PROC_STATE_next = 0; return;    // test_cthread_read_defined_param3.cpp:396:13;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge clk or negedge nrst) 
begin : ptr_ch_thread1_ff
    if ( ~nrst ) begin
        pc <= 0;
        ptr_ch_thread1_PROC_STATE <= 0;    // test_cthread_read_defined_param3.cpp:392:9;
    end
    else begin
        pc <= pc_next;
        ll <= ll_next;
        ptr_ch_thread1_PROC_STATE <= ptr_ch_thread1_PROC_STATE_next;
    end
end

endmodule


