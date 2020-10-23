//
// Created by ripopov on 8/18/17.
//

#include <stdint.h>
#include "apb_dma.h"

namespace intel_hls {

    apb_dma::apb_dma(::sc_core::sc_module_name)
            : clk("clk"), areset_n("areset_n"), interrupt("interrupt"), active_transfer("active_transfer"), mmio_apb_tar("mmio_apb_tar")
            , memory_apb_init("memory_apb_init"), src_ptr_sig("src_ptr_sig"), dst_ptr_sig("dst_ptr_sig"), trans_bytes_sig("trans_bytes_sig")
            , apb_error_sig("apb_error_sig"), start_sig("start_sig"), clear_flags_sig("clear_flags_sig"), abort_sig("abort_sig") {

        mmio_apb_tar.clk_rst(clk, areset_n);
        mmio_apb_tar.impl.bind(*this);

        SC_THREAD(dma_thread);
        sensitive << clk.pos();
        async_reset_signal_is(areset_n, false);

    }


    void apb_dma::apb_reset() {
        src_ptr_sig = 0;
        dst_ptr_sig = 0;
        trans_bytes_sig = 0;
        start_sig = 0;
        clear_flags_sig = 0;
        abort_sig = 0;
    }

    apb_slverr_t apb_dma::apb_read(bus_addr_t addr, bus_data_t &rdata) {

        uint32_t raddr = addr;

        switch (raddr) {

            case DMA_CSR_OFFSET::STATUS:
                rdata[DMA_STATUS_BIT::INT] = interrupt;
                rdata[DMA_STATUS_BIT::ACTIVE] = active_transfer;
                rdata[DMA_STATUS_BIT::APB_ERR] = apb_error_sig;
                break;

            case DMA_CSR_OFFSET::SOURCE:
                rdata = src_ptr_sig;
                break;

            case DMA_CSR_OFFSET::DEST:
                rdata = dst_ptr_sig;
                break;

            case DMA_CSR_OFFSET::TRAN_SIZE:
                rdata = trans_bytes_sig;
                break;

            case DMA_CSR_OFFSET::CONTROL:
                rdata = 0xDEADC0DE;
                break;

            default:
                return APB_RESP::ERROR;
        }
        return APB_RESP::SUCCESS;
    }

    apb_slverr_t apb_dma::apb_write(bus_addr_t addr, bus_data_t wdata, bus_strobe_t pstrb) {
        uint32_t waddr = addr;

        switch (waddr) {

            case DMA_CSR_OFFSET::STATUS:
                break;

            case DMA_CSR_OFFSET::SOURCE:
                if (!active_transfer)
                    src_ptr_sig.write(wdata);
                break;

            case DMA_CSR_OFFSET::DEST:
                if (!active_transfer)
                    dst_ptr_sig.write(wdata);
                break;

            case DMA_CSR_OFFSET::TRAN_SIZE:
                if (!active_transfer)
                    trans_bytes_sig.write(wdata);
                break;

            case DMA_CSR_OFFSET::CONTROL:
                start_sig = wdata[DMA_CONTROL_BIT::START];
                clear_flags_sig = wdata[DMA_CONTROL_BIT::CLEAR];
                abort_sig = wdata[DMA_CONTROL_BIT::ABORT];
                wait();
                start_sig = 0;
                clear_flags_sig = 0;
                // abort_sig = 0; deasserted by dma thread
                break;

            default:
                return APB_RESP::ERROR;
        }
        return APB_RESP::SUCCESS;
    }

    void apb_dma::do_clear_flags() {
        interrupt = 0;
        apb_error_sig = 0;
    }

    void apb_dma::dma_thread() {

        memory_apb_init.apb_reset();
        do_clear_flags();
        active_transfer = 0;

        wait();

        while (1) {

            while (!start_sig) {
                if (clear_flags_sig)
                    do_clear_flags();
                wait();
            }
            active_transfer = 1;
            do_clear_flags();

            LOG_DEBUG("DMA transfer started");

            while ((trans_bytes_sig.read() != 0 || data_buffer.size() != 0) && !abort_sig.read()) {

                bus_addr_t bytes_rem = trans_bytes_sig.read();

                if (trans_bytes_sig.read() > 0) {

                    bus_data_t read_data;

                    if (memory_apb_init.apb_read(src_ptr_sig.read() & 0xfffffffc, read_data) == APB_RESP::ERROR) {
                        apb_error_sig = 1;
                        interrupt = 1;
                        break;
                    }

                    // align data read, so lsb is valid
                    const sc_uint<2> shamt = src_ptr_sig.read()(1, 0);
                    const bus_data_t shifted_data = read_data >> shamt;
                    const sc_uint<3> n_valid_bytes = uint_min(trans_bytes_sig.read().to_uint(), 4 - shamt);

                    data_buffer.put(shifted_data, n_valid_bytes);

                    bytes_rem -= n_valid_bytes;
                    trans_bytes_sig = bytes_rem;

                    src_ptr_sig = (src_ptr_sig.read() & 0xfffffffc) + 4;
                }

                const sc_uint<2> wshamt = dst_ptr_sig.read()(1, 0);
                const sc_uint<3> n_bytes_can_transfer = 4 - wshamt;

                if ((data_buffer.size() >= n_bytes_can_transfer) || (bytes_rem == 0)) {

                    const sc_uint<3> n_write_bytes = uint_min(data_buffer.size(), n_bytes_can_transfer);

                    const bus_data_t shifted_wdata = data_buffer.get(n_write_bytes) << wshamt;

                    sc_uint<4> wstrb = (0xf >> (4 - n_write_bytes)) << wshamt;

                    if (memory_apb_init.apb_write(dst_ptr_sig.read() & 0xfffffffc, shifted_wdata, wstrb) == APB_RESP::ERROR) {
                        apb_error_sig = 1;
                        interrupt = 1;
                        break;
                    }

                    dst_ptr_sig = (dst_ptr_sig.read() & 0xfffffffc) + 4;
                }

                wait();
            }

            abort_sig = 0;
            active_transfer = 0;
            interrupt = 1;

            LOG_DEBUG("DMA transfer FINISHED");

            wait();
        }
    }

}
