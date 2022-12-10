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

class top : sc_module {
public:

    sc_vector<sc_signal<int>> sig_vec{"sig_vec",3};
    sc_vector<sc_in<int>>     in_vec{"in_vec",3};
    sc_vector<sc_out<int>>    out_vec{"out_vec",3};


    SC_HAS_PROCESS(top);
    top (sc_module_name) {

        out_vec.bind(sig_vec);
        in_vec.bind(sig_vec);

        SC_METHOD(test_method);
    }

    void test_method() {
    }

};


int sc_main(int argc, char* argv[])
{
    cout << "test_state_sc_vector\n";
    top t{"t"};
    sc_start();
    return 0;
}

