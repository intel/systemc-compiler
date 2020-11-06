/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Cross-hierarchy bind at the same level, there is conflict at @cons.sig
// as it is used as input but assigned in @cons, consider as OK
struct Prod : sc_module {

    sc_in<int>      in;
    sc_out<int>     out;

    SC_CTOR(Prod) {
        SC_METHOD(proc);
        sensitive << in;
    }
    
    void proc() {
        out = in;
    }
};

struct Cons : sc_module {

    sc_in<int>      in;
    sc_signal<int>  sig;

    SC_CTOR(Cons) {
        SC_METHOD(proc);
        sensitive << in;
    }
    
    void proc() {
        sig = in+1;
    }
};

struct Top : sc_module {

    Prod prod{"prod"};
    Cons cons{"cons"};
    
    sc_in<int>   in{"in"};

    SC_CTOR(Top) 
    {
        prod.in(in);
        cons.in(in);
        
        prod.out(cons.sig);
    }
};


int sc_main(int argc, char** argv)
{
    sc_signal<int> s;
    Top top("top");
    top.in(s);
    
    sc_start();
    return 0;
}

