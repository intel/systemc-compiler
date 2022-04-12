/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

using namespace sc_core;

// Local variables name conflicts with empty method names
class mod_if : public sc_module, sc_interface 
{
public:
    sc_in_clk       clk;
    sc_signal<bool> rstn;
    sc_signal<int>  s;

    SC_CTOR(mod_if) 
    {
        SC_METHOD(local_var); sensitive << s;
        SC_METHOD(member_var); sensitive << s;
        SC_METHOD(both_var); sensitive << s;
    }
        
    void local_var() 
    {
	bool a;
        int i;
        sc_uint<4> x;
        
        i = s.read();
        long sum = a ? i : x.to_int();
    }

    
    bool a = false;
    int j;
    long sum;
    
    void member_var() 
    {
        j = s.read();
        sum = a ? j : j+1;
    }


    int i;
    void both_var() 
    {
        {
            int i = 0;  // Local
            s = i + 1;
        }
        
	i = 1;          // Module
        s = i + 2;
        
        int i;          // Local
        i = 2;      
        s = i + 3;
    }
};

SC_MODULE(Top) 
{
    sc_in_clk clk;
    mod_if    minst{"minst"};
    mod_if*   marr[2];

    SC_CTOR(Top) {
        minst.clk(clk);
        for (int i = 0; i < 2; ++i) {
            marr[i] = new mod_if("mod_if");
            marr[i]->clk(clk);
        }
    }
};

int sc_main(int argc, char *argv[]) 
{
    sc_clock clk{"clk", 10, SC_NS};
    Top top{"top"};
    top.clk(clk);
    
    sc_start();
    return 0;
}

