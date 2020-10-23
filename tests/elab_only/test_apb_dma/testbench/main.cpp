#include <systemc.h>
#include <hls_tlm/hls_tlm.h>
#include "dma_test_system.h"
#include <dma/dma_params.h>

int sc_main(int argc, char **argv) {

#if HLS_IO_DEFAULT_LEVEL == HLS_IO_TLM_LEVEL
    SC_REPORT_INFO(APB_DMA_PRJ_NAME, "Running testbench with TLM IO (fast mode)");
#else
    SC_REPORT_INFO(APB_DMA_PRJ_NAME, "Running testbench with Pin-level cycle accurate IO (slow mode)");
#endif

    //TLM_BASE::vpCommon::setDbgLevelInitValue(TLM_BASE::PEDANTIC);

    #pragma GCC diagnostic ignored "-Wunused-variable"
    auto test_sys = new intel_hls::dma_test_system("test_sys");

    sc_start();

    return 0;
}