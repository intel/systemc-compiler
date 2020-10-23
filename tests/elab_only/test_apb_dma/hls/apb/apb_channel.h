#pragma once

#include <hls_tlm/hls_tlm.h>

#include "apb_sif.h"
#include "apb_if.h"

namespace intel_hls {

    template<typename addr_t, typename data_t, typename pstrb_t, bool IO_LEVEL>
    struct apb_channel_imp : sc_module {
    };


    // *****************************************************************************
    // TLM specialization **********************************************************
    // *****************************************************************************
    template<typename addr_t, typename data_t, typename pstrb_t>
    struct apb_channel_imp<addr_t, data_t, pstrb_t, HLS_IO_TLM_LEVEL> : sc_module {

        explicit apb_channel_imp(const sc_module_name &nm)
                : p("p"), e("e"), port_bound(false), export_bound(false) {}

        typedef sc_port<apb_if < addr_t, data_t, pstrb_t> > port_t;
        typedef sc_export<apb_if < addr_t, data_t, pstrb_t> > export_t;

        void bind_export(export_t &ex) {
            e.bind(ex);
            export_bound = true;
            bind_pe();
        }

        void bind_port(port_t &po) {
            po.bind(p);
            port_bound = true;
            bind_pe();
        }

    private:
        void bind_pe() {
            if (port_bound && export_bound)
                p.bind(e);
        }

        port_t p;
        export_t e;

        bool port_bound;
        bool export_bound;

    };
    // *****************************************************************************



    // *****************************************************************************
    // Signal-level specialization *************************************************
    // *****************************************************************************
    template<class addr_t, class data_t, typename pstrb_t>
    struct apb_channel_imp<addr_t, data_t, pstrb_t, HLS_IO_SIGNAL_LEVEL>
            : sc_module, apb_channel_sif<addr_t, data_t, pstrb_t> {
        explicit apb_channel_imp(const sc_module_name &nm) {}
    };
    // *****************************************************************************


    template<class addr_t, class data_t, typename pstrb_t, bool IO_LEVEL = HLS_IO_DEFAULT_LEVEL>
    struct apb_channel : apb_channel_imp<addr_t, data_t, pstrb_t, IO_LEVEL> {
        explicit apb_channel(const sc_module_name &nm = sc_gen_unique_name("apb_channel"))
                : apb_channel_imp<addr_t, data_t, pstrb_t, IO_LEVEL>(nm) {}
    };

}
