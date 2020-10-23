//
// Created by ripopov on 8/20/17.
//

#include "csr_rw_test.h"
#include <dma/dma_params.h>

namespace intel_hls {

    int csr_rw_test::run_tests() {
        LOG_DEBUG("STARTING CSR read/write test");

        expect_equal(csr_read(DMA_CSR_OFFSET::STATUS) , 0x0);

        csr_write(DMA_CSR_OFFSET::TRAN_SIZE, 0x42);
        expect_equal(csr_read(DMA_CSR_OFFSET::TRAN_SIZE) , 0x42);

        csr_write(DMA_CSR_OFFSET::SOURCE, 0x43);
        expect_equal(csr_read(DMA_CSR_OFFSET::SOURCE), 0x43);

        csr_write(DMA_CSR_OFFSET::DEST, 0x44);
        expect_equal(csr_read(DMA_CSR_OFFSET::DEST), 0x44);

        LOG_DEBUG("CSR read/write test FINISHED");

        return error_count;
    }

    csr_rw_test::csr_rw_test(dma_test_api &test_driver)
            : dma_test_base(test_driver) {
    }

}
