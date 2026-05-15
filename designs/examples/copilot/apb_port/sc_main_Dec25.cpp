/******************************************************************************
 * Copyright (c) 2025, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

 #include "apb_port.h"
#include <systemc.h>

// Lightweight scoreboard/response provider
template<unsigned ADDR_W, unsigned DATA_W>
struct apb_scoreboard : sc_module {
	sct_target<typename apb_port<ADDR_W,DATA_W>::Req>   req_target{"req_target"};
	sct_initiator<typename apb_port<ADDR_W,DATA_W>::Resp> resp_init{"resp_init"};

	static const unsigned DEPTH = 256;
	sc_uint<DATA_W> mem[DEPTH];

	SC_HAS_PROCESS(apb_scoreboard);
	explicit apb_scoreboard(const sc_module_name& name) : sc_module(name) {
		req_target.clk_nrst(clk, nrst);
		resp_init.clk_nrst(clk, nrst);
		SC_THREAD(proc);
		async_reset_signal_is(nrst, false);
	}

	sc_in_clk clk{"clk"};
	sc_in<bool> nrst{"nrst"};

	void proc() {
		for (unsigned i=0;i<DEPTH;i++) mem[i]=0;
		wait();
		while(true) {
			if (req_target.request()) {
				auto rq = req_target.get();
				unsigned idx = rq.addr & (DEPTH-1);
				if (rq.write) {
					sc_uint<DATA_W> data = mem[idx];
					for (unsigned b=0;b<DATA_W/8;b++) if (rq.wstrb[b]) {
						data.range(b*8+7, b*8) = rq.wdata.range(b*8+7, b*8);
					}
					mem[idx] = data;
				} else {
					typename apb_port<ADDR_W,DATA_W>::Resp rp; rp.slverr=false; rp.rdata=mem[idx];
					if (resp_init.ready()) resp_init.put(rp);
				}
			}
			wait();
		}
	}
};

// Testbench driving APB cycles
SC_MODULE(tb) {
	static const unsigned ADDR_W = 8;
	static const unsigned DATA_W = 32;

	sc_clock        PCLK{"PCLK", 10, SC_NS};
	sc_in<bool>		SC_NAMED(clk);
	sc_signal<bool> PRESETn{"PRESETn"};
	sc_signal< sc_uint<ADDR_W> >  PADDR{"PADDR"};
	sc_signal<bool> PSEL{"PSEL"};
	sc_signal<bool> PENABLE{"PENABLE"};
	sc_signal<bool> PWRITE{"PWRITE"};
	sc_signal< sc_uint<DATA_W> >  PWDATA{"PWDATA"};
	sc_signal< sc_uint<DATA_W/8> > PSTRB{"PSTRB"};
	sc_signal<bool> PREADY{"PREADY"};
	sc_signal< sc_uint<DATA_W> > PRDATA{"PRDATA"};
	sc_signal<bool> PSLVERR{"PSLVERR"};

	apb_port<ADDR_W,DATA_W> dut{"dut"};
	apb_scoreboard<ADDR_W,DATA_W> scb{"scb"};

	SC_CTOR(tb) {
		clk(PCLK);
		dut.PCLK(PCLK); dut.PRESETn(PRESETn);
		dut.PADDR(PADDR); dut.PSEL(PSEL); dut.PENABLE(PENABLE); dut.PWRITE(PWRITE);
		dut.PWDATA(PWDATA); dut.PSTRB(PSTRB);
		dut.PREADY(PREADY); dut.PRDATA(PRDATA); dut.PSLVERR(PSLVERR);

		scb.clk(PCLK); scb.nrst(PRESETn);

		// Bind channels: design initiator -> scoreboard target; scoreboard initiator -> design target
		dut.resp_init.bind(scb.req_target);
		scb.resp_init.bind(dut.req_target);

		SC_CTHREAD(stimulus, clk.pos());
		async_reset_signal_is(PRESETn, false);
	}

	void apb_setup(bool write, sc_uint<ADDR_W> addr, sc_uint<DATA_W> data, sc_uint<DATA_W/8> strb) {
		PADDR = addr; PWRITE = write; PWDATA = data; PSTRB = strb; PSEL = 1; PENABLE = 0; wait();
		PENABLE = 1; wait();
		while(!PREADY.read()) wait();
		// Complete
		PSEL = 0; PENABLE = 0; wait();
	}

	void stimulus() {
		// Reset
		PSEL=0; PENABLE=0; PWRITE=0; PADDR=0; PWDATA=0; PSTRB=0; PRESETn=0;
		wait(5); PRESETn=1; wait();

		// Write some data
		apb_setup(true, 0x04, 0xAABBCCDD, 0xF);
		apb_setup(true, 0x08, 0x11223344, 0xF);

		// Read back
		apb_setup(false, 0x04, 0, 0);
		sc_assert(PRDATA.read() == 0xAABBCCDD);
		apb_setup(false, 0x08, 0, 0);
		sc_assert(PRDATA.read() == 0x11223344);

		sc_stop();
	}
};

int sc_main(int argc, char* argv[]) {
	tb tbt{"tbt"};
	sc_start();
	return 0;
}
