//
// Created by ripopov on 8/21/17.
//

#pragma once

#include <systemc.h>
#include <tlm.h>
#include <tlm_utils/simple_initiator_socket.h>
#include <apb/apb.h>
#include <dma/dma_params.h>
#include <sstream>

namespace intel_hls {

    template <bool IO_LEVEL = HLS_IO_DEFAULT_LEVEL>
    struct apb2tlm : hls_module, apb_if<bus_addr_t, bus_data_t, bus_strobe_t>  {

        apb_target<bus_addr_t, bus_data_t, bus_strobe_t, IO_LEVEL> apb_tar;
        tlm_utils::simple_initiator_socket<apb2tlm> tlm_initiator;

        apb2tlm(::sc_core::sc_module_name)
                : apb_tar("apb_tar") {

            apb_tar.impl.bind(*this);

            // not supported by Simics
            // payload.set_byte_enable_length(4);
            // payload.set_byte_enable_ptr(byte_enable);

        }

        tlm::tlm_generic_payload  payload;
        //unsigned char byte_enable[4];

        void apb_reset() override {
            LOG_DEBUG("apb2tlm::apb_reset not supported by tlm20 bridge, ignoring");
        }

        apb_slverr_t apb_read(bus_addr_t addr, bus_data_t &rdata) override {

            payload.set_streaming_width(4);
            payload.set_data_length(4);
            payload.set_address(addr);
            uint32_t read_data;
            payload.set_data_ptr(reinterpret_cast<unsigned char *>(&read_data));
            payload.set_read();

            sc_time delay = SC_ZERO_TIME;

            std::stringstream ss;

            tlm_initiator->b_transport(payload, delay);
            wait(delay);

            if (payload.get_response_status() != tlm::TLM_OK_RESPONSE) {
                LOG_WARNING("apb2tlm::apb_read tlm error response : " << dec << payload.get_response_status());
                return APB_RESP::ERROR;
            }

            rdata = *(reinterpret_cast<uint32_t *>(payload.get_data_ptr()));
            return APB_RESP::SUCCESS;
        }

        apb_slverr_t apb_write(bus_addr_t addr, bus_data_t wdata, bus_strobe_t pstrb) override {

            if (pstrb == 0)
                LOG_ERROR("pstrb is 0");

            uint32_t write_data = wdata;

            uint32_t data_len = 4;

            while (pstrb[0] == 0) {
                pstrb >>= 1;
                addr ++;
                data_len --;
                write_data >>= 8;
            }

            payload.set_address(addr);
            payload.set_streaming_width(data_len);
            payload.set_data_length(data_len);

            payload.set_data_ptr(reinterpret_cast<unsigned char *>(&write_data));
            payload.set_write();
            //for (int ii = 0; ii < 4; ++ii) {
            //    byte_enable[ii] = static_cast<unsigned char>(pstrb[ii].to_bool() ? 0xff : 0x00);
            //}

            sc_time delay(0, SC_NS);
            tlm_initiator->b_transport(payload, delay);
            wait(delay);

            if (payload.get_response_status() != tlm::TLM_OK_RESPONSE) {
                LOG_WARNING("apb2tlm::apb_write tlm error response : " << dec << payload.get_response_status());
                return APB_RESP::ERROR;
            }
            return APB_RESP::SUCCESS;

        }
    };

}