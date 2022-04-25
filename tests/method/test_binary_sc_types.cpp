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

// Binary operations for SC types
class AA : public sc_module 
{
public:
    sc_signal<bool> clk{"clk"};
    sc_signal<bool> rstn{"rstn"};
    
    sc_in<bool>         a{"a"};
    sc_out<bool>        b{"b"};
    sc_out<bool>        c{"c"};
    sc_out<bool>*       p;
    sc_signal<sc_uint<32> > s{"s"};
    
    int                 m;
    int                 k;
    int*                q;
    int iter = 0;

    sc_signal<bool> dummy{"dummy"};

    SC_CTOR(AA)
    {
        SC_METHOD(tilda); sensitive << dummy;
        SC_METHOD(sc_signed_ops_neg_consts_fns); sensitive << dummy;
    }
    
    #define CHECK(ARG) sct_assert(ARG); sct_assert_const(ARG);
    
    static const unsigned STORED_RESP_WIDTH = 66;
    static const unsigned ACTIVE_TRANS_NUM = 1;
    static const unsigned ACTIVE_BEAT_NUM = 2;
    
    typedef sc_biguint<STORED_RESP_WIDTH> StoredResp_t;
    sc_signal<StoredResp_t>    sig;
    
    void tilda() {
        sc_int<10> A = 0;
        sc_bigint<64> bn = ~A;       
        
        sc_uint<16> ad = 0xABCD;
        sc_uint<16> wd = 0x0;
        wd.bit(0) = ~ad.bit(8);
        wd.bit(1) = ~ad.bit(9);
        wd.bit(2) = ~ad.bit(10);
        wd.bit(3) = ~ad.bit(11);
    }
    
    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22>
    void sc_signed_ops_neg_consts (T1 par1, T2 par2, T3 par3, T4 par4, T5 par5, T6 par6, T7 par7, T8 par8, T9 par9, T10 par10, T11 par11, T12 par12, T13 par13, T14 par14, T15 par15, T16 par16, T17 par17, T18 par18, T19 par19, T20 par20, T21 par21, T22 par22) {
        T1 A = par1;
        T2 B = par2;
        T5 E = par5;
        T6 F = par6;

        // Comparison
        //cout << "A == B: " << eq << endl;
        bool eq = (A==B);
        bool lll = (eq==par3);
        CHECK(eq==par3);
        bool neq = (A!=B);
        CHECK(neq==par4);
        bool lt = E>F;
        bool gte = A>=B;
        bool lte = A<=B;
        CHECK(gte==par17);
        CHECK(lte==par18);
        CHECK(lt==par7);
        bool gt = E<F;
        CHECK(gt==par8);
        // Arithmatic
        sc_bigint<64> sum = A+B;
        sc_bigint<64> diff = A-B;
        CHECK(sum==par9);

        // "expected initializer before ‘+=’ token" //
        sum += B;
        CHECK(sum==par12);

        CHECK(diff==par19);
        sc_bigint<64> mult = E*F;
        sc_bigint<64> div = E/F;
        CHECK(mult==par10);
        CHECK(div==par11);

        // Logic Operations
        // &:         Bitwise AND:        expr1 & expr2 :
        sc_bigint<64> ba = A & B;
        //cout << "Bitwise AND ba = " << ba << endl;
        CHECK(ba==par13);
        // |:         Bitwise OR:         expr1 | expr2 :
        sc_bigint<64> bo = A | B;
        //cout << "Bitwise OR bo = " << bo << endl;
        CHECK(bo==par14);
        // ^:         Bitwise XOR:        expr1 ^ expr2 :
        sc_bigint<64> bx = A ^ B;
        //cout << "Bitwise XOR bx = " << bx << endl;
        CHECK(bx==par15);
        // ~:         Bitwise NOT:        ~expr :
        sc_bigint<64> bn = ~A;
        //cout << "Bitwise NOT A " << hex << A << " bn = " << bn << " par16 " << par16 << dec << endl;
        // Does not work for bigint as type width is not determinable
        //CHECK(bn==par16);
        
        // &=         AND assignment:     variable &= expr :
        bo &= A;
        //cout << "Bitwise AND bx = " << bo << endl;
        CHECK(bo==par20);
        //  |=         OR assignment:      variable |= expr :
        ba |= bo;
        CHECK(bo==par21);
        //cout << "Bitwise OR bx = " << ba << endl;
        //  ^=:        XOR assignment:     variable ^= expr :
        bo ^= A;
        CHECK(bo==par22);
        //cout << "Bitwise XOR bo = " << bo << endl;

    }


   void sc_signed_ops_neg_consts_fns()
    {

    	sc_signed_ops_neg_consts (sc_int<10>(-5), sc_int<12>(-5), sc_uint<22>(1), sc_uint<30>(0), // A, B, A==B, A!=B
            sc_int<16>(-10), sc_int<14>(-5), sc_uint<22>(0), sc_uint<22>(1), // E, F, E>F, E<F
            sc_int<32>(-10), sc_int<64>(50), sc_int<64>(2), sc_int<64>(-15), // A+B, E*F,E/F, A+B+A
            sc_int<32>(-5),sc_int<32>(-5), sc_int<32>(0), sc_int<32>(4), // A & B, A | B, A ^ B, ~A
            sc_int<32>(1), sc_int<10>(1), sc_int<32>(0), sc_int<32>(-5), // A >=B, A<=B, A-B, (A | B) &=A
            sc_int<32>(-5), sc_int<10>(0) // (A & B) |= A, (A | B) ^ = A

                                  );

    	sc_signed_ops_neg_consts (
            sc_bigint<10>(-0), sc_bigint<12>(-5), sc_bigint<22>(0), sc_bigint<30>(1), // A, B, A==B, A!=B
            sc_bigint<16>(-0), sc_bigint<14>(-10), sc_bigint<22>(1), sc_bigint<22>(0), // E, F, E>F, E<F
            sc_bigint<32>(-5), sc_bigint<64>(0), sc_bigint<64>(0), sc_bigint<64>(-10), // A+B, E*F,E/F, A+B+A
            sc_bigint<32>(0),sc_bigint<32>(-5), sc_bigint<32>(-5), sc_bigint<32>(-1), // A & B, A | B, A ^ B, ~A
            sc_bigint<64>(1),sc_bigint<55>(0), sc_bigint<40>(5), sc_int<32>(0), // A >=B, A<=B, A-B, (A | B) &=A
            sc_int<32>(0), sc_int<10>(0) // (A & B) |= A, (A | B) ^ = A // A >=B, A<=B, A-B, (A | B) &=A
            );
    }
};

class B_top : public sc_module 
{
public:
    sc_signal<bool>  a{"a"};
    sc_signal<bool>  b{"b"};
    sc_signal<bool>  c{"c"};

    AA a_mod{"a_mod"};

    SC_CTOR(B_top) {
        a_mod.a(a);
        a_mod.b(b);
        a_mod.c(c);
    }
};

int sc_main(int argc, char* argv[])
{
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}

