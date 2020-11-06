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

SC_MODULE(top) {

    sc_clock    clk_gen{"clk_gen", 10 , SC_NS};

    SC_CTOR(top) {
        SC_METHOD(test_method);
    }

    sc_signal<bool> din{"din", true};

    void test_method() {

        int x = 0;

        for (size_t i = 0; i < 1000000; ++i) {
            x++;
        }

    }


};

int sc_main (int argc, char ** argv ) {

    top t_inst{"t_inst"};
    sc_start(100, SC_NS);

    return 0;
}
