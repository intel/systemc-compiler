/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Array of modular interface pointers with port/signal pointers
struct mod_if : public sc_module, sc_interface 
{
    static const unsigned N = 2;

    sc_in_clk               clk;
    sc_in<bool>             rst;
    
    sc_in<int>*             in[N];
    sc_out<int>*            out[N];
    sc_signal<int>*         sig[N];
    sc_uint<4>*             var[N];

    sc_out<int>*            outt[N];
    sc_uint<4>*             vart[N];
    
    sc_signal<int>          s;
    
    SC_CTOR(mod_if) 
    {
        for (int i = 0; i < N ; i++) {
            in[i] = new sc_in<int>("in");
            sig[i] = new sc_signal<int>("sig");
            out[i] = new sc_out<int>("out");
            var[i] = sc_new<sc_uint<4>>();
            outt[i] = new sc_out<int>("out");
            vart[i] = sc_new<sc_uint<4>>();
        }

        SC_METHOD(methProc);
        sensitive << s << *in[0] << *sig[0];
    }

    void methProc() {
        int j = s.read();
        *var[j+1] = in[j]->read();
        *out[j] = *sig[j+1] + *var[j];
    }
    
};

SC_MODULE(Top) {

    sc_in_clk           clk{"clk"};
    sc_signal<bool>     rst;
    
    mod_if*             minst[2];
    sc_signal<int>      a[2][2];
    sc_signal<int>      b[2][2];

    SC_CTOR(Top) {
        for (int i = 0; i < 2; i++) {
            minst[i] = new mod_if("mod_if");
            minst[i]->clk(clk);
            minst[i]->rst(rst);

            for (int j = 0; j < 2; j++) {
                minst[i]->in[j]->bind(a[i][j]);
                minst[i]->out[j]->bind(a[i][j]);
                minst[i]->outt[j]->bind(b[i][j]);
            }
        }
        
        SC_METHOD(top_method);
        sensitive << a[1][1] << b[1][1] << *minst[0]->in[0] << *minst[0]->sig[0];
    }

    void top_method() 
    {
        int j = a[1][1];
        int i = b[1][1];
        *minst[i]->vart[j+1] = minst[i]->in[j]->read();
        *minst[i]->outt[j] = minst[i]->sig[j+1]->read();
        
        for (int k = 0; k < 2; k++) {
            *minst[0]->sig[k] = k;
        }
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
