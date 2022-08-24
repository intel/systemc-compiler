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

#include "systemc.h"
#include <iostream>
#include <chrono>
#include <vector>

static const unsigned DATA_WIDTH = 32;
static const unsigned ADDR_WIDTH = 20;
static const unsigned MEMORY_SIZE = 1024 * 1024; // size in 64-bit words
static const unsigned REG_SIZE = 10;
static const unsigned BURST_SIZE = 16;
// CfgRgister Addresses
static const unsigned CFG_SRC_ADDR = 0;
static const unsigned CFG_DST_ADDR = 1;
static const unsigned CFG_TRANSFER_SIZE = 2;
static const unsigned CFG_SRC_STRIDE = 3;
static const unsigned CFG_DST_STRIDE = 4;
static const unsigned CFG_DMA_START = 5;
static const unsigned CFG_INTERRUPT_STATUS = 6;
static const unsigned CFG_INTERRUPT_RAW = 7;
static const unsigned CFG_INTERRUPT_MASK = 8;
static const unsigned CFG_INTERRUPT_CLEAR = 9;

//#ifdef RTL_SIM
//#include "GenDma.h"
//#else
#include "GenDma_sc.h"
#include "register.h"
//#endif

SC_MODULE(tbtop) {
    typedef sc_uint<DATA_WIDTH> Data_t;
    typedef sc_uint<ADDR_WIDTH> Addr_t;
    public:

//    sct_initiator<Data_t> init{"init"};
    sc_clock refclk{"refclk", sc_time(10, SC_NS)};
    sc_in_clk test_clk{"test_clk"};

    sc_signal<bool> rstn{"rstn"};

    // memory interface signals
    sc_signal<bool> mem_req{"mem_req"}; // request
    sc_signal<bool> mem_req_oper{"mem_req_oper"}; // op type: 0 - read, 1 - write
    sc_signal<Addr_t> mem_req_addr{"mem_req_addr"};
    sc_signal<Data_t> mem_req_data{"mem_req_data"};

    sc_signal<bool> mem_rsp{"mem_rsp"};
    sc_signal<bool> mem_rsp_oper{"mem_rsp_oper"};
    sc_signal<Data_t> mem_rsp_data{"mem_rsp_data"};

    // Cfg Interface
    sc_signal<bool> cfg_req{"cfg_req"};
    sc_signal<bool> cfg_req_oper{"cfg_oper"}; // 0:read , 1:write
    sc_signal<Addr_t> cfg_req_addr{"cfg_req_addr"};
    sc_signal<Data_t> cfg_req_data{"cfg_req_data"};

    sc_signal<bool> cfg_rsp{"cfg_rsp"};
    sc_signal<bool> cfg_rsp_oper{"cfg_rsp_oper"}; // 0:read , 1:write
    sc_signal<Data_t> cfg_rsp_data{"cfg_rsp_data"};

    // interrupt output
    sc_signal<bool> interrupt{"interrupt"};

    GenDma<BURST_SIZE> dut{"dut"};

    SC_CTOR(tbtop) {
        test_clk(refclk);

        // bind dut ports to the memory interface
        // and to the control/status register fields
        // ...

        SC_CTHREAD(test_proc, test_clk.pos());

        // allocate memory buffer for memory model
        memory.resize(MEMORY_SIZE);

        dut.clk(test_clk);
        dut.rstn(rstn);
        dut.mem_req(mem_req);
        dut.mem_req_oper(mem_req_oper);
        dut.mem_req_addr(mem_req_addr);
        dut.mem_req_data(mem_req_data);
        dut.mem_rsp(mem_rsp);
        dut.mem_rsp_oper(mem_rsp_oper);
        dut.mem_rsp_data(mem_rsp_data);

        dut.cfg_req(cfg_req);
        dut.cfg_req_oper(cfg_req_oper);
        dut.cfg_req_addr(cfg_req_addr);
        dut.cfg_req_data(cfg_req_data);
        dut.cfg_rsp(cfg_rsp);
        dut.cfg_rsp_oper(cfg_rsp_oper);
        dut.cfg_rsp_data(cfg_rsp_data);
        dut.interrupt(interrupt);

        SC_CTHREAD(memModelProc, test_clk.pos());
        async_reset_signal_is(rstn, false);

    }

    // Test scheduler process.

    void test_proc() {
        // reset
        rstn = 0;
        wait(2);
        rstn = 1;
        wait();
        Addr_t src_addr_arr[5] = {};
        Addr_t dst_addr_arr[5] = {};
        Addr_t transfer_size_arr[9] = {1,4,8,16,32,64,128,256,512};
        Addr_t src_stride_arr[5] = {1,5,16,64,128};
        Addr_t dst_stride_arr[5] = {1,5,16,64,128};
        for(int i = 0;i<5;i++){
            src_addr_arr[i] = 1024 * i;
            dst_addr_arr[i] = 1024 * (100+i);
        }
        //    initialize src data in memory
        for (unsigned i = 0; i < MEMORY_SIZE; i++) {
            memory[i] = i;
        }
        cout << " --- <<< uSMEM TB Started >>> ---" << endl;
        std::chrono::steady_clock::time_point t_begin = 
        std::chrono::steady_clock::now();
//        testFunc(0,100,1,1,1);
        unsigned i = 0;
        for(auto& src_addr:src_addr_arr){
            for(auto& dst_addr:dst_addr_arr){
                for(auto& transfer_size:transfer_size_arr){
                    for(auto& src_stride:src_stride_arr){
                        for(auto& dst_stride:dst_stride_arr){
                            testFunc(src_addr,dst_addr,transfer_size,src_stride,dst_stride);
                            i++;
                        }
                    }
                }
            }
        }
        // Testing done
        cout << "No. " << i << endl;
        std::chrono::steady_clock::time_point t_end = 
                std::chrono::steady_clock::now();
        std::cout << "Test run time = " << dec 
            << std::chrono::duration_cast<std::chrono::milliseconds>(t_end - t_begin).count() 
            << " [ms]" << std::endl;

        // Call sc_stop() to finish simulation
        sc_stop();
    }
    
    void testFunc(unsigned src_addr,unsigned dst_addr,unsigned transfer_size,
            unsigned src_stride,unsigned dst_stride){
        writeReg(CFG_SRC_ADDR, src_addr); // src_addr
        writeReg(CFG_DST_ADDR, dst_addr); // dst_addr
        writeReg(CFG_TRANSFER_SIZE, transfer_size); // transfer_size,test cases:1,15,16,32
        writeReg(CFG_SRC_STRIDE, src_stride); // src_stride
        writeReg(CFG_DST_STRIDE, dst_stride); // dst_stride
        writeReg(CFG_INTERRUPT_CLEAR, 0);
        
        // Configure and run dma
        assert(interrupt.read()==0);
        writeReg(CFG_DMA_START, 1); // set dma_start to 1
        writeReg(CFG_DMA_START, 0); // set dma_start to 0
        writeReg(CFG_INTERRUPT_MASK, 1); // write interrupt_mask to 1
        
        // wait interrupt
        while(!interrupt.read()) wait();
        writeReg(CFG_INTERRUPT_CLEAR, 1); // write interrupt_clear to 1
        wait(3);
        assert(interrupt.read() == 0);
        //    check dst data in memory
        for (unsigned i = 0; i < transfer_size; i++) {
            assert(memory[src_addr + i * src_stride] 
                == memory[dst_addr + i * dst_stride]);
        }
    }
    
    // Process to emulate memory interface for dut
    std::vector<Data_t> memory;

    void memModelProc() {
        mem_rsp = 0;
        mem_rsp_data = 0;
        mem_rsp_oper = 0;
        wait();

        while (true) {
            // clear response
            mem_rsp = 0;

            // handle incoming request
            if (mem_req) {
                assert(mem_req_addr.read() < MEMORY_SIZE);
                if (mem_req_oper) { // write
                    memory[mem_req_addr.read()] = mem_req_data;
                } else { // read
                    mem_rsp_data = memory[mem_req_addr.read()];
                }
                // set response
                mem_rsp = 1;
                mem_rsp_oper = mem_req_oper;
            }
            wait();
        }
    }

    void writeReg(Addr_t addr, Data_t data) {
        // Write into CfgRegister
        cfg_req = 1;
        cfg_req_oper = 1;
        cfg_req_addr = addr;
        cfg_req_data = data;
        wait();
        cfg_req = 0;
        while (!cfg_rsp.read()) wait();
    }

    Data_t readReg(Addr_t addr) {
        // Read from CfgRegister
        cfg_req = 1;
        cfg_req_oper = 0;
        cfg_req_addr = addr;
        wait();
        cfg_req = 0;
        while (!cfg_rsp.read()) wait();
        return cfg_rsp_data.read();
    }


    // test memory model interface

    void testMemModel() {
        testMemModelWrRd(0, 1);
        wait();
        testMemModelWrRd(120, 1);
        wait(2);
        testMemModelWrRd(10, 2);
        wait();
        testMemModelWrRd(110, 2);
        wait();
        testMemModelWrRd(200, 8);
    }

    void testMemModelWrRd(unsigned start_addr = 0, unsigned size = 8) {
        // write 16 words
        // check no response before requests
        assert(mem_rsp == 0);
        for (unsigned i = 0; i < size; ++i) {
            mem_req = 1;
            mem_req_oper = 1; // write
            mem_req_addr = start_addr + i;
            mem_req_data = i + 10;
            wait();

            // after the second request we will be 
            // receiving write responses (latency 1 cycle)
            if (i > 0) {
                assert(mem_rsp == 1);
                assert(mem_rsp_oper == 1);
            }
        }
        mem_req = 0; // clear req
        wait();
        // check the last write response
        assert(mem_rsp == 1);
        assert(mem_rsp_oper == 1);
        wait();
        //check write response cleared
        assert(mem_rsp == 0);

        // check data was written successfully
        for (unsigned i = 0; i < size; ++i) {
            assert(memory[start_addr + i] == i + 10);
        }

        // read back 16 words and verify data
        Data_t ref_data;
        for (unsigned i = 0; i < size; ++i) {
            mem_req = 1;
            mem_req_oper = 0; // read
            mem_req_addr = start_addr + i;
            wait();

            // after the second request we will be 
            // receiving read responses (latency 1 cycle)
            if (i > 0) {
                assert(mem_rsp == 1);
                assert(mem_rsp_oper == 0);
                assert(mem_rsp_data.read() == ref_data);
            }
            ref_data = i + 10;
        }
        mem_req = 0; // clear req
        wait();
        // check the last read response
        assert(mem_rsp == 1);
        assert(mem_rsp_oper == 0);
        assert(mem_rsp_data.read() == ref_data);
        wait();
        //check read response cleared
        assert(mem_rsp == 0);
    }
};

/** SC main function */
int sc_main(int argc, char** argv) {
    sc_set_time_resolution(1, SC_PS);

    tbtop tb("tb");

    cout << " --- <<< TEST RUN >>> ---" << endl;

    do {
        sc_start(sc_time(10, SC_US));
    } while (sc_is_running());

    cout << endl;
    cout << "=====================================================" << endl;
    cout << "||              All tests passed OK                ||" << endl;
    cout << "=====================================================" << endl;

    return 0;
}
