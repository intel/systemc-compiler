/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

using namespace sc_core;

// Method with empty sensitivity with ++, --, += and other prohibited operators
class A : public sc_module {
public:
    sc_in<bool>     a{"a"};
    sc_out<bool>    b{"b"};

    SC_CTOR(A) {
        SC_METHOD(empty_fail1); 
        SC_METHOD(empty_fail2); 
        SC_METHOD(empty_rec_fail1); 
        SC_METHOD(empty_rec_fail2); 
    }
    
    void empty_fail1() 
    {
        int i = 1;
        int j = i++;
        j *= 2;
    }
    
    void empty_fail2() 
    {
        sc_int<12> x;
        sc_bigint<100> y;
        sc_uint<12> i = 42;
        i--;
        i += 1;
    }
    
    struct R {
        bool a;
        sc_uint<14> b = 42;
    };
    void empty_rec_fail1() 
    {
        R r;
        int z = r.b;
    }
    
    struct RR {
        int b;
        RR() {
            b = 43;
        }
    };
    void empty_rec_fail2() 
    {
        RR r;
        int z = r.b;
    }
};

class B_top : public sc_module {
public:
    sc_signal<bool> a{"a"};
    sc_signal<bool> b{"b"};

    A a_mod{"a_mod"};

    SC_CTOR(B_top) {
        a_mod.a(a);
        a_mod.b(b);
    }
};

int sc_main(int argc, char *argv[]) {
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}

