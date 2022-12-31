/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

using namespace sc_core;

// Record with array member in METHOD
class A : public sc_module {
public:
    sc_in_clk           clk{"clk"};
    sc_signal<bool>     rst;
    
    sc_signal<sc_uint<4>> sig;

    SC_CTOR(A) 
    {
        SC_METHOD(member_fcall_ref);
        sensitive << sig;
        
        SC_METHOD(loc_array_decl);
        sensitive << sig;
        
        SC_METHOD(loc_array_init);
        sensitive << sig;
        
        SC_METHOD(simple_access);
        sensitive << sig;

        SC_METHOD(array_access);
        sensitive << sig;

        SC_METHOD(loc_array_access);
        sensitive << sig;

        SC_METHOD(local_fcall_param);
        sensitive << sig;
        
        SC_METHOD(fcall_param);
        sensitive << sig;
        
        SC_METHOD(local_fcall_return);
        sensitive << sig;
        
        SC_METHOD(fcall_return);
        sensitive << sig;
    }
    
    struct Rec {
        bool        a = 0;
        sc_uint<4>  b = 0;
    };
    
    struct ArrRec {
        bool        a[3] = {};
        sc_uint<4>  b[3] = {};

        void setA(bool par, int i) {
            a[i] = par;
        }

        bool getA(int i ) {
            return a[i];
        }
    };
    
    // Declaration of local record array with array member
    void loc_array_decl()
    {
        ArrRec xarr[2];
        xarr[1].b[2] = 0;
    }

    // Local array field initialization
    void loc_array_init()
    {
        Rec z;
        ArrRec x; 
        ArrRec xarr[2];
    }
    
    
    // Lost index of global record array when passing by reference
    template<class T>
    void rec_param_ref(T& par) {
        par.b[2] = 3;
    }
    
    ArrRec harr[2];
    void member_fcall_ref()
    {
        rec_param_ref(harr[1]);
        harr[1].b[2] = 3;
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

        int j = sig.read();
        s.b[j] = s.getA(j+1) ? j : 0;
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

        int j = sig.read();
        auto l = arr[j].b[j+1];
        arr[j].b[j+1] = arr[j].getA(j+1) ? j : 0;
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
        
        int j = sig.read();
        auto l = x.b[j] + xarr[j+1].b[j+2];

        x.b[j] = 1;
        xarr[j+1].b[j+2] = 2;
        xarr[j].setA(false, 1);
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

    
    // Local record array unknown element passing test
    void local_fcall_param()
    {
        Rec yy[3];
        ArrRec y;
        ArrRec yarr[2];
        
        rec_param2_val(y, 0);
        rec_param1(yy, 1);
        rec_param2_ref(y, 1);

        int j = sig.read();
        rec_param0(yarr[j]);
        rec_param2_val(yarr[j], 0);
        rec_param2_ref(yarr[j], 1);
    }


    // Member record array unknown element passing test
    Rec rr[3];
    ArrRec r;
    ArrRec rarr[2];
    
    void fcall_param()
    {
        rec_param2_val(r, 0);
        rec_param1(rr, 1);
        rec_param2_ref(r, 1);

        int j = sig.read();
        rec_param0(rarr[j]);
        rec_param2_val(rarr[j], 0);
        rec_param2_ref(rarr[j], 1);
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
        return res;
    }

    // Local record array unknown element return 
    void local_fcall_return()
    {
        ArrRec z;
        ArrRec zarr[2];

        auto lrec = rec_return1(z);
        auto llrec = rec_return2(z);
        int j = sig.read();
        auto lllrec = rec_return2(zarr[j]);
    }

    
    // Member record array unknown element return
    ArrRec y;
    ArrRec yarr[2];
    
    void fcall_return()
    {
        auto lrec = rec_return1(y);
        auto llrec = rec_return2(y);
        int j = sig.read();
        auto lllrec = rec_return2(yarr[j]);
    }
    
};

int sc_main(int argc, char *argv[]) {
    sc_clock clk("clk", 1, SC_NS);
    A a{"a"};
    a.clk(clk);
    sc_start();
    return 0;
}
 
