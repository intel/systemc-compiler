/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Bind signal to port in various cases
SC_MODULE(A) {
    sc_in<bool>         i;
    sc_out<sc_uint<4>>  o;

    sc_in<bool>         ip;
    sc_out<sc_uint<4>>  op;
    
    sc_out<sc_uint<4>>  oap[2];
    sc_vector<sc_out<sc_uint<4>>>  ovp{"ovp", 2};
    
    sc_signal<int>      s;
    
    SC_CTOR(A) {
        SC_METHOD(meth);
        sensitive << ip << i << s;
    }
    
    void meth()
    {
        bool b = ip;
        op = (sc_uint<4>)(b ? s.read() : 0);
        o = i.read() ? 1 : 0;
    }
};

SC_MODULE(C) {
    sc_in<sc_uint<4>>    i;
    sc_out<bool>         o;

    sc_signal<bool>         os;
    sc_signal<sc_uint<4>>   is;
    
    sc_signal<sc_uint<4>>   ias[2];
    sc_vector<sc_signal<sc_uint<4>>>  ivs{"ivs", 2};
    
    SC_CTOR(C) {
        SC_METHOD(meth);
        sensitive << is << i;
    }
    
    void meth()
    {
        os = is.read() == 1;
        o = i.read() == 1;
    }

};

SC_MODULE(Top) 
{
    A a{"a"};
    C c{"c"};
    
    sc_signal<bool>         t1;
    sc_signal<sc_uint<4>>   t2;

    SC_CTOR(Top)
    {
        a.i(t1);
        a.o(t2);
        c.i(t2);
        c.o(t1);

        a.ip(c.os);
        a.op(c.is);
        for (int i = 0; i < 2; i++) {
            a.oap[i](c.ias[i]);
        }
        a.ovp(c.ivs);
    }
};

int sc_main(int argc, char **argv) {
    Top mod{"mod"};
    sc_start();
    return 0;
}
