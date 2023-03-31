/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_sel_type.h"
#include "systemc.h"

using namespace sct;

// SC type zero width in signals/ports in sensitivity
template <unsigned N>
struct MyModule : public sc_module 
{
    SC_HAS_PROCESS(MyModule);

    sc_signal<sct_uint<N>>  s{"s"};
    sc_signal<unsigned>     t{"t"};
    
    MyModule(const sc_module_name& name) : sc_module(name) {
        SC_METHOD(sigProc); 
        sensitive << s;
    }

    void sigProc() {
        int l = 42;
        //s = l;
    }
};

struct Top : public sc_module 
{
    SC_HAS_PROCESS(Top);

    sc_vector<MyModule<0>>  m{"m",2};
    
    Top(const sc_module_name& name) : sc_module(name) {
    }
};

int sc_main(int argc, char **argv) 
{
    Top top_mod{"top_mod"};
    sc_start();

    return 0;
}


