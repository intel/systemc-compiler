//
// Created by ripopov on 8/20/17.
//

#pragma once

#include <testbench/dma_test_base.h>

namespace intel_hls {

    struct csr_rw_test : dma_test_base {

        csr_rw_test(dma_test_api &test_driver);

        int run_tests() override;

    };

}



