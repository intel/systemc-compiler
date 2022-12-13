/*************************************************************************

  This file is part of the ParaNut project.

  Copyright (C) 2019 Alexander Bahle <alexander.bahle@hs-augsburg.de>
                2022 Marco Milenkovic <marco.milenkovic@hs-augsburg.de>
      Hochschule Augsburg, University of Applied Sciences

  Redistribution and use in source and binary forms, with or without modification,
  are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice, this 
     list of conditions and the following disclaimer.

  2. Redistributions in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation and/or
     other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 *************************************************************************/


#include "dm.h"
#include "debug_rom.h"

#ifndef __SYNTHESIS__
void MDebugModule::Trace (sc_trace_file *tf, int level) {
    if (!tf || pn_trace_verbose) printf ("\nSignals of Module \"%s\":\n", name ());

    // Ports...
    PN_TRACE (tf, clk_i);
    PN_TRACE (tf, rst_i);
    //   to EXU ...
    PN_TRACE (tf, dbg_reset);
    //   WB
    PN_TRACE (tf, ack_o);
    PN_TRACE (tf, rty_o);
    PN_TRACE (tf, err_o);
    PN_TRACE (tf, dat_o);
    PN_TRACE (tf, dat_i);
    PN_TRACE (tf, adr_i);
    PN_TRACE (tf, stb_i);
    PN_TRACE (tf, cyc_i);
    PN_TRACE (tf, we_i);

    //   internal registers/signals...
    PN_TRACE_BUS (tf, dm_regs, DBG_NUM_REGISTERS);
    PN_TRACE_BUS (tf, dm_flags, CFG_NUT_CPU_CORES);
    PN_TRACE (tf, reg_sel);
    PN_TRACE (tf, reg_in);
    PN_TRACE (tf, reg_write);
    PN_TRACE (tf, dm_state);
    PN_TRACE (tf, dbg_request);
    PN_TRACE (tf, command_written);
    PN_TRACE (tf, flag_go);

    // abstracts
    PN_TRACE (tf, reg_abstracts_cmderr);
    PN_TRACE (tf, reg_abstracts_busy);
    PN_TRACE (tf, abstracts_cmderr);
    PN_TRACE (tf, abstracts_busy);

    // abstractauto
    PN_TRACE (tf, abstractauto_autoexecdata);

    // command
    PN_TRACE (tf, cmd);

    // dmcontrol
    PN_TRACE (tf, dmcontrol_hartsel);
    PN_TRACE (tf, dmcontrol_haltreq);
    PN_TRACE (tf, dmcontrol_active);
    PN_TRACE (tf, dmcontrol_ndmreset);

    // dmstatus
    PN_TRACE (tf, dmstatus_allhalted);
    PN_TRACE (tf, dmstatus_allresumeack);

    // haltsum0
    PN_TRACE (tf, haltsum);

    // DMI
    PN_TRACE (tf, dmi_adr_i);
    PN_TRACE (tf, dmi_dat_i);
    PN_TRACE (tf, dmi_dat_o);
    PN_TRACE (tf, dmi_rd);
    PN_TRACE (tf, dmi_wr);

    // WB
    PN_TRACE (tf, wb_ack_o);
}
#endif


void MDebugModule::TransitionMethod () {
    const sc_int<21> progbuf_jump = -DBG_PROGBUF_JUMP;
    sc_uint<32> cmd_var;
    sc_uint<8> cmd_type;
    sc_uint<7> cmd_size;
    sc_uint<16> cmd_regno;
    bool cmd_write, cmd_transfer, cmd_postexec;
    sc_uint<5> hartsel;
    sc_uint<CFG_NUT_CPU_CORES> haltsum_var, dbg_request_var;

    // Read input signals/ports
    cmd_var = cmd.read ();
    hartsel = dmcontrol_hartsel.read ();
    haltsum_var = haltsum.read ();

    // Command:
    cmd_type = cmd_var (31, 24);
    cmd_size = cmd_var (22, 20);
    cmd_regno = cmd_var (15, 0);
    cmd_write = cmd_var[16];
    cmd_transfer = cmd_var[17];
    cmd_postexec = cmd_var[18];

    // Towards EXU...
    for (unsigned int i = 0; i < CFG_NUT_CPU_CORES; i++)
        if (i == hartsel)
            dbg_request_var[i] = dmcontrol_haltreq.read ();
        else
            dbg_request_var[i] = 0;
    dbg_reset = dmcontrol_ndmreset.read ();
    dbg_request = dbg_request_var;

    // Preset control signals
    reg_sel = 0;
    reg_in = 0;
    reg_write = 0;
    flag_go = 0;
    abstracts_cmderr = reg_abstracts_cmderr.read ();
    abstracts_busy = reg_abstracts_busy.read ();

    dm_state_next = dm_state.read ();

    // State Transition
    switch (dm_state.read ()) {
    case Idle: // Idle: Wait for abstract command from host
        // Only accept a command if we did not encouter an error before
        if (command_written && reg_abstracts_cmderr.read () == CMDERR_NONE) {
            // Currently busy?
            if (reg_abstracts_busy)
                abstracts_cmderr = CMDERR_BUSY;
            else
                dm_state_next = CMD;
        }
        break;
    case CMD: // CMD:
        // Reset busy flag (might be set from earlier debug session)
        abstracts_busy = 0;
//        PN_INFOF(("DM Command:0x%08x = type: %d, size: %d, regno: 0x%04x, write: %d transfer: %d, postexec: %d",
//             (uint32_t) cmd_var, (uint32_t) cmd_type, (uint32_t) cmd_size, (uint32_t) cmd_regno, (uint32_t) cmd_write,
//             (uint32_t) cmd_transfer, (uint32_t) cmd_postexec));
        if (cmd_type == 0 && cmd_size == 2 && cmd_regno >= 0x1000 && cmd_regno <= 0x101f) {
            // Access Register Command with size 2
            if (haltsum_var[hartsel] == 0) {
                // PN_ERROR: Hart is not halted
                PN_WARNINGF (("DM: Selected hart is not halted!"));
                abstracts_cmderr = CMDERR_HALTRESUME;
                dm_state_next = Idle;
            } else {
                dm_state_next = CMD_ACCESSR;
            }
        } else {
            // PN_ERROR: cmd not supported
           PN_WARNINGF (("DM: Abstract Command of tpye %d with size %d not supported!",
                      cmd_type.value (), cmd_size.value ()));
            abstracts_cmderr = CMDERR_NOTSUP;
            dm_state_next = Idle;
        }
        break;
    case CMD_ACCESSR: // CMD_ACCESSR:
        // Set busy bit
        abstracts_busy = 1;
        // Write either LW or SW instruction to abstract0
        reg_sel = 0; // abstract0
        reg_write = 1;
        if (cmd_transfer) {
            if (cmd_write)
                // lw regno, data0(x0)
                reg_in = (sc_uint<12> (0), sc_uint<5> (0), sc_uint<3> (2), cmd_regno (4, 0),
                          sc_uint<7> (0x03));
            else
                // sw regno, data0(x0)
                reg_in = (sc_uint<7> (0), cmd_regno (4, 0), sc_uint<5> (0), sc_uint<3> (2),
                          sc_uint<5> (reg_data0 * 4), sc_uint<7> (0x23));
        } else {
            // nop (addi x0, x0, 0)
            reg_in = 0x13;
        }
        dm_state_next = CMD_POSTEXEC;
        break;
    case CMD_POSTEXEC: // CMD_POSTEXEC
        // Set busy bit
        abstracts_busy = 1;
        // Write either J or EBREAK instruction to abstract1
        reg_sel = 1; // abstract1
        reg_write = 1;
        if (cmd_postexec)
            // j r0, progbuf0
            reg_in = (progbuf_jump[20], progbuf_jump (10, 1), progbuf_jump[11],
                      progbuf_jump (19, 12), sc_uint<5> (0), sc_uint<7> (0x6F));
        else
            // ebreak
            reg_in = sc_uint<32> ((sc_uint<11> (1), sc_uint<13> (0), sc_uint<7> (0x73)));
        dm_state_next = CMD_GO;
        break;
    case CMD_GO: // CMD_GO:
        // Set busy bit
        abstracts_busy = 1;
        // Set go flag and go back to Idle
        flag_go = 1;
        dm_state_next = Idle;
        break;
    default:
        break;
    }

    // WB acknowlegde port (only active during stb & cyc)
    ack_o = wb_ack_o & stb_i & cyc_i;
}

void MDebugModule::RegisterMethod () {
#pragma HLS ARRAY_PARTITION variable = dm_regs complete dim = 1
#pragma HLS ARRAY_PARTITION variable = dm_flags complete dim = 1
#pragma HLS ARRAY_PARTITION variable = debug_rom complete dim = 1
#pragma HLS ARRAY_PARTITION variable = dmi_wr_last complete dim = 1
#pragma HLS ARRAY_PARTITION variable = dmi_rd_last complete dim = 1
#pragma HLS ARRAY_PARTITION variable = dmi_adr_last complete dim = 1
#pragma HLS ARRAY_PARTITION variable = dmi_dat_i_last complete dim = 1
    // DMI variables
    sc_uint<DTM_ADDR_WIDTH> dmi_adr;
    sc_uint<32> dmi_dat_in, dmi_dat_out;
    // WB variables
    sc_uint<32> wb_adr, wb_dat_i, wb_out;
    sc_uint<CFG_MEMU_BUSIF_WIDTH/8> wb_sel;
    sc_uint<11> wb_offset;
    sc_uint<3> wb_reg_offset;
    // Internal variables
    sc_uint<CFG_NUT_CPU_CORES> haltsum_var;
    sc_uint<MIN (MAX (CFG_NUT_CPU_CORES_LD, 1), 20)> hartsel, new_hartsel, wb_hartsel;
    sc_uint<20> hartsel_out;
    sc_uint<8> flags_var[CFG_NUT_CPU_CORES];
#pragma HLS ARRAY_PARTITION variable = flags_var complete dim = 1 // At 8 cores the flags_var will be mapped to BRAM without this pragma
    sc_uint<DBG_ABSTRACT_NUM_LD> regSel_var;
    sc_uint<32> regIn_var;
    bool reg_abstracts_busy_var;
    sc_uint<3> reg_abstracts_cmderr_var;

    // Reset Area
    
    for (unsigned int n = 0; n < DBG_NUM_REGISTERS; n++) dm_regs[n] = 0;
    for (unsigned int n = 0; n < DBG_FLAG_SIZE; n++) dm_flags[n] = 0;

    command_written = 0;
    cmd = 0;
    haltsum = 0;

    dmcontrol_hartsel = 0;
    dmcontrol_haltreq = 0;
    dmcontrol_active = 0;
    dmcontrol_ndmreset = 0;

    dmstatus_allhalted = 0;
    dmstatus_allresumeack = 0;

    abstractauto_autoexecdata = 0;

    reg_abstracts_cmderr = 0;
    reg_abstracts_busy = 0;

    for (int n = 0; n < REG_RD_WR_STAGES; n++) {
        dmi_wr_last[n] = 0;
        dmi_rd_last[n] = 0;
    }

    for (int n = 0; n < REG_STAGES; n++) {
        dmi_adr_last[n] = 0;
        dmi_dat_i_last[n] = 0;
    }

    dm_state = Idle;

    // Reset output port registers
    dat_o = 0x0;
    wb_ack_o = 0;
    rty_o = 0;
    err_o = 0;

    dmi_dat_o = 0x0;

    wait();
    
    while(true){   // infinity loop 
            // Read input signals
            // DMI
            dmi_adr = dmi_adr_last[REG_ID_LAST].read ();
            dmi_dat_in = dmi_dat_i_last[REG_ID_LAST].read ();
            // WB
            wb_sel = sel_i.read ();
    #if CFG_MEMU_BUSIF_WIDTH == 64
            // This implementation assumes that either top or bottom 32 bits are read/written never both!
            bool top_word = wb_sel.range(7, 4).or_reduce();
            wb_dat_i = top_word ? dat_i.read().range(63, 32) : dat_i.read().range(31, 0);
            wb_adr = top_word ? adr_i.read() + 4 : adr_i.read();
    #else
            wb_dat_i = dat_i.read ();
            wb_adr = adr_i.read ();
    #endif
            hartsel = dmcontrol_hartsel.read ();
            regSel_var = reg_sel.read ();
            regIn_var = reg_in.read ();
            if (CFG_NUT_CPU_CORES_LD == 0)
                new_hartsel = 0; // Hartsel is fixed to 0 if CFG_NUT_CPU_CORES_LD == 0
            else
                new_hartsel = (sc_uint<MIN (MAX (CFG_NUT_CPU_CORES_LD, 1), 20)>)(dmi_dat_in (15, 6),
                                                                                dmi_dat_in (25, 16));
            haltsum_var = haltsum.read ();

            for (unsigned int i = 0; i < CFG_NUT_CPU_CORES; i++) {
                flags_var[i] = dm_flags[i].read ();
            }
            reg_abstracts_busy_var = reg_abstracts_busy.read ();
            reg_abstracts_cmderr_var = reg_abstracts_cmderr.read ();


            // Preset output ports/variables
            dmi_dat_out = 0;
            hartsel_out = hartsel;

            wb_hartsel = wb_dat_i;
            wb_offset = wb_adr (10, 0); // implementation specific
            wb_reg_offset = wb_offset (4, 2); // implementation specific
            wb_out = 0;

            wb_ack_o = 0;
            err_o = 0;
            rty_o = 0;

            command_written = 0;

            // Preset cmderr and busy flags
            reg_abstracts_cmderr = abstracts_cmderr.read ();
            reg_abstracts_busy = abstracts_busy.read ();

            // Write go flag into flags register
            if (flag_go) flags_var[hartsel] = 1;
            // Handle DMI Read/Write
            if (dmi_rd_last[1] & !dmi_rd_last[2]) {
                // READ ACCESS
                switch (dmi_adr) {
                case data0:
                    if (reg_abstracts_busy_var) {
                        // Read to data0 during abstract command sets abstract_cmderr to CMDERR_BUSY
                        reg_abstracts_cmderr = CMDERR_BUSY;
                        dmi_dat_out = -1;
                    } else {
                        // If not busy and abstractauto_autoexecdata is set, set command_written
                        command_written = abstractauto_autoexecdata.read ();
                        dmi_dat_out = dm_regs[reg_data0].read ();
                    }
                    break;
                case dmcontrol:
                    dmi_dat_out = (dmcontrol_haltreq.read (),
                                sc_uint<5> (0),
                                hartsel_out (9, 0),
                                hartsel_out (19, 10),
                                sc_uint<4> (0),
                                dmcontrol_ndmreset.read (),
                                dmcontrol_active.read ());
                    break;
                case dmstatus:
                    dmi_dat_out = (sc_uint<14> (0),
                                dmstatus_allresumeack.read (),
                                dmstatus_allresumeack.read (),
                                sc_uint<4> (0), // no CPU should be nonexistant/unavailable
                                !dmstatus_allhalted.read (),
                                !dmstatus_allhalted.read (),
                                dmstatus_allhalted.read (),
                                dmstatus_allhalted.read (),
                                sc_uint<1> (1), // authenticated
                                sc_uint<3> (0),
                                sc_uint<4> (2)); // Version 1.3
                    break;
                case abstracts:
                    dmi_dat_out = (sc_uint<3> (0),
                                sc_uint<5> (DBG_NUM_PROGBUF),
                                sc_uint<11> (0),
                                reg_abstracts_busy_var,
                                sc_uint<1> (0),
                                reg_abstracts_cmderr_var,
                                sc_uint<4> (0),
                                sc_uint<4> (DBG_NUM_DATA));
                    break;
                case progbuf0:
                    dmi_dat_out = dm_regs[reg_progbuf0].read ();
                    break;
                case progbuf1:
                    dmi_dat_out = dm_regs[reg_progbuf1].read ();
                    break;
                case progbuf2:
                    dmi_dat_out = dm_regs[reg_progbuf2].read ();
                    break;
                case haltsum0:
                    dmi_dat_out = haltsum.read ();
                    break;
                default:
                    dmi_dat_out = 0x0;
                    PN_WARNINGF (("DM DMI READ to unknown address: (0x%08x)", (TWord)dmi_adr));
                    break;
                }
                dmi_dat_o = dmi_dat_out;
            } else if (dmi_wr_last[1] & !dmi_wr_last[2]) {
                // DMI WRITE ACCESS
                switch (dmi_adr) {
                case data0:
                    if (reg_abstracts_busy_var) {
                        // Write to data0 during abstract command sets abstract_cmderr to CMDERR_BUSY
                        reg_abstracts_cmderr = CMDERR_BUSY;
                    } else {
                        dm_regs[reg_data0] = dmi_dat_in;
                        // If abstractauto_autoexecdata is set write to data0 triggers command_execution
                        command_written = abstractauto_autoexecdata.read ();
                    }
                    break;
                case dmcontrol:
                    dmcontrol_hartsel = new_hartsel;
                    dmcontrol_active = dmi_dat_in[0];
                    dmcontrol_ndmreset = dmi_dat_in[1];
                    dmcontrol_haltreq = dmi_dat_in[31];
                    // Reset resumeack on haltreq
                    if (dmi_dat_in[31]) dmstatus_allresumeack = 0;
                    // Set resume flag if bit resumereq is set and haltreq == 0
                    if (dmi_dat_in[30] && !dmi_dat_in[31]) {
                        flags_var[new_hartsel] = 1U << 1;
                        dmstatus_allresumeack = 0;
                    }
    //                PN_INFOF (("   (%s)  DMCONTROL: hartsel: %d, active: %d, ndmreset: %d, haltreq: %d",
    //                        strrchr (name (), '.') + 1, new_hartsel.value (), (bool)dmi_dat[0],
    //                        (bool)dmi_dat[1], (bool)dmi_dat[31]));
                    break;
                case abstracts:
                    // Clear cmderr flags
                    reg_abstracts_cmderr = reg_abstracts_cmderr_var & ~(__uint8_t) (dmi_dat_in (10, 8));
                    break;
                case command:
                    cmd = dmi_dat_in;
                    command_written = 1;
                    break;
                case abstractauto:
                    abstractauto_autoexecdata = dmi_dat_in[0];
                    break;
                case progbuf0:
                    dm_regs[reg_progbuf0] = dmi_dat_in;
                    break;
                case progbuf1:
                    dm_regs[reg_progbuf1] = dmi_dat_in;
                    break;
                case progbuf2:
                    dm_regs[reg_progbuf2] = dmi_dat_in;
                    break;
                default:
                    // Nothing
                    PN_WARNINGF (("DM DMI WRITE to unknown address: (0x%08x):=0x%08x", (TWord)dmi_adr, (TWord)dmi_dat_in));
                    break;
                }
            } else if (stb_i == 1 && cyc_i == 1 && dm_state.read () == Idle) {
                // Wishbone Read/Write (only allowed if DM is in idle state)
                if (IsAdressed (wb_adr)) {
    #ifndef __SYNTHESIS__
                    // Warn if the debug mode is not enabled and the DMs memory is accessed by the processor.
                    // -> This should not happen and is probably an error in the software that is being executed
                    if (!pn_cfg_debug_mode)
                        PN_WARNINGF (("DM: Debug Module memory accessed while not running in debug mode: %s to 0x%08x", we_i.read () ? "write" : "read", (TWord)wb_adr));
    #endif
                    // Handle wishbone read/write
                    if (we_i) { // WRITE ACCESS
                        // DBG REG & OTHER
                        switch (wb_offset) {
                        case 0x0: // data0 address from WB is 0x0!
                            dm_regs[reg_data0] = wb_dat_i;
                            break;
                        case DBG_HALTED_OFFSET:
                            // Set halted bit in haltsum
                            haltsum_var[wb_dat_i.range(CFG_NUT_CPU_CORES-1, 0)] = 1;
                            // If this CPU is selected and has no GO flag pending abstratcs_busy flag can be reset
                            if (wb_hartsel == hartsel) {
                                if (flags_var[wb_hartsel][0] == 0) reg_abstracts_busy = 0;
                            }
                            break;
                        case DBG_GOING_OFFSET:
                            // EXU is going
                            flags_var[hartsel] = 0; // Reset flags for this EXU
                            break;
                        case DBG_RESUMING_OFFSET:
                            // EXU is resuming
                            flags_var[hartsel] = 0;  // Reset flags for this EXU
                            haltsum_var[hartsel] = 0; // Reset haltsum
                            dmstatus_allresumeack = 1; // Acknowledge the resume
                            break;
                        case DBG_EXCEPTION_OFFSET:
                            // EXU encountered an exception during execution of abstract commands or progbuf
                            reg_abstracts_cmderr = CMDERR_EXCEPTION;
                            break;
                        default:
                            PN_WARNINGF (("DM WISHBONE WRITE to write only or unknown address: (0x%08x):=0x%08x, "
                                    "reg_offset: (0x%x) ",
                                    (TWord)wb_adr, (TWord)wb_dat_i, (TWord)wb_offset));
                            break;
                        }
                    } else { // READ ACCESS
                        if (wb_offset >= DBG_MEMORY_OFFSET && wb_offset <= DBG_MEMORY_OFFSET + DBG_MEMORY_SIZE) {
                            // DBG ROM:
                            wb_out = debug_rom[wb_offset (6, 2)];
                        } else if (wb_offset >= DBG_FLAG_OFFSET && wb_offset <= DBG_FLAG_OFFSET + DBG_FLAG_SIZE) {
                            wb_out = (sc_uint<24> (0), dm_flags[(wb_offset ^ DBG_FLAG_OFFSET)].read ()); // << 24;
    //                        PN_INFOF(("DM FLAG READ: (0x%x) = 0x%08x", (TWord)offset, out));
                        } else if (wb_offset >= DBG_ABSTRACT_OFFSET &&
                                wb_offset <= DBG_ABSTRACT_OFFSET + DBG_ABSTRACT_NUM * 4) {
                            wb_out = dm_regs[DBG_NUM_DATA + DBG_NUM_PROGBUF + wb_reg_offset].read ();
    //                        PN_INFOF(("DM ABSTRACT READ: (0x%x) = 0x%08x", (TWord)reg_offset, out));
                        } else if (wb_offset == DBG_ABSTRACT_OFFSET + 0xc) {
                            ; // workaround to suppress warning caused by instruction buffer prefetching non-existent address
                        } else if (wb_offset < DBG_NUM_REGISTERS * 4) {
                            wb_out = dm_regs[wb_reg_offset].read ();
    //                        PN_INFOF(("DM REG READ: (0x%x) = 0x%08x", (TWord)reg_offset, out));
                        } else {
                            PN_WARNINGF (("DM WISHBONE READ to unknown address: (0x%x), returning 0x0", (TWord)wb_adr));
                        }
                    }
                
    #if CFG_MEMU_BUSIF_WIDTH == 64
                    dat_o = top_word ? (wb_out, sc_uint<32>(0)) : (sc_uint<32>(0), wb_out);
    #else
                    dat_o = wb_out;
    #endif
                    wb_ack_o = 1;
                }
            }

            // Writeback
            for (unsigned int i = 0; i < CFG_NUT_CPU_CORES; i++) {
                dm_flags[i] = flags_var[i];
            }

            // Write regFile from TransitionMethod
            if (reg_write) {
                switch (regSel_var) {
                case 0:
                    dm_regs[reg_abstract0] = regIn_var;
                    break;
                case 1:
                    dm_regs[reg_abstract1] = regIn_var;
                    break;
                default:
                    PN_WARNINGF (("DM ABSTRACT WRITE to unknown register address: (0x%x) = 0x%x",
                            (TWord)regSel_var, (TWord)reg_in.read ()));
                    break;
                }
            }
            dmstatus_allhalted = haltsum_var[hartsel];
            haltsum = haltsum_var;

            // Register stages for change of clock domain between JTAG and DM
            for (int i = REG_RD_WR_STAGES - 1; i > 0; i--) {
                dmi_wr_last[i] = dmi_wr_last[i - 1].read ();
                dmi_rd_last[i] = dmi_rd_last[i - 1].read ();
            }
            dmi_wr_last[0] = dmi_wr.read ();
            dmi_rd_last[0] = dmi_rd.read ();
            dmi_dat_i_last[1] = dmi_dat_i_last[0].read ();
            dmi_dat_i_last[0] = dmi_dat_i.read ();
            dmi_adr_last[1] = dmi_adr_last[0].read ();
            dmi_adr_last[0] = dmi_adr_i.read ();

            // State Transition
            dm_state = dm_state_next.read ();
            wait();
    }
}

