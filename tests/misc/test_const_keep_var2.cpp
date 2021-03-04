/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"
#include <sct_assert.h>
#include <iostream>
#include <string>

// Check correct constant variable
struct A : public sc_module
{
    sc_in<bool>     clk;
    sc_signal<int>  s;
    
    SC_HAS_PROCESS(A);

    const unsigned domIndx0;
    const unsigned domIndx1;
    const unsigned domIndx2;
    const unsigned defIndx = 42;
    
    A(const sc_module_name& name,
      const unsigned domIndx0_, 
      const unsigned domIndx1_,
      const unsigned domIndx2_) : sc_module(name)
        , domIndx0(domIndx0_)
        , domIndx1(domIndx1_)
        , domIndx2(domIndx2_)
            
    {
        SC_METHOD(switchProc); sensitive << s;
        SC_METHOD(switchForProc); sensitive << s;
        SC_METHOD(switchEmpty); sensitive << s;
    }
   
   
    void switchProc()   
    {
        int indx = 0;
        sc_uint<4> globIndex;
        switch (indx) {
            case 0:
                globIndex = domIndx0;
                break;
            case 1:
                globIndex = domIndx1;
                break;
            default:
                globIndex = 0;
                assert(false);
        }
    }
    
    static const unsigned DOMAIN_NUM = 2;
    void switchForProc()   
    {
        sc_uint<4> globIndex;
        for (int i = 0; i < DOMAIN_NUM; i++) {
            switch (i) {
            case 0:
                globIndex = domIndx0;
                break;
            case 1:
                globIndex = domIndx1;
                break;
            case 2:
                globIndex = domIndx2;
                break;
            default:
                globIndex = defIndx;
                assert(false);
            }
        }
    }
    
    void switchEmpty()   
    {
        int indx = 0;
        sc_uint<4> globIndex;
        switch (indx) {
            case 0:
            case 1:
            case 2:
                globIndex = domIndx1;
                break;
            default:
                globIndex = 0;
        }
    }
};

//-----------------------------------------------------------------------------

int sc_main(int argc, char *argv[]) 
{
    A mod{"mod", 1, 2, 3};
    sc_clock clk{"clk", 1, SC_NS};
    mod.clk(clk);
    
    sc_start();
    return 0;
}

