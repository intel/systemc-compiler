/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Constants and static constants in global scope
const bool BOOL_CONST = true;
static const bool BOOL_ST_CONST = true;

const int INT_CONST = 2;
static const int INT_ST_CONST = 3;

const sc_int<4> SC_INT_CONST = -2;
static const sc_uint<4> SC_UINT_ST_CONST = 3;

struct MyStruct : public sc_module, sc_interface
{
    SC_CTOR(MyStruct) {
    }
    
    virtual bool mb_read()
    {
        int i = INT_ST_CONST + INT_CONST;
        return (i == 1);
    }
};

SC_MODULE(MyModule) 
{
    sc_in_clk       clk;
    sc_signal<bool> rst;
    sc_signal<int>  sig;
    
    MyStruct        struct1;
    
    SC_CTOR(MyModule) : struct1("struct1") {
        SC_CTHREAD(proc, clk.pos());
        async_reset_signal_is(rst, 0);
    }    
    
    void proc() 
    {
        int i = (BOOL_ST_CONST) ? INT_CONST : INT_ST_CONST;
        wait();
        while (true) {
            bool b = struct1.mb_read();
            sig = SC_INT_CONST;
            sig = SC_UINT_ST_CONST;
            wait();
        }
    }
};


SC_MODULE(tb) {

    sc_clock clk{"clk", 1, SC_NS};
    MyModule top_mod{"top_mod"};

    SC_CTOR(tb) {
        top_mod.clk(clk);
    }

};


int sc_main(int argc, char **argv) {

    cout << "test_promote_ports\n";

    tb tb_inst{"tb_inst"};
    sc_start();

    return 0;
}


