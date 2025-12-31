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
        SC_METHOD(reference3); sensitive << s;
        
        // Pointer reference is not supported yet
        //SC_METHOD(pointer); sensitive << s;
    }

    // Array element unknown reference
    sc_signal<int> t0;
    void reference() {
        unsigned larr[5];
        
        int i = s.read();
        unsigned& r = larr[i];     // warning 
        unsigned& rn = larr[0];    // no warning
        i++;
        r = 1;   // larr[i] = 1; is incorrect as @i was updated before
        t0 = r;
    }    
    
    sc_signal<int> t1;
    void reference2() {
        int i = s.read();
        unsigned& r = arr[i][1];   // warning
        unsigned& rn = arr[1][1];  // no warning
        i++;
        i = r + rn;   // i = arr[i][1]; is incorrect as @i was updated before
        t1 = i;
    }
    
    unsigned marr[5];
    unsigned marr2[5][5];
    std::array<unsigned, 3> sarr;
    sc_signal<int> t2;
    void reference3() {
        unsigned res;
        unsigned larr[5];
        int i = 2;
        unsigned& r1 = marr[i];         // warning
        unsigned& r2 = larr[i+1];       // warning
        unsigned& r3 = marr2[i][1];     // warning
        unsigned& r4 = marr2[2][i];     // warning
        unsigned& rn = marr[4];         // no warning
        
        unsigned& r5 = sarr[i];         // warning
        unsigned& rn2 = sarr[1];        // no warning

        std::array<unsigned, 3> slarr;
        unsigned& r6 = slarr[i];        // warning
        
        i++;
        res = r1 + r2 + r3 + r4 + r5 + r6 + rn + rn2;
        t2 = res;
    }
    
    // Array element unknown pointer
    sc_signal<int> t5;
    void pointer() {
        sc_uint<4> larr[5];
        
        int i = s.read();
        sc_uint<4>* r = &larr[i];
        i++;
        i = *r;  // i = larr[i]; is incorrect as @i was updated before
        t5 = i;
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

