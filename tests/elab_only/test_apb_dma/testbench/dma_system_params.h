//
// Created by ripopov on 8/20/17.
//

#pragma once

#include <systemc.h>

namespace intel_hls {

    const sc_time apb_clock_period {10, SC_NS};

    const unsigned SYSTEM_MEMORY_SIZE = 0x10000000; // 256 Mb

    struct SYS_MEM_MAP {

        enum {
            APB_DMA_BASE = 0x1000,
            MEMORY_BASE =  0x10000000
        };

    };

}