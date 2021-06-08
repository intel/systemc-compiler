/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by ripopov on 7/6/18.
//

#include <systemc.h>
#include <sct_assert.h>

SC_MODULE(top)
{

    SC_CTOR(top)
    {
        SC_METHOD(test_method);
        sensitive << din;
    }

    sc_signal<bool> din{"din", 0};

    int yg;

    int
    fork0(int &x1)
    {

        int &ygr = yg;

        ygr++;

        if (din) {
            x1 = 1;
            return x1 + ygr++;
        }
        else {
            x1 = 2;
            return x1 + ygr++;
        }

    }

    int
    fork1(int &x2)
    {

        if (din) {
            fork0(x2);
            yg++;
            if (x2 != 3)
                x2 = 3;
            else
                x2 = 3;
            return yg;
        }
        else {
            fork0(x2);
            x2 = (yg == 4) ? yg - 1 : 3;
            if (x2 != 3)
                x2 = 3;
            yg++;
            return yg;
        }

    }

    void
    test_method()
    {
        yg = 0;
        int x = 0;
        int y = 0;

        y = fork0(x);

        sct_assert_const(yg == 2);

        sct_assert_unknown(x);
        sct_assert_unknown(y);

        y = fork1(x);

        sct_assert_const(yg == 5);
        sct_assert_const(y == 5);
        sct_assert_const(x == 3);

    }

};

int
sc_main(int argc, char **argv)
{

    top t_inst{"t_inst"};
    sc_start();

    return 0;
}
