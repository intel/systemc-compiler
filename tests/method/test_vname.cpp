/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"

using namespace sc_core;

// Local and global variable names unique analysis

class C : public sc_module {
public:
    sc_uint<1> m;
    sc_uint<2> m2;
    
    SC_CTOR(C) {
    }
    
};

class A : public C {
public:
    sc_uint<1> i;
    int j;
    int k;
    sc_uint<1> l;
    sc_uint<3> m;
    sc_uint<4> m2;
    
    //C c_mod{"c_mod"};

    SC_HAS_PROCESS(A);

    sc_signal<bool> dummy{"dummy"};

    A(const sc_module_name& name) : C(name) {
        SC_METHOD(var1); sensitive << dummy;
        SC_METHOD(var2); sensitive << dummy;
        SC_METHOD(var3); sensitive << dummy;
    }

    // Local and global variable declaration
    void var1() {
	int i;
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 5; j++) {
                k = (j != i);
                bool k = (j == i);
            }
        }
        k = i;

        C::m = 0;
        m = 1;
        sc_uint<3> m = 2;
    }

    // Base module variables declaration
    void var2() {
        C::m2 = 0;
        m2 = 1;
        {
            sc_uint<3> m2 = 2;
            int k = C::m2 + m2;
        }
        j = C::m2 + m2;
    }

    // Original and constructed name equivalence
    void var3() {
        sc_uint<2> i_1 = 1;
        sc_uint<3> i_2 = 2;
        sc_uint<4> i = 3;
        
        sc_uint<2> l;
        sc_uint<3> l_1;
        
        sc_uint<1> n_1;
        sc_uint<2> n_2;
        sc_uint<3> n_3;
        sc_uint<4> n;
        {
            sc_uint<5> n;
        }
        
        sc_uint<1> x_1;
        sc_uint<2> x_2;
    }
};

class B_top : public sc_module {
public:
    A a_mod{"a_mod"};

    SC_CTOR(B_top) {
    }
};

int sc_main(int argc, char *argv[]) {
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}

