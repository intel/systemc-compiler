/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>
#include <sct_assert.h>

// Assignment, binary, unary and compound operations for negative literals cases
SC_MODULE(top)
{
    sc_signal<sc_uint<8>>   s;
    
    SC_CTOR(top)
    {
        SC_METHOD(assign_proc); sensitive << s;
        SC_METHOD(unary_proc); sensitive << s;
        SC_METHOD(binary_proc); sensitive << s;
        SC_METHOD(compound_proc); sensitive << s;
    }
    
#define CHECK(ARG) sct_assert(ARG); sct_assert_const(ARG);
    
// ---------------------------------------------------------------------------        

    const sc_uint<16> C1 = 42;
    const unsigned C2 = 42;

    // Assign literals to signed/unsigned variables
    // C1 is sc_uint
    template <class T>
    void assign(T neg_val) 
    {
        T j = 42;
        CHECK(j == 42);
        j = -42;
        CHECK(j == neg_val);
        
        j = C1;
        CHECK(j == 42);
        j = -C1;
        CHECK(j == neg_val);
        
        sc_uint<16> l1 = 42;
        j = -l1;
        CHECK(j == neg_val);
        //cout << "j " << j << endl;
    }
    
    // C2 is unsigned 
    template <class T>
    void assign_unsigned(T neg_val) 
    {
        T j = C2;
        CHECK(j == 42);
        j = -C2;
        CHECK(j == neg_val);
    }
    
    void assign_bigint() 
    {
        sc_biguint<16> l1 = 42;
        sc_biguint<16> k = -l1;
        CHECK(k == 65494);

        sc_biguint<16> l2 = 42;
        sc_bigint<16> j = -l2;
        CHECK(j == -42);
        //cout << "j " << j << endl;
    }
    
    void assign_proc() 
    {
        assign<int>(-42);
        assign<unsigned>(4294967254);
        assign<sc_int<16>>(-42);
        assign<sc_uint<16>>(4294967254);
        assign<sc_uint<33>>(8589934550ULL);
        assign<sc_bigint<33>>(-42);
        assign<sc_biguint<33>>(8589934550ULL);
        
        assign_unsigned<sc_bigint<16>>(4294967254);
        assign_bigint();
    }

// ---------------------------------------------------------------------------        

    // Unary operations with signed/unsigned literals
    template <class T>
    void unary(T neg_val1, T neg_val2) 
    {
        // Decr
        T j = 1;
        j--;
        j--;
        CHECK(j == neg_val1);
        
        // Incr
        j++;
        j++;
        CHECK(j == 1);
        
        // Plus
        j = -C1;
        j = +j;
        CHECK(j == neg_val2);
        
        // Minus
        j = -C1;
        j = -j;
        CHECK(j == 42);
        
        // LNot
        j = -C1;
        bool b = j;
        CHECK(b);
        b = !j;
        CHECK(!b);
        
        // BNot
        j = ~j;
        //cout << "j " << j << endl;
        CHECK(j == 41);
    }
    
    template <class T>
    void unary_bigint(T neg_val1, T neg_val2) 
    {
         // Decr
        T j = 1;
        j--;
        j--;
        CHECK(j == neg_val1);
        
        // Incr
        j++;
        j++;
        CHECK(j == 1);
        
        // Plus
        j = -C1;
        j = +j;
        //cout << "j " << j << endl;
        CHECK(j == neg_val2);
        
        // Minus
        j = -C1;
        j = -j;
        CHECK(j == 42);
        
        // No LNot
        j = -C1;
        bool b = j.to_long();
        CHECK(b);
        b = !j.to_long();
        CHECK(!b);
        
        // No CPA for BNot as type width is undefined
    }
    
    void unary_proc() 
    {
        unary<int>(-1, -42);
        unary<unsigned>(4294967295, 4294967254);
        unary<sc_int<16>>(-1, -42);
        unary<sc_uint<16>>(65535, 65494);
        unary<sc_uint<33>>(8589934591ULL, 8589934550ULL);
        unary_bigint<sc_bigint<33>>(-1, -42);
        unary_bigint<sc_biguint<33>>(8589934591ULL, 8589934550ULL);
    }
    
    
// ---------------------------------------------------------------------------        
    
    // Binary operations with signed/unsigned literals
    template <class T>
    void binary(T neg_val1, T neg_val2, T neg_val3, bool doDiv = true) 
    {
        // Plus
        T j = -C1;
        T r = j + 1;
        CHECK(r == neg_val1);
        r = -C1 + j;            // Warning reported
        //CHECK(r == neg_val2);
        
        // Minus
        r = j - (-1);
        CHECK(r == neg_val1);
        r = -C1 - (-j);         // Warning reported
        //CHECK(r == neg_val2);
        r = -C1 - j;            // Warning reported
        //CHECK(r == 0);  
        
        // Multiply
        r = j * 1 - 1*(-1);         
        CHECK(r == neg_val1);
        r = (-j) * (-2);
        CHECK(r == neg_val2);
        r = -C1 * 2;
        CHECK(r == neg_val2);
        
        // Division
        if (doDiv) {
            r = j / 1 - 1/(-1);
            CHECK(r == neg_val1);
            r = (-j*4) / (-2);
            //cout << "r " << r << endl;
            CHECK(r == neg_val3);
        }
    }

    // @biguint and @bigint specific, for @biguint it need to add signed even
    // for operations with integer literal as its operators have signed result
    void binary_bigint() 
    {
        sc_biguint<16> j = -42;
        sc_biguint<16> r = j / 1 - 1/(-1);
        CHECK(r == 65495);
        r = (-j*4) / (-2);
        CHECK(r == 65452);
        
        sc_int<16> y = -5;
        sc_bigint<16> z = y - j;
        CHECK(z == 37);
        z = -y * j;
        CHECK(z == -210);
        //cout << "z " << z << endl;
        
        sc_biguint<16> l1 = 42;
        sc_biguint<16> k = 1 + (-l1);
        //cout << "k " << k << endl;
        CHECK(k == 65495);

        sc_biguint<16> l2 = 42;
        sc_bigint<16> jj = 1 + (-l2);
        CHECK(jj == -41);
    }    

    
    void binary_proc() 
    {
        binary<int>(-41, -84, -84);
        binary<unsigned>(4294967255, 4294967212, 0, false);
        binary<sc_int<16>>(-41, -84, -84);
        binary<sc_uint<16>>(65495, 65452, 0, false);
        binary<sc_uint<33>>(8589934551ULL, 8589934508ULL, 0, false);
        binary<sc_bigint<33>>(-41, -84, -84);
        binary<sc_biguint<33>>(8589934551ULL, 8589934508ULL, 8589934508ULL);
        
        binary_bigint();
    }
    
// ---------------------------------------------------------------------------        
    
    // Compound assignments with signed/unsigned literals
    template <class T>
    void compound(T neg_val1, T neg_val2, T neg_val3) 
    {
        // Plus
        T j = -C1;
        j += 1;
        CHECK(j == neg_val1);
        j = -C1;
        j += -C1;           // Warning reported
        //CHECK(j == neg_val2);
        
        // Minus
        j = -C1;
        j -= -1;
        CHECK(j == neg_val1);
        j = -C1;
        j -= -j;            // Warning reported    
        //CHECK(j == neg_val2);
        j = -C1;
        j -= -C1;           // Warning reported
        //CHECK(j == 0);
        
        // Multiply
        j = -C1;
        j *= +1;
        CHECK(j+1 == neg_val1);
        j = C1;
        j *= -2;
        CHECK(j == neg_val2);
        j = -C1;
        j *= 2;
        CHECK(j == neg_val2);
        
        // Division
        j = -C1+1;
        j /= 1;
        CHECK(j == neg_val1);
        j = -C1;
        j *= -4;
        j /= -2;            // Warning reported
        //cout << "j " << j << endl;
        //CHECK(j == neg_val3);
    }

    // @biguint and @bigint specific, for @biguint it need to add signed even
    // for operations with integer literal as its operators have signed result
    void compound_bigint() 
    {
        sc_biguint<16> j = -42;
        j /= 1;
        CHECK(j == 65494);
        j = -42;
        j *= -4;
        j /= -2;
        //CHECK(j == 65452);
        
        sc_bigint<16> y = -5;
        j = -42;
        y += -j;
        CHECK(y == 37);
        
        y *= j;
        CHECK(y == -1554);
        
        sc_biguint<16> l1 = 42;
        sc_biguint<16> k = 1;
        k += (-l1);             // Warning reported
        //CHECK(k == 65495);

        sc_biguint<16> l2 = 42;
        sc_bigint<16> jj = 1;
        jj += (-l2);
        CHECK(jj == -41);

        jj = -41;
        jj += l2;
        CHECK(jj == 1);

        jj = -41;
        jj -= l2;
        CHECK(jj == -83);
    }    

    
    void compound_proc() 
    {
        compound<int>(-41, -84, -84);
        compound<unsigned>(4294967255, 4294967212, 0);
        compound<sc_int<16>>(-41, -84, -84);
        compound<sc_uint<16>>(65495, 65452, 0);
        compound<sc_uint<33>>(8589934551ULL, 8589934508ULL, 0);
        compound<sc_bigint<33>>(-41, -84, -84);
        compound<sc_biguint<33>>(8589934551ULL, 8589934508ULL, 8589934508ULL);
        
        compound_bigint();
    }    
    
};

int sc_main(int argc, char **argv)
{

    top t_inst{"t_inst"};
    sc_start();

    return 0;
}
