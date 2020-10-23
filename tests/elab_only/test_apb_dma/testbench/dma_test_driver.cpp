//
// Created by ripopov on 8/20/17.
//

#include "dma_test_driver.h"
#include "testbench/tests/dma_test_suite.h"
#include "dma_system_params.h"

#include <sstream>

namespace intel_hls {

    dma_test_driver::dma_test_driver(::sc_core::sc_module_name) {

        apb2tlm_inst.tlm_initiator.bind(memory_initiator);

        payload.set_streaming_width(4);
        payload.set_data_length(4);
        payload.set_byte_enable_length(4);
        payload.set_byte_enable_ptr(byte_enable);

        SC_THREAD(test_thread);
    }

    void dma_test_driver::apb_reset() {
        LOG_DEBUG("Reset asserted");
        areset_n.write(0);
        wait(apb_clock_period * 2);
        areset_n.write(1);
        wait(apb_clock_period * 2);
        LOG_DEBUG("Reset deactivated");
    }

    apb_slverr_t dma_test_driver::apb_read(bus_addr_t addr, bus_data_t &rdata) {
        //std::stringstream ss;
        //ss << hex << "dma_test_driver::apb_read addr: " << addr;
        //ss << " delta cycle " << dec <<sc_delta_count();
        //SC_REPORT_INFO(APB_DMA_PRJ_NAME, ss.str().c_str());

        return apb2tlm_inst.apb_read(addr, rdata);
    }

    apb_slverr_t dma_test_driver::apb_write(bus_addr_t addr, bus_data_t wdata, bus_strobe_t pstrb) {
        //std::stringstream ss;
        //ss << hex << "dma_test_driver::apb_write addr: " << addr;
        //ss << " delta cycle " << dec << sc_delta_count();
        //SC_REPORT_INFO(APB_DMA_PRJ_NAME, ss.str().c_str());

        return apb2tlm_inst.apb_write(addr, wdata, pstrb);
    }

    void dma_test_driver::wait_interrupt() {
        if (!interrupt.read())
            wait(interrupt.posedge_event());

    }

    void dma_test_driver::wait_clock_cycle(int n) {
        wait(apb_clock_period);
    }

    void dma_test_driver::test_thread() {

        apb_reset();

        dma_test_suite suite{this};

        if (suite.run_tests() != 0) {
            LOG_ERROR("Test suite finished with errors");
        } else {
            LOG_DEBUG("Test suite passed");
        }

        sc_stop();
    }

}