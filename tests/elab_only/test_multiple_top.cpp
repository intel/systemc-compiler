/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by ripopov on 9/17/18.
//

#include <systemc.h>

SC_MODULE(top) {
    SC_CTOR(top) {}
};

int sc_main (int argc, char ** argv) {
    top top0{"top0"};
    top top1{"top1"};

    sc_start();

    return 0;
}
