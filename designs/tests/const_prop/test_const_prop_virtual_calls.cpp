/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by ripopov on 7/9/18.
//

#include <systemc.h>
#include <sct_assert.h>

struct base_mod : sc_module {

    SC_CTOR(base_mod) {}

    int x;

    virtual void vfun0() {
        x = 1;
    };

    virtual void vfun1() = 0;

    void fun() {
        x = 0;
        vfun1();
    }

};

struct derived_mod : base_mod {

    int y;

    derived_mod(sc_module_name nm) : base_mod(nm) {
        SC_HAS_PROCESS(derived_mod);
        SC_METHOD(test_method);
        sensitive << din;
    }
    
    sc_signal<bool> din;

    void vfun0() override {
        x = 2;
    }

    void vfun1() override {
        x = 3;
    }

    void test_method() {
        fun();

        cout << x << endl;
        sct_assert_const(x == 3);

        //base_mod::vfun0();
        vfun0();
        cout << x << endl;

        sct_assert_const(x == 2);
    }

};


int sc_main (int argc, char ** argv ) {

    derived_mod t{"t"};
    sc_start();

    return 0;
}
