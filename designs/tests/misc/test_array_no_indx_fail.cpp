/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Access array without index, error reported 
struct top : sc_module 
{
    sc_signal<bool> s;
    
    bool a[4];
    bool* ptr[4];
    sc_signal<bool> sig[4];
    sc_signal<bool> sig2d[4][4];
    sc_signal<bool>* sig_ptr[4];

    SC_CTOR(top) {
        
        for (int i = 0; i < 4; i++) {
            ptr[i] = sc_new<bool>("");
            sig_ptr[i] = new sc_signal<bool>("");
        }
        
        SC_METHOD(var_meth);
        sensitive << s;
        
        SC_METHOD(sig_meth);
        sensitive << sig[0];

        SC_METHOD(sig_ptr_meth);
        sensitive << *sig_ptr[0];
    }

    // Any of commented line leads to error
    
    void var_meth() {
        bool b;
        b = a[0];               // OK
        b = *ptr[0];            // OK
        b = *a;                 // Error
        //b = **ptr;              // Error
    }
    
    void sig_meth() {
        bool b;
        b = sig[0].read();      // OK
        //b = sig->read();        // Error
        //b = *sig;               // Error
        //b = (*sig).read();      // Error
        //b = **sig2d;            // Error
        //b = (*sig2d)->read();   // Error
    }
    
    void sig_ptr_meth() {
        bool b;
        b = sig_ptr[0]->read(); // OK
        //b = (*sig_ptr)->read(); // Error
        //b = **sig_ptr;          // Error
    }
    
};

int sc_main(int argc, char** argv)
{
    cout << "test virtual ports\n";
    auto t0 = std::make_unique<top>("top_inst");
    sc_start();
    return 0;
}

