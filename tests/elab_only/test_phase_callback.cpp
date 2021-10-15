/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by ripopov on 10/10/18.
//

#include <systemc.h>

struct top : sc_module {

    sc_in <bool>     in{"in"};
    sc_signal <bool> sig{"sig"};

    SC_CTOR(top) {
        SC_THREAD(test_thread);
        async_reset_signal_is(sig, 0);
    }

protected:
    const unsigned* A = nullptr;

    void before_end_of_elaboration() override
    {
        A = sc_new<unsigned>(42);
        in(sig);
        cout << "before end of elaboration callback\n";
    }

    void test_thread() {
        auto j = A ? *A : 0;
        wait();
        
        while (1) {
            if (A) {
                j += *A;
            }
            wait();
        }
    }

};

int sc_main(int argc, char **argv) {

    top tinst{"tinst"};
    sc_start();

    return 0;
}
