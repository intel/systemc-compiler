/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"

using namespace sc_core;

// Local references/constant references, local reference to signal/signal array
template <unsigned N>
class A : public sc_module {
public:
    sc_in_clk clk;
    sc_signal<bool> nrst;

    int m;
    int k = 2;
    int kk = 2;
    int n = 3;
    
    static const unsigned CONST_A = N;
    static const unsigned CONST_B = CONST_A << 1;
    static const unsigned CONST_C = CONST_A + CONST_B;
    // These constants are not used, that is a special case
    static constexpr unsigned CEXPR_A = CONST_A;
    static constexpr unsigned CEXPR_B = CEXPR_A << 1;
    static constexpr unsigned CEXPR_C = CEXPR_A + CEXPR_B;
    
    int& r1 = n;
    const int& r2 = kk;
    sc_uint<7> f;
    sc_uint<7>& r3 = f;

    sc_signal<sc_uint<4>> s;
    sc_signal<bool> dummy{"dummy"};

    SC_CTOR(A) {
        SC_METHOD(reference1); sensitive << s;
        SC_METHOD(reference2); sensitive << s;
        SC_METHOD(reference3); sensitive << s;
        
        SC_METHOD(const_reference); sensitive << dummy;
        SC_METHOD(const_reference2); sensitive << dummy;
        
        SC_METHOD(const_reference3); sensitive << s << dummy;
        
        SC_METHOD(const_reference_sig); sensitive << s << sig << sig2 << sig3;
        SC_METHOD(const_reference_sig_arr); sensitive << s << sig_arr[0] << sig_arr2[0];

        SC_METHOD(init_list); sensitive << dummy;

        SC_CTHREAD(array_ref_wait, clk.pos());
        async_reset_signal_is(nrst, 0);
        
        SC_METHOD(reference_init_func); sensitive << s;
    }

    // reference type 
    void reference1() {
        int a;
        int &b = a;
        b = 1;
        
        unsigned m[3];
        unsigned &c = m[1];
        c = 1;
        
        bool bb[5];
        for (int i = 0; i < 5; i++)  {
            bool &b = bb[i];
            b = i;
        }
        
        // Global references
        r1 = 1;
        int i = r2;
        
        // Reference as RValue
        i = r1 + r2;
        int j = b;
        j = b + c * a + i;
    }    
    
    // reference for SC data type
    void reference2() {
        sc_uint<4> a;
        sc_uint<4> &b = a;
        b = 1;
        
        sc_uint<3> m1[4];
        sc_uint<3> &c = m1[0];
        c = 1;
        
        sc_bigint<65> m2[3];
        sc_bigint<65> &d = m2[2];
        d = c;
        
        r3 = 1;
        
        sc_bigint<66> e = c + b*d - r3;
    }        

    // Array element unknown
    sc_uint<4> gm[3][4];
    void reference3() {
        unsigned lm[5];
        
        unsigned& d1 = lm[s.read()];
        d1 = 1;
        int i = lm[s.read()];
        unsigned& d2 = lm[i];
        i =  lm[i];
        unsigned& d3 = lm[i];
        d2 = d3;
        unsigned& d4 = lm[i+1];
        d4 = i;
        
        sc_uint<4>& e1 = gm[s.read()][1];
        e1 = 0;
        sc_uint<4>& e2 = gm[i][2];
        sc_uint<4>& e3 = gm[e2][0];
        e3 = e2;
        i = gm[s.read()][1];
        sc_uint<4>& e4 = gm[i][i];
        e4 = d4;
    }    
    
    // constant reference for SC data type
    void const_reference() 
    {
        int a;
        const int &b = a;
        a = 1;
        a = b + 1; 
        
        const int &c = k;
        k = b;
        m = c + b*a;
        
        const unsigned &d = 3;
        m = d + c;

        const unsigned &e = 3 + d;
        m = e - d;
        
        unsigned arr[3];
        const unsigned &f = arr[1];
        arr[0] = 1;
        arr[1] = 2;
        arr[2] = 3;
        
        m = f - 2*d - e*e;
    }        
    
    // constant reference for SC data type
    void const_reference2() 
    {
        sc_uint<4> a;
        const sc_uint<4> &b = a;
        a = 1;
        int m = b.range(3,1);
        
        sc_uint<3> arr[5];
        const sc_uint<3> &c = arr[2];
        const sc_uint<3> &d = arr[3];
        const sc_uint<3> &e = d;

        const sc_uint<3> &f = 4;
        
        sc_int<10> g = c + d*e - f;
    }  
    
    void const_reference3()
    {
        sc_uint<3> arr[3];
        int m = s.read();
        const sc_uint<3>& a = arr[s.read()];
        const sc_uint<3>& b = arr[s.read()+1];
        const sc_uint<3>& c = arr[m];
        const sc_uint<3>& d = arr[m-1];
        
        int sum = a + b + c + d;
    }

    sc_signal<sc_uint<4>> sig;
    sc_signal<sc_uint<8>> sig2;
    sc_signal<sc_biguint<4>> sig3;
    void const_reference_sig()
    {
        const sc_uint<4>& a = sig;
        const sc_uint<4>& b = sig.read();
        const sc_uint<4>& c = sig.read()+1;
        
        // Another type signal, no reference
        const sc_uint<4>& d = sig2.read();
        const sc_uint<4>& e = sig3.read();
        
        int sum = a + b + c + d + e;
    }
    
    sc_signal<sc_uint<4>> sig_arr[3];
    sc_signal<sc_uint<8>> sig_arr2[3];
    void const_reference_sig_arr()
    {
        const sc_uint<4>& a = sig_arr[1];
        const sc_uint<4>& b = sig_arr[1].read();
        const sc_uint<4>& c = sig_arr[s.read()];
        const sc_uint<4>& d = sig_arr[s.read()+1].read();
        
        const sc_uint<4>& e = sig_arr2[s.read()].read();

        int sum = a + b + c + d + e;
    }

    
    // Initialization list
    const int il1[3] = {1, 2, 3};
    const sc_uint<3> il2[2] = {1, 2};
    const int il3[3] = {1, n, k+1};

    void init_list() {
        int m = 1;
        int il4[2] = {0, m};
    }

// ----------------------------------------------------------------------------    
    // Constant reference initialized with operator or function
    int f1(int i ) {
        return (i+1);
    }
    
    void reference_init_func() {
        int m = 1;
        const int& r = m++;
        const int& rr = --m;
        const int& rrr = f1(m++);
        int a = r + rr + rrr;
        // return by reference not supported yet
    }
    
    
// ----------------------------------------------------------------------------
    // For #182, warning generated

    sc_signal<unsigned>     indx;
    
    void array_ref_wait() {
        sc_uint<4> arr[3];
        wait();
        
        while (true) {
            sc_uint<4>& b = arr[indx.read()];
            wait();
            sig = b;
            wait();
        }
    }};

class B_top : public sc_module {
public:
    sc_clock clk{"clk", 1, SC_NS};

    A<1> a_mod{"a_mod"};

    SC_CTOR(B_top) {
        a_mod.clk(clk);
    }
};

int sc_main(int argc, char *argv[]) {
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}

