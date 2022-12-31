/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by mmoiseev on 06/14/19.
//

#include "sct_assert.h"
#include <systemc.h>

// Array of modular interface with member variables and member constants
// generated as @localparam
struct mod_if : public sc_module, sc_interface 
{
    sc_signal<sc_uint<4>>   s {"s"};
    sc_uint<4>              v;          // localparam
    unsigned                w;
    const unsigned          c;
    const unsigned          d = 1;

    mod_if(const sc_module_name& name, unsigned par) : 
        sc_module(name), c(par), v(par), w(par)
    {}
    
    sc_uint<4> getV() {
        auto ll = v;
        wait();
        return ll;
    };
};

struct mod_if2 : public sc_module, sc_interface 
{
    sc_signal<sc_uint<4>>   s {"s"};
    sc_uint<4>              v;       // localparam

    SC_HAS_PROCESS(mod_if2);
    
    mod_if2(const sc_module_name& name, unsigned par) : 
        sc_module(name), v(par)
    {
        SC_METHOD(mod_meth); sensitive << s;
    }
    
    void mod_meth() {
        auto ll = v;
    }
};

SC_MODULE(Top) {

    sc_in_clk       clk{"clk"};
    sc_signal<bool> rst;
    
    sc_signal<int>  t;
    sc_vector<mod_if>  minsts{"minsts", 2, [](const char* name, size_t i)
                                          {return new mod_if(name, i);}};
    sc_vector<mod_if2> minsts2{"minsts2", 2, [](const char* name, size_t i)
                                          {return new mod_if2(name, i+1);}};
    mod_if          minst{"minst", 1};
    mod_if*         pinst;

    SC_CTOR(Top) {
        pinst = new mod_if("mod_if", 1);
        
        SC_METHOD(usedef_method); sensitive << t;
        
        //SC_CTHREAD(usedef_thread, clk.pos());
        //async_reset_signal_is(rst, true);
        
        SC_METHOD(const_meth); sensitive << t;
        SC_METHOD(record); sensitive << t;
        SC_METHOD(record_array); sensitive << t;
        SC_METHOD(record_array_def); sensitive << t;
    }

    // No @localparam for @w
    void usedef_method() 
    {
        int j = t.read();
        int l = minst.v + minst.w;
        if (j) {
            minst.w = 1;
        }
        
        l = pinst->v + pinst->w;
        if (j) {
            pinst->w = 1;
        }
        
        l = minsts[1].v + minsts[1].v;
        if (j) {
            minsts[j].w = 1;
        }
        
        l = minsts2[j].v;
    }
    
    void usedef_thread() 
    {
        wait();
        while (true) {
            int l = minst.getV();
            wait();
        }
    }
    
    void const_meth() {
        int j = t.read();
        if (minsts[j].c) {
            j++;
        }
        if (minsts[j].d) {
            j--;
        }
    }
    
    struct Simple {
        bool a = false;
        int b = 42;
    };
    
    Simple rec;
    void record() {
        int i = rec.b;
        if (i) {
            ++i;
        }
        if (t.read()) rec.b = 1;
        sct_assert_unknown(rec.b);
    }
    
    Simple arr[3];
    void record_array() {
        int k;
        int j = t.read();
        int i1 = arr[1].b;
        int i2 = arr[j].b;
        if (arr[1].b) {
            k = 1;
        }
        if (arr[j].b) {
            k = 2;
        }
        k = i1 ? 3 : 4;
        k = i2 ? 5 : 6;
    }
    
    Simple marr[3];
    Simple narr[3];
    void record_array_def() {
        int k;
        int j = t.read();
        if (marr[1].b) {
            k = 1;
        }
        if (narr[1].b) {
            k = 2;
        }
        if (marr[j].b) {
            k = 3;
        }
        if (narr[j].b) {
            k = 4;
        }
        marr[0].b = 0;
        narr[j].b = 0;
    }
};

int sc_main(int argc, char **argv) 
{
    sc_clock clk {"clk", sc_time(1, SC_NS)};
    Top top{"top"};
    top.clk(clk);
    
    sc_start();

    return 0;
}
