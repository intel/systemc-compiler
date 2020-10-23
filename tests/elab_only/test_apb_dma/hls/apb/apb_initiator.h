#pragma once

#include <systemc.h>
#include <hls_tlm/hls_tlm.h>

#include "apb_if.h"
#include "apb_sif.h"

namespace intel_hls {

    namespace apb_imp {

        template <typename addr_t, typename data_t, typename pstrb_t,  bool IO_LEVEL>
        struct apb_initiator_imp{};

        // *****************************************************************************
        // TLM implementation **********************************************************
        // *****************************************************************************
        template <typename addr_t, typename data_t, typename pstrb_t>
        struct apb_initiator_imp<addr_t, data_t, pstrb_t, HLS_IO_TLM_LEVEL > : sc_module, apb_if<addr_t, data_t, pstrb_t> {

            apb_initiator_imp(const sc_module_name &nm) : sc_module(nm), p("p") {}

            template<typename CHAN>
            void bind(CHAN& chan) { chan.bind_port(p); }

            template<typename CLK, typename RST>
            void clk_rst(CLK& clk, RST& rst) {
            }

            void apb_reset()  {
                p->apb_reset();
            }

            apb_slverr_t apb_read(addr_t addr, data_t &rdata) {
                return p->apb_read(addr, rdata);
            }

            apb_slverr_t apb_write(addr_t addr, data_t wdata, pstrb_t strb) {
                return p->apb_write(addr, wdata, strb);
            }

        private:
            sc_port<apb_if<addr_t, data_t, pstrb_t> > p;
        };

        // *****************************************************************************



        // *****************************************************************************
        // Signal level implementation *************************************************
        // *****************************************************************************

        template <typename addr_t, typename data_t, typename pstrb_t>
        struct apb_initiator_imp<addr_t, data_t, pstrb_t, HLS_IO_SIGNAL_LEVEL >
                : sc_module, apb_if<addr_t, data_t, pstrb_t>, apb_master_sif<addr_t, data_t, pstrb_t> {

            explicit apb_initiator_imp(const sc_module_name &nm) : sc_module(nm) {}

            template<typename CLK, typename RST>
            void clk_rst(CLK&, RST&) { }

            void apb_reset() {
                this->paddr = 0;
                this->psel = 0;
                this->penable = 0;
                this->pwrite = 0;
                this->pwdata = 0;
            }

            apb_slverr_t apb_read(addr_t addr, data_t &rdata) {
                this->paddr = addr;
                this->psel = 1;
                this->pwrite = 0;
                wait();
                this->penable = 1;
                while (!this->pready)
                    wait();
                this->paddr = 0;
                this->psel = 0;
                this->penable = 0;
                rdata = this->prdata;
                return this->pslverr;
            }

            apb_slverr_t apb_write(addr_t addr, data_t wdata, pstrb_t strb) {
                this->paddr = addr;
                this->psel = 1;
                this->pwrite = 1;
                this->pstrb = strb;
                this->pwdata = wdata;
                wait();
                this->penable = 1;
                wait();
                while (!this->pready)
                    wait();
                this->paddr = 0;
                this->psel = 0;
                this->penable = 0;
                this->pwrite = 0;
                this->pstrb = 0;
                this->pwdata = 0;
                return this->pslverr;
            }
        };

    }

    template <typename addr_t, typename data_t, typename pstrb_t, bool IO_LEVEL = HLS_IO_DEFAULT_LEVEL>
    struct apb_initiator : apb_imp::apb_initiator_imp<addr_t, data_t, pstrb_t, IO_LEVEL> {

        explicit apb_initiator(const sc_module_name &nm = sc_gen_unique_name("apb_initiator"))
                : apb_imp::apb_initiator_imp<addr_t, data_t, pstrb_t, IO_LEVEL>(nm) {}

    };

}
