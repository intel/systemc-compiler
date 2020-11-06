/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"

using namespace sc_core;

// References/pointer to array element at variable which is updated before use
// Currently error reported, it needs to be fixed #182
class A : public sc_module {
public:
    sc_in_clk clk;
    sc_signal<bool> nrst;

    unsigned arr[3][4];
    sc_signal<sc_uint<8>>   s;

    SC_CTOR(A) {
        SC_METHOD(reference); sensitive << s;
        SC_METHOD(reference2); sensitive << s;
        
        // Pointer reference is not supported yet
        //SC_METHOD(pointer); sensitive << s;
    }

    // Array element unknown reference
    void reference() {
        unsigned larr[5];
        
        int i = s.read();
        unsigned& r = larr[i];
        unsigned& rn = larr[0];    // no warning
        i++;
        r = 1;   // larr[i] = 1; is incorrect as @i was updated before
    }    
    
    void reference2() {
        int i = s.read();
        unsigned& r = arr[i][1];
        unsigned& rn = arr[1][1];  // no warning
        i++;
        i = r;   // i = arr[i][1]; is incorrect as @i was updated before
    }
    
    // Array element unknown pointer
    void pointer() {
        sc_uint<4> larr[5];
        
        int i = s.read();
        sc_uint<4>* r = &larr[i];
        i++;
        i = *r;  // i = larr[i]; is incorrect as @i was updated before
    }    
};

int sc_main(int argc, char *argv[]) 
{
    sc_clock clk{"clk", 1, SC_NS};
    A a_mod{"a_mod"};
    a_mod.clk(clk);
    
    sc_start();
    return 0;
}

