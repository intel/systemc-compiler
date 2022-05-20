/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Record local variable and member in MIF array 
struct mod_if : public sc_module, sc_interface 
{
    sc_signal<bool>     s {"s"};

    SC_CTOR(mod_if) 
    {
        // TODO: Fix me, see #158
        //SC_METHOD(useDefBug);
        //sensitive << s;

        SC_METHOD(memRecMeth);
        sensitive << s;

        // TODO: Fix me, see #158
        SC_METHOD(memRecArrMeth);
        sensitive << s;
    }
    

struct Inner {
    int c;
};

struct Simple {
    bool a;
    sc_uint<4> b[3];
    Inner  rec;
    Inner  rec_arr[2];
};

struct NotSimple {
    Inner  rec_arr[2];
    Inner  rec_oth_arr[2];
};


// ---------------------------------------------------------------------------    

    // Incorrect result of @getFirstArrayElementForAny() -- 
    // return t[1].rec_arr[0].c instead of t[0].rec_arr[0].c
    struct UDSimple {
        Inner  rec_arr[2];
    };
    UDSimple t[2];
    
    void useDefBug() 
    {
        t[1].rec_arr[1].c = 30;
    }
    
    
// ---------------------------------------------------------------------------    
    
    // Member record 
    Simple  r;
    Simple  rr;
    NotSimple f;

    void memRecMeth() 
    {
        int minst_r_b[2];   
        minst_r_b[0]  = 0;  // name conflict with @r.b
        r.b[0]  = 1;
        r.rec.c = 2;
        rr.a = s.read();
        rr.b[2] = 3;
        rr.rec_arr[1].c = 4;
        f.rec_arr[1].c = 5;
        f.rec_oth_arr[1].c = 6;
        
        int i = f.rec_arr[0].c + rr.rec_arr[0].c + r.b[1];
    }
    
    // Member record array
    Simple      w[2];
    NotSimple   ww[3];

    void memRecArrMeth() 
    {
        w[0].a = s.read();
        w[0].b[1] = 10;
        w[1].rec.c = 20;
        w[0].rec_arr[1].c = 30;
        
        // TODO: Uncomment after #158 fixed 
        /*int sum = 0;
        for (int i = 0; i < 3; ++i) {
            ww[i].rec_arr[1].c = i;
            sum += ww[i].rec_oth_arr[1].c;
        }*/
        
        int sum = 0;
        for (int i = 0; i < 2; ++i) {
            ww[0].rec_arr[i].c = i;
            sum += ww[0].rec_oth_arr[i].c;
        }
        
        sc_uint<4> x = w[0].rec_arr[1].c + ww[0].rec_oth_arr[0].c;
        // TODO: Uncomment after #158 fixed 
        //sc_uint<4> y = w[1].rec_arr[1].c + ww[2].rec_oth_arr[0].c;
    }
};

SC_MODULE(Top) 
{
    mod_if*         minst[2];
    
    SC_CTOR(Top) 
    {
        for (int i = 0; i < 2; i++) {
            minst[i] = new mod_if("mod_if");
        }
    }
};
    
int sc_main(int argc, char **argv) 
{
    Top top{"top"};
    sc_start();

    return 0;
}
