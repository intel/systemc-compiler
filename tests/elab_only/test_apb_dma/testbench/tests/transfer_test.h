//
// Created by ripopov on 8/22/17.
//

#pragma once

#include <testbench/dma_test_base.h>

namespace intel_hls {

    struct transfer_test : dma_test_base {

        transfer_test(dma_test_api &test_driver);

        int run_tests() override;

    protected:

        void dma_test(uint32_t source_base, uint32_t size);

        void dma_memcpy(uint32_t source_base, uint32_t dest_base, uint32_t size);

        void mem_write_byte(uint32_t addr, uint8_t data);
        uint8_t mem_read_byte(uint32_t addr);


    };

}



