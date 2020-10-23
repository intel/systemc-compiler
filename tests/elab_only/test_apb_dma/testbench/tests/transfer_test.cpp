//
// Created by ripopov on 8/22/17.
//

#include "transfer_test.h"
#include <dma/dma_params.h>
#include <testbench/dma_system_params.h>

namespace intel_hls {

    transfer_test::transfer_test(dma_test_api &test_driver)
            : dma_test_base(test_driver) {}

    int transfer_test::run_tests() {
        LOG_DEBUG("Starting DMA transfer test");
        const int TEST_TRAN_SIZE = 1024;

        for (int ii = 0; ii < 4; ++ii) {
            dma_test(SYS_MEM_MAP::MEMORY_BASE + ii, TEST_TRAN_SIZE);
        }

        LOG_DEBUG("PASS");

        wait(1,SC_SEC);

        LOG_DEBUG("DMA transfer test FINISHED");
        return 0;
    }

    void transfer_test::mem_write_byte(uint32_t addr, uint8_t data) {
        auto shamt = addr & 0x3;
        auto byte_en = 0x1 << shamt;
        auto wdata = data << (shamt * 8);
        tapi.apb_write(addr & 0xfffffffc, wdata, byte_en);
    }

    uint8_t transfer_test::mem_read_byte(uint32_t addr) {
        bus_data_t rdata;
        auto shamt = (addr & 0x3) * 8;
        tapi.apb_read(addr & 0xfffffffc, rdata);
        auto shifted_data = (rdata >> shamt) & 0xff;
        return static_cast<uint8_t>(shifted_data);
    }

    void transfer_test::dma_memcpy(uint32_t source_base, uint32_t dest_base, uint32_t size) {
        csr_write(DMA_CSR_OFFSET::SOURCE, source_base);
        csr_write(DMA_CSR_OFFSET::DEST, dest_base);
        csr_write(DMA_CSR_OFFSET::TRAN_SIZE, size);
        csr_write(DMA_CSR_OFFSET::CONTROL, 0x1);

        tapi.wait_interrupt();

        csr_write(DMA_CSR_OFFSET::CONTROL, 0x2);
    }

    void transfer_test::dma_test(uint32_t source_base, uint32_t size) {
        for (uint32_t ii = 0; ii < size; ii ++)
            mem_write_byte(source_base + ii, ii);

        dma_memcpy(source_base, source_base + size, size);

        for (uint32_t ii = 0; ii < size; ii ++)
            expect_equal(mem_read_byte(source_base + ii), (uint8_t)ii);
    }

}
