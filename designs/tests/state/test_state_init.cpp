/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by ripopov on 3/7/18.
//

#include <systemc.h>

class top : sc_module {
public:

    sc_in<sc_int<10>>     in10{"in10"};
    sc_out<sc_int<10>>    out10{"out10"};
    sc_signal<sc_int<10>> sig10{"sig10"};

    sc_signal<sc_int<10>> *sig10_p = &sig10;

    sc_signal<bool> *bsig_p = new sc_signal<bool>("bsig");

    const unsigned cu = 10;
    int xi;
    const sc_int<42> csi42 = 42;
    sc_uint<31> su31;

    sc_signal<sc_int<3>> sig_array[3];

    sc_signal<sc_int<4>> *sig_p_array[2];

    sc_signal<sc_int<5>> **sig_p_array2d[2];

    sc_in<sc_int<5>> in5{"in5"};

    int array2d [10][10];

    sc_signal<int> int_sig{"int_sig"};

    SC_HAS_PROCESS(top);
    top (sc_module_name) {

        sig_p_array[0] = new sc_signal<sc_int<4>>("sig_p_array0");
        sig_p_array[1] = new sc_signal<sc_int<4>>("sig_p_array1");

        sig_p_array2d[0] = sc_new_array<sc_signal<sc_int<5>>*>(2);
        sig_p_array2d[1] = sc_new_array<sc_signal<sc_int<5>>*>(2);

        for (size_t i = 0; i < 2; i++) {
            sig_p_array2d[0][i]
                = new sc_signal<sc_int<5>>(sc_gen_unique_name("sig_p_array2d0"));
            sig_p_array2d[1][i]
                = new sc_signal<sc_int<5>>(sc_gen_unique_name("sig_p_array2d1"));
        }

        in10(sig10);
        out10(sig10);

        in5(*sig_p_array2d[1][1]);

        SC_METHOD(test_method);
        sensitive << in10 << out10 <<sig10 << *bsig_p
                  << sig_array[1] << sig_array[2] << *sig_p_array[1]
                  << *sig_p_array2d[0][1] << *sig_p_array2d[1][0]
                  << *sig_p_array2d[1][1] << int_sig;
    }


    void test_method() {
        xi = cu + 1;

        for (int i = 0; i < 10; i++)
            int_sig.write( int_sig.read() + int_sig.read() + 1);
        //su31 = csi42;
    }

};


int sc_main(int argc, char* argv[])
{
    cout << "test_state_init\n";
    top t{"t"};
    sc_start();
    return 0;
}

