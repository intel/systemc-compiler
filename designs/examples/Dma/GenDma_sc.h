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
#ifndef GEN_DMA_H
#define GEN_DMA_H

#include "systemc.h"
#include "register.h"

/**
 * This is the basic version of DMA Device.
 * DMA device helps move data in memory.
 * It's on Burst mode and can be initialized with different burst size.
 * Source address and stride, Destination address and stride
 * and Transfer size of data are the basic input parameters.
 * Dma_start is the signal to switch on and off the DMA.
 * After task completed, Dma will raise an interrupt.
 */

template<unsigned int N>
class GenDma : sc_module
{
    typedef sc_uint<DATA_WIDTH> Data_t;
    typedef sc_uint<ADDR_WIDTH> Addr_t;
    public:

    sc_in<bool> clk{"clk"};
    sc_in<bool> rstn{"rstn"};
    sc_out<bool> interrupt{"interrupt"};
    sc_signal<bool> interrupt_raw{"interrupt_raw"};

    // memory interface
    // req & resp signals
    sc_out<bool> mem_req{"men_req"};
    sc_out<bool> mem_req_oper{"mem_oper"}; // 0:read , 1:write
    sc_out<Addr_t> mem_req_addr{"mem_req_addr"};
    sc_out<Data_t> mem_req_data{"mem_req_data"};
    sc_in<bool> mem_rsp{"mem_rsp"};
    sc_in<bool> mem_rsp_oper{"mem_rsp_oper"}; // 0:read , 1:write
    sc_in<Data_t> mem_rsp_data{"mem_rsp_data"};
    
    // register interface
    cfgRegister cfg{"cfg"};
    sc_in<bool>     cfg_req{"cfg_req"};
    sc_in<bool>     cfg_req_oper{"cfg_oper"}; // 0:read , 1:write
    sc_in<Addr_t>   cfg_req_addr{"cfg_req_addr"};
    sc_in<Data_t>   cfg_req_data{"cfg_req_data"};
    sc_out<bool>    cfg_rsp{"cfg_rsp"};
    sc_out<bool>    cfg_rsp_oper{"cfg_rsp_oper"}; // 0:read , 1:write
    sc_out<Data_t>  cfg_rsp_data{"cfg_rsp_data"};

    sc_signal<bool> interrupt_set{"interrupt_set"};

    SC_CTOR(GenDma) {
        cfg.clk(clk);
        cfg.rstn(rstn);
        cfg.cfg_req(cfg_req);
        cfg.cfg_req_addr(cfg_req_addr);
        cfg.cfg_req_oper(cfg_req_oper);
        cfg.cfg_req_data(cfg_req_data);
        cfg.cfg_rsp(cfg_rsp);
        cfg.cfg_rsp_data(cfg_rsp_data);
        cfg.cfg_rsp_oper(cfg_rsp_oper);
        
        SC_CTHREAD(main_proc, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(interruptOutProc,clk.pos());
        async_reset_signal_is(rstn,false);
    }

    Data_t dataBuff[N]; // DataBuffer of DMA

    void main_proc() {
        // reset section
        mem_req = 0;
        mem_req_oper = 0;
        mem_req_addr = 0;
        mem_req_data = 0;
        
        interrupt_set = 0;
        wait();

        while (true) {
            interrupt_set = 0; 

            if (cfg.dma_start) {
//                cout << sc_time_stamp() << " :Dma starts!" << endl;
                mem_req = 1;
                Addr_t toMove = cfg.transfer_size.read();
                Addr_t offset = 0;
                while(toMove > 0){
                    mem_req_oper = 0;
		    Addr_t steps = Addr_t(N) < toMove? Addr_t(N) : toMove;

                    for (Addr_t i = 0; i < steps; ++i) {
                        mem_req_addr = cfg.src_addr.read()
                                + ((i+offset)*cfg.src_stride.read());
                        wait();
                        if (i > 0){
                            assert(mem_rsp_oper==mem_req_oper && mem_rsp==1);
                            dataBuff[i-1] = mem_rsp_data.read();
                        }
                    }
                    mem_req = 0;
                    wait();
                    dataBuff[steps-1] = mem_rsp_data.read();
                    mem_req = 1;
                    mem_req_oper = 1;

                    for (Addr_t i = 0; i < steps; ++i) {
                        mem_req_addr = cfg.dst_addr.read()
                                + ((i+offset)*cfg.dst_stride.read());
                        mem_req_data = dataBuff[i];
                        wait();
                    }
                    toMove -= steps;
                    offset += steps;
                }
//                cout << sc_time_stamp() << " :Dma ends!" << endl;
                mem_req = 0;
                interrupt_set = 1;
            }
            wait();
        }
    }

    void interruptOutProc()
    {
        interrupt_raw = 0;
        interrupt = 0;
        wait();

        while (true) {
            if (cfg.interrupt_clear) {
                interrupt_raw = 0;
            } else if (interrupt_set){
                interrupt_raw = 1;
            }
            interrupt = interrupt_raw && cfg.interrupt_mask;
            wait();
        }
    }
};

#endif /* GEN_DMA_H */

