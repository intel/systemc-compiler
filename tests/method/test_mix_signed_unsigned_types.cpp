/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"
#include "sct_sel_type.h"
#include "sct_assert.h"
#include <iostream>
using namespace std;

class A : public sc_module 
{
public:
    sc_in<bool>         a{"a"};
    sc_out<bool>        b{"b"};
    sc_signal<sc_uint<32> > s{"s"};
    
    sc_signal<bool> dummy{"dummy"};

    SC_CTOR(A)
    {
        SC_METHOD(mix_sign_unsign); sensitive << dummy;
    }

    //
    template<typename T1, typename T2, typename T3>
    void mix_sign_unsign_fn (T1 par1, T2 par2, T3 par3) {
        T1 A = par1;
        //cout << "1.. A = " << A << " and par3 is " << par3 << endl;
        T2 B = par2;
        //cout << "2.. B = " << B << " A = " << A << " and par3 is " << par3 << endl;
        T3 C;
        //cout << "3.. B = " << B << " A = " << A << " C = " << C << " and par3 is " << par3 << endl;
        C = par2/par1;
        //sct_assert_const(C==par3);
        cout << "4.. B = " << B << " A = " << A << " C = " << C << " and par3 is " << par3 << endl;
        
    }
    //
    template<typename T1, typename T2, typename T3>
    void mix_sign_unsign_fn_assert (T1 par1, T2 par2, T3 par3) {
        T1 A = par1;
        //cout << "1.. A = " << A << " and par3 is " << par3 << endl;
        T2 B = par2;
        //cout << "2.. B = " << B << " A = " << A << " and par3 is " << par3 << endl;
        T3 C;
        //cout << "3.. B = " << B << " A = " << A << " C = " << C << " and par3 is " << par3 << endl;
        C = par2/par1;
        cout << "4.. B = " << B << " A = " << A << " C = " << C << " and par3 is " << par3 << endl;
        sct_assert_const(C==par3);
    }

    void mix_sign_unsign()
    {
        //// C++:
        int iter; iter=0;
        cout << "C++ combinations" << endl;
        cout << "ITERATION: " << iter++ << endl;
        mix_sign_unsign_fn((unsigned int)(2), (int)(-5), (int)(2147483645));
        cout << "ITERATION: " << iter++ << endl;
        mix_sign_unsign_fn((int)(-2), (unsigned int)(5), (int)(0));
        cout << "ITERATION: " << iter++ << endl;
        mix_sign_unsign_fn((unsigned long)(2), (long)(-5), (long)(9223372036854775805));
        cout << "ITERATION: " << iter++ << endl;
        mix_sign_unsign_fn((long)(-2), (unsigned long)(5), (long)(0));
        cout << "ITERATION: " << iter++ << endl;

        //// SC_INT/UINT
        cout << "SC_INT/UINT combinations" << endl;
        cout << "ITERATION: " << iter++ << endl;
        mix_sign_unsign_fn((sc_uint<4>)(2), (sc_int<4>)(-5), (sc_int<8>)(-3));
        cout << "ITERATION: " << iter++ << endl;
        mix_sign_unsign_fn((sc_int<32>)(-2), (sc_uint<32>)(5), (sc_int<32>)(0));

        //// SC_BIGINT/BIGUINT
        cout << "SC_BIGINT/BIGUINT combinations" << endl;
        cout << "ITERATION: " << iter++ << endl;
        mix_sign_unsign_fn_assert((sc_biguint<32>)(2), (sc_bigint<31>)(-5), 
                                  (sc_bigint<32>)(-2));
        cout << "ITERATION: " << iter++ << endl;
        mix_sign_unsign_fn_assert((sc_bigint<32>)(-2), (sc_biguint<32>)(5), 
                                  (sc_bigint<32>)(-2));
    }
};

class B_top : public sc_module 
{
public:
    sc_signal<bool>  a{"a"};
    sc_signal<bool>  b{"b"};

    A a_mod{"a_mod"};

    SC_CTOR(B_top) {
        a_mod.a(a);
        a_mod.b(b);
    }
};

int sc_main(int argc, char* argv[])
{
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}

