/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "sct_comb_signal.h"
#include "systemc.h"
#include <iostream>

using namespace sc_core;

// Record initialization/assignment with in-place initialized record T{} and T()
class A : public sc_module {
public:
    sc_in_clk               clk{"clk"};
    sc_signal<bool>         rst{"rst"};
    sc_signal<unsigned>     s{"s"};

    SC_CTOR(A) 
    {
        //SC_METHOD(inner_rec_init_meth); sensitive << s;

        SC_METHOD(loc_array_init_meth); sensitive << s;
        SC_METHOD(mem_array_init_meth); sensitive << s;

        SC_CTHREAD(loc_array_init_thrd, clk.pos()); 
        async_reset_signal_is(rst, 0);
        
        SC_CTHREAD(mem_array_init_thrd, clk.pos()); 
        async_reset_signal_is(rst, 0);
        
        SC_METHOD(loc_array_init_compl_meth); sensitive << s;
    }
    
    
//------------------------------------------------------------------------------    
// Record with inner record    
    struct Inner {
        int c;
        Inner() = default;
    };

    struct Outer {
        bool a;
        Inner  rec;
        Outer() = default;
    };
    
    //Outer mo;
    sc_signal<int> t9;
    void inner_rec_init_meth() {
        Outer lo;
        //lo = Outer{};
        t9 = lo.rec.c;
    }
    

//------------------------------------------------------------------------------    
// Simple record 
    struct Rec_t {
        bool        a;
        sc_uint<4>  b;
        
        Rec_t() = default;
        Rec_t(int i) : a(i == 42) {}
    };
    
    Rec_t f() {
        Rec_t r;
        r.a = true;
        r.b = 3;
        return r;
    }
    
    Rec_t g(Rec_t par) {
        par.b += 1;
        return par;
    }
    
    sc_signal<int> t0;
    void loc_array_init_meth() {
        // Workaround
        //Rec_t tmp{}; loc = tmp;
        //Rec_t arr[2] = {};  // Error still reported -- that is OK

        Rec_t loc0 = f();
        
        Rec_t loc1;
        loc1 = Rec_t();
        loc1 = Rec_t{};
        loc1 = Rec_t(42);
        loc1 = Rec_t{42};
        
        loc1 = f();
        loc1 = g(loc1);
        
        Rec_t loc_arr[2];
        for (unsigned i = 0; i != 2; ++i) {
            loc_arr[i] = Rec_t{};
        }
        
        t0 = loc0.b + loc1.b + loc_arr[s.read()].b;
    }
    
    Rec_t mem;
    Rec_t mem_arr[2];
    sc_signal<int> t1;
    void mem_array_init_meth() 
    {
        mem = Rec_t{};

        for (unsigned i = 0; i != 2; ++i) {
            mem_arr[i] = Rec_t{};
        }
        
        t1 = mem.b + mem_arr[s.read()].b;
    }
    
    sc_signal<int> t2;
    void loc_array_init_thrd() {
        Rec_t loca;
        Rec_t locb;
        loca = Rec_t{};
        
        Rec_t loca_arr[2];
        Rec_t locb_arr[2];
        for (unsigned i = 0; i != 2; ++i) {
            loca_arr[i] = Rec_t{};
        }
        wait();
        
        while (true) {
            locb = Rec_t{};

            for (unsigned i = 0; i != 2; ++i) {
                locb_arr[i] = Rec_t{};
            }
            
            t2 = loca.b + locb.b + loca_arr[s.read()].b + locb_arr[s.read()].b;
            wait();
        }
    }
    
    Rec_t mema; Rec_t memb;
    Rec_t mema_arr[2]; Rec_t memb_arr[2];
    sc_signal<int> t3;
    void mem_array_init_thrd() {
        mema = Rec_t{};
        
        for (unsigned i = 0; i != 2; ++i) {
            mema_arr[i] = Rec_t{};
        }
        wait();
        
        while (true) {
            memb = Rec_t{};

            for (unsigned i = 0; i != 2; ++i) {
                memb_arr[i] = Rec_t{};
            }
            
            t3 = mema.b + memb.b + mema_arr[s.read()].b + memb_arr[s.read()].b;
            wait();
        }
    }
    
//------------------------------------------------------------------------------    
// More complicated record types 
    
     struct InitRec_t {
        bool            a = true;
        sc_uint<4>      b = 2;
        int             c;
        sc_biguint<8>   d;
        
        InitRec_t() : c(3) {
            //d = 4;      -- Error for non-empty constructor
        }
    };
    
    struct ArrRec_t {
        bool        a[3];
        sc_uint<4>  b[3];

        void setA(bool par, int i) {
            a[i] = par;
        }

        bool getA(int i ) {
            return a[i];
        }
    };

    struct ArrRecRec_t {
        ArrRec_t rec;
    };


    struct MultArrRec_t {
        sc_uint<4>  b[3][2];
    };
    
    
    sc_signal<int> t4;
    void loc_array_init_compl_meth() {
        InitRec_t loc;
        loc = InitRec_t{};
        
        //ArrRec_t loca;
        //loca = ArrRec_t{};  -- Error reported -- OK
        
//        ArrRecRec_t locb;
//        locb = ArrRecRec_t{}; -- Error reported -- OK
        
//        MultArrRec_t locc;
//        locc = MultArrRec_t{}; -- Error reported -- OK
    }    

//------------------------------------------------------------------------------    

};

int sc_main(int argc, char *argv[]) {
    sc_clock clk("clk", 1, SC_NS);
    A a{"a"};
    a.clk(clk);
    sc_start();
    return 0;
}
 
