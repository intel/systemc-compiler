/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"

using namespace sc_core;

// Record pointer array in METHOD
template <unsigned N>
class A : public sc_module {
public:
    sc_signal<bool> dummy{"dummy"};

    struct Simple {
        bool a;
        bool getData() {return a;}
        void setData(bool par) {a = par;}
    };
    
    Simple* r1[N];
    Simple* r2[N];
    Simple* r3[N];
    
    SC_CTOR(A) 
    {
        for (int i = 0; i < N; i++) {
            r1[i] = sc_new<Simple>();
            r2[i] = sc_new<Simple>();
            r3[i] = sc_new<Simple>();
        }
        
        SC_METHOD(rec_ptr_simple);  
        sensitive << dummy;
        
        SC_METHOD(rec_ptr_loop);  
        sensitive << dummy;

        SC_METHOD(rec_ptr_unknw);  
        sensitive << dummy;
    }
    
    void rec_ptr_simple() 
    {
        bool b = r1[0]->a;
        b = r1[1]->getData();
    }
    
    void rec_ptr_loop() 
    {
        bool b = false;
        
        for (int i = 0; i < N; i++) {
            b = b || r2[i]->getData();
        }
    }
    
    void rec_ptr_unknw() 
    {
        int i = dummy;
        bool b = r3[i]->a;
        
        r3[i]->setData(!b);
    }  
};

class B_top : public sc_module {
public:
    A<2> a_mod{"a_mod"};

    SC_CTOR(B_top) {
    }
};

int sc_main(int argc, char *argv[]) {
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}

