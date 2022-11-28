/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Constants (static), constant (static) arrays and constant pointers in module and MIF
struct D {
    static const bool     COUPLED_BLOCK_DOMAIN[];
};

const bool D::COUPLED_BLOCK_DOMAIN[] = {1};

struct mod_if : public sc_module, sc_interface 
{
    sc_signal<unsigned>   s1;
    sc_signal<unsigned>   s2;
    sc_signal<unsigned>   s3;
    sc_signal<unsigned>   r;
    
    SC_HAS_PROCESS(mod_if);
    
    mod_if(const sc_module_name& name, unsigned par_) : 
        sc_module(name), B(par_), BR{32, 33}
    {
        par = par_;
        SC_METHOD (mifMeth); sensitive << r;
    }
    
    void before_end_of_elaboration() override {
        C = sc_new<unsigned>(par);
    }
     
    unsigned par;
    static const unsigned A = 12;
    static const unsigned AR[2];
    const unsigned B;
    const unsigned BR[2];
    unsigned br[2];
    const unsigned* C = nullptr;
    
    void mifMeth() {
        s1 = 0;
        if (B == 11) {
            s1 = r;
        }
        unsigned l;
        l = A;
        l = B;
        l = *C;
        l = AR[1] + BR[1];
        l = AR[r.read()] + BR[r.read()];
        l = B;
        l = BR[0] + br[0];
    }
    
    void f1() {
        if (A) {
            s1 = 1;
        } else {
            s1 = r.read();
        }
    }
    
    void f2() {
        if (B) {
            s2 = 1;
        } else {
            s2 = r.read();
        }
    }
    
    void f3() {
        if (*C == 12) {
            s3 = 1;
        } else {
            s3 = r.read();
        }
    }
};

const unsigned mod_if::AR[] = {22, 23};

SC_MODULE(Top) {

    sc_in_clk           clk{"clk"};
    sc_signal<bool>     rst;
    
    static const unsigned N = 3;
    sc_signal<unsigned> t;
    const int L[3] = {1,2,3};
    const bool BA[3] = {1, 1, 0};
    static const unsigned SA[3];
    static const bool SB[3];
    const sc_bigint<4> mmd[3] = {5,-5};
    
    mod_if*             minst[N];

    SC_CTOR(Top) {
        for (int i = 0; i < N; i++) {
            minst[i] = new mod_if("mod_if", 10+i);
        }
        
        SC_METHOD (top_method1); sensitive << t;
        SC_METHOD (top_method2); sensitive << t;
        SC_METHOD (top_method3); sensitive << t;
        SC_METHOD (top_method4); sensitive << t;
    }
    
    void top_method1() {
        unsigned i = t.read();
        minst[i]->f1();
    }

    void top_method2() {
        int j;
        unsigned i = t.read();
        unsigned res;
        
        res = minst[0]->B;
        res = minst[1]->B;
        res = minst[i]->B;
        
        minst[2]->f2();
        minst[i]->f2();
    }
    
    void top_method3() {
        unsigned i = t.read();
        unsigned res;

        res = bool(minst[i]->C);
        res = *minst[0]->C; // Incorrect
        res = *minst[1]->C; // Incorrect
        res = *minst[i]->C; // Incorrect
        
        minst[i]->f3();     // Incorrect
    }
    
    void top_method4() {
        unsigned i = t.read();
        unsigned lu;
        sc_bigint<8> bi;
        
        lu = minst[i]->BR[0] + minst[i]->br[0];
        lu = L[1] + BA[2];
        lu = SA[1] + SB[2] + D::COUPLED_BLOCK_DOMAIN[0];
        
        bi = mmd[0] + mmd[1] + 1;
    }
};

const unsigned Top::SA[] = {22, 23, 24};
const bool Top::SB[] = {1, 1, 0};

int sc_main(int argc, char **argv) 
{
    sc_clock clk {"clk", sc_time(1, SC_NS)};
    Top top{"top"};
    top.clk(clk);
    
    sc_start();

    return 0;
}
