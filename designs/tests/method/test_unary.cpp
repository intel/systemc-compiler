/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"
#include "sct_assert.h"


// Unary operations for C++ and SC types
class A : public sc_module 
{
public:
    sc_signal<int>        a{"a"};
    sc_signal<int>        b{"b"};
    sc_signal<int>        c{"c"};
    sc_signal<unsigned>   d{"d"};
    
    int                   m = 11;
    int                   k = 11;

    sc_signal<bool> dummy{"dummy"};

    SC_CTOR(A)
    {
        SC_METHOD(narrowCastNeg); sensitive << dummy;
        SC_METHOD(negativeLiterals); sensitive << dummy;
        
        SC_METHOD(minus1); sensitive << dummy;
        SC_METHOD(increment1); sensitive << dummy;
        SC_METHOD(increment2); sensitive << a;
        SC_METHOD(plus1); sensitive << a;
        SC_METHOD(not1); sensitive << a << b;
        SC_METHOD(unary_in_subscript); sensitive << a << b;
        SC_METHOD(unary_bug); sensitive << a << b;
    }
    
    #define CHECK(ARG) sct_assert(ARG); sct_assert_const(ARG);
    
    static const int NEG = -2;
    static const long int NEGL = -211;

    void narrowCastNeg()
    {
        // -29
        sc_int<8> k1 = -541;
        cout << "k1 " << k1 << endl;
        CHECK(k1 == -29);
        
        int ki = (-1L << 32) - 123;
        CHECK(ki == -123);

        // -48
        sc_bigint<8> k2 = (sc_int<8>)NEGL * 14 + NEGL + (sc_bigint<16>)NEGL;
        cout << "k2 " << k2 << endl;
        CHECK(k2 == -48);
    }
    
    void negativeLiterals()
    {
        int i = -1;
        sc_int<4> x = sc_int<3>(-2);
        sc_bigint<8> y = -3;
        i = -4 * (-5);
        i = NEG;
        i = (sc_int<12>)NEG;
        i = (sc_int<4>)NEG - 2*NEG;
        cout << "i " << i << endl;
        CHECK(i == 2);
        
        sc_bigint<8> j;
        j = (sc_bigint<8>)NEGL;
        cout << "j " << j << endl;
        CHECK(j == 45);
    }
    
    void minus1()
    {
        int i = -111;
        int j = -i;
        CHECK(j == 111);
        j = -(-i) + 1;
        CHECK(j == -110);
        
        unsigned u = -i;
        CHECK(u == 111);
        u = (-i) * 2;
        CHECK(u == 222);

        sc_int<9> x = -NEGL;
        CHECK(x == 211);
        x = NEGL;
        CHECK(x == -211);
        x = 1 + (-NEGL);
        CHECK(x == 212);
        x = (-NEGL+1) / 2;
        CHECK(x ==106);

        sc_bigint<16> z = NEGL;
        CHECK(z == -211);
        z = -NEGL;
        CHECK(z == 211);
        
        sc_bigint<44> zz;
        zz = -z - -x;
        cout << "zz " << zz << endl;
        CHECK(zz == -105);
    }
    
    // Increment/decrement variable with known value
    void increment1() {
        int i = 1;
        unsigned j = 2;
        
        i--;
        j++;
        int k1 = ++i;
        unsigned k2 = --j;
        
        cout << "k1 " << k1 << endl;
        CHECK(k1 == 1);
        cout << "k2 " << k2 << endl;
        CHECK(k2 == 2);
        
        sc_int<9> x = NEGL;
        x++; CHECK(x == -210);
        ++x; CHECK(x == -209);
        x--; CHECK(x == -210);
        --x; CHECK(x == -211);
        
        sc_uint<17> ux = -NEGL;
        ux++; CHECK(ux == 212);
        ++ux; CHECK(ux == 213);
        ux--; CHECK(ux == 212);
        --ux; CHECK(ux == 211);
        
        sc_bigint<18> z = (ux++) - (x--);       // Warning reported
        cout << "z " << z << endl;
        //CHECK(z == 422);
        
        z = (--j) * 2 + (++x) / (i--);
        cout << "z " << z << endl;
        CHECK(z == -209); 
    }
    
    // Increment/decrement with unknown value
    void increment2() {
        int i = a.read();
        unsigned j = a.read();
        
        i--;
        j++;
        int k1 = ++i;
        unsigned k2 = --j;
        
        b.write(k1);
        b.write(k2);
        
        b.write(i++);
        b.write(--j);
    }    
    
    // Plus
    void plus1() {
        int i = -a.read();
        unsigned j = +a.read();
        
        int k1 = i + (-j);          // Warning reported
        unsigned k2 = (+j) + i;     // Warning reported
        
        c.write(-k1);
        c.write(+k2);
        
        i = +NEGL;
        CHECK(i == -211);
        
        sc_int<9> x = +21;
        CHECK(x == 21);
        sc_bigint<18> z = +x;
        CHECK(x == 21);
        
        z = +x - +i;
        cout << "z " << z << endl;
        CHECK(z == 232);
        z = (+i) * (+x);
        cout << "z " << z << endl;
        CHECK(z == -4431);
    }        
    
    // Not and logic not
    void not1() {
        bool l1 = a.read() == b.read();
        bool l2 = !l1;
        bool l3 = l2 || !(k == m);
        bool l4 = !(l2 && !l3);
        k = l4 || !l3;
        l2 = !l2;
  
        sc_uint<1> ll1 = 1;
        sc_uint<1> ll2 = ~ll1;
        d.write(!l2 + ~ll2);

        unsigned x = 43;
        unsigned y = ~x;
        unsigned z = ~y;
        CHECK(z == 43);
        
        z = (!ll2) ? (~y) - 1 : ~z;
        z = ~(x++) + -(~y);
        
        sc_uint<8> t = a.read();
        z = (~a) - (~(~t));         // Warning reported
    }            
    
    // Unary operation for array index
    void unary_in_subscript() 
    {
        sc_uint<4> arr1[4] = {1,2,3,4};
        bool arr2[16] = {1,0,1};
        int arr3[4];
        
        int i = 0; unsigned j = 1;
        sc_uint<4> x = arr1[++i];
        CHECK(x == 2);
        x = arr1[-i + 4];
        CHECK(x == 4);
        x = arr1[i-- + ++j];
        CHECK(x == 4 && j == 2);

        bool bb = arr2[+j];
        CHECK(bb == 1);
        bb = arr2[--j];
        CHECK(bb == 0);
        
        arr3[a.read()] = 1;
        bb = arr3[0];
    }

    
    void unary_bug() 
    {
        int arr1[4] = {1,2,3,4};
        int i = 0; 
        int j = arr1[++i];
        sct_assert_const(i == 1);
    }
};


int sc_main(int argc, char* argv[])
{
    A a_mod{"a_mod"};
    sc_start();
    return 0;
}

