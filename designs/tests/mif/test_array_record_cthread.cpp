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
    sc_in_clk           clk{"clk"};
    sc_signal<bool>     nrst;
    sc_signal<bool>     s {"s"};

    SC_CTOR(mod_if) 
    {
        // TODO; Fix me, warnings see #159
        SC_CTHREAD(memRecThread, clk.pos());
        async_reset_signal_is(nrst, false);

        SC_CTHREAD(memRecArrThread, clk.pos());
        async_reset_signal_is(nrst, false);
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

    // Member record 
    Simple  r;
    Simple  rr;

    void memRecThread() 
    {
        wait();
   
        while (true) {
            r.b[0]  = 1;
            r.rec.c = 2;
            rr.a = s.read();
            rr.b[2] = 3;
            rr.rec_arr[1].c = 4;

            int i = rr.rec_arr[0].c + r.b[1];
            wait();
        }
    }
    
    // Member record array
    Simple      w[2];
    Simple      ww[3];

    void memRecArrThread() 
    {
        wait();
   
        while (true) {
            w[0].a = s.read();
            w[0].b[1] = 1;
            w[1].rec.c = 2;
            w[0].rec_arr[1].c = 3;
            ww[0].rec_arr[1].c = 4;
            
            int i = w[0].rec_arr[1].c + ww[0].rec_arr[1].c + 
                    w[0].rec.c + ww[0].rec.c;
            // TODO: Uncomment after #158 fixed 
            //int j = w[1].rec_arr[0].c + ww[2].rec_arr[0].c + 
            //        w[1].rec.c + ww[2].rec.c;
            
            wait();
        }
    }
};

SC_MODULE(Top) 
{
    sc_in_clk       clk{"clk"};
    mod_if*         minst[2];

    SC_CTOR(Top) 
    {
        for (int i = 0; i < 2; i++) {
            minst[i] = new mod_if("mod_if");
            minst[i]->clk(clk);
        }
    }
};

int sc_main(int argc, char **argv) 
{
    sc_clock  clk("clk", sc_time(1, SC_NS));
    Top top{"top"};
    top.clk(clk);
    
    sc_start();

    return 0;
}
