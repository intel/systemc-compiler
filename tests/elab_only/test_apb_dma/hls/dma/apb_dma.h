//
// Created by ripopov on 8/18/17.
//

#pragma once

#include <systemc.h>
#include <apb/apb.h>
#include <hls_tlm/hls_utils.h>
#include "dma_params.h"
#include "dma_circ_buffer.h"

namespace intel_hls {

    struct apb_dma : hls_module,
                     apb_if<bus_addr_t, bus_data_t, bus_strobe_t>,
                     sc_dont_collapse {

        SC_HAS_PROCESS(apb_dma);

        sc_in_clk clk;
        sc_in<bool> areset_n;

        sc_out<bool> interrupt;
        sc_out<bool> active_transfer;

        apb_target<bus_addr_t, bus_data_t, bus_strobe_t> mmio_apb_tar;
        apb_initiator<bus_addr_t, bus_data_t, bus_strobe_t> memory_apb_init;

        apb_dma(::sc_core::sc_module_name);

    private:

        dma_circ_buffer data_buffer;

        // signals
        shared_signal(bus_addr_t) src_ptr_sig;
        shared_signal(bus_addr_t) dst_ptr_sig;
        shared_signal(bus_addr_t) trans_bytes_sig;

        sc_signal<bool> apb_error_sig;
        sc_signal<bool> start_sig;
        sc_signal<bool> clear_flags_sig;
        shared_signal(bool) abort_sig;

        // APB Slave implementation
        void apb_reset();
        apb_slverr_t apb_read(bus_addr_t addr, bus_data_t &rdata);
        apb_slverr_t apb_write(bus_addr_t addr, bus_data_t wdata, bus_strobe_t pstrb);

        void do_clear_flags();
        void dma_thread();
    };

}
