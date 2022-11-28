/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by ripopov on 6/22/18.
//

#include <systemc.h>
#include <sct_assert.h>

struct pack
{
    int x = 0;
    const int cx = 1;
};

SC_MODULE(top)
{

    SC_CTOR(top)
    {
        //SC_METHOD(test_method);  #251
        //sensitive << din;
    }

    sc_signal<bool> din;

    int sum(int &xr, int yr)
    {
        int sum = xr + yr;
        yr = sum;
        xr = sum;
        return sum;
    }

    int sum_wrapper(int &xr2, int yr2)
    {
        yr2--;
        xr2--;
        return sum(xr2, yr2); // 7 2
    }

    void increment(int &xr3)
    {
        ++xr3;
    }

    void increment_wrapper(int &xr4)
    {
        increment(xr4);
    }

    const sc_int<10> intarray[4] = {1,2,3,4};

    sc_int<10> sc_int_math(sc_int<10> in,
                           const sc_int<10> in2,
                           const sc_int<10> &in3)
    {
        in = intarray [in2];
        return in + in2 + in3;
    }

    void test_method()
    {
        int x = 1;

        increment_wrapper(x);
        increment_wrapper(x);

        sct_assert_const(x == 3);

        for (size_t ii = 0; ii < 2; ++ii) {
            ++x;
        }

        sct_assert_const(x == 5);

        int y = 4;
        int z = sum(++x, y--);

        sct_assert_const(x == 10);
        sct_assert_const(y == 3);
        sct_assert_const(z == 10);

        z = sum_wrapper(x, y);

        sct_assert_const(x == 11);
        sct_assert_const(y == 3);
        sct_assert_const(z == 11);

        if (din) {
            y = 1;
        }

        sct_assert_unknown(y);

        for (size_t i = 0; i < 3; ++i) {
            y++;
            x++;
        }

        sct_assert_const(x == 14);
        sct_assert_unknown(y);
        sct_assert_const(z == 11);


        sc_int<10> sci1 = 1;
        sc_int<10> sci2 = 2;
        sc_int<10> sci3 = 3;
        sci1 = sc_int_math(sci1, sci2,sci3);
    }

};

int sc_main(int argc, char **argv)
{

    top t_inst{"t_inst"};
    sc_start();

    return 0;
}
