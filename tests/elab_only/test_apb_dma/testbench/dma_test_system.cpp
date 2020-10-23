//
// Created by ripopov on 8/17/17.
//

#include "dma_test_system.h"
#include "dma_system_params.h"

namespace intel_hls {


    dma_test_system::dma_test_system(::sc_core::sc_module_name) {

        memory.resize(SYSTEM_MEMORY_SIZE);

        dut_inst.interrupt(interrupt);
        dut_inst.areset_n(areset_n);
        dut_inst.mmio_target.bind(dma_mmio_slave_initiator);
        dut_inst.memory_initiator.bind(dma_memory_target);

        driver_inst.interrupt(interrupt);
        driver_inst.areset_n(areset_n);
        driver_inst.memory_initiator(driver_memory_target);

        dma_memory_target.register_b_transport(this, &dma_test_system::b_transport);
        driver_memory_target.register_b_transport(this, &dma_test_system::b_transport);
    }

    void dma_test_system::b_transport(tlm::tlm_generic_payload &trans, sc_core::sc_time &t) {
        auto address = trans.get_address();

        if (address < SYS_MEM_MAP::MEMORY_BASE && address >= SYS_MEM_MAP::APB_DMA_BASE) {
            trans.set_address(address - SYS_MEM_MAP::APB_DMA_BASE);
            dma_mmio_slave_initiator->b_transport(trans, t);
        } else if (address >= SYS_MEM_MAP::MEMORY_BASE){

            auto base_offset = trans.get_address() - SYS_MEM_MAP::MEMORY_BASE;

            if (trans.is_write()) {

                for (unsigned ii = 0; ii < trans.get_data_length(); ++ii) {
                    //if (trans.get_byte_enable_ptr()[ ii % trans.get_byte_enable_length() ] == 0xff)
                        memory.at(base_offset + ii) = trans.get_data_ptr()[ii];
                }

            } else if (trans.is_read()) {

                for (unsigned ii = 0; ii < trans.get_data_length(); ++ii)
                    trans.get_data_ptr()[ii] = memory.at(base_offset + ii);

            }

            trans.set_response_status(tlm::TLM_OK_RESPONSE);

        } else {
            trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
        }
    }

}