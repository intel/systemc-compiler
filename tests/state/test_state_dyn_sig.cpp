/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by ripopov on 6/13/18.
//

#include <systemc.h>

class top : sc_module {
public:

    sc_signal<int> *sigp = new sc_signal<int>("sigp");
    sc_signal<int> *sigp2 = new sc_signal<int>("sigp2");

    SC_HAS_PROCESS(top);
    top (sc_module_name) {

        SC_METHOD(test_method);
        sensitive << *sigp;
    }

    void test_method() {
        *sigp2 = sigp->read() + 1;
        sigp2->write(*sigp);
    }

};

int sc_main(int argc, char* argv[])
{
    cout << "test_state_dyn_sig\n";
    top t{"t"};
    sc_start();
    return 0;
}

