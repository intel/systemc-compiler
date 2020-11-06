/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by ripopov on 1/30/18.
//

#include <systemc.h>

struct foobar
{
    sc_int<10> foo;
    int bar[4];
    sc_signal<int> sig_array[4];
    void reset()
    {
        foo = 0;
    }
};

SC_MODULE(top)
{
    foobar farray[2];
    SC_CTOR(top)
    {
        SC_METHOD(test_method);
    }

    void test_method()
    {
        int i = 0;
        farray[0].foo = 1;
        auto &f1 = farray[1];
        f1.bar[i] = 2;
        f1.sig_array[i] = 3;
        f1.reset();
    }
};

int sc_main(int argc, char **argv)
{
    auto t0 = std::make_unique<top>("top_inst");
    sc_start();
    return 0;
}

