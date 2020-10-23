//
// Created by ripopov on 8/18/17.
//

#include "dma_circ_buffer.h"

namespace intel_hls {

    sc_uint<4> dma_circ_buffer::size() const {
        return m_size;
    }

    void dma_circ_buffer::put(const sc_uint<32> &data, const sc_uint<3> &n_bytes) {
        if (n_bytes > 0) m_buffer[m_w_ptr++] = data(7, 0);
        if (n_bytes > 1) m_buffer[m_w_ptr++] = data(15, 8);
        if (n_bytes > 2) m_buffer[m_w_ptr++] = data(23, 16);
        if (n_bytes > 3) m_buffer[m_w_ptr++] = data(31, 24);

        m_size += n_bytes;
    }

    sc_uint<32> dma_circ_buffer::get(const sc_uint<3> &n_bytes) {
        assert(n_bytes <= size());
        sc_uint<32> rdata = 0;

        if (n_bytes > 0) rdata(7, 0) = m_buffer[m_r_ptr++];
        if (n_bytes > 1) rdata(15, 8) = m_buffer[m_r_ptr++];
        if (n_bytes > 2) rdata(23, 16) = m_buffer[m_r_ptr++];
        if (n_bytes > 3) rdata(31, 24) = m_buffer[m_r_ptr++];

        m_size -= n_bytes;
        return rdata;
    }

}
