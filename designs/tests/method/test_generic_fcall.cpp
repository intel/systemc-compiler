/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by ripopov on 7/24/18.
//

#include <systemc.h>
#include <sct_assert.h>


int getOne() {
    return 1;
}

struct base {
    static int sum_plus_1(int &x, const int &y) {
        return x + y + 1;
    }
};


struct bundle {

    static auto generate() {
        unsigned X = 1;
        unsigned Y = 2;

        return X + Y;
    }

    static auto increment(unsigned &out) {
        return ++ out;
    }

};

sc_int<2> invert(sc_int<2> in) {
    return ~in;
}


struct top : sc_module, base {

    sc_signal<bool> a;
    
    SC_CTOR(top) {
        SC_METHOD(test_method);
        sensitive << a;
    }

    static int getZero() {
        return 0;
    }

    void test_method () {

        int x = getZero();
        int y = getOne();
        int z = sum_plus_1(x,y);

        sct_assert_const(x == 0);
        sct_assert_const(y == 1);
        sct_assert_const(z == 2);

        unsigned k = bundle::generate();

        sct_assert_const(k == 3);

        x = bundle::increment(k);

        sct_assert_const(k == 4);
        sct_assert_const(x == 4);

        sc_int<2> i2 = 0;
        i2 = invert(i2);

        cout << "done\n";
    }

};

int sc_main (int argc, char ** argv ) {

    top t_inst{"t_inst"};
    sc_start();

    return 0;
}
