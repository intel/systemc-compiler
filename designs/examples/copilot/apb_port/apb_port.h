/******************************************************************************
 * Copyright (c) 2025, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

// Incorrect example -- needs to be fixed!!!

// APB subordinate port translating APB transfers into sct_target (request)
// and sct_initiator (response). Follows repository SystemC coding rules.
//
// Template parameters:
//   ADDR_W  - address width of PADDR
//   DATA_W  - data width of PWDATA/PRDATA (multiple of 8)
//
// Protocol (simplified):
//   Setup cycle: PSEL=1, PENABLE=0 with stable PADDR/PWRITE/PWDATA/PSTRB.
//   Enable cycle: next cycle PENABLE=1 (PSEL must stay 1). Transfer occurs
//                  when PREADY=1. Reads produce PRDATA, writes accept PWDATA.
//   This module issues a request to target on rising edge entering enable
//   phase. Response (read data) is returned through initiator.
//
// Channels usage:
//   req_target  : sct_target<Req>  (design receives APB request)
//   resp_init   : sct_initiator<Resp> (design sends read response)
//
// Request record fields:
//   addr  : address
//   write : bool write (1) or read (0)
//   wdata : write data
//   wstrb : byte lane strobes
//
// Response record fields:
//   rdata : read data
//   slverr: transfer error flag
//
// Processes:
//   apb_fsm : SC_THREAD with async reset, implements APB handshake and
//             channel transactions.
//   comb_out : SC_METHOD drives outputs PREADY/PRDATA/PSLVERR combinationally
//              from state and stored response.
//   const_assign : SC_METHOD without sensitivity for constant signal drives.
//
// Notes:
//   - Only one outstanding transaction supported (APB is non-pipelined).
//   - PREADY deasserts until request accepted & (for reads) response arrived.
//   - Writes complete when request accepted (response channel not used).
//
// Limitations:
//   - PSLVERR always 0 (no error generation) unless response sets it.
//
#ifndef COPILOT_APB_PORT_H
#define COPILOT_APB_PORT_H

#include <systemc.h>
#include "sct_common.h"
#include "sct_target.h"
#include "sct_initiator.h"

template<unsigned ADDR_W, unsigned DATA_W>
struct apb_port : public sc_module {
	// APB clock & active-low reset
	sc_in_clk        PCLK{"PCLK"};
	sc_in<bool>      PRESETn{"PRESETn"};

	// APB inputs
	sc_in< sc_uint<ADDR_W> >   PADDR{"PADDR"};
	sc_in<bool>                PSEL{"PSEL"};
	sc_in<bool>                PENABLE{"PENABLE"};
	sc_in<bool>                PWRITE{"PWRITE"};
	sc_in< sc_uint<DATA_W> >   PWDATA{"PWDATA"};
	sc_in< sc_uint<DATA_W/8> > PSTRB{"PSTRB"};

	// APB outputs
	sc_out<bool>               PREADY{"PREADY"};
	sc_out< sc_uint<DATA_W> >  PRDATA{"PRDATA"};
	sc_out<bool>               PSLVERR{"PSLVERR"};

	// Channel request/response records
	struct Req {
		sc_uint<ADDR_W> addr;
		bool            write;
		sc_uint<DATA_W> wdata;
		sc_uint<DATA_W/8> wstrb;
		bool operator==(const Req& o) const {
			return addr==o.addr && write==o.write && wdata==o.wdata && wstrb==o.wstrb;
		}
        inline friend std::ostream& operator<< (std::ostream& os, const Req &obj)
        {return os;}
        inline friend void sc_trace(sc_trace_file*& f, const Req& val, std::string name) 
        {}
	};
	struct Resp {
		sc_uint<DATA_W> rdata;
		bool            slverr;
		bool operator==(const Resp& o) const {
			return rdata==o.rdata && slverr==o.slverr;
		}
		inline friend std::ostream& operator<< (std::ostream& os, const Resp &obj)
        {return os;}
        inline friend void sc_trace(sc_trace_file*& f, const Resp& val, std::string name) 
        {}
	};

	// Target (accept requests), Initiator (provide responses)
	sct_target<Req>      req_target{"req_target"};
	sct_initiator<Resp>  resp_init{"resp_init"};

	// Internal signals/state
	sc_signal<bool>      busy{"busy"};
	sc_signal<bool>      read_pending{"read_pending"};
	sc_signal< sc_uint<DATA_W> > rdata_reg{"rdata_reg"};
	sc_signal<bool>      err_reg{"err_reg"};
	sc_signal<bool>      ready_int{"ready_int"};

	// FSM local enum encoded in signals
	sc_signal<bool>      setup_phase{"setup_phase"};
	sc_signal<bool>      enable_phase{"enable_phase"};

	SC_HAS_PROCESS(apb_port);
	explicit apb_port(const sc_module_name& name) : sc_module(name) {
		req_target.clk_nrst(PCLK, PRESETn);
		resp_init.clk_nrst(PCLK, PRESETn);

		SC_THREAD(apb_fsm);
		async_reset_signal_is(PRESETn, false);

		SC_METHOD(comb_out);
		sensitive << busy << read_pending << rdata_reg << err_reg << enable_phase;
		// Inputs that may affect ready: PSEL/PENABLE detected only inside thread

		SC_METHOD(const_assign);
	}

	void const_assign() {
		// constant / default outputs if any (none here)
	}

	void apb_fsm() {
		// Reset
		busy = 0;
		read_pending = 0;
		rdata_reg = 0;
		err_reg = 0;
		ready_int = 0;
		setup_phase = 0;
		enable_phase = 0;
		PREADY = 0;
		PRDATA = 0;
		PSLVERR = 0;
		wait();
		while (true) {
			// Default ready low until decided
			ready_int = 0;

			bool sel  = PSEL.read();
			bool en   = PENABLE.read();
			bool wr   = PWRITE.read();

			if (!busy.read()) {
				// Look for setup phase (PSEL=1, PENABLE=0)
				if (sel && !en) {
					setup_phase = 1;
					enable_phase = 0;
				}
				// Transition to enable phase and issue request
				if (sel && en && setup_phase.read()) {
					enable_phase = 1;
					setup_phase = 0;
					Req rq; rq.addr = PADDR.read(); rq.write = wr; rq.wdata = PWDATA.read(); rq.wstrb = PSTRB.read();
					if (rq.write) {
						if (req_target.ready()) { req_target.put(rq); busy = 1; read_pending = 0; }
					} else {
						if (req_target.ready()) { req_target.put(rq); busy = 1; read_pending = 1; }
					}
				}
			} else {
				// Busy: for write complete immediately; for read wait resp
				if (read_pending.read()) {
					if (resp_init.request()) {
						Resp rp = resp_init.get();
						rdata_reg = rp.rdata;
						err_reg = rp.slverr;
						ready_int = 1; // present data now
						busy = 0;
						read_pending = 0;
					}
				} else {
					// Write completed after enable cycle -> ready
					ready_int = 1;
					busy = 0;
				}
			}

			wait();
		}
	}

	void comb_out() {
		// PREADY high when ready_int asserted in that cycle during enable phase
		bool en = enable_phase.read();
		PREADY = (ready_int.read() && en) ? 1 : 0;
		PRDATA = rdata_reg.read();
		PSLVERR = err_reg.read();
	}
};

#endif // COPILOT_APB_PORT_H
