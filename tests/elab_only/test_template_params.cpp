/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by ripopov on 1/30/18.
//

#include <systemc.h>

template<template <typename, sc_writer_policy> class SigT, int IVAL, unsigned UVAL, unsigned long long ULLVAL>
SC_MODULE(mod_template) {

    SigT<int, SC_ONE_WRITER> sig_int{"sig_int"};

    int a1[IVAL];
    int a2[UVAL];
    int a3[ULLVAL];

    sc_int<IVAL> i1;
    sc_int<UVAL> i2;
    sc_int<ULLVAL> i3;

    SC_CTOR(mod_template) {}

};

template <typename TraitsT>
struct mod_with_traits : sc_module {
    SC_CTOR(mod_with_traits) {}


    typename TraitsT::bool_sig bsig{"bsig"};
    int a3[TraitsT::const_ival];
};

template <typename T>
struct traits {
    typedef sc_signal<bool> bool_sig;
    static const unsigned const_ival = 3;
};

struct top : sc_module {

    mod_template<sc_signal, 1, 2, 3> mod0{"mod0"};

    mod_with_traits<traits<decltype(mod0)>> mod1{"mod1"};

    SC_CTOR(top) {
        cout << "test_template_params\n";
    }
};

int sc_main(int argc, char** argv)
{
    auto t0 = std::make_unique<top>("top_inst");
    sc_start();
    return 0;
}

