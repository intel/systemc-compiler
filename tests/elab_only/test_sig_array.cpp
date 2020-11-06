/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by ripopov on 3/27/18.
//

#include <systemc.h>

struct base: sc_module {

    sc_signal<sc_int<10>>  sig_array[2];

};


struct modif : sc_modular_interface {
    SC_CTOR(modif) {}

    sc_signal<sc_int<10>>  array[3];

};

struct top : base {

    modif   sig{"sig"};

    sc_signal<sc_int<10>>  sig_array[2];
    sc_in<sc_int<10>>      in_array [2];
    sc_out<sc_int<10>>     out_array[2];

    sc_signal<sc_int<10>>  sig_array2d[2][2];
    sc_in<sc_int<10>>      in_array2d[2][2];
    sc_out<sc_int<10>>     out_array2d[2][2];

    sc_vector<sc_signal<sc_int<3>>> vec_sig{"vec_sig",3};
    sc_vector<sc_in<sc_int<3>>>     vec_in {"vec_in" ,3};
    sc_vector<sc_out<sc_int<3>>>    vec_out{"vec_out",3};

    sc_signal<sc_int<4>> ** dyn_sig_array;
    sc_in<sc_int<4>> ** dyn_in_array;

    SC_CTOR(top) {
        for (size_t i = 0; i < 2; ++i) {
            in_array[i].bind(sig_array[i]);
            out_array[i].bind(sig_array[i]);
        }

        for (size_t i = 0; i < 2; ++i) {
            for (size_t j = 0; j < 2; ++j) {
                in_array2d[i][j].bind(sig_array2d[i][j]);
                out_array2d[i][j].bind(sig_array2d[i][j]);
            }
        }

        vec_in.bind(vec_sig);
        vec_out.bind(vec_sig);

        dyn_sig_array = sc_new_array<sc_signal<sc_int<4>> *>(2);
        dyn_in_array = sc_new_array<sc_in<sc_int<4>> *>(2);

        for (size_t i = 0; i < 2; ++i) {
            dyn_sig_array[i]
                = new sc_signal<sc_int<4>>(sc_gen_unique_name("dyn_sig_array"));

            dyn_in_array[i]
                = new sc_in<sc_int<4>>(sc_gen_unique_name("dyn_in_array"));

            dyn_in_array[i]->bind(*dyn_sig_array[i]);
        }

        SC_METHOD(test_method);
    }

    void test_method() { }

};

int sc_main(int argc, char** argv)
{
    auto t0 = std::make_unique<top>("top_inst");
    sc_start();
    return 0;
}

