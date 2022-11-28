/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// std::vector with module pointers
struct bottom : sc_module {

    sc_in<int> in{"in"};
    sc_out<int> *out = nullptr;

    SC_CTOR(bottom) {
        SC_METHOD(proc);
        sensitive << in;
    }
    
    void proc() {
        out->write(in.read());
    }
};

struct top : sc_module 
{
    sc_signal<int>  out;
    sc_signal<int>  sig[4];

    std::vector<bottom*> mods;
    
    SC_CTOR(top) {

        mods.push_back(new bottom("mods0"));
        mods.push_back(new bottom("mods1"));

        mods[0]->out = new sc_out<int>("out");
        mods[0]->in(sig[0]);
        mods[0]->out->bind(sig[1]);
        
        mods[1]->out = new sc_out<int>("out");
        mods[1]->in(sig[2]);
        mods[1]->out->bind(sig[3]);
        
        // Add and remove module
        auto mod2 = new bottom("mods2"); 
        mods.push_back(mod2);
        mods.erase((mods.rbegin()+1).base());
        delete mod2;
        
        SC_METHOD(proc);
        sensitive << sig[1] << sig[3];
    }
    
    void proc() {
        sig[0] = 1;
        sig[2] = 2;
        out = sig[1] + sig[3];
    }
    
};

int sc_main(int argc, char** argv)
{
    cout << "test virtual ports\n";
    auto t0 = std::make_unique<top>("top_inst");
    sc_start();
    return 0;
}

