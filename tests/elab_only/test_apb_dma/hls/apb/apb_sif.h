#pragma once

#include <systemc.h>

#define DECLARE_APB_PORTS(hport_t, dport_t) \
    hport_t<addr_t>     paddr; \
    hport_t<bool>       psel; \
    hport_t<bool>       penable; \
    hport_t<bool>       pwrite; \
    hport_t<pstrb_t>    pstrb; \
    hport_t<data_t>     pwdata; \
    dport_t<bool>       pready; \
    dport_t<data_t>     prdata; \
    dport_t<bool>       pslverr

#define APB_PORT_CTORS \
      paddr("paddr") \
    , psel("psel") \
    , penable("penable") \
    , pwrite("pwrite") \
    , pstrb("pstrb") \
    , pwdata("pwdata") \
    , pready("pready") \
    , prdata("prdata") \
    , pslverr("pslverr")

#define APB_BIND_PORTS \
template <class chan_t> \
void bind(chan_t &chan) { \
    paddr.bind(chan.paddr); \
    psel.bind(chan.psel); \
    penable.bind(chan.penable); \
    pwrite.bind(chan.pwrite); \
    pstrb.bind(chan.pstrb); \
    pwdata.bind(chan.pwdata); \
    pready.bind(chan.pready); \
    prdata.bind(chan.prdata); \
    pslverr.bind(chan.pslverr); \
}

namespace intel_hls {

    template <typename addr_t, typename data_t, typename pstrb_t>
    struct apb_slave_sif {
        #ifdef STRATUS
            HLS_METAPORT;
        #endif

        DECLARE_APB_PORTS(sc_in, sc_out);
        apb_slave_sif() : APB_PORT_CTORS {}
        APB_BIND_PORTS
    };

    template < typename addr_t, typename data_t, typename pstrb_t >
    struct apb_master_sif {
        #ifdef STRATUS
            HLS_METAPORT;
        #endif
        DECLARE_APB_PORTS(sc_out, sc_in);
        apb_master_sif() : APB_PORT_CTORS {}
        APB_BIND_PORTS
    };

    template < typename addr_t, typename data_t, typename pstrb_t >
    struct apb_channel_sif {
        DECLARE_APB_PORTS(sc_signal, sc_signal);
        apb_channel_sif() : APB_PORT_CTORS {}
    };

}
