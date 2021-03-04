/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

using namespace sc_core;

// Null array of pointers checked in condition
class A : public sc_module {
public:
    sc_in<bool> clk;
    bool*                   p = nullptr;
    int*                    q = nullptr;
    bool*                   arr1[3];
    sc_uint<4>*             arr2[3] = {};
    sc_signal<bool>*        sarr1[3];
    sc_signal<sc_int<8>>*   sarr2[3] = {};
    
    sc_signal<int>          s;
    sc_signal<bool>         nrst;
    
    SC_CTOR(A) {
        for (int i = 0; i < 3; i++) {
            arr1[i] = nullptr; 
            sarr1[i] = nullptr;
        }
        
        SC_METHOD(meth_if); 
        sensitive << s;

        SC_METHOD(meth_other); 
        sensitive << s;
        
        SC_CTHREAD(thread_other, clk.pos());
        async_reset_signal_is(nrst, false);
    }
    
    void meth_if() 
    {
        int k = 0;
        if (p) {
            *p = 1;
        }
        if (arr1[0]) {
            *arr1[0] = 1;
        }
        if (sarr1[1]) {
            *sarr1[1] = 1;
        }
        if (arr2[0] && *arr2[0]) {
            k = 1;
        }
        if (sarr2[1] && sarr2[1]->read()) {
            k = 2;
        }
        
        // Unknown index does not work
//        int i = s.read();
//        if (arr1[i]) {
//            *arr1[i] = i;
//        }
//        if (sarr1[i]) {
//            *sarr1[i] = i;
//        }
    }  
    
    void meth_other() 
    {
        int k = 0;
        k = (arr1[1]) ? *arr1[1] : 1;
        
        do {
            k++;
        } while (sarr1[1] && k < 2);
        
        if (arr1[2] || sarr1[2]) {
            k++;
        }
    }

    
    void thread_other() {
        
        wait();
        while (true) {
            while (sarr2[1]) {
                *sarr2[1] = 1;
                wait();
            }

            for (; arr2[1]; ) {
                if (s.read()) break;
                wait();
            }
            wait();
        }
    }

};

int sc_main(int argc, char *argv[]) {
    sc_clock clk("clk", 1, SC_NS);
    A a_mod{"a_mod"};
    a_mod.clk(clk);
    sc_start();
    return 0;
}


