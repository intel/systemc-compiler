//
// Created by ripopov on 2/27/18.
//

#include "dma_tlm20/dma_tlm_wrapper.cpp"
#include "hls/dma/apb_dma.cpp"
#include "hls/dma/dma_circ_buffer.cpp"
#include "hls_tlm/hls_utils.cpp"
#include "hls_tlm/sc_smart_clock.cpp"

using namespace std;

namespace intel_hls {

struct top : sc_module {

    top(::sc_core::sc_module_name) {
        apb_dma_inst.clk(clk_gen);
        apb_dma_inst.areset_n(areset_n);
        apb_dma_inst.interrupt(interrupt);
        apb_dma_inst.active_transfer(clock_enable_dma);

        apb_dma_inst.mmio_apb_tar.bind(mmio_apb_channel);
        apb_dma_inst.memory_apb_init.bind(memory_apb_channel);
    }

private:

    apb_dma apb_dma_inst{"apb_dma_inst"};
    sc_signal<bool> areset_n{"areset_n"};
    sc_signal<bool> interrupt{"interrupt"};
    sc_signal<bool> clk_gen{"clk_gen"};
    sc_signal<bool> clock_enable_dma{"clock_enable_dma"};
    apb_channel<bus_data_t, bus_data_t, bus_strobe_t> mmio_apb_channel{"mmio_apb_channel"};
    apb_channel<bus_data_t, bus_data_t, bus_strobe_t> memory_apb_channel{"memory_apb_channel"};

};

}

int sc_main(int argc, char **argv) {
    cout << "test_apb_dma\n";
    auto top = std::make_unique<intel_hls::top>("top");
    sc_start();

    return 0;
}