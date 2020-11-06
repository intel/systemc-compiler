/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by ripopov on 2/2/18.
//

#include <systemc.h>

using namespace sc_core;

SC_MODULE(top) {

    sc_in <bool> clk{"clk"};

    sc_in <bool>  in_bool {"in_bool"};
    sc_out <bool> out_bool {"out_bool"};
    sc_out <int>  out_int {"out_int"};

    sc_out <int>  out_int2 {"out_int2"};

    sc_vector<sc_out<bool>> out_vec {"out_vec", 3};

    int int_val = 0;
    const int cint_val = 2;

    sc_out <int> out_int3{"out_int3"};
    const int array[3] = {1,2,3};


    SC_CTOR(top) {
        SC_METHOD(test_method1);
        sensitive << clk.pos();

        SC_METHOD(test_method2);
        sensitive << clk.pos();

        SC_METHOD(test_method3);
        sensitive << clk.pos();

        SC_METHOD(test_method4);
        sensitive << clk.pos();

    }

    void test_method1 () {
        if (in_bool.read())
            out_int.write(out_int.read() + 1);
        else
            out_int.write(out_int.read() - 1);
    }

    void test_method2 () {
        out_bool.write( ~in_bool.read() );
    }

    void test_method3 () {
        if (int_val > 0) {
            out_int2.write(cint_val + int_val + in_bool.read());
        } else {
            out_int2.write(cint_val + int_val + out_bool.read());
        }
    }

    void test_method4 () {
        out_int3.write(7);
        if (array[0] > 2) {
            out_int3.write(4);
        } else if (array[1] > 2) {
            out_int3.write(5);
        } else if (array[2] > 3) {
            out_int3.write(6);
        }
    }

};

SC_MODULE(top_wrap) {

    sc_signal <bool> clk{"clk"};
    sc_signal <bool>  in_bool {"in_bool"};
    sc_signal <bool> out_bool {"out_bool"};
    sc_signal <int>  out_int {"out_int"};
    sc_signal <int>  out_int2 {"out_int2"};

    sc_signal <int> out_int3{"out_int3"};

    sc_vector<sc_signal<bool>> out_vec {"out_vec", 3};

    top tinst{"tinst"};

    SC_CTOR(top_wrap) {

        tinst.clk.bind(clk);
        tinst.in_bool(in_bool);
        tinst.out_bool(out_bool);
        tinst.out_int(out_int);
        tinst.out_vec(out_vec);
        tinst.out_int2(out_int2);
        tinst.out_int3(out_int3);

    }

};




int sc_main(int argc, char *argv[]) {
    top_wrap twrap{"twrap"};
    sc_start();
    return 0;
}

