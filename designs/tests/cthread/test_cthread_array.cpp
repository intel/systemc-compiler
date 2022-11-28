/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Array general cases and some special cases
class top : sc_module
{
public:
    sc_in<bool>     clk{"clk"};
    sc_signal<bool> arstn{"arstn", 1};
    
    sc_signal<bool> a{"a"};
    sc_signal<bool>* ps;
    
    static const unsigned DOMAIN_NUM = 1;
    sc_signal<bool>*    arbt_req[DOMAIN_NUM];

    sc_signal<bool>    chans[3];
    sc_signal<bool>    chans2d[3][3];

    int*                m[3];
    int*                n[3][3];
    
    SC_HAS_PROCESS(top);
    top(sc_module_name)
    {
        ps = new sc_signal<bool>("ps");
        for (int i = 0; i < DOMAIN_NUM; i++) {   
            arbt_req[i] = new sc_signal<bool>("arbt_req");
        }
        
        for (int i = 0; i < 3; ++i) {
            m[i] = sc_new<int>();
            for (int j = 0; j < 3; ++j) {
                n[i][j] = sc_new<int>();
            }
        }
        
        SC_CTHREAD(bug_in_array_index, clk.pos());
        async_reset_signal_is(arstn, false);

        SC_CTHREAD(bug_in_array_index2, clk.pos());
        async_reset_signal_is(arstn, false);

        SC_METHOD(array_of_pointers);
        sensitive << a;
        
        SC_METHOD(chan_array_read);
        sensitive << a << chans[0] << chans2d[0][0];
        
        // No reset process
        SC_CTHREAD(operProc, clk.pos())
        
        SC_CTHREAD(array_of_pointers2, clk.pos());
        async_reset_signal_is(arstn, false);
    }

    // BUG in real design -- in ConfigCtrl.h extra copy of @reqSource in Verilog
    // Shorten version
    sc_signal<sc_uint<2>>   fifo_source;
    sc_signal<sc_uint<2>>   master_resp_data[3];
    static const unsigned ECC_NUM           = 0;
    
    void bug_in_array_index() {
        wait();
        
        while (1) {
            sc_uint<2> reqSource = fifo_source.read(); 
            bool reqOper = a.read();
            bool reqReg;
            
            master_resp_data[reqSource] = (
                        ((ECC_NUM > 0) ? (sc_uint<1>)a : 
                                        (sc_uint<1>)0),
                        ((ps) ? (sc_uint<1>)ps->read() : 
                                         (sc_uint<1>)0),
                        ((ps) ? (sc_uint<1>)ps->read() : 
                                           (sc_uint<1>)0));
            
            wait();
        }
    }
    
    // BUG in real design -- in ConfigCtrl.h extra copy of @reqSource in Verilog
    // Full version
    void bug_in_array_index2() {
        wait();
        
        while (1) {
            sc_uint<2> reqSource = fifo_source.read(); 
            bool reqOper = a.read();
            bool reqReg;
            
            master_resp_data[reqSource] = (
                    (ECC_NUM > 0) ? (sc_uint<1>)a : 
                                    (sc_uint<1>)0,
                    (ECC_NUM > 0) ? (sc_uint<1>)a : 
                                    (sc_uint<1>)0,
                    (ps) ? (sc_uint<1>)ps->read() : 
                                    (sc_uint<1>)0,
                    (sc_uint<5>)0,
                    (ps) ? (sc_uint<1>)ps->read() : 
                                    (sc_uint<1>)0,
                    (ps) ? (sc_uint<1>)ps->read() : 
                                     (sc_uint<1>)0,
                    (ps) ? (sc_uint<1>)ps->read() : 
                                    (sc_uint<1>)0,
                    (ps) ? (sc_uint<1>)ps->read() : 
                                   (sc_uint<1>)0,
                    (ps) ? (sc_uint<1>)ps->read() : 
                                       (sc_uint<1>)0,
                    (ps) ? (sc_uint<1>)ps->read() : 
                                       (sc_uint<1>)0,
                    (ps) ? (sc_uint<1>)ps->read() : 
                                     (sc_uint<1>)0,
                    (ps) ? (sc_uint<1>)ps->read() : 
                                       (sc_uint<1>)0,
                    (sc_uint<1>)reqOper,
                    (sc_uint<1>)reqOper);
            
            wait();
        }
    }
    
    // ------------------------------------------------------------------------
    void array_of_pointers()
    {
        for (int i = 0; i < DOMAIN_NUM; i++) {
            *(this->arbt_req[i]) = 0;
        }
    }

    void chan_array_read()
    {
        for (int i = 0; i < DOMAIN_NUM; i++) {
            bool b = chans[i].read();
            b = chans[i];
            b = !chans[i];

            for (int j = 0; j < DOMAIN_NUM; j++) {
                b = chans2d[i][j];
                b = !chans2d[i][j];
            }
        }
    }
    
    // ------------------------------------------------------------------------
    static const unsigned WORD_NUMBER = 4;
    sc_uint<16> block_memory[WORD_NUMBER];
    
    sc_signal<bool> pwrout_nenable;
    sc_signal<bool> pwrin_nenable;
    sc_signal<bool> renbl;
    sc_signal<sc_uint<2>> addr;
    sc_signal<sc_uint<16>> wdata;
    
    sc_signal<bool> pwrout_;
    
    // No reset process
    void operProc() 
    {
        while (true) {
            pwrout_nenable = pwrout_;
            pwrout_        = pwrin_nenable;
            
            if (!pwrout_nenable) {
                if (renbl) {
                    sc_uint<16> readData = block_memory[addr.read()];

                } else {
                    block_memory[addr.read()] = wdata;
                }
            }
            
            wait();
        }
    }
    
    // array of pointers to non-channels
    void array_of_pointers2() {
        for (int i = 0; i < 3; ++i) *m[i] = 0;
        int i = *m[0];
        wait();
        
        while (true) {
            *m[1] = *m[0] + i;
            wait();
            *n[0][1] = *m[1];
        }
    }
    
};

int sc_main(int argc, char *argv[])
{
    sc_clock clk{"clk", 10, SC_NS};
    top top_inst{"top_inst"};
    top_inst.clk(clk);
    sc_start(100, SC_NS);
    return 0;
}

