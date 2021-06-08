/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>
#include <sct_assert.h>

// Function call and other side effects in complex condition with &&/||
SC_MODULE(top) {

    sc_signal<int> s;

    SC_CTOR(top) {
        //SC_METHOD(test_method); sensitive << s;
        SC_METHOD(test_method2); sensitive << s;
    }

    int dec(int &x1) {
        return x1--;
    }

    void test_method () 
    {
        int x = 1;
        int y = dec(x) && dec(x) && dec(x) && dec(x);

        sct_assert_const(x == -1);
        sct_assert_const(y == 0);

        x = (y++ == 0) && y;

        sct_assert_const(x == 1);
        sct_assert_const(y == 1);

        int z = dec(y) || dec(x) || dec(x);

        sct_assert_const(x == 1);
        sct_assert_const(y == 0);
        sct_assert_const(z == 1);

    }
    
    
    static const bool A = true;
    void test_method2 () 
    {
        int x = 2;
        bool b = true || dec(x);
        sct_assert_const(x == 2);
        
        b = A || dec(x);
        sct_assert_const(x == 2);
        
        b = s.read()  || dec(x);
        sct_assert_const(x == 1);
    }

};

int sc_main (int argc, char ** argv ) {

    top t_inst{"t_inst"};
    sc_start();

    return 0;
}
