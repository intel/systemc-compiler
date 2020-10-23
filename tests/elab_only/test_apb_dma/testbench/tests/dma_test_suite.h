//
// Created by ripopov on 8/20/17.
//

#pragma once

#include <testbench/dma_test_base.h>

namespace intel_hls {

    struct dma_test_suite : dma_test_base {

        int run_tests() override;

        dma_test_suite(dma_test_api *test_driver);

    };

}
