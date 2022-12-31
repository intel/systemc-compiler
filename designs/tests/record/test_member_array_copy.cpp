/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

using namespace sc_core;

// Record with array member copy, array of record with array inside
class A : public sc_module {
public:
    sc_in_clk           clk{"clk"};
    sc_signal<bool>     rst;
    
    sc_signal<sc_uint<4>> s;

    SC_CTOR(A) 
    {
        SC_METHOD(simple_loc_rec_meth);
        sensitive << s;

        
        SC_METHOD(loc_rec_meth);
        sensitive << s;
        
        // TODO: Fix me, #141, #127
        //SC_METHOD(loc_rec_rec_meth);
        //sensitive << s;

        SC_METHOD(loc_arr_rec_arr_meth);
        sensitive << s;
        
        // TODO: Fix me, #127
        //SC_METHOD(loc_rec_arr_rec_meth);
        //sensitive << s;
        
        SC_METHOD(glob_rec_meth);
        sensitive << s;
        
        // TODO: Fix me, #141, #127
        //SC_METHOD(glob_rec_rec_meth);
        //sensitive << s;

        // TODO: Fix me, #141, #127
        //SC_METHOD(glob_arr_rec_meth);
        //sensitive << s;

        SC_CTHREAD(loc_array_copy, clk.pos());
        async_reset_signal_is(rst, true);
    }
    
    struct SimpleRec {
        bool a;
        sc_uint<4>  b;
    };
    
    struct ArrRec {
        sc_uint<4>  b[3];
    };
    
    struct RecArrRec {
        ArrRec rec;
    };
    
    struct ArrRecArrRec {
        ArrRec rec[2];
    };

    void simple_loc_rec_meth() 
    {
        SimpleRec ar;
        ar.b = 1;
        SimpleRec br(ar);
    }
    
    // Simple methods to manually check state after copy constructor
    void loc_rec_meth() 
    {
        ArrRec ar;
        ar.b[0] = 1; ar.b[1] = 2; ar.b[2] = 4;
        int i = s.read();
        ar.b[i] = i;
        i = ar.b[i+1] + 1;
        
        ArrRec br(ar);
        
//        sct_assert_const(br.b[0] == 1);
//        sct_assert_const(br.b[1] == 2);
//        sct_assert_const(br.b[2] == 4);
    }

    void loc_rec_rec_meth() 
    {
        RecArrRec arr;
        arr.rec.b[0] = 1; arr.rec.b[1] = 2; arr.rec.b[2] = 4;
        RecArrRec brr(arr);        

        sct_assert_const(brr.rec.b[0] == 1);
        sct_assert_const(brr.rec.b[1] == 2);
        sct_assert_const(brr.rec.b[2] == 4);
    }

    void loc_arr_rec_arr_meth() 
    {
        ArrRec ar[2];
        int i = s.read();
        int j = s.read()+1;
        ar[1].b[0] = 1; 
        ar[i].b[j] = 2; 
        j = ar[i+1].b[j-1] + ar[i].b[1];
    }
    
    // #127, incorrect state for @aar declaration, no array in record in state
    void loc_rec_arr_rec_meth() 
    {
        ArrRecArrRec aar;
        aar.rec[0].b[0] = 3; //aar.rec[1].b[1] = 5; aar.rec[0].b[2] = 7;
        sct_assert_const(aar.rec[0].b[0] == 3);

        /*ArrRecArrRec bbr(aar);

        sct_assert_const(bbr.rec[0].b[0] == 3);
        sct_assert_const(bbr.rec[1].b[1] == 5);
        sct_assert_const(bbr.rec[0].b[2] == 7);*/
    }
    
// ----------------------------------------------------------------------------    

    ArrRec gr;
    RecArrRec grr;
    ArrRecArrRec gar;
    
    void glob_rec_meth() 
    {
        gr.b[0] = 1; gr.b[1] = 2; gr.b[2] = 4;
        ArrRec lr(gr);

        sct_assert_const(lr.b[0] == 1);
        sct_assert_const(lr.b[1] == 2);
        sct_assert_const(lr.b[2] == 4);
    }
    
    void glob_rec_rec_meth() 
    {
        grr.rec.b[0] = 1; grr.rec.b[1] = 2; grr.rec.b[2] = 4;
        RecArrRec lrr(grr);

        sct_assert_const(grr.rec.b[0] == 1);
        sct_assert_const(grr.rec.b[1] == 2);
        sct_assert_const(grr.rec.b[2] == 4);
    }
    
    void glob_arr_rec_meth() 
    {
        gar.rec[0].b[0] = 3; gar.rec[1].b[1] = 5; gar.rec[0].b[2] = 7; 
        ArrRecArrRec lar(gar);
        
        sct_assert_const(lar.rec[0].b[0] == 3);
        sct_assert_const(lar.rec[1].b[1] == 5);
        sct_assert_const(lar.rec[0].b[2] == 7);
    }
    
// ----------------------------------------------------------------------------    
    
    // Copy of record with array, check @par_b is register
    template<class T>
    void rec_param_copy(T par) {    // reg
        par.b[s.read()] = 1;
        
        wait();
        
        auto l = par.b[1];
    }
    
    void loc_array_copy()
    {
        wait();
        
        while (true) 
        {
            ArrRec xlarr;        
            rec_param_copy(xlarr);
            
            wait();
        }
    }
    
};

int sc_main(int argc, char *argv[]) {
    sc_clock clk("clk", 1, SC_NS);
    A a{"a"};
    a.clk(clk);
    sc_start();
    return 0;
}
 
