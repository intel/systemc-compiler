//
// Created by ripopov on 8/20/17.
//

#include "dma_test_suite.h"
#include <testbench/dma_system_params.h>
#include <dma/dma_params.h>
#include "csr_rw_test.h"
#include "transfer_test.h"
#include <chrono>

namespace intel_hls {

    dma_test_suite::dma_test_suite(dma_test_api *test_driver) : dma_test_base(*test_driver) {

    }

    int dma_test_suite::run_tests() {

        LOG_DEBUG("Starting test suite");
        auto start = std::chrono::system_clock::now();

        csr_rw_test csr_rw_test_inst(tapi);
        csr_rw_test_inst.run_tests();

        wait(sc_time{1,SC_SEC});

        transfer_test transfer_test_inst(tapi);
        transfer_test_inst.run_tests();

        wait(sc_time{1,SC_SEC});

        auto end = std::chrono::system_clock::now();
        auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "Real time elapsed: " << elapsed_ms.count() << " ms\n";
        std::cout << "Model time : " << sc_time_stamp() << endl;

        return error_count;
    }

}