/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by ripopov on 02/08/18.
//

#include <systemc.h>

struct bottom : sc_module {

    sc_in<int> *in = nullptr;
    sc_out<int> *out = nullptr;
    sc_signal<int> *sig = nullptr;
    bottom * b = nullptr;

    SC_CTOR(bottom) {}
};

struct top : sc_module {

    std::vector<bottom*> mods;
    std::vector<sc_in<int>*> ins;
    std::vector<sc_out<int>*> outs;
    std::vector<sc_signal<int>*> sigs;

    SC_CTOR(top) {

        // case 0 --------------------------------------------------------
        // cross-hierarchy bind
        mods.push_back(new bottom("b0"));
        mods.push_back(new bottom("b1"));

        //sc_get_curr_simcontext()->hierarchy_push(mods[0]);
        mods[0]->in = new sc_in<int>("in");
        mods[0]->out = new sc_out<int>("out");
        //sc_get_curr_simcontext()->hierarchy_pop();

        //sc_get_curr_simcontext()->hierarchy_push(mods[1]);
        mods[1]->sig = new sc_signal<int>("sig");
        //sc_get_curr_simcontext()->hierarchy_pop();

        mods[0]->in->bind(*mods[1]->sig);
        mods[0]->out->bind(*mods[1]->sig);

        // case 1 --------------------------------------------------------
        // same module bind
        mods.push_back(new bottom("b2"));

        //sc_get_curr_simcontext()->hierarchy_push(mods[2]);
        mods[2]->in = new sc_in<int>("in");
        mods[2]->out = new sc_out<int>("out");
        mods[2]->sig = new sc_signal<int>("sig");
        //sc_get_curr_simcontext()->hierarchy_pop();

        mods[2]->in->bind(*mods[2]->out);
        mods[2]->out->bind(*mods[2]->sig);

        // case 2 --------------------------------------------------------
        // bind port down hierarchy
        mods.push_back(new bottom("b3"));
        //sc_get_curr_simcontext()->hierarchy_push(mods[3]);
        mods[3]->b = new bottom("b0");
        mods[3]->in = new sc_in<int>("in");
        mods[3]->out = new sc_out<int>("out");
        //sc_get_curr_simcontext()->hierarchy_pop();

        //sc_get_curr_simcontext()->hierarchy_push(mods[3]->b);
        mods[3]->b->sig = new sc_signal<int>("sig");
        //sc_get_curr_simcontext()->hierarchy_pop();

        mods[3]->in->bind(*mods[3]->b->sig);
        mods[3]->out->bind(*mods[3]->b->sig);

        // case 3 --------------------------------------------------------
        // deep crosss-hierarchy
        mods.push_back(new bottom("b4"));
        mods.push_back(new bottom("b5"));

        //sc_get_curr_simcontext()->hierarchy_push(mods[4]);
        mods[4]->b = new bottom("b0");
        //sc_get_curr_simcontext()->hierarchy_pop();

        //sc_get_curr_simcontext()->hierarchy_push(mods[5]);
        mods[5]->b = new bottom("b0");
        //sc_get_curr_simcontext()->hierarchy_pop();

        //sc_get_curr_simcontext()->hierarchy_push(mods[4]->b);
        mods[4]->b->in = new sc_in<int>("in");
        mods[4]->b->out = new sc_out<int>("out");
        //sc_get_curr_simcontext()->hierarchy_pop();


        //sc_get_curr_simcontext()->hierarchy_push(mods[5]->b);
        mods[5]->b->sig = new sc_signal<int>("sig");
        //sc_get_curr_simcontext()->hierarchy_pop();

        mods[4]->b->in->bind(*mods[5]->b->sig);
        mods[4]->b->out->bind(*mods[5]->b->sig);

        // case 4 --------------------------------------------------------
        // deep down hierarchy
        ins.push_back(new sc_in<int>("in0"));
        ins.push_back(new sc_in<int>("in1"));
        ins[0]->bind(*mods[4]->b->in);
        ins[1]->bind(*mods[5]->b->sig);

        // case 5 --------------------------------------------------------
        // bind module up
        mods.push_back(new bottom("b6"));

        //sc_get_curr_simcontext()->hierarchy_push(mods.at(6));
        mods.at(6)->b = new bottom("b0");
        //sc_get_curr_simcontext()->hierarchy_pop();

        //sc_get_curr_simcontext()->hierarchy_push(mods.at(6)->b);
        mods.at(6)->b->in = new sc_in<int>("in");
        //sc_get_curr_simcontext()->hierarchy_pop();


        sigs.push_back(new sc_signal<int>("sig0"));
        mods[6]->b->in->bind(*sigs[0]);

    }
};

int sc_main(int argc, char** argv)
{
    cout << "test virtual ports\n";
    auto t0 = std::make_unique<top>("top_inst");
    sc_start();
    return 0;
}

