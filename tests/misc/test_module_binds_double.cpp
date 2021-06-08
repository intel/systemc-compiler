/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Bind signal to two ports in different modules
SC_MODULE(A) {
    sc_in<bool>         i;
    sc_signal<int>      s;
    
    SC_CTOR(A) {
        SC_METHOD(meth);
        sensitive << i;
    }
    
    void meth()
    {
        s = i.read() ? 1 : 0;
    }
};

SC_MODULE(C) {
    sc_signal<bool>     os;
    sc_signal<int>      s;
    
    SC_CTOR(C) {
        SC_METHOD(meth);
        sensitive << s;
    }
    
    void meth()
    {
        os = s.read() == 1;
    }

};

SC_MODULE(Top) 
{
    A a1{"a1"};
    A a2{"a2"};
    C c{"c"};
    
    SC_CTOR(Top)
    {
        a1.i(c.os);
        a2.i(c.os);
    }
};

int sc_main(int argc, char **argv) {
    Top mod{"mod"};
    sc_start();
    return 0;
}
