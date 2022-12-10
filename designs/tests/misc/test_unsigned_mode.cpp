/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"
#include "sct_assert.h"
#include "stdint.h"


// Unsigned mode tests
class A : public sc_module 
{
public:
    sc_in<bool>         a{"a"};
    sc_out<bool>        b{"b"};
    sc_out<bool>        c{"c"};
    
    sc_signal<sc_uint<4>> s;

    SC_CTOR(A)
    {
        for (int i = 0; i < 3; ++i) {
            m[i] = new sc_signal<sc_uint<4>>("m");
        }
                
        SC_METHOD(binary); sensitive << s;
        SC_METHOD(biguint); sensitive << s;
        SC_METHOD(c99types); sensitive << s;
        
        SC_METHOD(warnings); sensitive << s;
        SC_METHOD(others); sensitive << s << *m[0] << sb;
        
    }
    
    #define CHECK(ARG) sct_assert(ARG); sct_assert_const(ARG);
    
    void binary() 
    {
        bool b = true;
        sc_uint<16> x = 14;
        unsigned u = 12; unsigned u1 = 13; unsigned u2 = 14;
        sc_biguint<15> bx = 13;
        unsigned long res;

        unsigned uu = -1;
        uu = (int)42;
        sc_int<16> y = (sc_int<16>)42;      // warning
        y = (sc_int<10>)42;
        y = (sc_int<10>)(-42);
        
        res = u1 - u;
        res = -u + u1;
        
        res = x - u;
        res = bx.to_uint() - u;
        res = bx.to_uint() + x;
        
        res = u - 1;
        res = u + (-1U);
        res = u + (-1);      
        res += -1;      
        
        res = u - u1 + u2;
        CHECK (res == 13);
        res = u - x + 3;
        CHECK (res == 1);
        res = 2*(u - x) + u1;
        CHECK (res == 9);
    }
    
    enum E1 {EA = 31, EB = 32};
    enum class E2 {EC = 42, ED = 44};
    
    void biguint() {
        unsigned u = 11;
        sc_uint<16> x = 12;             
        sc_biguint<65> bx = 13;   
        unsigned long res;
        sc_biguint<65> bres;
        
        bres = bx - 1;
        bres = 4 / (bx - 15) + x;       // 'signed required to provide correct result
        CHECK (bres == 10);
        
        bres = EA + u;
        // Fix me #282
        bres = u - sc_uint<12>(int(E2::EC));   
        bres = u - unsigned(E2::EC);   
    }
    
    void c99types() {
        uint8_t a = 1;
        uint16_t b;
        uint32_t c;
        uint64_t d;
        int32_t z;                      // warning      
        
        int i = 10;                     // warning      
        unsigned u = 11;
        sc_uint<16> x = 12;             
        sc_biguint<65> bx = 13;             
        
        a = x - 1;
        b = x + bx.to_uint();
        c = bx.to_uint() - 1;
        d = i + u;                      // warning      
        
        a = b - c;
        c = a - b;                      // false warning      
    }

    void warnings() 
    {
        int i = 10;                     // warning  
        sc_uint<16> x = 14;
        unsigned u = 12;
        sc_int<16> y = 11;              // warning  
        sc_biguint<15> bx = 13;
        sc_bigint<33> by = 12;          // warning  
        sc_bigint<33> bres;             
        long res;                       // warning  
        
        res = -1;
        res = -u;                       
        res = i + u;                    // warning
        res = i + x;                    // warning
        res = y + x;                    // warning
        res = y + u;                    // warning
        res *= y;                       // warning
        bres = 1;
        bres = -u;                      

        i = (int)u;                     
    }
    
    sc_signal<sc_uint<4>> *m[3] = {};
    sc_signal<bool> sb{"sb"};
    
    void others() 
    {   
        sc_int<16> y = 14;              // warning    
        sc_biguint<15> bx = 13;
        unsigned u = 12;
        
        // #1
        sc_uint<4> w = s.read();
        sc_uint<10> x = (1 << w) + bx;  // 2x warning, OK
        
        // #2
        bool a = s.read() > 2;
        bool b = s.read() > 3;
        bool c; 
        c = a == b;
        c = a || b;
        c = y == u;                     // warning, OK
        
        // #3
        bool d = this->m[0] ? this->m[0]->read().bit(3) : 0;
        
        // #4
        sc_uint<1> e = 0;
        e = a;
        e = e || sb;
        
        // #5
        a = a | (s.read() > 1);     
        a |= b | c;                     // 2x warning, OK
        
        // #6
        sc_uint<1> t;
        t = b ? 0 : 1;
        a = b ? 0 : 1;
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

