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

// Shift with LHS/RHS type extension
class A : public sc_module 
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

    SC_CTOR(A) {
        pa = sc_new<sc_uint<8>>();
        for (int i = 0; i < 3; i++) {
            arrp[i] = sc_new<sc_uint<4>>();
            arrsigp[i] = new sc_signal<sc_uint<4>>("arrsigp");
            vecsig2d[i].init(2);
        }
        
        SC_METHOD(sc_shift_type_extension_array_binary); 
        sensitive << s << vecsig2d[0][0] << arrsig2d[0][0] << vecsig[0]
                  << *arrsigp[0] << arrsig[0];
        SC_METHOD(sc_shift_type_extension_binary); sensitive << s;
        SC_METHOD(sc_shift_type_extension_big_binary); sensitive << s;
        SC_METHOD(sc_shift_type_extension_unary); sensitive << s;
        SC_METHOD(sc_shift_type_extension_channel); 
        sensitive << c1 << c2 << c3 << c4;
        SC_METHOD(sc_shift_type_extension_ref_ptr); sensitive << s;
        SC_METHOD(sc_shift_type_extension_part_select); sensitive << s;
                
        SC_METHOD(sc_shift_type_extension_cast); sensitive << s;

        SC_METHOD(sc_shift_type_extension_cond); sensitive << s;
        SC_METHOD(sc_shift_type_extension_concat); sensitive << s;
        SC_METHOD(sc_shift_type_extension_comma); sensitive << s;
        
        SC_METHOD(sc_shift_type_extension_fcall); sensitive << s;
        
        SC_METHOD(sc_shift_type_extension); sensitive << s;
        SC_METHOD(cpp_shift_type_extension); sensitive << s;
        SC_METHOD(chan_shift_type_extension); sensitive << ch0 << ch1 << ch2;
        SC_METHOD(div_type_extension); sensitive << s;
        SC_METHOD(compound_type_extension); sensitive << s;
    }
    
    static const unsigned STORED_RESP_WIDTH = 66;
    static const unsigned ACTIVE_TRANS_NUM = 1;
    static const unsigned ACTIVE_BEAT_NUM = 2;
    
    typedef sc_biguint<STORED_RESP_WIDTH> StoredResp_t;
    sc_signal<StoredResp_t>    sig;
    
    // BUG in real design -- fixed
    void syncProc() 
    {
        sig = (StoredResp_t(1) << STORED_RESP_WIDTH-1);
        wait();
        
        while (true) {
            wait();
        }
    }
    
    // Type extension with arrays and channel arrays
    sc_uint<4> arr[3];
    sc_uint<4>* arrp[3];
    sc_uint<4> arr2d[3][2];
    sc_signal<sc_uint<4>> arrsig[3];
    sc_signal<sc_uint<4>>* arrsigp[3];
    sc_signal<sc_uint<4>> arrsig2d[3][2];
    sc_vector<sc_signal<sc_uint<4>>> vecsig{"vecsig", 3};
    sc_vector<sc_vector<sc_signal<sc_uint<4>>>> vecsig2d{"vecsig2d", 3};
    
    void sc_shift_type_extension_array_binary() 
    {
        int i = s.read();
        unsigned j = s.read();
        sc_uint<16> x = s.read();
                
        // Left shift in LHS
        sc_uint<8> la;
        la = (arr[i] << 1) >> 8;
        la = (*arrp[i] << 1) >> 8;
        la = (arr2d[i][j] << 1) >> 8;
        la = (arrsig[i].read() << 1) >> 8;
        la = (arrsigp[i]->read() << 1) >> 8;
        la = (arrsig2d[i][j].read() << 1) >> 8;
        la = (vecsig[i].read() << 1) >> 8;
        la = (vecsig2d[i][j].read() << 1) >> 8;
        
        // Multiplication in LHS
        la = (arr[i] * *arrp[i]) >> 8;
        la = (arrsig[i].read() * arrsigp[i]->read()) >> 8;
        la = (arrsig2d[i][j].read() * vecsig[i].read()) >> 8;
        la = (vecsig2d[i][j].read() * 2) >> 8;
        
        // Addition in LHS
        la = (arr[i] - *arrp[i]) >> 8;
        la = (arrsig[i].read() + arrsigp[i]->read()) >> 8;
        la = (arrsig2d[i][j].read() - vecsig[i].read() + 1) >> 8;
        la = (vecsig2d[i][j].read() + 2) >> 8;
        
        // Various in RHS
        la = 1 << (*arrp[i] << 1);
        la = 1 >> (arr[i] - *arrp[i]);
        la = 2 << (arrsig[i].read() + arrsigp[i]->read());
        
        // Unknown width
        la = (arrsig2d[i][j].read() << j) >> 4;  // 64'() >> 4
        la = (arrsig2d[i][j].read() << x) >> 4;  // 64'() >> 4

        la = 2 << (vecsig[i].read() << arrsigp[i]->read()); // 2 << 19`()
        la = 2 << (arrsig2d[i][j].read() << i); // 2 << 32'()
        
        // No extension
        la = 2 << (vecsig[i].read() / vecsig2d[i][j].read());
    }
    
    // With binary operations
    void sc_shift_type_extension_binary() 
    {
        sc_uint<8> a, b, c, d = 1;
        sc_uint<3> e;
        const unsigned K = 42;

        // Left shift in LHS -- OK
        a = (b << 1) >> 8;
        a = (b << 2) >> 8;
        a = (b << 5) >> 8;
        a = (b << c) >> 8;
        a = (b << e) >> 8;

        // Multiplication in LHS
        a = (b*c) >> 8;
        a = ((sc_uint<16>)(b*c)) >> 8;
        a = (b*K) >> 8;
        a = (b*42) >> 8;
        
        // Addition in LHS
        a = (b+c) >> 8;
        a = (b-c) >> 8;
        a = ((sc_uint<9>)(b+c)) >> 8;
        a = (b+K) >> 8;
        a = (b+42) >> 8;
        a = (b-42) >> 8;
        a = (1024+b) >> 8;
        
        a = ((b * c) * c) >> 8;

        // Others in LHS, no type extension
        a = (b | c) >> 8;
        a = (b ^ 1024) >> 8;
        a = (b / 1024) >> 8;
        a = (b >> 42) >> 8;
        
        // Unknown width
        int j = s.read();
        a = (b << j) >> 4;  // 64'()
        a = (j << b) >> 4;  // 32'()
        
    }
    
    // With binary operations
    void sc_shift_type_extension_big_binary() 
    {
        const unsigned long long M = 1ULL << 50;
        sc_biguint<66> x, y, z, u = 1;
        x = (y*z) >> 8;
        x = (z + y) / 8;
        x = (z - y) % 11;
        x = ((sc_biguint<131>)(y*z)) >> 8;
        x = ((sc_biguint<133>)(y*z)) >> 8;

        x = (y*1000000000000ULL) >> 8;
        x = (y*M) >> 8;
        x = (y - 1000000000000ULL) >> 8;
        x = ((M << 20) + y) >> 8;

        x = ((y << 20)) >> 8;
        //x = ((y << x)) >> 8;  -- error as @width is more than 64
        x = (sc_uint<42>(y << 20)) >> 8;
        x = (sc_uint<42>(y << x)) >> 8;
        
        x = ((z + y) * 2) >> 8; 
    }
    
    // With unary operations
    void sc_shift_type_extension_unary() 
    {
        sc_uint<8> a, b, c, d = 1;
        sc_uint<3> e;
        const unsigned K = 42;
        const unsigned long long M = 1ULL << 50;        // Warning reported
        sc_biguint<66> x, y, z, u = 1;

        // Left shift in LHS -- OK
        a = (b * (-42)) >> 8;
        a = (b * (-42) + 1) >> 8;
        a = ((-42) - b - c) >> 8;
        a = ((++b) * (c--)) >> 8;

        a = ((b || c) | c) >> 8;
        a = (~b + 1) >> 8;
        a = (!b + 1) >> 8;
        
        x = (z ^ y) / 42;
        x = (z++ - y) / 42;
    }
    
    sc_signal<sc_uint<8> > c1{"c1"};
    sc_signal<sc_int<16> > c2{"c2"};
    sc_signal<sc_biguint<66> > c3{"c3"};
    sc_signal<sc_bigint<70> > c4{"c4"};
    
    void sc_shift_type_extension_channel() 
    {
        sc_uint<8> a;
        sc_biguint<66> x;
        
        a = (c1.read() + c2.read()) >> 8;
        a = (c1.read() * c2.read()) >> 8;   // Warning reported
        x = (c3.read() * c4.read() - c2.read()) >> (c1.read() + c2.read()); // Warning reported
    }
    
    // References and pointers
    sc_uint<8>* pa;
    void sc_shift_type_extension_ref_ptr() 
    {
        sc_uint<8> a, b;
        sc_uint<16> c;
        sc_uint<8>& r1 = b;
        sc_uint<16>& r2 = c;
        
        a = (r1 * c) >> 8;
        a = (b - r2) >> 8;
        a = (r1 + r2) >> 8;
        a = (*pa + 42) >> 8;

        sc_uint<8>* pb = pa;
        a = (*pb + 42) >> 8;
        a = (r2 * (*pb)) >> (*pa+1);
    }
    
    void sc_shift_type_extension_part_select()
    {
        sc_uint<8> a, b;
        sc_uint<16> c;
        
        a = (b.range(4,1) + 1) >> 8;
        a = (c.bit(3) * b) >> 8;
        
        int i;
        a = (c.range(i*8 + 5, i*8) + 1) >> 8;
        
    }
    
    void sc_shift_type_extension_cast()
    {
        sc_uint<8> a, b;
        sc_uint<16> c;
        
        a = (sc_uint<12>(b) + 1) >> 8;
        a = (sc_uint<10>(sc_uint<12>(b)) + 1) >> 8;
        a = (sc_uint<12>(sc_uint<10>(b)) + 1) >> 8;
        a = (sc_uint<20>(b*c)) >> 8;
    }
    
    void sc_shift_type_extension_cond()
    {
        bool cond;
        sc_uint<8> a;
        sc_uint<16> c, b;
        
        a = ((cond ? b : c) + 1) >> 8;
        a = ((cond ? (b*c) : (c*b)) + 1) >> 8;
    }
    
    void sc_shift_type_extension_concat()
    {
        bool cond;
        int i;
        unsigned u;
        sc_uint<8> a, b;
        sc_uint<16> c;
        
        a = (b, c) + 3;         // signed, OK
        a = (b, c) * 3;         // signed, OK
        a = (b, c) * 3U;
        a = (b, c) * i;         // signed, OK
        a = (b, c) * u;
        a = (b, c) * a;
        a = ((b, c) * 3) >> 8;  // signed, OK
        a = ((b, sc_uint<6>(42)) * c) >> 8;     
    }

    void sc_shift_type_extension_comma()
    {
        sc_int<8> a;
        int b, c;
        
        a = (b++, c) >> 8;
        a = ((b++, c) * 3) >> 8;
        a = (b++, c * 5) >> 8;   
        a = (b++, c+1) >> 8;   
        a = ((sc_int<5>)(b++, c)) >> 8;   
    }
    
    sc_uint<17> h(sc_uint<16> i) {
        return ++i;
    }
    
    void sc_shift_type_extension_fcall()
    {
        bool cond;
        sc_uint<8> a;
        sc_uint<16> c, b;
        
        a = (h(b)+1) >> 8;
    }

    // Shifts with expression in left/right operand, 
    // no overflow should be there 
    void sc_shift_type_extension() {
        sc_uint<8> a = 3;
        sc_uint<8> b, c;
        a = b >> 8;
        a = 4 >> c;
        a = (b*c) >> 8;
        b = (c+a) << a;
        b = (c-a) << (b*a+1);
        
        sc_bigint<41> u;
        sc_biguint<67> w;
        w = u >> 8;
        w = u << a;
        a = (w + u) >> (w * u);
        a = (w + u) >> 2;
        w = w << (w - u);
        
        u = (w*a) << 10;
        b = (c - u) >> a;
    }
    
    void cpp_shift_type_extension() {
        int a = 3;
        unsigned b, c;
        a = b >> 8;
        a = 4 >> c;
        a = (b*c) >> 8;
        b = (c+a) << a;
        b = (c-a) >> (b*a+1);
        
        unsigned long l1, l2;
        l1 = (b*c) >> 8;
        l2 = (l1 * 3) << 3;
        
        sc_uint<4> e;
        a = (e*b) << 3;
        a = (e*b) >> 3;
        a = l1 >> (c-e);
    }
    
    sc_signal<unsigned> ch0;
    sc_signal<sc_uint<14>> ch1;
    sc_signal<sc_bigint<77>> ch2;
    
    void chan_shift_type_extension() {
        int a = 3;
        unsigned b = 4;
        a = (ch0.read() * 2) >> 8;
        a = 4 >> (b * ch1.read());
        sc_bigint<77> d = (ch1.read() + 1) >> a;
        d = (ch1.read() * 2) >> a;
    }
    
    void div_type_extension() {
        int f = 3;
        unsigned g;
        long h;
        sc_int<5> x;
        sc_biguint<44> y;
        sc_bigint<66> z;
        
        x = f / int(g);
        y = (f + 1) / (int(g) + 1);
        y = (f + 1) % (int(g) + 1);
        
        h = (h * f) / 2;
        z = (h * f) % y;
        
        y = (h - x) / 3;
        y = (y / h) % 3;
        
        y = (y * h) % 3;
    }

    void compound_type_extension() {
        int f = 3;
        unsigned g;
        long h;
        sc_uint<5> x;
        sc_biguint<44> y;
        sc_bigint<66> z;
        
        x = x >> (f + 1);
        x >>= (f + 1);
        
        y >>= (x * 3);
        y >>= (x);
        
        z <<= (y << 1);
        z <<= (y / 2);
        
        z <<= 42;
    }
    
};

class B_top : public sc_module 
{
public:
    sc_signal<bool>  a{"a"};
    sc_signal<bool>  b{"b"};
    sc_signal<bool>  c{"c"};

    A a_mod{"a_mod"};

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

