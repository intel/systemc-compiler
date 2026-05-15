/******************************************************************************
 * Copyright (c) 2025, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

#include "apb_port.h"
#include <systemc.h>

SC_MODULE(tb) {
    static const unsigned ADDR_W = 8;
    static const unsigned DATA_W = 32;
    static const unsigned STRB_W = DATA_W / 8;

    using Dut = apb_port<ADDR_W, DATA_W>;
    using Req = typename Dut::Req;
    using Resp = typename Dut::Resp;

    sc_in_clk PCLK{"PCLK"};
    sc_signal<bool> PRESETn{"PRESETn"};
    sc_signal<sct_uint<ADDR_W>> PADDR{"PADDR"};
    sc_signal<bool> PSEL{"PSEL"};
    sc_signal<bool> PENABLE{"PENABLE"};
    sc_signal<bool> PWRITE{"PWRITE"};
    sc_signal<sct_uint<DATA_W>> PWDATA{"PWDATA"};
    sc_signal<sct_uint<STRB_W>> PSTRB{"PSTRB"};
    sc_signal<bool> PREADY{"PREADY"};
    sc_signal<sct_uint<DATA_W>> PRDATA{"PRDATA"};
    sc_signal<bool> PSLVERR{"PSLVERR"};

    Dut dut{"dut"};
    sct_target<Req> req_target{"req_target"};
    sct_initiator<Resp> resp_init{"resp_init"};

    SC_CTOR(tb) {
        dut.PCLK(PCLK);
        dut.PRESETn(PRESETn);
        dut.PADDR(PADDR);
        dut.PSEL(PSEL);
        dut.PENABLE(PENABLE);
        dut.PWRITE(PWRITE);
        dut.PWDATA(PWDATA);
        dut.PSTRB(PSTRB);
        dut.PREADY(PREADY);
        dut.PRDATA(PRDATA);
        dut.PSLVERR(PSLVERR);

        req_target.clk_nrst(PCLK, PRESETn);
        resp_init.clk_nrst(PCLK, PRESETn);

        dut.req_init.bind(req_target);
        resp_init.bind(dut.resp_target);

        SC_THREAD(stimulus_thread);
        req_target.addTo(sensitive);
        resp_init.addTo(sensitive);
    }

    void trace(sc_trace_file* file) {
        sc_trace(file, PRESETn, "PRESETn");
        sc_trace(file, PADDR, "PADDR");
        sc_trace(file, PSEL, "PSEL");
        sc_trace(file, PENABLE, "PENABLE");
        sc_trace(file, PWRITE, "PWRITE");
        sc_trace(file, PWDATA, "PWDATA");
        sc_trace(file, PSTRB, "PSTRB");
        sc_trace(file, PREADY, "PREADY");
        sc_trace(file, PRDATA, "PRDATA");
        sc_trace(file, PSLVERR, "PSLVERR");
    }

    void idle_bus() {
        PSEL = false;
        PENABLE = false;
        PWRITE = false;
        PADDR = 0;
        PWDATA = 0;
        PSTRB = 0;
    }

    void check_request(const Req& actual, bool write, sct_uint<ADDR_W> addr,
                       sct_uint<DATA_W> wdata, sct_uint<STRB_W> wstrb) {
        sc_assert(actual.write == write);
        sc_assert(actual.addr == addr);
        sc_assert(actual.wdata == wdata);
        sc_assert(actual.wstrb == wstrb);
    }

    void apb_transfer(bool write, sct_uint<ADDR_W> addr, sct_uint<DATA_W> wdata,
                      sct_uint<STRB_W> wstrb, sct_uint<DATA_W> rdata,
                      bool slverr = false) {
        bool req_seen;
        bool resp_sent;
        Req req;
        Resp resp;

        PADDR = addr;
        PWRITE = write;
        PWDATA = wdata;
        PSTRB = wstrb;
        PSEL = true;
        PENABLE = false;
        wait();

        PENABLE = true;

        req_seen = false;
        while (!req_seen) {
            if (req_target.request()) {
                req = req_target.get();
                check_request(req, write, addr, wdata, wstrb);
                req_seen = true;
            }
            wait();
        }

        resp.rdata = rdata;
        resp.slverr = slverr;
        resp_sent = false;
        while (!resp_sent) {
            resp_sent = resp_init.put(resp);
            wait();
        }
        resp_init.clear_put();

        while (!PREADY.read()) {
            wait();
        }
        sc_assert(PSLVERR.read() == slverr);
        if (!write) {
            sc_assert(PRDATA.read() == rdata);
        }

        idle_bus();
        wait();
    }

    void stimulus_thread() {
        req_target.reset_get();
        resp_init.reset_put();
        idle_bus();

        PRESETn = false;
        wait(3);
        PRESETn = true;
        wait();

        apb_transfer(true, 0x10, 0x11112222, 0xF, 0);
        apb_transfer(true, 0x14, 0x33334444, 0xF, 0);
        apb_transfer(false, 0x10, 0, 0, 0x11112222);
        apb_transfer(false, 0x14, 0, 0, 0x33334444);

        cout << "Test completed successfully" << endl;
        sc_stop();
    }
};

int sc_main(int argc, char* argv[]) {
    sc_clock PCLK{"PCLK", 1, SC_NS};
    tb tbt{"tbt"};
    sc_trace_file* trace_file = sc_create_vcd_trace_file("copilot_apb_port");

    tbt.PCLK(PCLK);
    sc_trace(trace_file, PCLK, "PCLK");
    tbt.trace(trace_file);

    sc_start();

    sc_close_vcd_trace_file(trace_file);
    return 0;
}