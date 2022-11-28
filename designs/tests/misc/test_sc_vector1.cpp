/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"

// Vector of channels (sc_vector) in CTHREAD and METHOD test
SC_MODULE(Top) {

    sc_in<bool>  clk;
    sc_signal<bool>  rstn;
    
    sc_vector<sc_in<bool>>  req{"req"};
    sc_vector<sc_out<bool>> res{"resp", 3};
    sc_out<bool>            res_[3];

    sc_vector<sc_signal<bool>> enb{"enb", 4};
    sc_vector<sc_signal<sc_uint<5>>> val{"val", 4};
    sc_vector<sc_signal<sc_int<7>>> mval{"mval", 2};
    
    sc_vector<sc_vector<sc_signal<int>>> val2d{"val", 2};

    SC_CTOR(Top) {
        
        req.init(3);
        val2d[0].init(3);
        val2d[1].init(3);

        SC_METHOD(simpleMeth);
        for (int i = 0; i < 3; ++i) {
            sensitive << req[i];
        }
        for (int i = 0; i < 2; ++i) {
            for(int j = 0; j < 3; ++j) {
                sensitive << val2d[i][j];
            }
        }
        sensitive << enb[3];
        
        SC_CTHREAD(matchThread, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(signalThread, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(vector2DThread, clk.pos());
        async_reset_signal_is(rstn, false);
    }
    
    // Method read and write @sc_vector
    void simpleMeth() 
    {
        bool b = enb[3];
        for(int i = 0; i < 3; ++i) {
            b = b ^ req[i] ^ req[i];
        }
        
        for(int i = 0; i < 2; ++i) {
            int r = 0;
            for(int j = 0; j < 3; ++j) {
                r = r - val2d[i][j].read();
            }
            mval[i] = r;
        }
    }
    
    // Match array of sc_out and vector of sc_out
    void matchThread() 
    {
        for(int i = 0; i < 3; ++i) {
            res[i] = 0; res_[i] = 0;
        }
        wait();
        
        while(true) {
            for(int i = 0; i < 3; ++i) {
                res[i] = req[i]; res_[i] = req[i]; 
            }
            wait();
            bool b = res[1];
            bool c = res_[1];
        }
    }

    // Access some elements of signal vector
    void signalThread() 
    {
        for(int i = 0; i < 3; ++i) {
            val[i+1] = i;
            enb[i] = 0;
        }
        enb[3] = 1;
        wait();
        
        while(true) {
            for(int i = 0; i < 4; ++i) {
                val[i] = enb[i] ? i : i+1; 
            }
            if (val[1] == val[2]) {
                enb[1] = 0;
            }
            wait();
        }
    }
    
    // Vector of vectors of signals
    void vector2DThread() 
    {
        for(int i = 0; i < 2; ++i) {
        for(int j = 0; j < 3; ++j) {
            val2d[i][j] = 0;
        }}
        wait();
        
        while(true) {
            for(int j = 0; j < 3; ++j) {
                val2d[0][j] = val[j].read() + 1;
            }
            wait();
            bool b = val2d[0][1].read() + val2d[1][2].read();
        }
    }
};

int sc_main(int argc, char **argv) {

    Top top("top");
    
    sc_signal<bool> clk;
    sc_signal<bool> s[3];
    top.clk(clk);
    
    for(int i = 0; i < 3; ++i) {
         top.req[i](s[i]);
         top.res[i](s[i]);
         top.res_[i](s[i]);
    }
    
    sc_start();

    return 0;
}
