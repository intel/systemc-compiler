/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by ripopov on 2/15/18.
//

#include <systemc.h>
namespace test {
    struct complex {
        int x;
        int y;
    };

    template <int N>
    struct shared_base {
        int s = 42;
    };

    template <typename T>
    struct virtual_shared_base {
        int v = 13;
    };

    struct base0 : shared_base<11>, virtual virtual_shared_base<int> {
        static constexpr int c0 = 10;
        static const int c1 = 11;
        sc_in<int> in{"in"};
    };

    struct base1 : virtual virtual_shared_base<int> {
        sc_signal <int> sig{"sig"};

    };

    struct base2 : base1 , shared_base<11> {
        int x0, x1;

        void method_call() {}

        complex comp;
    };

    struct top : sc_module, base0, base2 {

        SC_CTOR(top) {
            in.bind(sig);
            base0::s = 0;
            base2::s = 2;
            v = 111;

            SC_THREAD(test_thread);
        }

        int t;

        void test_thread() {
            base0::s;
            base2::s;
            v;
            x0;
            x1;
            t;
            comp.x ++;

            method_call();

            cout << "base0::s " <<  base0::s << endl;
            cout << "base2::s " <<  base2::s << endl;
        }

    };

}

int sc_main(int argc, char** argv)
{
    cout << "test base class 3\n";
    auto t0 = std::make_unique<test::top>("top_inst");
    sc_start();
    return 0;
}

