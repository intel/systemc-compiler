/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"

using namespace sc_core;

// Error method non sensitive to array
class A : public sc_module {
public:
    SC_HAS_PROCESS(A);

    sc_in_clk clk;
    sc_signal<int> s;
    sc_signal<bool> arr1[3];
    sc_signal<int>* parr1[3];
    sc_signal<sc_uint<3>> arr2[2][3];
    sc_vector<sc_signal<bool>> vec1{"vec1", 3};
    
    A(const sc_module_name& name) : sc_module(name) 
    {
        for (int i = 0; i < 3; i++) {
            parr1[i] = new sc_signal<int>("1");
        }
        SC_METHOD(no_sens_arr1);
        sensitive << s;

        SC_METHOD(no_sens_arr2);
        sensitive << s;

        SC_METHOD(no_sens_vec1);
        sensitive << s;
    }
    
    void no_sens_arr1() 
    {
        bool i = arr1[s.read()];
    }

    void f(bool b) {auto l = b;}
    
    void no_sens_arr2() 
    {
        sc_uint<3> i = arr2[s.read()][0];
        f(*parr1[0]);
    }

    void no_sens_vec1() 
    {
        auto i = 1 + vec1[1];
    }
};


int sc_main(int argc, char *argv[]) {
    sc_clock clk{"clk", 1, SC_NS};
    A a_mod{"a_mod"};
    a_mod.clk(clk);
    
    sc_start();
    return 0;
}

