/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by ripopov on 3/8/18.
//

#include <systemc.h>

struct virt_base {
    int v = 1;
};

struct nonvirt_base {
    int nv = 2;
};

struct base0 : virtual virt_base, nonvirt_base {
    int x = 3;
};

struct base1 : virtual virt_base, nonvirt_base {
    int y = 4;
};

class top : sc_module, base0, base1 {
public:

    int z = 5;

    SC_HAS_PROCESS(top);
    top (sc_module_name) {
        SC_METHOD(test_method);
    }

    void test_method() {
    }

};


int sc_main(int argc, char* argv[])
{
    cout << "test_state_bases\n";
    top t{"t"};
    sc_start();
    return 0;
}

