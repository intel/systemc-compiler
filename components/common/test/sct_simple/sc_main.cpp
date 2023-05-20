/******************************************************************************
 * Copyright (c) 2023, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 *
 *****************************************************************************/

#include "sct_ipc_if.h"

#ifdef RTL_SIM
  #include "sct_common.h"
  #include "simple_test.h"
#else 
    #ifdef METHOD_SIG
      #include "method_sig_test.h"
    #endif
    #ifdef METHOD
      #include "method_test.h"
    #endif
    #ifdef THREAD
      #include "thread_test.h"
    #endif
    #ifdef MULTI_PUT
      #include "thread_multi_put_test.h"
    #endif
    #ifdef MULTI_STATE
      #include "multi_state_thread.h"
    #endif
    #ifdef BPUT
      #include "bput_test.h"
    #endif
    #ifdef CLK_RST
      #include "clkres_traits_test.h"
    #endif
    #ifdef DEF
      #include "def_traits_test.h"
    #endif
    #ifdef TARG_VECT
      #include "targ_vect.h"
    #endif
#endif
#include <systemc.h>

class Test_top : public sc_module
{
public:
    sc_in_clk           clk{"clk"};
    sc_signal<bool>     nrst{"nrst"};

    simple_test dut{"dut"};

    SC_CTOR(Test_top) {
        dut.clk(clk);
        dut.nrst(nrst);

        SCT_CTHREAD(resetProc, clk, SCT_CMN_TRAITS::CLOCK);
    }

    void resetProc() {
    #ifdef CLK_RST
        nrst = 1;
        wait();
        nrst = 0;
    #else
        nrst = SCT_CMN_TRAITS::RESET;
        wait();
        nrst = !SCT_CMN_TRAITS::RESET;
    #endif
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
    sct_clock<> clk{"clk", 1, SC_NS};
    Test_top test_top{"test_top"};
    test_top.clk(clk);
    sc_start();
    return 0;
}
