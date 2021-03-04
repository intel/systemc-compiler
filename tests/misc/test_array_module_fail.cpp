/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Cross-hierarchy member access, child module fields access not supported yet
struct bottom : sc_module {

    sc_in<int> *in = nullptr;
    sc_out<int> *out = nullptr;
    sc_signal<int> *sig = nullptr;

    SC_CTOR(bottom) {}
};

struct top : sc_module {

    std::vector<bottom*> mods;
    sc_signal<bool> dummy{"dummy"};
    
    SC_CTOR(top) {

        // cross-hierarchy bind
        mods.push_back(new bottom("mods0"));
        mods.push_back(new bottom("mods1"));

        mods[0]->in = new sc_in<int>("in");
        mods[0]->out = new sc_out<int>("out");

        mods[1]->sig = new sc_signal<int>("sig");

        mods[0]->in->bind(*mods[1]->sig);
        mods[0]->out->bind(*mods[1]->sig);
        
        SC_METHOD(proc);
        sensitive << dummy;
    }

    // Will fail as access to child module fields is not supported yet
    void proc() 
    {
        if (mods[0]->in->read()) {
            mods[1]->sig->write(1);
        } else {
            mods[0]->out->write(2);
        }
    }
    
};

int sc_main(int argc, char** argv)
{
    cout << "test virtual ports\n";
    auto t0 = std::make_unique<top>("top_inst");
    sc_start();
    return 0;
}

