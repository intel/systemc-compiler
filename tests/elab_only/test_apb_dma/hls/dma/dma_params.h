//
// Created by ripopov on 8/17/17.
//

#pragma once

#include <systemc.h>

namespace intel_hls {

    #define APB_DMA_PRJ_NAME "/intel/hls_dma"

    typedef sc_uint<32> bus_addr_t;
    typedef sc_uint<32> bus_data_t;
    typedef sc_uint<4> bus_strobe_t;

    struct DMA_CSR_OFFSET {

        enum offsets {
            STATUS = 0x0,
            SOURCE = 0x4,    // Source base address
            DEST = 0x8,      // Destination base address
            TRAN_SIZE = 0xc, // Transfer size in bytes, returns bytes left to transfer on read
            CONTROL = 0x10   // Write-only register
        };

    };

    struct DMA_STATUS_BIT {

        enum {
            INT = 0,    // Interrupt flag
            ACTIVE = 1, // Transfer in progress flag
            APB_ERR = 2 // Bus error flag (error during dma transfer)
        };

    };

    struct DMA_CONTROL_BIT {

        enum {
            START = 0, // Start dma transfer
            CLEAR = 1, // Clear error and interrupt flags
            ABORT = 2  // Abort DMA transfer
        };

    };

}
