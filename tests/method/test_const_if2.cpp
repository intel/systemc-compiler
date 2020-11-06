/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>
#include <sct_assert.h>

SC_MODULE(test_if) {

    static constexpr int TRUE = 1;
    static constexpr int FALSE = 0;

    sc_signal<bool> dummy{"dummy"};

    SC_CTOR(test_if) {
        SC_METHOD(method_true); sensitive << dummy;
        SC_METHOD(method_false); sensitive << dummy;
        SC_METHOD(method_const_prop); sensitive << dummy;
    }

    sc_signal<int> * null_signal = nullptr;

    void method_true () {

        int x = 0;

        if (TRUE) {
            x = 1;
        } else {
            sct_assert_const(0);
            null_signal->write(1);
        }

        sct_assert_const(x == 1);

        cout << "method_true: OK\n";
    }

    void method_false() {

        int x = 1;

        if (FALSE) {
            x = 2;
            sct_assert_const(0);
            null_signal->write(1);
        } else {
            x ++;
        }

        sct_assert_const(x == 2);
        cout << "method_false: OK\n";
    }

    void method_const_prop() {

        const int x = 1;
        const int y = 2;
        const int z = x + y;

        if (x + x > z) {
            sct_assert_const(0);
            null_signal->write(1);
        }

        sct_assert_const(x + 2 == z);

        cout << "method_const_prop: OK\n";

    }

};


int sc_main(int argc, char **argv) {

    test_if tinst{"tinst"};

    sc_start();

    return 0;
}
