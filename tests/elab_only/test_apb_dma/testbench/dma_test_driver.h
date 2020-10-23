//
// Created by ripopov on 8/20/17.
//

#pragma once

#include <systemc.h>
#include <tlm.h>
#include <tlm_utils/simple_initiator_socket.h>
#include <dma_tlm20/apb2tlm.h>
#include "dma_test_api.h"
#include "dma_test_base.h"

namespace intel_hls {

    struct dma_test_driver : hls_module, dma_test_api {
        SC_HAS_PROCESS(dma_test_driver);

        sc_out<bool> areset_n{"areset_n"};
        sc_in<bool>  interrupt{"interrupt"};

        tlm::tlm_initiator_socket<> memory_initiator{"memory_initiator"};

        dma_test_driver(::sc_core::sc_module_name);
    private:

        apb2tlm<HLS_IO_TLM_LEVEL> apb2tlm_inst{"apb2tlm_inst"};

        tlm::tlm_generic_payload  payload;
        unsigned char byte_enable[4];

        void test_thread();

        void apb_reset() override;
        apb_slverr_t apb_read(bus_addr_t addr, bus_data_t &rdata) override;
        apb_slverr_t apb_write(bus_addr_t addr, bus_data_t wdata, bus_strobe_t pstrb) override;

        void wait_interrupt() override;
        void wait_clock_cycle(int n) override;

    };

}
