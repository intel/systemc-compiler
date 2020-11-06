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

struct top: sc_module
{

    SC_CTOR(top)
        {
            for (size_t i = 0; i < SIZE; ++i) {
                intport_array[i].bind(intsig_array[i]);
            }
            intport->bind(*intsig);
            SC_THREAD(test_thread);
        }

    static const std::size_t SIZE = 2;

    sc_signal<int> *intsig = sc_new<sc_signal<int>>("sig");
    sc_signal<int> *intsig_array = sc_new_array<sc_signal<int>>(SIZE);

    sc_out<int> *intport = sc_new<sc_out<int>>("port");
    sc_out<int> *intport_array = sc_new_array<sc_out<int>>(SIZE);

    int *int_array_shifted = sc_new_array<int>(10) + 5;

    void test_thread()
    {
        for (size_t i = 0; i < SIZE; ++i)
            intport_array[i] = 10 + i;
        *intport = 1;
        wait(1, SC_NS);
        for (size_t i = 0; i < SIZE; ++i)
            cout << intsig_array[i] << endl;
        cout << *intsig << endl;
    }

};

int sc_main(int argc, char **argv)
{
    top top_inst{"top_inst"};


    sc_start();
    return 0;
}

