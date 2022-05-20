/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

using namespace sc_core;

// Record with array member in CTHREAD
class A : public sc_module {
public:
    sc_in_clk           clk{"clk"};
    sc_signal<bool>     rst;
    
    sc_signal<sc_uint<4>> sig;

    SC_CTOR(A) 
    {
        SC_CTHREAD(mult_array_decl, clk.pos());
        async_reset_signal_is(rst, true);

        SC_CTHREAD(loc_array_decl1, clk.pos());
        async_reset_signal_is(rst, true);
        SC_CTHREAD(loc_array_decl2, clk.pos());
        async_reset_signal_is(rst, true);
        
        SC_CTHREAD(loc_array_decl3, clk.pos());
        async_reset_signal_is(rst, true);
        
        SC_CTHREAD(loc_array_decl4, clk.pos());
        async_reset_signal_is(rst, true);
        SC_CTHREAD(loc_array_decl5, clk.pos());
        async_reset_signal_is(rst, true);
        SC_CTHREAD(loc_array_decl6, clk.pos());
        async_reset_signal_is(rst, true);

        SC_METHOD(loc_array_decl6_meth);
        sensitive << sig;
        
        SC_CTHREAD(loc_array_copy, clk.pos());
        async_reset_signal_is(rst, true);
        
        // Incorrect code as inner record not supported yet, #141
        //SC_CTHREAD(loc_array_copy2, clk.pos());
        //async_reset_signal_is(rst, true);
        
        SC_CTHREAD(loc_array_init, clk.pos());
        async_reset_signal_is(rst, true);
        
        SC_CTHREAD(fcall_array_unknown, clk.pos());
        async_reset_signal_is(rst, true);

        SC_CTHREAD(simple_access, clk.pos());
        async_reset_signal_is(rst, true);
        
        SC_CTHREAD(array_access, clk.pos());
        async_reset_signal_is(rst, true);
        
        SC_CTHREAD(loc_array_access, clk.pos());
        async_reset_signal_is(rst, true);
        
        SC_CTHREAD(local_fcall_param, clk.pos());
        async_reset_signal_is(rst, true);
        
        SC_METHOD(local_fcall_param_noinit);
        sensitive << sig;

        // Incorrect code as inner record not supported yet, #141
        //SC_CTHREAD(fcall_param, clk.pos());
        //async_reset_signal_is(rst, true);
        
        //SC_CTHREAD(fcall_param_simple, clk.pos());
        //async_reset_signal_is(rst, true);

        //SC_CTHREAD(fcall_return, clk.pos());
        //async_reset_signal_is(rst, true);
    }
    
    struct Rec {
        bool        a;
        sc_uint<4>  b;
    };
    
    struct ArrRec {
        bool        a[3];
        sc_uint<4>  b[3];

        void setA(bool par, int i) {
            a[i] = par;
        }

        bool getA(int i ) {
            return a[i];
        }
    };
    
    struct ArrRecRec {
        ArrRec rec;
    };
    
    
    struct MultArrRec {
        sc_uint<4>  b[3][2];
    };
    
    void mult_array_decl()
    {
        ArrRec qarr[5][4];
        MultArrRec parr[4];
        
        qarr[4][3].b[2] = 0;
        parr[3].b[2][1] = 0;
        wait();
        
        while (true) 
        {
            bool aa = qarr[3][2].a[1];
            sc_uint<4> bb = parr[2].b[1][0];
            wait();
        }
    }
    
    // Declaration of local record array with array member
    void loc_array_decl1()
    {
        ArrRec xarr1[2];
        xarr1[1].b[2] = 0;
        wait();
        
        while (true) 
        {
            wait();
        }
    }

    void loc_array_decl2()
    {
        wait();
        
        while (true) 
        {
            ArrRec xarr2[2];
            xarr2[1].b[2] = 0;
            wait();
        }
    }

    void loc_array_decl3()
    {
        wait();
        
        while (true) 
        {
            ArrRec xarr3[2];         // reg
            xarr3[1].b[2] = 0;
            wait();
            auto l = xarr3[1].b[1];
        }
    }

    void loc_array_decl4()
    {
        wait();
        
        while (true) 
        {
            ArrRec xarr4[2];         // reg
            xarr4[1].b[2] = 0;
            wait();
            auto l = xarr4[1].b[2];
        }
    }

    // Unknown index for member array
    ArrRec xarr5[2];         // reg
    void loc_array_decl5()
    {
        wait();
        
        while (true) 
        {
            int j = sig.read();
            ArrRec xlarr5[2];         // reg
            
            xarr5[1].b[j]  = 0;
            xlarr5[1].b[j] = 0;
            
            wait();
            auto l = xarr5[1].b[2] + xlarr5[1].b[2];
        }
    }
    
    // Unknown index for record array
    void loc_array_decl6()
    {
        wait();
        
        while (true) 
        {
            int j = sig.read();
            ArrRec xarr6[2];         // reg

            xarr6[j].b[2] = 0;       
            auto l = xarr6[1].b[2];

            wait();
        }
    }

    void loc_array_decl6_meth()
    {
        int j = sig.read();
        ArrRec xarr6[2];         

        xarr6[j].b[2] = 0;       
        auto l = xarr6[1].b[2];
    }

    // Copy of record with array, check @par_b is register
    template<class T>
    void rec_param_copy(T par) {    // reg
        int j = sig.read();
        par.b[j] = 1;
        
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
    
    // Copy of record  with record with array, check @par_rec_b is register
    template<class T>
    void rec_param_copy2(T par) {    // reg
        int j = sig.read();
        par.rec.b[j] = 1;
        
        wait();
        
        auto l = par.rec.b[1];
    }
    
    void loc_array_copy2()
    {
        wait();
        
        while (true) 
        {
            ArrRecRec xlarr;            // #141
            rec_param_copy2(xlarr);
            
            wait();
        }
    }
    
    // Local array field initialization
    void loc_array_init()
    {
        ArrRec x; 
        ArrRec xarr[2];
        
        wait();
        
        while (true) 
        {
            wait();
        }
    }    
    
    void fcall_array_unknown()
    {
        ArrRec yarr[2];
        int j = sig.read();
        yarr[j].a[0] = 0;
        
        wait();
        
        while (true) 
        {
            rec_param0(yarr[j]);
            wait();
        }
    }
    
    
//----------------------------------------------------------------------------    

    // Record with array member read/write access
    ArrRec s;
    
    void simple_access()
    {
        for (int i = 0; i < 3; ++i) {
            s.setA(false, i);
            s.b[i] = 0;
        }
        wait();
        
        while (true) 
        {
            int j = sig.read();
            s.b[j] = s.getA(j+1) ? j : 0;
            
            wait();
        }
    }
    
//----------------------------------------------------------------------------    

    // Array of record with array member read/write access
    ArrRec arr[2];
    
    void array_access()
    {
        for (int j = 0; j < 2; ++j) {
            for (int i = 0; i < 3; ++i) {
                arr[j].setA(false, i);
                arr[j].b[i] = 0;
            }
        }
        wait();
        
        while (true) 
        {
            int j = sig.read();
            auto l = arr[j].b[j+1];
            arr[j].b[j+1] = arr[j].getA(j+1) ? j : 0;
            
            wait();
        }
    }    

//----------------------------------------------------------------------------    

    // Local array of record with array member read/write access
    
    void loc_array_access()
    {
        ArrRec x; 
        ArrRec xarr[2];
        
        for (int i = 0; i < 3; ++i) {
            x.b[i] = i;
        }
        for (int j = 0; j < 2; ++j) {
            for (int k = 0; k < 3; ++k) {
                xarr[j].b[k] = 0;
            }
        }
        wait();
        
        while (true) 
        {
            int j = sig.read();
            auto l = x.b[j] + xarr[j+1].b[j+2];

            x.b[j] = 1;
            xarr[j+1].b[j+2] = 2;
            xarr[j].setA(false, 1);
            
            wait();
        }
    }    
//----------------------------------------------------------------------------    

    // Record with array member as function parameter
    
    // Local record array unknown element passing test
    template<class T>
    void rec_param0(T par) {
        auto l = par.b[0];
    }

    // Record array passed as pointer
    template<class T>
    void rec_param1(T par, int i) {
        par[i].b = par[i].a ? 1 : 2;
    }
    
    template<class T>
    void rec_param2_val(T par, int j) {
        par.b[j] = par.getA(j) ? 1 : 2;
    }
    
    template<class T>
    void rec_param2_ref(T& par, int k) {
        par.b[k] = par.getA(k) ? 1 : 2;
    }
    
    void local_fcall_param()
    {
        Rec vv[3];
        ArrRec v;
        ArrRec varr[2];
        
        for (int d = 0; d < 2; ++d) 
            for (int e = 0; e < 3; ++e) varr[d].a[e] = 0;
        
        int j = sig.read();
        rec_param2_ref(varr[j], 0);
        wait();
        
        while (true) 
        {
            rec_param1(vv, 1);
            rec_param2_ref(v, 1);
            
            rec_param0(varr[j]);
            rec_param2_val(varr[j], 0);
            rec_param2_ref(varr[j], 1);
        
            wait();
        }
    }
    
    // Check if non-initialized record leads to unknown condition in @rec_param2_ref
    void local_fcall_param_noinit()
    {
        ArrRec varr[2];
        int j = sig.read();
        rec_param2_ref(varr[j], 0);
    }

    Rec rr[3];
    ArrRec r;
    ArrRec rarr[2];
    
    void fcall_param()
    {
        for (int e = 0; e < 3; ++e) r.a[e] = 0;

        rec_param2_val(r, 0);   // par is reg as its reused after reset
        wait();
        
        while (true) 
        {
            rec_param1(rr, 1);
            rec_param2_ref(r, 1);
            
            int j = sig.read();
            rec_param0(rarr[j]);
            rec_param2_val(rarr[j], 0);
            rec_param2_ref(rarr[j], 1);
        
            wait();
        }
    }

    template<class T>
    void simple_param(T par) {
        int j = sig.read();
        j = par.a ? 1 : 2;
    }

    Rec lr;
    void fcall_param_simple()
    {
        simple_param(lr);
        wait();
        
        while (true) 
        {
            simple_param(lr);
            wait();
        }
    }

//----------------------------------------------------------------------------    

    // Record with array member return from function
    
    template<class T>
    T rec_return1(T par) {
        return par;
    }
    
    template<class T>
    T rec_return2(T par) {
        T res = par;
        wait();
        return res;
    }

    
    ArrRec z;
    ArrRec zarr[2];
    
    void fcall_return()
    {
        for (int e = 0; e < 3; ++e) {
            z.a[e] = 0; z.b[e] = 0;
        }
        auto lrec = rec_return1(z);
        wait();
        
        while (true) 
        {
            auto llrec = rec_return2(z);
            int j = sig.read();
            auto lllrec = rec_return2(zarr[j]);
            
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
 
