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

// Constant literals, literal suffixes, negative literals and comparison with 
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
        SC_METHOD(literFromString); sensitive << s;

        SC_METHOD(literSignedWarning);
        sensitive << s;
        
        SC_METHOD(extra_signed_remove);
        sensitive << s;

        SC_METHOD(liter_overflow_oper64);
        sensitive << s << t;
        
        SC_METHOD(liter_extend_oper);
        sensitive << s << t;

        SC_METHOD(short_operations);
        sensitive << s;
        
        SC_METHOD(suffixes); sensitive << s;
    }

    #define CHECK(ARG) sct_assert(ARG); sct_assert_const(ARG);
    
    void literFromString() 
    {
        sc_uint<6> ux = "0b110011";
        ux = "42";
        cout << "42" << ux << endl;
        ux = sc_uint<5>("42");
        ux = "0";
        ux = "0x0";
        ux = "00";
        
        //ux += "42";  -- not supported in SC
        //bool b = ux == "42"; -- not supported in SC
        //ux = ux + "42"; // -- lead to string result

        sc_int<10> ix = "-11";
        ix = "-0x26";
        ix = "-0";
        ix = "-01";
        ix = "-0b1";

        sc_biguint<65> bu;// = "0x1F";
        bu = "0x1FFFF1111FFFF1111";
        bu = "36893225441449414929";

        sc_bigint<100> bi = "-0x1F";
        bi = "-0xAFFFF1111FFFF1111";
         
    }
    
// ---------------------------------------------------------------------------    

    void literSignedWarning() 
    {
        sc_uint<3> a;
        int i = 1;
        bool b;
        sc_uint<3> c = s.read();
        c = a | i;          // Warning signed/unsigned mixed    
        
        c |= i;             // Warning signed/unsigned mixed
        c |= 2;             // No warning for literal
        sc_int<3> d;
        d ^= c;             // Warning signed/unsigned mixed
    }
    
// ---------------------------------------------------------------------------    
    
    const sc_uint<16> C1 = 42;
    const unsigned C2 = 42;

    // Check clear sign cast for operation with non-negative literal
    void extra_signed_remove() 
    {
        sc_uint<8> e = 257;
        e += 1;
        CHECK(e == 2);
        e -= 1;
        e *= 1;
        e &= 1;
        e >>= 1;
        
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
        unsigned u = 1;
        u += c;
        //CHECK(u == 4294967292);
        
        int w = -42;
        w += u;
        //CHECK(w == -46);
        
        int ww = -42;
        unsigned uu = 5;
        ww += uu;
        CHECK(ww == -37);
        
        int j;
        j += -C1;
        CHECK(32725);
        j += -C2;
        CHECK(32683);
        //cout << "j " << j << endl;
    }
    
// ---------------------------------------------------------------------------    
        
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
        
        // Compound operators with mix of SC/CPP types
        c = 101;
        c += UI;
        CHECK(c == 521);
        c /= C;
        CHECK(c == 4);
        c = UI;
        c |= C;
        CHECK(c == 493);
        c = 42;
        c >>= 2;
        CHECK(c == 0xA);

        // Negative value    
        c = -101;
        c -= UI;
        CHECK(c == -521);
        c += UI;
        CHECK(c == -101);
        UI = 41;
        c += UI;
        CHECK(c == -60);
        
        sc_biguint<128> d = 71;
        d += c;
        CHECK(d == 11);
    }
    
// ---------------------------------------------------------------------------    
    
   template<class T1, class T2>
    void pos_compound() 
    {
        cout << "pos_compound" << endl;
        T1 a = 0x11;
        T2 b = 0x22;
        b += a;
        CHECK(b == 0x33);
        
        b -= a;
        CHECK(b == 0x22);
        
        b *= 2;
        CHECK(b == 0x44);

        b /= a;
        CHECK(b == 4);

        b <<= a-0xF;
        cout << "b " << hex <<b << endl;
        CHECK(b == 0x10);

        b += 2*a;
        CHECK(b == 0x32);
        
        b >>= 1;
        CHECK(b == 0x19);

        b ^= 0x10;
        CHECK(b == 0x9);
    }    
   
    template<class T1, class T2>
    void neg_compound() 
    {
        cout << "neg_compound" << endl;
        T1 a = -0x11;
        T2 b = -0x22;
        b += a;
        CHECK(b == -0x33);
        
        b -= a;
        CHECK(b == -0x22);
        
        b -= -0x11;
        CHECK(b == -0x11);

        b *= 2;
        CHECK(b == -0x22);

        b += -a;
        CHECK(b == -0x11);

        b /= -1;
        CHECK(b == 0x11);

        b <<= 2;
        CHECK(b == 0x44);

        b >>= 1;
        CHECK(b == 0x22);

        b &= 0x20;
        CHECK(b == 0x20);

        b |= 0x2;
        CHECK(b == 0x22);
    }
    
    template<class T1, class T2>
    void run_opers(bool isSigned = true) 
    {
        pos_compound<T1, T2>();
        if (isSigned) neg_compound<T1, T2>();
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

    void short_operations() {
        short l = 1;
        l += 1;
        CHECK (l == 2);
        
        l -= 3;
        CHECK (l == -1);

        l *= -31;
        CHECK (l == 31);

        char c = 23;
        c *= -2;
        CHECK (c == -46);

        c += 20;
        CHECK (c == -26);
    }
    
// ---------------------------------------------------------------------------    
    // Suffixes
    
    sc_signal<int> r0;
    void suffixes() {
        int i = 12;
        unsigned u = 12U;
        long li = 12L;
        unsigned long lu = 12UL;
        long long lli = 12LL;
        unsigned long long llu = 12ULL;

        i = ~0U;
        CHECK (i == 0xFFFFFFFF);
        u = ~0U;
        CHECK (u == 0xFFFFFFFF);
        li = ~0L;  CHECK (li == 0xFFFFFFFFFFFFFFFF);
        lu = ~0UL; CHECK (lu == 0xFFFFFFFFFFFFFFFF);
        lli = ~0L;  CHECK (lli == 0xFFFFFFFFFFFFFFFF);
        llu = ~0UL; CHECK (llu == 0xFFFFFFFFFFFFFFFF);
        
        i = -1000000000;  CHECK (i == -0x3B9ACA00); 
        u =  1000000000U; CHECK (u == 0x3B9ACA00);
        lu = 10000000000LU; CHECK (lu == 0x2540BE400);
        li = 10000000000L;  CHECK (li == 0x2540BE400);
        
        i = ~12;  CHECK (i == 0xFFFFFFF3); 
        u = ~12U; CHECK (u == 0xFFFFFFF3);
        lu = ~12LU; CHECK (lu == 0xFFFFFFFFFFFFFFF3);
        li = ~12L;  CHECK (li == 0xFFFFFFFFFFFFFFF3);
        llu = ~12LLU; CHECK (llu == 0xFFFFFFFFFFFFFFF3);
        lli = ~12LL;  CHECK (lli == 0xFFFFFFFFFFFFFFF3);
                
        i = 12 - 24;  CHECK (i == -12); 
        u = 12U - 6U; CHECK (u == 6);
        u = 12U - 24U; CHECK (u == 0xFFFFFFF4);
        lu = 12LU - 6LU; CHECK (lu == 6);
        lu = 12LU - 24LU; CHECK (lu == 0xFFFFFFFFFFFFFFF4);
        li = 12L - 24L;  CHECK (li == -12);
        
        r0 = i + u + li + lu;
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

