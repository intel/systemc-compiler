//
// Created by ripopov on 8/20/17.
//

#pragma once

#include <systemc.h>
#include <tlm.h>
#include <tlm_utils/simple_initiator_socket.h>
#include <tlm_utils/simple_target_socket.h>
#include <dma/apb_dma.h>
#include <hls_tlm/sc_smart_clock.h>
#include "tuple_ostream.h"
#include "apb2tlm.h"

namespace intel_hls {

    struct dma_tlm_wrapper : sc_module {
        SC_HAS_PROCESS(dma_tlm_wrapper);

        sc_in<bool> areset_n{"areset_n"};
        sc_out<bool> interrupt{"interrupt"};

        tlm_utils::simple_target_socket<dma_tlm_wrapper> mmio_target{"mmio_target"};
        tlm::tlm_initiator_socket<> memory_initiator{"memory_initiator"};

        dma_tlm_wrapper(::sc_core::sc_module_name);

    private:

        sc_smart_clock clk_gen{"clk_gen", 10, SC_NS};

        sc_signal<bool> clock_enable_mmio{"clock_enable_mmio"};
        sc_signal<bool> clock_enable_dma{"clock_enable_dma"};


        typedef bool is_write_t;
        typedef std::tuple<is_write_t, bus_addr_t, bus_data_t, bus_strobe_t > mmio_req_t;
        typedef std::tuple<bus_data_t,apb_slverr_t>  mmio_resp_t;

        sc_signal<bool> mmio_active{"mmio_active",0};
        sc_fifo<mmio_req_t> mmio_request_fifo{"mmio_request_fifo"};
        sc_fifo<mmio_resp_t> mmio_response_fifo{"mmio_response_fifo"};


        apb_initiator<bus_addr_t, bus_data_t, bus_strobe_t> mmio_apb_initiator{"mmio_apb_initiator"};

        apb_channel<bus_data_t, bus_data_t, bus_strobe_t> mmio_apb_channel{"mmio_apb_channel"};
        apb_channel<bus_data_t, bus_data_t, bus_strobe_t> memory_apb_channel{"memory_apb_channel"};

        apb2tlm<> apb2tlm_inst{"apb2tlm_inst"};

        apb_dma apb_dma_inst{"apb_dma_inst"};

        void mmio_b_transport(tlm::tlm_generic_payload &trans, sc_core::sc_time &t);
        void apb_csr_thread();

    };

}
