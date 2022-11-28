/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include <systemc.h>

// Conditional pointer/array assignment to pointer -- error reported
SC_MODULE(test) {

    sc_signal<bool>         a;

    int i1, i2;
    int *p1, *p2;

    SC_CTOR(test) 
    {
        SC_METHOD(array_return1);
        sensitive << a;
        
        p1 = &i1;
        p2 = &i2;
    }
  
    int* arr_ret1(int* arr) {
        arr[0] = 1; arr[1] = 2; arr[2] = 3;
        return arr;
    }
    
    int* ptr_ret() {
        return p2;
    }
    
    void array_return1() 
    {
        int c[3];
        int cc[4];
        int* d = a.read() ? c : cc;  // Error
        int i = d[0];
        
        int l1, l2;
        int& r = a.read() ? l1 : l2; // Error
        r = 1;
        
        int* p = a.read() ? p1 : p2; // Error
        *p = 1;
        
        auto q = a.read() ? arr_ret1(c) : ptr_ret(); // Error
        *q = 1;
    }
};

int sc_main(int argc, char **argv) {
    test t_inst{"t_inst"};
    sc_start();
    return 0;
}


