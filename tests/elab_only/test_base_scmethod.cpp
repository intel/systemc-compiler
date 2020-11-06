/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by ripopov on 2/26/18.
//

#include <systemc.h>

namespace bases {

template <unsigned U>
struct base0 : sc_module {
    sc_signal<bool> base0_sig{"base0_sig"};

    SC_HAS_PROCESS(base0);
    base0(const sc_module_name &name)
    {
        SC_METHOD(test_method);
        sensitive << base0_sig;
    }

    void test_method() {}
};

template <unsigned U, int I>
struct base1 : base0<U> {
    sc_signal<bool> base1_sig{"base1_sig"};

    SC_HAS_PROCESS(base1);
    base1(const sc_module_name &name)
        : base0<U>(name) {
        SC_METHOD(test_method);
        this->sensitive << base1_sig;
    }

    void test_method() {}
};

}

struct top : bases::base1<10,11> {
    sc_signal<bool> top_sig{"top_sig"};

    SC_HAS_PROCESS(top);
    top(const sc_module_name &name)
        : base1(name) {
        SC_METHOD(test_method);
        sensitive << top_sig;
    }

    void test_method() {}
};

int sc_main (int argc, char **argv) {
    cout << "test_base_scmethod\n";
    top t0{"t0"};
    sc_start();
    return 0;
}

