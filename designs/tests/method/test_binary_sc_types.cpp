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
        SC_METHOD(sc3_assign1); sensitive << uu;
        SC_METHOD(tildaUint); sensitive << uu;
        
        SC_METHOD(tildaUnsigned); sensitive << a << dummy << su;
        SC_METHOD(tildaUnsigned2); sensitive << a << dummy << su;
        SC_METHOD(tildaUnsigned2s); sensitive << a << dummy << su;
        
        SC_METHOD(tildaUnsignedT); sensitive << a << dummy;
        SC_METHOD(tildaSigned); sensitive << a << dummy;  // Warning reported
        SC_METHOD(tilda); sensitive << a << dummy;
        SC_METHOD(minusUnsigned); sensitive << a << dummy;
        SC_METHOD(sc_signed_ops_neg_consts_fns); sensitive << dummy;
//        SC_METHOD(tildaBv); sensitive << a << dummy; //Error reported
    }
    
    #define CHECK(ARG) sct_assert(ARG); sct_assert_const(ARG);
    
    static const unsigned STORED_RESP_WIDTH = 66;
    static const unsigned ACTIVE_TRANS_NUM = 1;
    static const unsigned ACTIVE_BEAT_NUM = 2;
    
    typedef sc_biguint<STORED_RESP_WIDTH> StoredResp_t;
    sc_signal<StoredResp_t>    sig;

    
    // Check SC integer types updated in Accellera repository
    void sc3_assign1() {
        sc_dt::sc_bigint<5> x = -1;
        sc_dt::sc_uint<5>   y;
        y = x;
        //cout << "x " << x << " y " << y << endl;
        sct_assert(y.xor_reduce() == 1);
        
        sc_int<5> x1 = -1;
        auto y1 = x1 << 4;
        sc_uint<5> z1 = x1 << 4;
        x1 <<= 4;
        //cout << "x1 " << x1 << " y1 " << y1 << " z1 " << z1 << endl;
        sct_assert(y1 != z1);
        
        sc_bigint<12> x2 = -17;
        sc_biguint<12> y2 = 42;
        auto z2 = x2 ^ y2;
        //cout << "x " << x2 << " y " << y2 << " z " << z2 << endl;
        sct_assert(z2 == -59);

        sc_biguint<12> x3 = 117;
        sc_biguint<12> y3 = 42;
        auto z3 = x3 % y3;
        //cout << "x " << x3 << " y " << y3 << " z " << z3 << endl;
        sct_assert(z3 == 33);
    }
    
    // Avoid type cast for same types in assignment LHS and RHS
    sc_signal<sc_uint<16>> uu;
    sc_signal<sc_uint<16>> tu;
    void tildaUint() {
        sc_uint<16> i = 42;
        sc_uint<16> j;
        j = ~i;
        j = ~uu.read();
        tu = ~uu.read();
        tu = ~j;

        // Initialization in declaration
        sc_uint<16> k = ~i;       // There is cast, that is OK, do not touch !!!

        tu.write(~k);             // signal write()
        tu.write(~uu.read());

        sc_uint<15> l1;
        sc_uint<16> l2;
        sc_uint<17> l3;
        l1 = ~i;
        l2 = ~i;
        l3 = ~i;
        cout << hex << "sc_uint<15, 16, 17> l1 " << l1 << " l2 " << l2 << " l3 " << l3 << endl;
        tu = l1+l2+l3;
    }
    

    sc_signal<sc_biguint<65>> t0;

    void tildaUnsignedT() {
        tildaUnsignedTT<unsigned long>(); 
        tildaUnsignedTT<sc_uint<64>>(); 
        tildaUnsignedTT<sc_biguint<65>>(); 
    }
    
    template<class T>
    void tildaUnsignedTT() {
        T i1; T i2; T i3;
        sc_uint<63> k = 42; sc_uint<64> j = 42; uint64_t i = 42; 
        unsigned mm = 42; unsigned long lm = 42; unsigned short sm = 42;

        i1 = ~k; i2 = ~j; i3 = ~i;
        cout << hex << "sc_uint<63>/sc_uint<64>/uint64_t: i1 " << i1 << " i2 " << i2 << " i3 " << i3  << endl;
        i1 = ~sm;  // implicitly converted to @int, but lets ignore that
        i2 = ~mm; i3 = ~lm;
        cout << hex << "short/unsigned/long: i1 " << i1 << " i2 " << i2 << " i3 " << i3 << dec << endl;

        t0 = i1+i2+i3;
    }

    sc_signal<unsigned> su;
    void tildaUnsigned() {
        sc_biguint<65> i1; sc_biguint<65> i2; sc_biguint<65> i3;
        sc_uint<4> h = 12; sc_biguint<4> bh = 12;
        sc_uint<62> l = 42; sc_biguint<62> bl = 42;
        sc_uint<63> k = 42; sc_biguint<63> bk = 42;
        sc_uint<64> j = 42; sc_biguint<64> bj = 42;
        sc_uint<64> i = 42; sc_biguint<65> bi = 42;
        sc_uint<64> n = 42; sc_biguint<66> bn = 42;

        i1 = ~h; i2 = ~bh;
        cout << hex << "4: i1 " << i1 << " i2 " << i2 << endl;
        i1 = ~l; i2 = ~bl;
        cout << hex << "62: i1 " << i1 << " i2 " << i2 << endl;
        i1 = ~k; i2 = ~bk;
        cout << hex << "63: i1 " << i1 << " i2 " << i2 << endl;
        i1 = ~j; i2 = ~bj;
        cout << hex << "64: i1 " << i1 << " i2 " << i2 << endl;
        i1 = ~i; i2 = ~bi;
        cout << hex << "65: i1 " << i1 << " i2 " << i2 << endl;
        i1 = ~n; i2 = ~bn;
        cout << hex << "66: i1 " << i1 << " i2 " << i2 << endl;

        i1 = (sc_biguint<65>)(~j);
        i2 = (sc_biguint<65>)(~bj);
        cout << hex << "i1 " << i1 << " i2 " << i2 << endl;

        i1 = a.read() ? (sc_biguint<65>)(~j) : (sc_biguint<65>)j;
        i2 = a.read() ? (sc_biguint<65>)(~bj) : (sc_biguint<65>)bj;
        cout << hex << "i1 " << i1 << " i2 " << i2 << dec << endl;

        // Tilda is not allowed in binary expressions
        unsigned ll; 
        i1 = ll - (~j);  // Warning reported
        i1 = (~j) + i2;  // Warning reported
        ll = ll - (~j);  // Warning reported
        ll += (~j);      // Warning reported

        // Check no tilda for same width assignment
        unsigned jj;
        sc_uint<33> ui; sc_uint<33> uj = 42;
        ll = ~jj;   // No cast for tilda
        ll = ~su;   // No cast for tilda
        ll = ~su.read();   // No cast for tilda
        
        t0 = i1+i2+i3+ll+ui;
    }
    
    sc_signal<unsigned> su2;
    void tildaUnsigned2() {
        sc_uint<33> i1; sc_uint<33> i2; sc_uint<33> i3;
        sc_uint<31> l = 42; 
        sc_uint<32> k = 42; 
        sc_uint<33> j = 42; 

        i1 = ~l; 
        i2 = ~k; 
        i3 = ~j; 
        cout << hex << "i1 " << i1 << " i2 " << i2 << " i3 " << i3 << endl;
        
        i1 = (sc_uint<33>)(~l);
        i2 = (sc_uint<33>)(~k);
        i3 = (sc_uint<33>)(~j);
        cout << hex << "i1 " << i1 << " i2 " << i2 << " i3 " << i3 << endl;
        
        su2 = i1+i2+i3;
    }
    
    sc_signal<unsigned> su2s;
    void tildaUnsigned2s() {
        sc_int<33> i1; sc_int<33> i2; sc_int<33> i3;
        sc_uint<31> l = 42; 
        sc_uint<32> k = 42; 
        sc_uint<33> j = 42; 

        i1 = ~l; 
        i2 = ~k; 
        i3 = ~j; 
        cout << hex << "i1 " << i1 << " i2 " << i2 << " i3 " << i3 << endl;
        
        i1 = (sc_int<33>)(~l);
        i2 = (sc_int<33>)(~k);
        i3 = (sc_int<33>)(~j);
        cout << hex << "i1 " << i1 << " i2 " << i2 << " i3 " << i3 << endl;
        
        su2s = i1+i2+i3;
    }

    void tildaBv() {
        sc_biguint<65> i1; sc_biguint<65> i2; sc_biguint<65> i3;
        sc_bv<63> l = 42;
        sc_bv<64> k = 42;
        sc_bv<65> j = 42;
        i1 = ~l; i2 = ~k; i3 = ~j;        
        cout << hex << "i1 " << i1 << " i2 " << i2 << " i3 " << i3 << dec << endl;
        t0 = i1+i2+i3;
    }
    
    sc_signal<sc_bigint<65>> t1;
    void tildaSigned() {
        sc_bigint<67> i1;
        sc_bigint<67> i2;
        sc_int<64> j = 42; sc_bigint<64> bj = 42;
        
        i1 = ~j; i2 = ~bj;  // Warnong reported
        cout << hex << "64: i1 " << i1 << " i2 " << i2 << endl;

        t0 = i1;
        t0 = i2;
    }
    
    sc_signal<sc_bigint<65>> t2;
    void tilda() {
        sc_int<10> A = 0;
        sc_bigint<64> bn = ~A;       
        
        sc_uint<16> ad = 0xABCD;
        sc_uint<16> wd = 0x0;
        wd.bit(0) = ~ad.bit(8);
        wd.bit(1) = ~ad.bit(9);
        wd.bit(2) = ~ad.bit(10);
        wd.bit(3) = ~ad.bit(11);
        t2 = sc_bigint<65>(wd);
    }
    
    
    sc_signal<sc_biguint<65>> t3;
    void minusUnsigned() {
        sc_bigint<65> i1;
        sc_bigint<65> i2;
        sc_uint<62> l = 42; sc_biguint<62> bl = 42;
        sc_uint<63> k = 42; sc_biguint<63> bk = 42;
        sc_uint<64> j = 42; sc_biguint<64> bj = 42;
        sc_uint<64> i = 42; sc_biguint<65> bi = 42;
        sc_uint<64> n = 42; sc_biguint<66> bn = 42;
        
        i1 = -l; i2 = -bl;
        cout << hex << "62: i1 " << i1 << " i2 " << i2 << dec << " " << i2 << endl;
        i1 = -k; i2 = -bk;
        cout << hex << "63: i1 " << i1 << " i2 " << i2 << dec << " " << i2 << endl;
        i1 = -j; i2 = -bj;
        cout << hex << "64: i1 " << i1 << " i2 " << i2 << dec << " " << i2 << endl;
        i1 = -i; i2 = -bi;
        cout << hex << "65: i1 " << i1 << " i2 " << i2 << dec << " " << i2 << endl;
        i1 = -n; i2 = -bn;
        cout << hex << "66: i1 " << i1 << " i2 " << i2 << dec << " " << i2 << endl;

        t0 = i1;
        t0 = i2;
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

