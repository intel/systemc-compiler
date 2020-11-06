/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

using namespace sc_core;

// Record array accessed at unknown index, check tuples removed from state in CPA
class A : public sc_module {
public:
    sc_in_clk           clk{"clk"};
    sc_signal<bool>     rst;
    
    sc_signal<sc_uint<4>> sig;

    SC_CTOR(A) 
    {
        SC_CTHREAD(loc_rec_array_thread, clk.pos());
        async_reset_signal_is(rst, true); 

        SC_METHOD(loc_rec_array_meth);
        sensitive << sig;

        SC_METHOD(loc_array_meth);
        sensitive << sig;

        SC_METHOD(loc_array2d_meth);
        sensitive << sig;
    }
    
    struct ArrRec {
        bool        a[3];
        sc_uint<4>  b[3];

        void setA(bool par, int i) {
            a[i] = par;
        }

        bool getA(int i ) {
            return a[i];
        }
    };
    
    struct ArrRecRec {
        ArrRec rec;
    };
    
    
    // Unknown index for record array
    void loc_rec_array_thread()
    {
        wait();
        
        while (true) 
        {
            int j = sig.read();
            ArrRec xarr[2];         // reg
            xarr[1].b[1] = 1;
            xarr[1].b[2] = 2;
            xarr[j].b[2] = 0;       

            sct_assert_unknown(xarr[1].b[1]);
            sct_assert_unknown(xarr[1].b[2]);
            
            wait();

            auto l = xarr[1].b[2];
        }
    }

    void loc_rec_array_meth()
    {
        int j = sig.read();
        ArrRec arr[2];         

        arr[1].b[1] = 1;
        arr[1].b[2] = 2;
        arr[j].b[2] = 0;       

        sct_assert_unknown(arr[1].b[1]);
        sct_assert_unknown(arr[1].b[2]);

        auto l = arr[1].b[2];
    }
    
    void loc_array_meth()
    {
        int j = sig.read();
        sc_uint<4> arr[2];         

        arr[0] = 1;
        arr[1] = 2;
        arr[j] = 0;   

        sct_assert_unknown(arr[0]);
        sct_assert_unknown(arr[1]);

        auto l = arr[1];
    }

    void loc_array2d_meth()
    {
        int j = sig.read();
        sc_uint<4> arr[2][3];         

        arr[1][1] = 1;
        arr[1][2] = 2;
        arr[j][1] = 0;   

        sct_assert_unknown(arr[1][1]);
        sct_assert_unknown(arr[1][2]);

        auto l = arr[1][1];
    }
};

int sc_main(int argc, char *argv[]) {
    sc_clock clk("clk", 1, SC_NS);
    A a{"a"};
    a.clk(clk);
    sc_start();
    return 0;
}
 
