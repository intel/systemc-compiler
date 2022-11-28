/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"
#include <sct_assert.h>

using namespace sc_core;

// Pass unknown element of pointer array to function and access via "->"
class A : public sc_module
{
public:
    sc_in<bool>         clk{"clk"};
    sc_in<bool>         nrst{"nrst"};
    sc_in<bool>         a{"a"};

    int*                pi;
    sc_uint<4>*         pia[2];
    sc_uint<4>*         pia1[2];
    sc_uint<4>*         pia2[2];
    sc_uint<4>*         pia3[2];
    sc_uint<4>*         piat[2];
    sc_uint<4>*         piaa[2][2];
    int*                pib[2];
    int*                pibb[2];
    sc_uint<4>          ia[2];
    sc_uint<4>          ia1[2];
    sc_uint<4>          iat[2];
    sc_uint<4>          iaa[2][2];

    sc_signal<bool>*    pc;
    sc_signal<bool>*    pca[2];
    sc_signal<bool>*    pca1[2];
    sc_signal<bool>*    pcat[2];
    sc_signal<int>*     pcb[2];
    
    SC_CTOR(A)
    {
        // Dynamically allocated array
        pc = sc_new_array< sc_signal<bool> >(2);
        pi = sc_new<int>();
        
        for (int i = 0; i < 2; i++) {
            pia[i] = sc_new<sc_uint<4>>();
            pia1[i] = sc_new<sc_uint<4>>();
            pia1[i] = sc_new<sc_uint<4>>();
            pia2[i] = sc_new<sc_uint<4>>();
            pia3[i] = sc_new<sc_uint<4>>();
            piat[i] = sc_new<sc_uint<4>>();
            pib[i] = sc_new<int>();
            pibb[i] = sc_new<int>();
            pca[i] = new sc_signal<bool>("pca");
            pca1[i] = new sc_signal<bool>("pca");
            pcat[i] = new sc_signal<bool>("pca");
            pcb[i] = new sc_signal<int>("pcb");
            for (int j = 0; j < 2; j++) {
                piaa[i][j] = sc_new<sc_uint<4>>();
            }
        }
        
        SC_METHOD(read_pointer_array_unknown1); sensitive << a<< *pca[0];

        SC_METHOD(chan_pointer_array_param); sensitive << a << *pca1[0];
        SC_CTHREAD(chan_pointer_array_param_thread, clk.pos()); 
        async_reset_signal_is(nrst, 0);

        SC_METHOD(var_pointer_array_param); sensitive << a;
        SC_CTHREAD(var_pointer_array_param_thread, clk.pos()); 
        async_reset_signal_is(nrst, 0);


        SC_METHOD(var_pointer_array_init); sensitive << a;
        SC_METHOD(var_pointer_array_plus); sensitive << a;
        SC_METHOD(pointer_array_param); sensitive << a;
        
        SC_METHOD(read_pointer_array_unknown_b1); sensitive << a<< *pcb[0];
        SC_METHOD(read_pointer_array_unknown_b2); sensitive << a << *pcb[0];
    }

    template <typename VarType>
    void f_var_ref(VarType& var) {
        sc_uint<4> z = var;
    }

    template <typename ChanType>
    void f_ch_ref(ChanType& chan) {
        bool w = chan.read();
    }
    
    template <typename VarType>
    void f_var_ptr(VarType var) {
        sc_uint<4> x = *var;
    }

    template <typename ChanType>
    void f_ch_ptr(ChanType chan) {
        bool y = *chan ^ chan->read();
    }


    // Check UseDef from pointers array element passed to function
    void read_pointer_array_unknown1() {
        f_ch_ref(*pca[a]);
        
        sct_assert_read(pca);
        sct_assert_defined(pca, false);
    }

    // Passing array of channel pointers element to function
    void chan_pointer_array_param()
    {
        bool b1 = *pca1[a];
        bool b2 = (*pca1[a]).read();
        bool b3 = pca1[a]->read();
        f_ch_ref(*pca1[a]);
        f_ch_ptr(pca1[a]);
    }
    
    void chan_pointer_array_param_thread()
    {
        *pcat[0] = 1;
        wait();
        
        while (true) {
            f_ch_ref(*pcat[a]);
            f_ch_ptr(pcat[a]);
            wait();
        }
    }
    
    // Passing array of non-channel pointers element to function
    void var_pointer_array_param()
    {
        sc_uint<4> c1 = *pia[a];
        f_var_ref(ia[a]);     
        f_var_ref(*pia[a]); 
        f_var_ptr(pia[a]);
    }

    void var_pointer_array_param_thread()
    {
        iat[0] = 1;
        *piat[0] = 1;
        wait();
        
        while (true) {
            sc_uint<4> c1 = *piat[a];
            f_var_ref(iat[a]);     
            f_var_ref(*piat[a]); 
            f_var_ptr(piat[a]);
            wait();
        }
    }

    // Declare and initialize pointer with array unknown element
    void var_pointer_array_init()
    {
        sc_uint<4>* d0 = pia1[1];
        *d0 = 0;
        sc_uint<4>* d1 = pia1[a];
        *d1 = 1;
        int j = *d1 / 2;
        
        sc_uint<4>* d2 = d1;
        *d2 = 2;
        j = *d2 - *d1;
        
        sc_uint<4>* d3 = nullptr;
        bool b = d3;
        
        sc_uint<4>* d4;     // warning here
    }
    
     // Unary plus for pointer with array unknown element
    void var_pointer_array_plus()
    {
        sc_uint<4>* d1 = +pia2[a];
        *d1 = 2;
    }

// ---------------------------------------------------------------------------

    template <typename ArrType>
    void f_arr(ArrType arr) {
        sc_uint<4> y = arr[1];
    }

    template <typename ArrType>
    void f_arr_ptr(ArrType arr) {
        sc_uint<4> y = *arr[1];
    }
    
    void f_arr_ref(sc_uint<4> (&arr)[2]) {
        int k = arr[1];
    }
    
    // Array/sub-array passed to function
    void pointer_array_param()
    {
        f_arr(ia1);     
        f_arr_ptr(pia3);     

        f_arr(iaa[1]);
        f_arr_ptr(piaa[1]);
        f_arr_ptr(piaa[a]);
        
        f_arr_ref(ia1);     
        f_arr_ref(iaa[1]);     
        f_arr_ref(iaa[a]);     
    }

// -------------------------------------------------------------------------
// Multiple parameters
    
    template <typename VarType, typename ChanType>
    void f_ref2(VarType& var, ChanType& chan) {
        sc_uint<4> z = var + chan.read();
    }

    
    template <typename VarType, typename ChanType>
    void f_ptr2(VarType var, ChanType chan) {
        sc_uint<4> x = *var + chan->read();
    }

    void read_pointer_array_unknown_b1() {
        f_ref2(*pib[a.read()+1], *pcb[a]);
        
        sct_assert_read(pcb);
        sct_assert_read(pib);
        sct_assert_defined(pcb, false);
        sct_assert_defined(pib, false);
    }
    
    void read_pointer_array_unknown_b2() {
        f_ptr2(pibb[a.read()-1], pcb[a]);
        
        sct_assert_read(pcb);
        sct_assert_read(pibb);
        sct_assert_defined(pcb, false);
        sct_assert_defined(pibb, false);
    }
};

class B_top : public sc_module
{
    sc_signal<bool> a{"a"};
    sc_signal<bool> clk{"clk"};
    sc_signal<bool> nrst{"nrst"};

public:
    A a_mod{"a_mod"};

    SC_CTOR(B_top) {
        a_mod.clk(clk);
        a_mod.nrst(nrst);
        a_mod.a(a);
    }
};

int sc_main(int argc, char* argv[])
{
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}

