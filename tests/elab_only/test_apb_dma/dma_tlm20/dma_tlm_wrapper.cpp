//
// Created by ripopov on 8/20/17.
//

#include "dma_tlm_wrapper.h"
#include <sstream>

namespace intel_hls {

    dma_tlm_wrapper::dma_tlm_wrapper(::sc_core::sc_module_name) {

        apb_dma_inst.clk(clk_gen);
        apb_dma_inst.areset_n(areset_n);
        apb_dma_inst.interrupt(interrupt);
        apb_dma_inst.active_transfer(clock_enable_dma);

        apb_dma_inst.mmio_apb_tar.bind(mmio_apb_channel);
        apb_dma_inst.memory_apb_init.bind(memory_apb_channel);

        apb2tlm_inst.apb_tar.clk_rst(clk_gen, areset_n);
        apb2tlm_inst.apb_tar.bind(memory_apb_channel);
        apb2tlm_inst.tlm_initiator.bind(memory_initiator);

        mmio_apb_initiator.bind(mmio_apb_channel);
        mmio_apb_initiator.clk_rst(clk_gen, areset_n);

        mmio_target.register_b_transport(this, &dma_tlm_wrapper::mmio_b_transport);

        clk_gen.register_cg_enable(clock_enable_mmio);
        clk_gen.register_cg_enable(clock_enable_dma);

        SC_THREAD(apb_csr_thread);
        sensitive << clk_gen.posedge_event();
        async_reset_signal_is(areset_n, false);

    }

    void dma_tlm_wrapper::mmio_b_transport(tlm::tlm_generic_payload &trans, sc_core::sc_time &t) {

        if (trans.get_data_length() != 4 || !areset_n.read() ) {
            trans.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
            return;
        }

        uint32_t *tlm_data_ptr = reinterpret_cast<uint32_t *>(trans.get_data_ptr());
        bool is_write = trans.is_write();
        bus_addr_t addr = trans.get_address();
        bus_data_t bus_data;
        bus_strobe_t wstrobe;

        if (is_write) {
            bus_data = *tlm_data_ptr;
            wstrobe = 0xff;
        }

        mmio_active.write(true);
        auto mmio_request = mmio_req_t{is_write, addr, bus_data, wstrobe};

        mmio_request_fifo.write(mmio_request);

        auto mmio_response = mmio_response_fifo.read();

        if (std::get<1>(mmio_response) == APB_RESP::SUCCESS) {

            trans.set_response_status(tlm::TLM_OK_RESPONSE);

            if (!is_write)
                *tlm_data_ptr = static_cast<uint32_t>(std::get<0>(mmio_response));

        } else {
            trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
        }
        mmio_active.write(false);
    }

    void dma_tlm_wrapper::apb_csr_thread() {
        clock_enable_mmio.write(0);
        while (1) {

            clock_enable_mmio.write(0);
            auto req = mmio_request_fifo.read();
            clock_enable_mmio.write(1);
            apb_slverr_t apb_err;
            bus_data_t rdata;

            if (std::get<0>(req)) {
                apb_err = mmio_apb_initiator.apb_write(std::get<1>(req), std::get<2>(req), std::get<3>(req));
            } else {
                apb_err = mmio_apb_initiator.apb_read(std::get<1>(req), rdata);
            }

            mmio_response_fifo.write(mmio_resp_t{rdata, apb_err});
        }
    }

}
