/*************************************************************************

  This file is part of the ParaNut project.

  Copyright (C) 2019 Alexander Bahle <alexander.bahle@hs-augsburg.de>
                2022 Marco Milenkovic <marco.milenkovic@hs-augsburg.de>
      Hochschule Augsburg, University of Applied Sciences

  Description:
    This is a SystemC model of a Debug Module compatible with the
    RISC-V External Debug Support Version 0.13

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


#ifndef _DM_
#define _DM_

#include "base.h"
#include "paranut-config.h"

#include <systemc.h>

// **************** Defines *************
#define DTM_ADDR_WIDTH      6
#define DTM_IR_WIDTH      5

#define DBG_CMD_WIDTH       32
#define DBG_ADDRESS         0x00000000

#define DBG_REG_OFFSET      0x0
#define DBG_REG_SIZE        0x40
#define DBG_NUM_DATA        1U          // Number of 32 bit registers
#define DBG_NUM_PROGBUF     3           // Number of 32 bit registers

#define DBG_FLAG_OFFSET     0x100
#define DBG_FLAG_SIZE       CFG_NUT_CPU_CORES // One byte per CPU
#define DBG_FLAG_REG_OFFSET 0x10

#define DBG_ABSTRACT_OFFSET 0x200
#define DBG_ABSTRACT_REG_OFFSET (DBG_NUM_DATA + DBG_NUM_PROGBUF)
#define DBG_ABSTRACT_NUM_LD 1U
#define DBG_ABSTRACT_NUM    (1U << DBG_ABSTRACT_NUM_LD)
#define DBG_PROGBUF_JUMP    (DBG_ABSTRACT_OFFSET + (DBG_ABSTRACT_NUM-1)*4 - DBG_NUM_DATA*4)

#define DBG_HALTED_OFFSET   0x300
#define DBG_GOING_OFFSET    0x304
#define DBG_RESUMING_OFFSET 0x308
#define DBG_EXCEPTION_OFFSET 0x30C

#define DBG_MEMORY_OFFSET   0x400
#define DBG_MEMORY_SIZE     0x54
#define DBG_ROM_ADDRESS     (DBG_ADDRESS + DBG_MEMORY_OFFSET)

#define DBG_NUM_REGISTERS   (DBG_NUM_DATA + DBG_NUM_PROGBUF + DBG_ABSTRACT_NUM )
#define DBG_NUM_REGISTES_BITS 3 // log2(DBG_NUM_REGISTERS)

#define REG_RD_WR_STAGES 3
#define REG_STAGES 2
#define REG_ID_LAST 1

// **************** DM Register Addresses *************
typedef enum {
    data0 = 0x4,

    dmcontrol = 0x10,
    dmstatus = 0x11,

    abstracts = 0x16,
    command = 0x17,
    abstractauto = 0x18,

    progbuf0 = 0x20,
    progbuf1,
    progbuf2,
    progbuf15 = 0x2f,

    haltsum0 = 0x40,
} EDMRegAddress;

typedef enum {
    reg_data0,

    reg_progbuf0,
    reg_progbuf1,
    reg_progbuf2,

    reg_abstract0,
    reg_abstract1,
} EDMRegs;

// **************** DM Command Error Codes *************
typedef enum {
    CMDERR_NONE = 0,
    CMDERR_BUSY = 1,
    CMDERR_NOTSUP = 2,
    CMDERR_EXCEPTION = 3,
    CMDERR_HALTRESUME = 4,
    CMDERR_OTHER = 7,
} ECMDErr;

// **************** DM States *************
typedef enum {
    Idle,
    CMD,
    CMD_ACCESSR,
    CMD_POSTEXEC,
    CMD_GO,
} EDMState;


// **************** MDebugModule *************
class MDebugModule : ::sc_core::sc_module {
public:

    // Ports (WISHBONE slave)...
    sc_in_clk             clk_i{"clk_i"};     // clock input
    sc_in<bool>           rst_i{"rst_i"};     // reset

    sc_in<bool>           stb_i{"stb_i"};     // strobe input
    sc_in<bool>           cyc_i{"cyc_i"};     // cycle valid input
    sc_in<bool>           we_i{"we_i"};      // indicates write transfer
    sc_in<sc_uint<CFG_MEMU_BUSIF_WIDTH/8> >    sel_i{"sel_i"};     // byte select inputs
    sc_out<bool>          ack_o{"ack_o"};     // normal termination
    sc_out<bool>          err_o{"err_o"};     // termination w/ error
    sc_out<bool>          rty_o{"rty_o"};     // termination w/ retry

    sc_in<sc_uint<32> >   adr_i{"adr_i"};     // address bus inputs
    sc_in<sc_uint<CFG_MEMU_BUSIF_WIDTH> >          dat_i{"dat_i"};     // input data bus
    sc_out<sc_uint<CFG_MEMU_BUSIF_WIDTH> >         dat_o{"dat_o"};     // output data bus

    //   to EXU
    // Debug request: EXUs jump to DBG_ROM_ADDRESS upon handling this signal
    sc_out<sc_uint<CFG_NUT_CPU_CORES> >   dbg_request{"dbg_request"};
    // Debug reset: Gets set through the ndmreset bit of dmcontrol register and
    //    resets all modules (except DM and DTM)
    sc_out<bool>                          dbg_reset{"dbg_reset"};

    //   from DTM
    sc_in<sc_uint<DTM_ADDR_WIDTH> >   dmi_adr_i{"dmi_adr_i"};
    sc_in<sc_uint<32> >               dmi_dat_i{"dmi_dat_i"};
    sc_out<sc_uint<32> >              dmi_dat_o{"dmi_dat_o"};
    sc_in<bool>                       dmi_rd{"dmi_rd"},
                                    dmi_wr{"dmi_wr"};

    // Constructor...
    SC_HAS_PROCESS (MDebugModule);
    MDebugModule (sc_module_name name)
        : sc_module (name) {
        SC_METHOD (TransitionMethod);
            sensitive << dm_state << command_written
                      << reg_abstracts_busy << reg_abstracts_cmderr
                      << dmcontrol_haltreq << dmcontrol_hartsel << dmcontrol_ndmreset
                      << haltsum << cmd
                      << wb_ack_o << stb_i << cyc_i;

        SC_CTHREAD (RegisterMethod, clk_i.pos());
            async_reset_signal_is(rst_i, true);
            // sensitive << clk_i.pos ();
    }


    // Functions...
    void Trace (sc_trace_file *tf, int levels = 1);
    static inline bool IsAdressed (TWord adr) { return (adr & 0xffff0000) == DBG_ADDRESS; }

#ifndef __SYNTHESIS__
    void SetNdmreset(bool val) { dmcontrol_ndmreset = val; }
    void SetHaltreq(bool val) { dmcontrol_haltreq = val; }
#endif

    // Processes...
    void TransitionMethod ();
    void RegisterMethod ();

protected:
    // Registers ...
    sc_signal<sc_uint<3> >
          dm_state,
          dm_state_next;
    sc_signal<bool>
            dmi_wr_last[REG_RD_WR_STAGES],
            dmi_rd_last[REG_RD_WR_STAGES];
    sc_signal<sc_uint<32> > dmi_dat_i_last[REG_STAGES];
    sc_signal<sc_uint<DTM_ADDR_WIDTH> > dmi_adr_last[REG_STAGES];

    sc_signal<sc_uint<32> > dm_regs[DBG_NUM_REGISTERS];
    sc_signal<sc_uint<8> > dm_flags[CFG_NUT_CPU_CORES];

    // dmcontrol:
    sc_signal<sc_uint<MIN(MAX(CFG_NUT_CPU_CORES_LD, 1), 20)> > dmcontrol_hartsel;
    sc_signal<bool>
          dmcontrol_haltreq,
          dmcontrol_active,
          dmcontrol_ndmreset;

    // dmstatus:
    sc_signal<bool>
          dmstatus_allhalted,
          dmstatus_allresumeack;

    // abstracts:
    sc_signal<bool>
          abstracts_busy,
          reg_abstracts_busy;
    sc_signal<sc_uint<3> >
          abstracts_cmderr,
          reg_abstracts_cmderr;

    // abstractauto
    sc_signal<bool> abstractauto_autoexecdata;

    // haltsum0: // todo: add measures for more than 32 cores
    sc_signal<sc_uint<CFG_NUT_CPU_CORES> > haltsum;

    // commmand:
    sc_signal<sc_uint<32> > cmd;


    // Internal signals ...
    sc_signal<bool>
          command_written,
          flag_go,
          wb_ack_o;
    sc_signal<sc_uint<DBG_ABSTRACT_NUM_LD> > reg_sel;
    sc_signal<bool> reg_write;
    sc_signal<sc_uint<32> > reg_in;

};


#endif
