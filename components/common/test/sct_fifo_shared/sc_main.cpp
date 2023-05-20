/******************************************************************************
 * Copyright (c) 2023, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 *
 *****************************************************************************/

#ifdef METHOD_SIG
  #include "method_sig_test.h"
#endif
#ifdef METHOD
  #include "method_test.h"
#endif
#ifdef THREAD
    #include "thread_test.h"
#endif
#ifdef FSIZE
    #include "check_size_test.h"
#endif
#ifdef MULTI_PUT
  #include "thread_multi_put_test.h"
#endif
#include <systemc.h>

class Test_top : public sc_module
{
public:
    sc_clock clk{"clk", 1, SC_NS};
    sc_signal<bool>     nrst{"nrst"};

    simple_test dut{"dut"};

    SC_CTOR(Test_top) {
        dut.clk(clk);
        dut.nrst(nrst);

        SC_CTHREAD(testProc, clk);
    }

    void testProc() {
        nrst = 0;
        wait();
        nrst = 1;
        wait(100);

        cout << endl;
        cout << "--------------------------------" << endl;
        cout << "|       Test passed OK         |" << endl;
        cout << "--------------------------------" << endl;
        sc_stop();
    }
};

int sc_main(int argc, char* argv[])
{
    Test_top test_top{"test_top"};
    sc_start();
    return 0;
}
