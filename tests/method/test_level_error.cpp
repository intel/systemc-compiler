/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <sct_assert.h>
#include <systemc.h>
#include <stdint.h>
#include <iostream>
#include <string>

// Incorrect level error
class A : public sc_module {
public:
    sc_signal<bool> rstn;
    sc_signal<int>  s;
    
    SC_HAS_PROCESS(A);
    A(const sc_module_name& name) : sc_module(name) 
    {
        SC_METHOD(popRespProc);
        this->sensitive << b;

        SC_METHOD(popRespProc2);
        this->sensitive << a << resp_prior_cntr;
        
        SC_METHOD(cgEnableProc);
        sensitive << no_active_request;
        for (int i = 0; i < ACCESS_POINT_NUM; i++) {
            sensitive << master_req[i] << req_fifo_out_valid[i];
        }
        
        SC_METHOD(long_literal1);
        sensitive << s;
        
        SC_METHOD(long_literal2);
        sensitive << s;
        
        // Mix of break and return does not work -- incorrect code generated 
        // Will not be supported
        //SC_METHOD(long_literal3);
        //sensitive << s;
    }
    
// ---------------------------------------------------------------------------    
    // From MemoCat FullAccessPointBase
    
    static const unsigned SLAVE_NUM = 1;
    static const unsigned SLAVE_NUM_WIDTH = 1;
    static const unsigned RESP_PRIORITY = 0;
    sc_signal<sc_uint<SLAVE_NUM_WIDTH + 1> > resp_prior_cntr;
    sc_signal<bool> a;
    sc_signal<int> b;
    
    void popRespProc() {
        int j = b.read();
        
        if (SLAVE_NUM > 0) {            // B4
            switch (j) {
                case 0:                 // B3
                    break;      
                default: assert(false); // B2
            }
        }       // B0
    }
    
    void popRespProc2() {
        bool rsSlave = a.read();
        bool rsLocal = a.read();
        
        if (SLAVE_NUM > 0) {
            switch (RESP_PRIORITY) {
                case 0:
                    // Local has last index
                    if (resp_prior_cntr.read() == SLAVE_NUM && rsLocal)
                        rsSlave = 0;
                    if (resp_prior_cntr.read() != SLAVE_NUM && rsSlave)
                        rsLocal = 0;
                    break;
                case 1:
                    // If there is no local response give the slot to slave response
                    if (rsLocal) rsSlave = 0;
                    break;
                case 2:
                    // If there is no slave response give the slot to local response
                    if (rsSlave) rsLocal = 0;
                    break;
                default:
                    assert(false);
            }
        }    
    }
    
// ---------------------------------------------------------------------------    

    // From MemoCat CC
    static const unsigned ACCESS_POINT_NUM = 3;
    sc_signal<bool> cg_enable;
    sc_signal<bool> no_active_request;
    sc_signal<bool> master_req[ACCESS_POINT_NUM];
    sc_signal<bool> req_fifo_out_valid[ACCESS_POINT_NUM];
    
    void cgEnableProc()
    {
        bool masterReq = 0;
        for (int i = 0; i < ACCESS_POINT_NUM; i++) {
            masterReq = masterReq || master_req[i] || req_fifo_out_valid[i];
        }
        cg_enable = masterReq || !no_active_request;
    }
    
// ---------------------------------------------------------------------------    
    
    int f(int par) {
        int res;
        switch (par) {
            case 0:
                res = 1; break; // B5
            case 1:
                res = 2; break; // B4
            default:
                res = 3; break; // B3
        }
        return res;         // B1
    }

    int g(int par) {
        switch (par) {      // B1
            case 0:
                return 1;   // B4
            case 1:
                return 2;   // B3
            default:
                return 5;   // B2
        }
    }                       // B0
    
    // Mix of break and return does not work -- incorrect code generated
    int h(int par) {
        switch (par) {  
            case 0:
                break;  
            case 1:
                return 2;
            default:
                return 5;
        }
        return 1;
    }    
    
    void long_literal1() 
    {
        int j = s.read();
        int i = f(j);
    }

    void long_literal2() 
    {
        int j = s.read();
        int i = g(j);
    }

    void long_literal3() 
    {
        int j = s.read();
        int i = h(j);
    }
};


int sc_main(int argc, char *argv[]) 
{
    sc_clock clk{"clk", 1, SC_NS};
    A a_mod{"a_mod"};
    sc_start();
    return 0;
}

