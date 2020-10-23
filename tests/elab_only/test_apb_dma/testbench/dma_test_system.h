//
// Created by ripopov on 8/17/17.
//

#pragma once

#include <systemc.h>
#include <tlm_utils/simple_target_socket.h>
#include <tlm_utils/simple_initiator_socket.h>
#include <vector>
#include <dma_tlm20/dma_tlm_wrapper.h>
#include "dma_test_driver.h"
#include "dma_system_params.h"

namespace intel_hls {

    struct dma_test_system : hls_module {

        dma_test_system(::sc_core::sc_module_name);

    private:
        void b_transport(tlm::tlm_generic_payload &trans, sc_core::sc_time &t);

        sc_signal<bool> areset_n{"areset_n"};
        sc_signal<bool> interrupt{"interrupt"};

        tlm_utils::simple_initiator_socket<dma_test_system> dma_mmio_slave_initiator{"dma_mmio_slave_initiator"};
        tlm_utils::simple_target_socket<dma_test_system>    dma_memory_target{"dma_memory_target"};
        tlm_utils::simple_target_socket<dma_test_system>    driver_memory_target{"driver_memory_target"};

        std::vector<uint8_t > memory;

        dma_tlm_wrapper dut_inst{"dut_inst"};
        dma_test_driver driver_inst{"driver_inst"};
    };

}
