//
// Created by ripopov on 8/18/17.
//

#pragma once

#include <systemc.h>

namespace intel_hls {

    class dma_circ_buffer {
        sc_uint<8>  m_buffer[8];
        sc_uint<4>  m_size;
        sc_uint<3>  m_w_ptr;
        sc_uint<3>  m_r_ptr;

    public:

        sc_uint<4> size() const;
        void put(const sc_uint<32> &data, const sc_uint<3> &n_bytes);
        sc_uint<32> get(const sc_uint<3> &n_bytes);

    };

}
