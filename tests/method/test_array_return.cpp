/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include <systemc.h>

// Array returned from function
SC_MODULE(test) {

    sc_signal<bool>         a;
    sc_signal<sc_uint<3>>   b;

    SC_CTOR(test) 
    {
        SC_METHOD(const_array_return);
        sensitive << a << b;
        
        // TODO: Fix me, #227
        SC_METHOD(use_array_return);
        sensitive << a << b;

    }
  
    int* arr_ret1(int* arr) 
    {
        arr[0] = 1; arr[1] = 2; arr[2] = 3;
        
        return arr;
    }
    
    const int carr[3] = {4,5,6};
    const int* arr_ret2() 
    {
        const int* p = carr;
        return p;
    }

    void const_array_return() 
    {
        int c[3];
        int* d = arr_ret1(c);
        sct_assert_const (d[0] == 1);
        sct_assert_const (d[1] == 2);
        sct_assert_const (d[2] == 3);
        
        const int* e = arr_ret2();
        sct_assert_const (e[0] == 4);
        sct_assert_const (e[1] == 5);
        sct_assert_const (e[2] == 6);
    }
    
    // Using array returned from function, #227
    int marr[3];
    int* arr_ret3() 
    {
        int* p = marr;
        p[b.read()] = 1;
        return p;
    }
    
    void use_array_return() 
    {
        int c[3];
        int* d = arr_ret1(c);
        const int* e = arr_ret2();
        int* f = arr_ret3();
        
        int i = d[0] + e[0] + d[0];
    }
};

int sc_main(int argc, char **argv) {
    test t_inst{"t_inst"};
    sc_start();
    return 0;
}

