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
        SC_METHOD(array_return);
        sensitive << a;

    }
  
    int* arr_ret1(int* arr) 
    {
        arr[0] = 1; arr[1] = 2; arr[2] = 3;
        
        return arr;
    }
    
    void array_return() 
    {
        int c[3];
        int* d = arr_ret1(c);

        sct_assert_const (d[0] == 1);
        sct_assert_const (d[1] == 2);
        sct_assert_const (d[2] == 3);
    }
};

int sc_main(int argc, char **argv) {
    test t_inst{"t_inst"};
    sc_start();
    return 0;
}

