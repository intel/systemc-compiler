//==============================================================================
//
// The code is generated by Intel Compiler for SystemC, version 1.4.18
// see more information at https://github.com/intel/systemc-compiler
//
//==============================================================================

//==============================================================================
//
// Module: tb ()
//
module tb // "tb_inst"
(
);

// Variables generated for SystemC signals
logic clk;
logic rstn;
logic [31:0] paddr;
logic psel;
logic penable;
logic pwrite;
logic [31:0] pwdata;
logic pready;
logic [31:0] prdata;
logic pslverr;


//------------------------------------------------------------------------------
// Child module instances

dut dut_inst
(
  .clk(clk),
  .rstn(rstn),
  .apb_tar_paddr(paddr),
  .apb_tar_psel(psel),
  .apb_tar_penable(penable),
  .apb_tar_pwrite(pwrite),
  .apb_tar_pwdata(pwdata),
  .apb_tar_pready(pready),
  .apb_tar_prdata(prdata),
  .apb_tar_pslverr(pslverr)
);

endmodule



//==============================================================================
//
// Module: dut (test_dac2019_apb.cpp:150:5)
//
module dut // "tb_inst.dut_inst"
(
    input logic clk,
    input logic rstn,
    input logic [31:0] apb_tar_paddr,
    input logic apb_tar_psel,
    input logic apb_tar_penable,
    input logic apb_tar_pwrite,
    input logic [31:0] apb_tar_pwdata,
    output logic apb_tar_pready,
    output logic [31:0] apb_tar_prdata,
    output logic apb_tar_pslverr
);

// Variables generated for SystemC signals
logic apb_tar_clk;
logic apb_tar_rstn;

// Assignments generated for C++ channel arrays
assign apb_tar_clk = clk;
assign apb_tar_rstn = rstn;

//------------------------------------------------------------------------------
// Clocked THREAD: apb_tar_apb_thread (test_dac2019_apb.cpp:87:5) 

// Thread-local variables
logic apb_tar_pready_next;
logic [31:0] apb_tar_prdata_next;
logic apb_tar_pslverr_next;
logic [31:0] reg_file[16];
logic [31:0] reg_file_next[16];
logic apb_thread_PROC_STATE;
logic apb_thread_PROC_STATE_next;

// Next-state combinational logic
always_comb begin : apb_tar_apb_thread_comb     // test_dac2019_apb.cpp:87:5
    apb_tar_apb_thread_func;
end
function void apb_tar_apb_thread_func;
    logic [31:0] data;
    logic TMP_0;
    logic [31:0] addr;
    logic is_read;
    logic TMP_1;
    apb_tar_prdata_next = apb_tar_prdata;
    apb_tar_pready_next = apb_tar_pready;
    apb_tar_pslverr_next = apb_tar_pslverr;
    reg_file_next = reg_file;
    apb_thread_PROC_STATE_next = apb_thread_PROC_STATE;
    
    case (apb_thread_PROC_STATE)
        0: begin
            apb_tar_pready_next = 0;
            if (apb_tar_psel)
            begin
                data = 0;
                if (apb_tar_pwrite)
                begin
                    data = apb_tar_pwdata;
                    addr = apb_tar_paddr; is_read = 0;
                    // Call apb_trans() begin
                    if (addr < 16)
                    begin
                        reg_file_next[addr] = data;
                        TMP_0 = 0;
                    end else begin
                        TMP_0 = 1;
                    end
                    // Call apb_trans() end
                    apb_tar_pslverr_next = TMP_0;
                end else begin
                    addr = apb_tar_paddr; is_read = 1;
                    // Call apb_trans() begin
                    if (addr < 16)
                    begin
                        data = reg_file_next[addr];
                        TMP_1 = 0;
                    end else begin
                        TMP_1 = 1;
                    end
                    // Call apb_trans() end
                    apb_tar_pslverr_next = TMP_1;
                    apb_tar_prdata_next = data;
                end
                apb_tar_pready_next = 1;
                apb_thread_PROC_STATE_next = 1; return;    // test_dac2019_apb.cpp:105:17;
            end
            apb_tar_pready_next = 0;
            apb_thread_PROC_STATE_next = 0; return;    // test_dac2019_apb.cpp:93:13;
        end
        1: begin
            apb_tar_pready_next = 0;
            apb_thread_PROC_STATE_next = 0; return;    // test_dac2019_apb.cpp:93:13;
        end
    endcase
endfunction

// Synchronous register update
always_ff @(posedge apb_tar_clk or negedge apb_tar_rstn) 
begin : apb_tar_apb_thread_ff
    if ( ~apb_tar_rstn ) begin
        apb_tar_pready <= 0;
        apb_tar_prdata <= 0;
        apb_tar_pslverr <= 0;
        apb_thread_PROC_STATE <= 0;    // test_dac2019_apb.cpp:93:13;
    end
    else begin
        apb_tar_pready <= apb_tar_pready_next;
        apb_tar_prdata <= apb_tar_prdata_next;
        apb_tar_pslverr <= apb_tar_pslverr_next;
        reg_file <= reg_file_next;
        apb_thread_PROC_STATE <= apb_thread_PROC_STATE_next;
    end
end

endmodule


