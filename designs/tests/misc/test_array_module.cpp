/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// C++ array of modules/signal w/o specific names and sc_vector bind to sc_vector
struct Bottom : public sc_module {

    sc_in<int>      in{"in"};
    sc_vector<sc_out<int>>  out{"out",2};

    SC_CTOR(Bottom) {
        SC_METHOD(proc);
        sensitive << in;
    }
    
    void proc() {
        out[0].write(in.read());
        out[1].write(in.read());
    }
};

struct Top : public sc_module 
{
    sc_in<int>      prt[2];
    sc_signal<int>  sig[2];
    sc_vector<sc_signal<int>>  vsig{"vsig", 2};

    Bottom SC_NAMED(mod);
    //Bottom mods[2];       // Is not compiled, no name for module
    
    SC_CTOR(Top) {
        prt[0](sig[0]); prt[1](sig[1]);
        mod.in(sig[0]);
        
        mod.out.bind(vsig);  // Bind vector to vector

        SC_METHOD(proc);
        sensitive << sig[0] << sig[1];
    }
    
    void proc() {
        sig[0] = 1;
        cout << "sig" << sig[0].name() << " " << sig[1].name() << endl;
        cout << "prt" << prt[0].name() << " " << prt[1].name() << endl;
    }
    
};

int sc_main(int argc, char** argv)
{
    Top top("top");
    sc_start();
    return 0;
}

