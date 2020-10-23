//
// Created by ripopov on 8/20/17.
//

#pragma once

#include <dma/dma_params.h>
#include <apb/apb_if.h>

namespace intel_hls {

    struct dma_test_api : apb_if<bus_addr_t, bus_data_t, bus_strobe_t> {

        virtual void wait_interrupt() = 0;
        virtual void wait_clock_cycle(int n) = 0;

    };

}
