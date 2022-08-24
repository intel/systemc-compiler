/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Directed Memory Access(DMA).
 * 
 * Author: Jiacheng Wang
 */

#ifndef REGISTER_H
#define REGISTER_H
#include "systemc.h"
/**
 * This is a basic register model for DMA device.
 * All the parameters of DMA device are stored here for read and write.
 */



class cfgRegister : sc_module, sc_interface 
{
    
    typedef sc_uint<DATA_WIDTH> Data_t;
    typedef sc_uint<ADDR_WIDTH> Addr_t;
    public:

    sc_in<bool> clk{"clk"};
    sc_in<bool> rstn{"rstn"};
    sc_in<bool>     cfg_req{"cfg_req"};
    sc_in<bool>     cfg_req_oper{"cfg_oper"}; // 0:read , 1:write
    sc_in<Addr_t>   cfg_req_addr{"cfg_req_addr"};
    sc_in<Data_t>   cfg_req_data{"cfg_req_data"};
    sc_out<bool>    cfg_rsp{"cfg_rsp"};
    sc_out<bool>    cfg_rsp_oper{"cfg_rsp_oper"}; // 0:read , 1:write
    sc_out<Data_t>  cfg_rsp_data{"cfg_rsp_data"};
    
    sc_signal<Addr_t> src_addr{"src_addr"};
    sc_signal<Addr_t> dst_addr{"dst_addr"};

    sc_signal<Addr_t> transfer_size{"transfer_size"};
    sc_signal<Addr_t> src_stride{"src_stride"};
    sc_signal<Addr_t> dst_stride{"dst_stride"};

    sc_signal<bool> dma_start{"dma_start"};
    // interrupt status, interrupt mask, interrupt raw status
//    sc_signal<bool> interrupt_raw{"interrupt_raw"};
    sc_signal<bool> interrupt_clear{"interrupt_clear"};
    sc_signal<bool> interrupt_mask{"interrupt_mask"};
    sc_signal<bool> interrupt_status{"interrupt_status"};
    
    SC_CTOR(cfgRegister) {
        SC_CTHREAD(config_proc, clk.pos());
        async_reset_signal_is(rstn, false);
    }
    
    void config_proc(){
        cfg_rsp = 0;
        cfg_rsp_data = 0;
        cfg_rsp_oper = 0;
        wait();
        
        while(true)
        {
            cfg_rsp = 0;
            
            if(cfg_req)
            {
                assert(cfg_req_addr.read() < REG_SIZE);
                switch (cfg_req_addr.read()){
                    case 0:
                        if(cfg_req_oper.read()) src_addr = cfg_req_data.read();
                        else cfg_rsp_data = src_addr.read();
                        break;
                    case 1:
                        if(cfg_req_oper.read()) dst_addr = cfg_req_data.read();
                        else cfg_rsp_data = dst_addr.read();
                        break;
                    case 2:
                        if(cfg_req_oper.read()) transfer_size = cfg_req_data.read(); 
                        else cfg_rsp_data = transfer_size.read();
                        break;
                    case 3:
                        if(cfg_req_oper.read()) src_stride = cfg_req_data.read();
                        else cfg_rsp_data = src_stride.read();
                        break;
                    case 4:
                        if(cfg_req_oper.read()) dst_stride = cfg_req_data.read(); 
                        else cfg_rsp_data = dst_stride.read();
                        break;
                    case 5:
                        if (cfg_req_oper.read()) dma_start = cfg_req_data.read();
                        break;
                    case 6:
                        if(cfg_req_oper.read()) interrupt_status = cfg_req_data.read();
                        else cfg_rsp_data = interrupt_status.read();
                        break;
                    case 8:
                        if(cfg_req_oper.read()) interrupt_mask= cfg_req_data.read();
                        else cfg_rsp_data = interrupt_mask.read();
                        break;
                    case 9:
                        if(cfg_req_oper.read()) interrupt_clear= cfg_req_data.read();
                        else cfg_rsp_data = interrupt_clear.read();
                        break;
                    default:
                        cout << "Invalid Address" << endl;
                }
                cfg_rsp = 1;
                cfg_rsp_oper = cfg_req_oper;
            }
            wait();
        }
    }
    
};


#endif /* REGISTER_H */

