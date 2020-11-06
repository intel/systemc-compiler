/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

struct Simple {
    bool a;
    sc_uint<4> b;
};

// Record local variable and member in MIF 
struct mod_if : public sc_module, sc_interface 
{
    sc_signal<bool>     s {"s"};

    SC_CTOR(mod_if) 
    {
        SC_METHOD(locRecMeth);
        sensitive << s;
    
        SC_METHOD(locRecArrMeth);
        sensitive << s;

        SC_METHOD(memRecMeth);
        sensitive << s;

        SC_METHOD(memRecArrMeth);
        sensitive << s;
    }
    
    // Local record 
    void locRecMeth() {
        Simple t;
        Simple tt;
        
        t.a = false;
        t.b = 4;
        
        tt.a = true;
        tt.b = 5;
        
        sc_uint<4> x = t.b + tt.b;
    }
    
    // Local record array 
    void locRecArrMeth() 
    {
        Simple v[2];
        Simple vv[4];
        
        v[0].a = false;
        v[0].b = 10;
        v[1].b = 20;
        
        int sum = 0;
        for (int i = 0; i < 4; ++i) {
            vv[i].b = (i < 2) ? v[i].b : (sc_uint<4>)i;
            sum += vv[i].b;
        }
        
        sc_uint<4> x = v[1].b + vv[3].b;
    }
    
    // Member record 
    Simple  r;
    Simple  rr;

    void memRecMeth() 
    {
        r.b = 4;
        rr.a = s.read();
        rr.b = 5;
        
        sc_uint<4> x = rr.a ? r.b : rr.b;
    }
    
    // Member record array
    Simple  w[2];
    Simple  ww[4];

    void memRecArrMeth() 
    {
        w[0].a = false;
        w[0].b = 10;
        w[1].b = 20;
        
        int sum = 0;
        for (int i = 0; i < 4; ++i) {
            ww[i].b = (i < 2) ? w[i].b : (sc_uint<4>)i;
            sum += ww[i].b;
        }
        
        sc_uint<4> x = w[1].b + ww[3].b;
    }
};

SC_MODULE(Top) 
{
    mod_if          minst{"minst"};
    mod_if          ninst{"ninst"};

    SC_CTOR(Top) {
        SC_METHOD(memRecMeth);
        sensitive << t;

    }
    
    sc_signal<bool>     t;
    
    Simple  rr;

    void memRecMeth() {
        rr.b = 5;
    }

};

int sc_main(int argc, char **argv) 
{
    Top top{"top"};
    sc_start();

    return 0;
}
