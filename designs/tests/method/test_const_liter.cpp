/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <sct_assert.h>
#include <sct_sel_type.h>
#include <systemc.h>
#include <cstdint>
#include <iostream>

// Constant literals, including negative literals and comparison with 
// narrow variable. 
// More tests with negative in unsigned variable are in 
// const_prop/test_const_negative.cpp
class A : public sc_module {
public:
    sc_in_clk clk;
    sc_signal<bool> rstn;
    
    SC_HAS_PROCESS(A);

    sc_signal<bool> r;
    sc_signal<sc_uint<3>> s;
    sc_signal<sc_int<3>> t;
    sc_signal<sc_biguint<66>> bs;
    sc_signal<sc_bigint<66>> bt;
    
    A(const sc_module_name& name) : sc_module(name) 
    {
        SC_METHOD(literSignedWarning);
        sensitive << s;
        
        SC_METHOD(boolAsSigned);
        sensitive << s << r;

        SC_METHOD(sc_shift_type_extension_channel);
        sensitive << c1 << c2 << c3 << c4;
        
        SC_METHOD(smemByte2Bit);
        sensitive << byte_enbl;
        
        SC_METHOD(extra_signed_remove);
        sensitive << r;

        SC_METHOD(signed_unsigned);
        sensitive << s << t;
        
        SC_METHOD(liter_trunc);
        sensitive << s << t;
        
        SC_METHOD(liter_extend_assign);  
        sensitive << s << t;
        
        SC_METHOD(liter_overflow_oper);
        sensitive << s << t;

        SC_METHOD(liter_overflow_oper64);
        sensitive << s << t;

        SC_METHOD(liter_extend_oper);
        sensitive << s << t;
    }

    
    #define CHECK(ARG) sct_assert(ARG); sct_assert_const(ARG);
    
// ---------------------------------------------------------------------------    

    void literSignedWarning() 
    {
        sc_uint<4> a;
        int i = 1;
        bool b = a & i;     // Warning signed/unsigned mixed
        b = a & 1;          // No warning for literal
        sc_uint<4> c = a & 3;
        c = a | i;          // Warning signed/unsigned mixed    
    }
    
    void boolAsSigned() 
    {
        sc_uint<4> a = s.read();
        bool b = r.read();
        sc_uint<4> c = a || b;  // No sign cast
        bool d = b && a == 1;
        d = a && b || c != 2;
    }
    
// ---------------------------------------------------------------------------    

    sc_signal<sc_uint<8> > c1{"c1"};
    sc_signal<sc_int<16> > c2{"c2"};
    sc_signal<sc_biguint<66> > c3{"c3"};
    sc_signal<sc_bigint<70> > c4{"c4"};
    
    void sc_shift_type_extension_channel() 
    {
        sc_uint<8> a;
        sc_biguint<66> x;
        
        a = (c1.read() + c2.read()) >> 8;
        a = (c1.read() * c2.read()) >> 8;
        x = (c3.read() * c4.read() - c2.read()) >> (c1.read() + c2.read());
    }    
    
// ---------------------------------------------------------------------------    

    static const unsigned DAT_WIDTH = 32;
    static const unsigned BYTE_NUMBER = 4;
    typedef sc_uint<DAT_WIDTH> Data_t;
    typedef sc_uint<4> Wstrb_t;

    Data_t byteEn2bitEn(const Wstrb_t& byteEn)
    {
        Data_t bitEn = 0; int i;
        for (int i = int(BYTE_NUMBER - 1); i >= 0; --i) {
            bitEn = (bitEn << 8) | (byteEn[i] ? 0xFF : 0x00);
        }
        return bitEn;
    }
    
    sc_signal<sc_uint<4>> byte_enbl;
    void smemByte2Bit()
    {
        Data_t biten = byteEn2bitEn(byte_enbl.read());
    }
    
// ---------------------------------------------------------------------------    
    
    static const unsigned PORT_NUM = 1;
    static const unsigned DATA_WIDTH = 8;
    const sc_uint<16> C1 = 42;
    const unsigned C2 = 41;
    const unsigned E = 1;
    bool arr[10];
    
    // Check clear sign cast for operation with non-negative literal
    void extra_signed_remove() 
    {
        unsigned short d = 257;
        unsigned e = (unsigned char)d + 1;
        CHECK(e == 2);
        e = d - 1;
        e = d * 1;
        e = d & 1;
        e = d >> 1;
        
        // Check clear sign cast with overflow
        sc_uint<5> ee;
        sc_uint<4> dd = 14;
        ee = dd *3;
        CHECK(ee == 10);
        ee = dd + 42;
        CHECK(ee == 24);
        ee = dd << 2;
        CHECK(ee == 24);
        ee = dd - 4;
        CHECK(ee == 10);
        
        sc_int<4> c = -5;
        unsigned uu = 1;
        unsigned u = uu + c;    
        CHECK(u == 4294967292);         // Signed, OK
        int ww = -42;
        int w = ww + u;
        CHECK(w == -46);
        
        int j;
        j = -C1;
        CHECK(j == -42);
        j = j + (-C2);
        CHECK(j == -83);
        j = j - C2;
        CHECK(j == -124);
        j--;
        CHECK(j == -125);
        
        unsigned jj;
        jj = C1;
        jj = jj + C1;
        jj = jj + (-10);
        CHECK(jj == 74);
        jj = -10 + jj;
        CHECK(jj == 64);
        jj = jj - C1;
        CHECK(jj == 22);
        
        sc_uint<16> a;
        for (int i = 0; i < 2; i++) {
            int k = a.range(i*DATA_WIDTH + 1, i*DATA_WIDTH);
            k = a.bit(i+DATA_WIDTH+PORT_NUM+1);
        }
        
        for (int i = 0; i < 2; i++) {
            bool k = arr[i + 1 + PORT_NUM];
        }
        
        bool b = r.read();
        if (b == r) {
            int k = 1;
        }
    }
    
// ----------------------------------------------------------------------------    
    
    static const unsigned U = 4;
    static const int I = -4;
    const sc_uint<4> SU = 4;
    const sc_int<4> SI = -4;
    
    // Negative signed variable binary operations with different types
    template<class T>
    void neg_sign_binary() 
    {
        T a = -5;
        
        int i = 1;
        unsigned u = 1;
        sc_uint<4> su = 2;
        sc_uint<4> si = -2;
        sc_biguint<4> bu = 3;
        sc_bigint<4> bi = -3;
        
        bool c;

        c = (a+1 == -4);
        CHECK(c);
        c = (a+sc_int<4>(1) == -4);
        CHECK(c);
        c = (a+sc_uint<4>(1) == -4);
        CHECK(c);
        
        c = (a+(sc_bigint<4>)1 == -4);
        CHECK(c);
        c = (a+(sc_biguint<4>)1 == -4);
        CHECK(c);

        c = (a+i == -4);
        CHECK(c);
        c = (a+u == -4);
        CHECK(c);
        c = (a+su == -3);
        CHECK(c);
        c = (a+bu == -2);
        CHECK(c);
        
        c = (a+U == -1);
        CHECK(c);
        c = (a+I == -9);
        CHECK(c);
        c = (a+SU == -1);
        CHECK(c);
        c = (a+SI == -9);
        CHECK(c);
        
    }
    
    // Assign negative value/signed expression to unsigned LHS
    void assign_sign_unsign() 
    {
        cout << "assign_sign_unsign" << endl;
        
        // Assign negative to unsigned
        unsigned ua = -1;
        CHECK(ua == 0xFFFFFFFF);
        sc_uint<4> sua = -2;
        CHECK(sua == 0xE);
        sc_biguint<4> bua = -3;
        CHECK(bua == 0xD);

        int i = 1;
        unsigned u = 1;
        sc_uint<4> su = 2;
        sc_int<4> si = -2;
        sc_biguint<4> bu = 3;
        sc_bigint<4> bi = -3;
        
        // Assign signed/negative expression to unsigned LHS
        bool c;
        u = i + I;
        CHECK(u == 0xFFFFFFFD);
        u = si + SI;
        CHECK(u == 0xFFFFFFFA);
        su = i + I;
        CHECK(su == 0xD);
        sc_uint<8> ssu = si + SI;
        cout << "ssu " << hex << ssu << endl;        
        CHECK(ssu == 0xFA);
        ssu = bi + 1;
        CHECK(ssu == 0xFE);
        bu = i + I;
        CHECK(bu == 0xD);
        sc_biguint<8> buu = si + SI;
        CHECK(buu == 0xFA);
        buu = bi + 1;
        cout << "buu " << hex << buu << endl;        
        CHECK(buu == 0xFE);
        
        // Operation where negative casted to unsigned
        u = 5;
        bu = u + I;
        CHECK(bu == 1);
        su = 4;
        u = su + I;
        CHECK(u == 0);
        
        // Operation where signed casted to unsigned by literal
        i = 1;
        bu = i + SU;
        CHECK(bu == 5);
        
        // Variables
        i = u;
        si = u;
        bi = u;
        si = su;
        bi = su;
        bi = bu;
    }

    // Signed/unsigned cast for variables
    void sign_unsign_var() 
    {
        cout << "sign_unsign_var" << endl;
        
        int i = 1;
        unsigned u = -1;
        sc_uint<8> su = 2;
        sc_int<8> si = -2;
        sc_biguint<8> bu = 3;
        sc_bigint<8> bi = -3;

        // Unsigned to signed assignment
        i = u;
        si = u;
        bi = u;
        si = su;
        bi = su;
        bi = bu;
        
        // Unsigned and signed operands
        i = -5;
        u = 4;
        si = -9;
        su = 10;
        auto la = i + u;        // unsigned    
        //cout << "la " << hex << la << endl;
        CHECK(la == 0xFFFFFFFF);
        
        sc_int<8> lb = si + i;       // signed       
        //cout << "lb " << lb << endl;
        CHECK(lb == -14);
        
        lb = si - i;
        CHECK(lb == -4);
        
        lb = i - si;
        CHECK(lb == 4);

        sc_int<16> lc = si + u;       // signed, signed cast for @u
        //cout << "lc " << lc << endl;
        CHECK(lc == -5);
        
        lc = si - u;   
        CHECK(lc == -13);
        
        lc = u - si;   
        CHECK(lc == 13);
        
        sc_int<64> ld = si + u;
        //cout << "ld " << ld << endl;
        CHECK(ld == -5);
        
        ld = si + si;
        CHECK(ld == -18);
        
        ld = si - (si + 1);
        CHECK(ld == -1);

        
        sc_uint<8> le = si + su;      // unsigned
        CHECK(le == 1);   
    }
     
    // Signed/unsigned cast for variables
    void sign_unsign_var2() 
    {
        cout << "sign_unsign_var" << endl;
        
        // Additional tests
        sc_uint<16> lf; sc_int<16> lfs;
        sc_int<8> si = -9; sc_uint<8> su = 10; 
        sc_int<8> sii = 10; sc_uint<8> suu = -9;
        int i = -9; 
        unsigned u = 10;
        
        sc_uint<35> llf = i + u;   // unsigned, OK
        //CHECK(llf == 1);     
        llf = i + 10;              
        CHECK(llf == 1);
        llf = i - 10;              
        cout << "llf " << llf << endl;
        CHECK(llf == 0x7FFFFFFed);
        
        lf = si + u;        // signed added for @u
        CHECK(lf == 1);

        lf = si + sii;      // signed
        CHECK(lf == 1);
        
        lf = su + suu;      // unsigned
        cout << "lf2 " << lf << endl;
        CHECK(lf == 257); 

        lf = si + su;       // unsigned, warning reported
        lfs= si + su;       // unsigned, warning reported
        cout << "lf3 " << lf << " " << lfs << endl;
        //CHECK(lf == 1);   
        //CHECK(lfs == 1);

        si = -9; su = 8;
        lf = si + su;       // unsigned, warning reported
        lfs= si + su;       // unsigned, warning reported
        cout << "lf4 " << lf << " " << lfs << endl;
        //CHECK(lf == 0xFFFF);          
        //CHECK(lfs == -1);

        lf = si + sii + su;    // unsigned, warning reported
        lfs = si + sii + su;   // unsigned, warning reported
        cout << "lf5 " << lf << " " << lfs << endl;
        //CHECK(lf == 9);
        //CHECK(lfs == 9);
    }
    
    // Bitwise operations with signed/unsigned variables
    void sign_unsign_bitwise_var_pos() 
    {
        cout << "sign_unsign_bitwise_var_pos" << endl;
        
        // Additional tests
        sc_uint<16> lf; sc_int<16> lfs;
        sc_int<8> si = 0x55; sc_uint<8> su = 0xCC; 
        sc_int<8> sii = 0xCC; sc_uint<8> suu = 0x55;
        int i = 0x55; 
        unsigned u = 0xCC;
        
        sc_uint<35> llf = i | u;   // unsigned, warning reported
        cout << "llf " << hex << llf << endl;
        CHECK(llf == 0xDD);      
        llf = i | 0xCC;              
        cout << "llf " << llf << endl;
        CHECK(llf == 0xDD);
        llf = i ^ 0xCC;              
        cout << "llf " << llf << endl;
        CHECK(llf == 0x99);
        
        lf = si | u;        
        cout << "lf0 " << lf << endl;
        CHECK(lf == 0xDD);

        lf = si | sii;      
        cout << "lf1 " << lf << endl;
        CHECK(lf == 0xFFDD);
        
        lf = su | suu;      
        cout << "lf2 " << lf << endl;
        CHECK(lf == 0xDD); 

        lf = si | su;       
        cout << "lf3 " << lf << endl;
        CHECK(lf == 0xDD);   

        sc_int<12> si2 = 0x555; su = 0xBB;
        lf = si2 | su;       
        cout << "lf4 " << lf << endl;
        CHECK(lf == 0x5FF);          

        lf = si2 ^ sii | su;    
        cout << "lf5 " << lf << endl;
        //CHECK(lf == 0xFABB);
    }
    
    void sign_unsign_bitwise_var_neg() 
    {
        cout << "sign_unsign_bitwise_var_neg" << endl;
        
        // Additional tests
        sc_uint<16> lf; sc_int<16> lfs;
        sc_int<8> si = -9; sc_uint<8> su = 10; 
        sc_int<8> sii = 10; sc_uint<8> suu = -9;
        int i = -9; 
        unsigned u = 10;
        
        sc_uint<35> llf = i | u;   // signed added
        cout << "llf " << hex << llf << endl;
        //CHECK(llf == 0xFFFFFFFF);     // Verilog 0x7FFFFFFFF  
        llf = i | 10;              
        cout << "llf " << llf << endl;
        CHECK(llf == 0x7FFFFFFFF);
        llf = i ^ 10;              
        cout << "llf " << llf << endl;
        CHECK(llf == 0x7FFFFFFFD);
        
        lf = si | u;        
        cout << "lf0 " << lf << endl;
        CHECK(lf == 0xFFFF);

        lf = si | sii;      
        cout << "lf1 " << lf << endl;
        //CHECK(lf == 0xFFFF);
        
        lf = su | suu;      
        cout << "lf2 " << lf << endl;
        //CHECK(lf == 0x00FF); 

        lf = si | su;       
        cout << "lf3 " << lf << endl;
        //CHECK(lf == 0xFFFF);   

        si = -9; su = 8;
        lf = si | su;       
        cout << "lf4 " << lf << endl;
        //CHECK(lf == 0xFFFF);          

        lf = si ^ sii | su;    
        cout << "lf5 " << lf << endl;
        //CHECK(lf == 0xFFFD);
    }
  
  
    void signed_unsigned() 
    {
        neg_sign_binary<int>();
        neg_sign_binary<sc_int<4>>();
        neg_sign_binary<sc_bigint<4>>();
        
        assign_sign_unsign();
        
        sign_unsign_var();
        sign_unsign_var2();
        
        sign_unsign_bitwise_var_pos();
        sign_unsign_bitwise_var_neg();
    }
    
// ---------------------------------------------------------------------------    
    
    void liter_trunc() 
    {
        sc_int<3> a1 = -0x11;
        sc_bigint<3> a2 = -0x11;
        cout << "a1 " << a1 << " a2 " << a2 << endl;
        sct_assert_const(a1 == -0x1);
        sct_assert_const(a2 == -0x1);

        signed char b = -0x111;
        int8_t c = -0x111;
        short d = -0x11111;
        int16_t e = -0x11111;
        int f = -0x111111111L;
        int32_t g = -0x111111111L;
        cout << "b " << (int)b << " c " << (int)c << " d " << d << endl;
        cout << "e " << e << " f " << f << " g " << g << endl;
        
        sct_assert_const(b == -0x11);
        sct_assert_const(c == -0x11);
        sct_assert_const(d == -0x1111);
        sct_assert_const(e == -0x1111);
        sct_assert_const(f == -0x11111111);
        sct_assert_const(g == -0x11111111);
    }
    
    void liter_extend_assign() 
    {
        sc_int<16> a1 = 0x11;
        sc_int<16> a2 = -0x11;
        sc_bigint<16> b1 = 0x11;
        sc_bigint<16> b2 = -0x11;
        cout << "a1 " << a1 << " a2 " << a2 << endl;
        cout << "b1 " << b1 << " b2 " << b2 << endl;

        CHECK(a1 == 0x11);
        CHECK(a2 == -0x11);
        CHECK(b1 == 0x11);
        CHECK(b2 == -0x11);
        
        sc_uint<16> u1 = -0x11;
        cout << "u1 " << hex << u1 << endl;
        CHECK(u1 == 0xFFEF);
        
        signed char b = -0x1;
        int8_t c = -0x1;
        short d = -0x11;
        int16_t e = -0x11;
        int f = -0x11;
        int32_t g = -0x11;
        cout << "b " << (int)b << " c " << (int)c << " d " << d << endl;
        cout << "e " << e << " f " << f << " g " << g << endl;
        
        CHECK(b == -0x1);
        CHECK(c == -0x1);
        CHECK(d == -0x11);
        CHECK(e == -0x11);
        CHECK(f == -0x11);
        CHECK(g == -0x11);
    }
    
    // Check MUL operation with more than 64bit result
    template<class T1, class T2, class T>
    void pos_overflow_mul(T1 val1, T2 val2, T mres) 
    {
        T1 a = val1;
        T2 b = val2;
        sc_biguint<128> c = a * b;
        cout << hex << "c " << c << " res " << mres << endl;
        CHECK(c == mres);
    }
    
    void liter_overflow_oper() 
    {
        pos_overflow_mul<sc_uint<2>, sc_uint<2>>(3, 3, 9);
        pos_overflow_mul<sc_biguint<2>, sc_biguint<2>>(3, 3, 9);
        pos_overflow_mul<sc_uint<2>, sc_biguint<2>>(3, 3, 9);
        
        pos_overflow_mul<sc_uint<32>, sc_uint<32>>(1ULL << 31, 2, 1ULL << 32);
        pos_overflow_mul<sc_biguint<32>, sc_biguint<32>>(1ULL << 31, 2, 1ULL << 32);
        pos_overflow_mul<sc_uint<32>, sc_biguint<32>>(1ULL << 31, 1ULL << 31, 1ULL << 62);
        
    }
    
    // Big int/uint operators with SC/CPP operands
    void liter_overflow_oper64() 
    {
        sc_biguint<65> A = 1;
        sc_biguint<70> B = 1;
        sc_bigint<128> RES = 1;

        // Operators with mix of SC/CPP types
        unsigned UI = 420;
        sc_biguint<16> C = 105;
        sc_bigint<128> c;
        c = C * UI;
        CHECK(c == 44100);
        c = C + UI;
        CHECK(c == 525);
        c = UI - C;         // signed added
        CHECK(c == 315);
        c = UI / C;
        CHECK(c == 4);
        c = UI % C;
        CHECK(c == 0);
        c = UI | C;
        CHECK(c == 493);
        c = C >> 3;
        CHECK(c == 13);
        c = C << 40;
        CHECK(c == 0x690000000000);
        
        A = A << 64; RES = RES << 65;
        pos_overflow_mul<sc_biguint<65>, sc_biguint<65>>(A, 2, RES);
        pos_overflow_mul<sc_biguint<66>, unsigned>(A, 2, RES);
        pos_overflow_mul<unsigned, sc_biguint<66>>(2, A, RES);
        
        A = 41; A = A << 58; 
        B = 11; B = B << 65;
        RES = 0x18; RES = RES << 120;
        pos_overflow_mul<sc_biguint<65>, sc_biguint<70>>(A, B, RES);
        
    }
    
// ---------------------------------------------------------------------------    
    
    // Using literals in operations
    template<class T1, class T2>
    void pos_add_sub() 
    {
        cout << "pos_add_sub" << endl;
        T1 a = 0x11;
        T2 b = 0x22;
        T2 c = b + a;
        sct_assert(c == 0x33);
        sct_assert_const(c == 0x33);
        
        T2 d = b - a;
        sct_assert(d == 0x11);
        sct_assert_const(d == 0x11);
        
        T2 e = b + (-0x11);
        sct_assert(e == 0x11);
        sct_assert_const(e == 0x11);

        T2 f = b - 0x11;
        sct_assert(f == 0x11);
        sct_assert_const(f == 0x11);

        T2 g = b - (-0x22);
        sct_assert(g == 0x44);
        sct_assert_const(g == 0x44);
    }

    template<class T1, class T2>
    void neg_add_sub() 
    {
        cout << "neg_add_sub" << endl;
        T1 a = -0x11;
        T2 b = -0x22;
        T2 c = b + a;
        sct_assert(c == -0x33);
        sct_assert_const(c == -0x33);
        
        T2 d = b - a;
        sct_assert(d == -0x11);
        sct_assert_const(d == -0x11);
        
        T2 e = b + (-0x11);
        sct_assert(e == -0x33);
        sct_assert_const(e == -0x33);

        T2 f = b - 0x11;
        sct_assert(f == -0x33);
        sct_assert_const(f == -0x33);

        T2 g = b - (-0x11);
        sct_assert(g == -0x11);
        sct_assert_const(g == -0x11);
    }
    
    template<class T1, class T2>
    void pos_shift() 
    {
        cout << "pos_shift" << endl;
        T1 a = 2;
        T2 b = 0x22;
        T2 c = b << a;
        sct_assert(c == 0x88);
        sct_assert_const(c == 0x88);
        
        T2 d = b >> a;
        sct_assert(d == 0x8);
        sct_assert_const(d == 0x8);
        
        T2 e = b << 0x3;
        sct_assert(e == 0x110);
        sct_assert_const(e == 0x110);

        T2 f = b >> 0x3;
        sct_assert(f == 0x4);
        sct_assert_const(f == 0x4);
    }
    
    template<class T1, class T2>
    void run_opers(bool isSigned = true) 
    {
        pos_add_sub<T1, T2>();
        pos_shift<T1, T2>();

        if (isSigned) neg_add_sub<T1, T2>();
        
        // Add other operators ...
    }
    
    void liter_extend_oper() 
    {
        run_opers<sc_bigint<8>, sc_bigint<16>>();
        
        run_opers<sc_uint<8>, sc_uint<16>>(false);
        run_opers<sc_uint<24>, sc_uint<42>>(false);
        run_opers<sc_biguint<11>, sc_biguint<33>>(false);
        run_opers<sc_biguint<37>, sc_biguint<66>>(false);
        run_opers<sc_biguint<65>, sc_biguint<173>>(false);
        
        run_opers<sc_int<8>, sc_int<16>>();
        run_opers<sc_int<33>, sc_int<64>>();
        run_opers<sc_bigint<64>, sc_bigint<127>>();
        run_opers<sc_bigint<45>, sc_bigint<132>>();
        
        run_opers<int, int>();
        run_opers<short, unsigned>(false);
        run_opers<int64_t, long int>();
        run_opers<int16_t, unsigned long long>(false);
        
        run_opers<int, sc_uint<16>>(false);
        run_opers<unsigned, sc_uint<30>>(false);
        run_opers<unsigned, sc_uint<63>>(false);
        run_opers<int, sc_int<64>>();

        run_opers<unsigned, sc_bigint<18>>(false);
        run_opers<int, sc_biguint<32>>(false);
        run_opers<int, sc_bigint<67>>();
        run_opers<unsigned, sc_biguint<68>>(false);
        run_opers<int, sc_bigint<100>>();
        run_opers<long int, sc_biguint<65>>(false);
        
        cout << "All test PASSED" << endl;
    }
    
// ---------------------------------------------------------------------------    

    void sign_unsign_var_bug() 
    {
        // Additional tests
        sc_int<16> lfs;
        int i = -9; 
        unsigned u = 10;
        
        sc_uint<35> llf = i + u;   // signed added
        CHECK(llf == 1);
        
        sc_int<4> a = -5;
        bool c = (a+SI == -9);
        CHECK(c);
    }
    
    
};

int sc_main(int argc, char *argv[]) 
{
    sc_clock clk{"clk", 1, SC_NS};
    A a_mod{"a_mod"};
    a_mod.clk(clk);
    sc_start();
    
    return 0;
}

