#pragma once

#include <systemc.h>

#include <hls_tlm/hls_tlm.h>
#include "apb_if.h"
#include "apb_sif.h"

namespace intel_hls {

    namespace apb_imp {

        template<typename addr_t, typename data_t, typename pstrb_t, bool IO_LEVEL>
        struct apb_target_imp {
        };

        // *****************************************************************************
        // TLM level specialization ****************************************************
        // *****************************************************************************
        template<typename addr_t, typename data_t, typename pstrb_t>
        struct apb_target_imp<addr_t, data_t, pstrb_t, HLS_IO_TLM_LEVEL> : sc_module {

            typedef sc_export<apb_if<addr_t, data_t, pstrb_t> >  export_t ;

            apb_target_imp(const sc_module_name &nm) :
                    sc_module(nm), impl("impl"){}

            template<typename CHAN>
            void bind(CHAN &chan) { chan.bind_export(impl); }

            template<typename CLK, typename RST>
            void clk_rst(CLK &, RST &) {}

            export_t impl;
        };
        // *****************************************************************************


        // *****************************************************************************
        // Signal level specialization *************************************************
        // *****************************************************************************

        template<typename addr_t, typename data_t, typename pstrb_t>
        struct apb_target_imp<addr_t, data_t, pstrb_t, HLS_IO_SIGNAL_LEVEL>
                : sc_module, apb_slave_sif<addr_t, data_t, pstrb_t> {

            sc_in_clk clk;
            sc_in<bool> arst_n;
            SC_HAS_PROCESS(apb_target_imp);

            apb_target_imp(const sc_module_name &nm)
                    : sc_module(nm), clk("clk"), arst_n("arst_n") {
                SC_THREAD(apb_slave_cthread);
                sensitive << clk.pos();
                async_reset_signal_is(arst_n, false);
            }

            template<typename CLK, typename RST>
            void clk_rst(CLK &c, RST &r) {
                clk(c);
                arst_n(r);
            }

            sc_port<apb_if<addr_t, data_t, pstrb_t> > impl;

        private:

            void apb_slave_cthread() {

                impl->apb_reset();
                this->pready = 0;
                this->prdata = 0;
                this->pslverr = 0;
                wait();

                while (1) {

                    this->pready = 0;

                    if (this->psel) {

                        if (this->pwrite) {
                            this->pslverr = impl->apb_write(this->paddr, this->pwdata, this->pstrb);
                        } else {
                            data_t rdata;
                            this->pslverr = impl->apb_read(this->paddr, rdata);
                            this->prdata = rdata;
                        }

                        this->pready = 1;
                        wait();
                    }

                    this->pready = 0;
                    wait();
                }
            }
        };

    }

    template<typename addr_t, typename data_t, typename pstrb_t, bool IO_LEVEL = HLS_IO_DEFAULT_LEVEL>
    struct apb_target : apb_imp::apb_target_imp<addr_t, data_t, pstrb_t, IO_LEVEL>, virtual sc_interface {

        apb_target(const sc_module_name &nm)
                : apb_imp::apb_target_imp<addr_t, data_t, pstrb_t, IO_LEVEL>(nm) {}

    };

}

