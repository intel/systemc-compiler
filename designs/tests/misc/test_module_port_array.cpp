/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Port/signal arrays in child module interface
// Currently arrays not flatten
SC_MODULE(A) {
    sc_in<bool> r;
    sc_in<bool> t[1];
    sc_in<bool> w[1];
    sc_in<bool> p[2];   // not used
    sc_in<bool> q[2];
    
    SC_CTOR(A) {
        SC_METHOD(meth);
        sensitive << p[0] << t[0] << r << q[1] << w[0];
    }
    
    void meth()
    {
        bool b = t[0] ^ r ^ q[1] | w[0];
    }
};

SC_MODULE(C) {
    sc_signal<bool> rs;
    sc_signal<bool> ws[1];
    sc_signal<bool> qs[2];
    
    SC_CTOR(C) {
        SC_METHOD(meth);
        sensitive << s;
    }
    
    sc_signal<bool> s;
    
    void meth()
    {
        rs = 1;
        ws[0] = 0;
        for (int i = 0; i < 2; i++) {
            qs[i] = s.read();
        }
    }
};

SC_MODULE(B) {

    sc_signal<bool> s1[2];
    sc_signal<bool> s2[3][2];
    
    A a;
    C c{"c"};
    
    SC_CTOR(B) : a("a") 
    {
        a.r(c.rs);
        a.t[0](s1[1]);
        a.w[0](c.ws[0]);
    
        for (int j = 0; j < 1; j++) {
            for (int i = 0; i < 2; i++) {
                a.p[i](s2[j][i]);
            }
        }
        for (int i = 0; i < 2; i++) {
            a.q[i](c.qs[i]);
        }
    }
};

int sc_main(int argc, char **argv) {
    B b_mod{"b_mod"};
    sc_start();
    return 0;
}
