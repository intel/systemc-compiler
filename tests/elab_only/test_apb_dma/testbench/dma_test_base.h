//
// Created by ripopov on 8/20/17.
//

#pragma once

#include <testbench/dma_system_params.h>
#include <hls_tlm/hls_tlm.h>
#include "dma_test_api.h"

namespace intel_hls {

    /**
     * A base class for all test cases
     */
    struct dma_test_base {

        dma_test_base (dma_test_api &test_driver) : tapi(test_driver) {}

        /**
         * Run all tests
         * @return Number of test errors
         */
        virtual int run_tests() = 0;

    protected:

        template <typename T1, typename T2>
        void expect_equal(const T1 &val, const T2& ref) {
            if (val != (T1)ref) {
                SC_REPORT_WARNING(APB_DMA_PRJ_NAME, "Value mismatch");
                error_count++;
            }
        }

        void csr_write(uint32_t offset, uint32_t wdata) {
            tapi.apb_write(SYS_MEM_MAP::APB_DMA_BASE + offset, wdata, 0xf );
        }

        uint32_t csr_read(uint32_t offset) {
            bus_data_t read_data;
            tapi.apb_read(SYS_MEM_MAP::APB_DMA_BASE + offset, read_data);
            return static_cast<uint32_t>(read_data);
        }

        int error_count = 0; // Errors detected during tests execution
        dma_test_api &tapi; // reference to test driver api implementation, used to control DUT

    };


}