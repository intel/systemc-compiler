#pragma once

#include <systemc.h>

namespace intel_hls {

    typedef bool apb_slverr_t;

    struct APB_RESP {
        enum {
            ERROR = 1,
            SUCCESS = 0
        };
    };

    template <typename addr_t, typename data_t, typename pstrb_t>
    struct apb_if : virtual sc_interface {

        virtual void  apb_reset() = 0;
        virtual apb_slverr_t apb_read(addr_t addr, data_t &rdata) = 0;
        virtual apb_slverr_t apb_write(addr_t addr, data_t wdata, pstrb_t pstrb) = 0;

    };

}

