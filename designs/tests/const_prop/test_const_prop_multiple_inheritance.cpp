/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by ripopov on 7/19/18.
//

#include <systemc.h>
#include <sct_assert.h>

struct base {
    int x = 0;

    void inc() { x++; }
    void inc2() { x+=2; }

};

struct mbase0 : base {

    void mbase0_method() { x = 1; }
    void forward_inc() { inc(); }

};

struct mbase1 : base {

    void mbase1_method() { x = 2; }
    void forward_inc2() { inc2(); }

};

struct derived_mod : sc_module, mbase0, mbase1 {

    sc_signal<bool> s;
    
    derived_mod(sc_module_name nm)  {
        SC_HAS_PROCESS(derived_mod);
        SC_METHOD(test_method);
        sensitive << s;
    }


    void test_method() {

        mbase0_method();
        mbase1_method();

        const int y = mbase0::x + mbase1::x;

        cout << mbase0::x << endl;
        cout << mbase1::x << endl;
        cout << y << endl;

        sct_assert_const(mbase0::x == 1);
        sct_assert_const(mbase1::x == 2);
        sct_assert_const(y == 3);


        forward_inc();
        forward_inc2();

        cout << mbase0::x << endl;
        cout << mbase1::x << endl;

        sct_assert_const(mbase0::x == 2);
        sct_assert_const(mbase1::x == 4);

    }

};

int sc_main (int argc, char ** argv ) {

    derived_mod dinst{"dinst"};
    sc_start();

    return 0;
}
