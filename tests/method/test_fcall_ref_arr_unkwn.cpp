/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"

using namespace sc_core;

// Function parameter references/pointer to array element at variable index 
// which is updated before use. Currently warning reported, it needs to be fixed #182
class A : sc_module {
public:
    SC_HAS_PROCESS(A);

    sc_in_clk clk;
    sc_signal<bool> nrst;
    
    sc_signal<unsigned> indx;

    A(const sc_module_name& name) : sc_module(name) 
    {
        for (int i = 0; i < 3; ++i) {
            parr[i] = sc_new<bool>();
        }

        SC_METHOD(array_unknown_ref);
        sensitive << indx;
        
        SC_METHOD(array_unknown_ptr);
        sensitive << indx;

        SC_CTHREAD(array_ref_wait, clk.pos());
        async_reset_signal_is(nrst, 0);
    }
    

    // For #182, reference to array unknown element, error reported
    template<class T>  
    T ref_elem(T& ref) {
        gindx++;
        return ref;
    }
    
    int gindx;
    void array_unknown_ref() {
        bool larr[3] = {1,0,1};
        gindx = indx.read();
        bool b = ref_elem(larr[0]);     // no warning
        b = ref_elem(larr[gindx]);
    }
    
    // The same with pointer
    template<class T>  
    T ptr_elem(T* ptr) {
        mindx++;
        return *ptr;
    }
    
    bool* parr[3];
    int mindx;
    void array_unknown_ptr() {
        mindx = indx.read();
        bool b = ptr_elem(parr[mindx]);
        b = ptr_elem(parr[1]);          // no warning
    }
    
    // For #182, warning generated
    template<class T>  
    T ref_wait(T& ref) {
        wait();
        return ref;
    }
    
    void array_ref_wait() {
        bool arr[3];
        bool lb;
        wait();
        
        while (true) {
            bool b = ref_wait(arr[indx.read()]);
            wait();
        }
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

