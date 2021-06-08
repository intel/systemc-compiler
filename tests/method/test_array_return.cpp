/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include <systemc.h>

// Array/pointer returned from function
SC_MODULE(test) {

    sc_signal<bool>         a;
    sc_signal<sc_uint<3>>   b;

    int i1;
    int i2;
    int *p1;
    int *p2;

    SC_CTOR(test) 
    {
        p1 = &i1;
        p2 = &i2;
        
        SC_METHOD(array_return1);
        sensitive << a << b;
        
        SC_METHOD(array_return2);
        sensitive << a << b;

    }
  
    int* arr_ret1(int* arr) {
        arr[0] = 1; arr[1] = 2; arr[2] = 3;
        return arr;
    }
    
    const int carr[3] = {4,5,6};
    const int* arr_ret2() {
        const int* p = carr;
        return p;
    }

    int* ptr_ret() {
        return p2;
    }

    void array_return1() 
    {
        int* p = p1;
        *p = 1;
        sct_assert_const (i1 == 1);
        auto q = ptr_ret();
        *q = 2;
        sct_assert_const (i2 == 2);

        int c[3];
        int cc[4];
        int* d = arr_ret1(c);
        int i = d[0] + d[1] + d[b.read()];
        d[2] = 4;
        sct_assert_const (d[0] == 1);
        sct_assert_const (d[1] == 2);
        sct_assert_const (d[2] == 4);
        
        const int* e = arr_ret2();
        i = e[i] - d[0];
        sct_assert_const (e[0] == 4);
        sct_assert_const (e[1] == 5);
        sct_assert_const (e[2] == 6);
    }
    
    int marr[3];
    int* arr_ret3() 
    {
        int* p = marr;
        p[b.read()] = 1;
        return p;
    }
    
    void array_return2() 
    {
        int c[3];
        int* d = arr_ret1(c);
        const int* e = arr_ret2();
        int* f = arr_ret3();
        
        int i = d[0] + e[1] + f[2];
    }
};

int sc_main(int argc, char **argv) {
    test t_inst{"t_inst"};
    sc_start();
    return 0;
}


