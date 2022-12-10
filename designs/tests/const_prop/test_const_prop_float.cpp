/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by ripopov on 7/30/18.
//

#include <systemc.h>
#include <sct_assert.h>


SC_MODULE(top) {

    SC_CTOR(top) {
        SC_METHOD(test_float);
    }

    const float cf = 12.1;

    void test_float () {
        int x = 3.14;
        int k = 3.14f;

        double y = 0;
        float z = 0;

        cout << "test_float done\n";
    }

};

int sc_main (int argc, char ** argv ) {

    top t_inst{"t_inst"};
    sc_start();

    return 0;
}
