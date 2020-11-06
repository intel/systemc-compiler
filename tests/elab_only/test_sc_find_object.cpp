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

class submodule : sc_module {

    sc_signal<bool> private_sig{"private_sig"};

    sc_in<int>  private_in{"private_in"};
    sc_signal<int> private_int_sig{"private_int_sig"};

public:
    SC_CTOR(submodule) {
        private_in(private_int_sig);
    }
};

class top : sc_module {

    submodule sub_inst{"sub_inst"};

    sc_in<int>  backdoor_to_in{"backdoor_to_in"};
    sc_in<bool> backdoor_to_sig{"backdoor_to_sig"};

public:

    template <typename T>
    T * get_child_as(const std::string & child_name) {
        sc_object * child_obj = sc_find_object((std::string(name())
            + "." + child_name).c_str());

        T * childT = dynamic_cast<T*> (child_obj);
        assert(childT);
        return childT;
    }

    SC_CTOR(top) {
        auto *priv_sig = get_child_as<sc_signal<bool>> ("sub_inst.private_sig");
        auto *priv_in = get_child_as<sc_in<int>> ("sub_inst.private_in");

        backdoor_to_in(*priv_in);
        backdoor_to_sig(*priv_sig);
    }

};

int sc_main(int argc, char **argv) {

    top top_inst{"top_inst"};
    sc_start();

    return 0;
}
