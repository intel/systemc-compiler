/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by ripopov on 2/14/18.
//

#include <systemc.h>

SC_MODULE(bot0_deep) {
    sc_in<int>  in{"in"};
    sc_out<int>  out{"out"};

    SC_CTOR(bot0_deep) {}
};

SC_MODULE(bot0) {
    bot0_deep b0deep{"b0deep"};
    SC_CTOR(bot0) {}
};

SC_MODULE(bot1_deep) {
    sc_signal<int> sig{"sig"};

    SC_CTOR(bot1_deep) {}
};

SC_MODULE(bot1) {
    bot1_deep b1deep{"b1deep"};
    SC_CTOR(bot1) {}
};


struct top : sc_module {

    bot0 b0{"b0"};
    bot1 b1{"b1"};


    SC_CTOR(top) {
        b0.b0deep.in(b1.b1deep.sig);
        b0.b0deep.out(b1.b1deep.sig);
    }


};

int sc_main(int argc, char** argv)
{
    cout << "test virtual ports cross\n";
    auto t0 = std::make_unique<top>("top_inst");
    sc_start();
    return 0;
}

