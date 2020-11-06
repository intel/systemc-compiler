/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Cross-hierarchy bind at the same level, all is correct here
struct Prod : sc_module {

    sc_in<int>      in;
    sc_out<int>     out;
    sc_signal<int>  sig;

    SC_CTOR(Prod) {
        SC_METHOD(proc);
        sensitive << in;
    }
    
    void proc() {
        sig = 1;
        out = in;
    }
};

struct Cons : sc_module {

    sc_in<int>      in;
    sc_out<int>     out;
    sc_signal<int>  sig;

    SC_CTOR(Cons) {
        SC_METHOD(proc);
        sensitive << in << sig;
    }
    
    void proc() {
        out = in + sig;
    }
};

struct Top : sc_module {

    Prod prod{"prod"};
    Cons cons{"cons"};
    
    sc_in<int>   in{"in"};
    sc_out<int>  out{"out"};
    
    SC_CTOR(Top) {

        prod.in(in);
        prod.out(cons.sig);

        cons.in(prod.sig);
        cons.out(out);
    }
};


int sc_main(int argc, char** argv)
{
    sc_signal<int> s;
    Top top("top");
    top.in(s);
    top.out(s);
    
    sc_start();
    return 0;
}

