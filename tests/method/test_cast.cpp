/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"
#include "sct_assert.h"


// Implicit and explicit cast operations for variables and constants, 
// including multiple casts
class A : public sc_module 
{
public:
    int                 m;
    int                 k;

    static const unsigned CONST_A = 1;
    static const unsigned CONST_Z = 0;
    const int M1 = 42;
    const int M2 = 44;
    const int* pm = &M1;
    
    sc_signal<bool> dummy{"dummy"};
    sc_signal<sc_uint<4>> s;

    SC_CTOR(A)
    {
        *(const_cast<int*>(pm)) = 43;
        const_cast<int&>(M2) = 45;
        
        SC_METHOD(literals); sensitive << s;
        SC_METHOD(errors); sensitive << s;
        SC_METHOD(warnings); sensitive << s;
        SC_METHOD(shift_signed_warnings); sensitive << s;
        
        SC_METHOD(unary_cast); sensitive << s;
        SC_METHOD(binary_comparison_pos); sensitive << s;
        SC_METHOD(binary_comparison_neg); sensitive << s;
        SC_METHOD(binary_shift); sensitive << s;
        
        SC_METHOD(explicit_cast_required); sensitive << is << xs << us << bs;
        SC_METHOD(cast_to_unsigned); sensitive << is << xs << us << bs;
        SC_METHOD(cast_to_unsigned32); sensitive << is << xs << us << bs;
        SC_METHOD(cast_to_unsigned_compound); sensitive << s;
        SC_METHOD(cast_to_unsigned_unary); sensitive << s;
        
        SC_METHOD(unsigned_binary_overflow); sensitive << s;
        
        SC_METHOD(const_cast_for_variable); sensitive << dummy;
        
        SC_METHOD(cond_const); sensitive << dummy;
        SC_METHOD(bool_cast); sensitive << dummy;
        SC_METHOD(const_bool_type_cast); sensitive << dummy;
        SC_METHOD(const_cpp_type_cast); sensitive << dummy;
        SC_METHOD(var_cpp_type_cast); sensitive << dummy;
        SC_METHOD(const_cpp_ref_impl_cast); sensitive << dummy;
        SC_METHOD(const_cpp_ref_expl_cast); sensitive << dummy;
        SC_METHOD(var_cpp_expl_cast); sensitive << dummy;
        
        SC_METHOD(const_sc_type_cast); sensitive << dummy;
        SC_METHOD(var_sc_type_cast); sensitive << dummy;
        SC_METHOD(multi_sc_type_cast); sensitive << s;
        
        SC_METHOD(read_to_int); sensitive << s << a << b << c << d;
    }
    
    #define CHECK(ARG) sct_assert(ARG); sct_assert_const(ARG);
    
    const sc_uint<16> C1 = 14;

    void literals() {
        int i = 10;
        unsigned u = 12;
        sc_uint<16> x = 14;
        sc_int<16> y = 11;
        sc_biguint<15> bx = 13;
        sc_bigint<33> by = 12;
        sc_bigint<70> bres;

        bres = 12;
        bres = 12LL;
        bres = -12;
        bres = -12LL;
        
        bres = 0x100000001;
        bres = 0x100000001LL;
        bres = -0x100000001;
        bres = -0x100000001LL;

        // Check that LL literal is signed in SV
        bres = 4611686018427387904LL + 4;
        bres = -4611686018427387904LL + 4;
        bres = 4611686018427387904LL + i;
        bres = 4611686018427387904LL + y;
        bres = 4611686018427387904LL + by;
        
        bres = 4611686018427387904LL + u;
        bres = 4611686018427387904LL + x;           // Unsigned, OK
        bres = 4611686018427387904LL + bx;
    }
    
    // Incorrect/unsupported code checking 
    void errors() 
    {
        int i = 10;
        sc_uint<16> x = 14;
        unsigned u = 12;
        sc_int<16> y = 11;
        sc_biguint<15> bx = 13;
        sc_bigint<33> by = 12;
        sc_bigint<66> bres;
        
        // 1. incorrect signed for negative value makes it unsigned
        bres = -C1 + by;                // Warning reported, OK
        bres = -u;                      // OK
        bres = u + by;                  // OK
        bres = -u + by;                 // Warning reported, OK
        bres = u + x + by;              // OK
        bres = u - x + by;              // Warning reported, OK
        bres = x - u + x + by;          // Warning reported, OK
        
        // Unsigned binary- and literal
        bres = u - x + 1;               // OK
        unsigned char cu = 12;
        unsigned short su = 13;
        bres = cu - su + 1;             // signed binary-, OK 
        
        // Liter expression
        bres = by + 0x100000000;        // OK
        CHECK (bres == 0x10000000C);
        bres = by - 0x100000000;        // OK
        sct_assert (bres.range(63,0) == 0xFFFFFFFF0000000C);
        
        bres = (3 - 5) + u;             // OK
        bres = (3 - 5) + by;            // OK
        bres = (3U - 5U) + u;           // OK
        bres = (3U + -5U) + u;          // OK
        bres = (3U - 5U) + by;          // Warning reported, OK      
        bres = (3U + -5U) + by;         // Warning reported, OK      
        bres = (-3U) + u;               // OK
        bres = -3 + by;                 // OK
        bres = (-3U) + by;              // Warning reported, OK
        bres = (-3) + by;               // OK
        bres = -(-3U) + by;             // Warning reported, OK
        bres = -(~3U) + by;             // Warning reported, OK           

        bres = (s.read() ? 3 : -5) + by;             
        bres = (s.read() ? 3 : (-5)) + by;             
        bres = (s.read() ? 3U : -5U) + by;      // Warning reported, OK
        bres = (s.read() ? 3U : (-5U)) + by;    // Warning reported, OK  

        bres = i * x + by;              // Warning reported, OK
        
        bres = -bx;                     // signed, OK
        CHECK (bres == -13);            
        bres = -bx + by;                // signed, OK
        CHECK (bres == -1);             
        bres = bx + by;                 // signed, OK
        bres = bx - by;                 // signed, OK     
        bres = bx - x;                  // signed, OK
        
        bres = -bx + y;                 // signed` added, OK
        CHECK (bres == -2);
        bres = -bx + i;             
        CHECK (bres == -3);
        bres = -bx + u;
        CHECK (bres == -1);
        bres = -bx / i;         
        CHECK (bres == -1);
        bres = -bx / y;         
        CHECK (bres == -1);
        bres = -bx / by;        
        CHECK (bres == -1);
        bres = -bx * i;
        CHECK (bres == -130);
        bres = -bx * y;
        CHECK (bres == -143);

        bres = -x + by;             // Warning reported, OK
        bres = -x + y;              // Warning reported, OK  
        bres = (-x + by) / i;       // Warning reported, OK
        bres = (-x + y) / i;        // Warning reported, OK  

        // 2. biguint signed for minus operation
        bres = x - x;               // unsigned, OK
        bres = bx - x;              // signed, OK
        bres = bx - 1;              // signed, OK
        bres = x - bx;              // signed, OK
        bres = bx - bx;             // signed, OK
        bres = bx + (-x);           // unsigned, OK
        bres = bx + x;              // unsigned, OK
        bres = bx * x;              // unsigned, OK
        bres = bx / x;              // unsigned, OK
        
        bres = (u - x) + by;        // Warning reported, OK
        bres = (u + x) + by;        // signed +, OK
        bres = (u + y) + by;        // signed both +, OK

        bres = (-2 - bx + y) / 2;   // signed -, OK

        bres = 1 - bx;              // signed, OK
        CHECK (bres == -12);
        bres = -bx + 1;             // signed, OK
        CHECK (bres == -12);
        bres = -bx / 2;             // signed, OK
        CHECK (bres == -6);
        bres = bx / -2;             // signed, OK
        CHECK (bres == -6);
        bres = -bx * 2;             // signed, OK
        CHECK (bres == -26);
        bres = by - bx;             // signed, OK
        CHECK (bres == -1);
        bres = -bx - by;            // signed, OK
        CHECK (bres == -25);
        bres = y - bx;              // signed, OK
        CHECK (bres == -2);
        bres = i - bx;              // signed, OK
        CHECK (bres == -3);
    }
    
    // Check reported warnings
    void warnings() 
    {
        int i = 13;
        unsigned u = 13;
        sc_uint<15> x = 13;
        sc_biguint<15> bx = 13;
        sc_int<15> y = 13;
        sc_bigint<15> by = 13;
        unsigned long ul = 23;
        long res;
        sc_bigint<66> bres;
        bool b;
        
        res = u + i;                        // Warning reported, OK
        res = x + y;                        // Warning reported, OK
        bres = bx + by;                     // No warning, signed added    
        
        // Compound warnings
        u = u + y;                          // No warning, signed added    
        u += y;                             // Warning reported, OK
        
        u += i;                             // Warning reported, OK
        u += sc_int<12>(i);                 // Warning reported, OK
        u -= y;                             // Warning reported, OK
        x *= i;                             // Warning reported, OK
        x /= y;                             // Warning reported, OK
        bx %= i;                            // Warning reported, OK
        bx &= y;                            // Warning reported, OK
        bx |= by;                           // Warning reported, OK
        
        // Negative literal cast to unsigned warning
        res = u / -2;            // Warning reported, OK             
        u /= -2;                 // Warning reported, OK             
        res = -2 / ul;           // Warning reported, OK
        res = -2 % x;            // Warning reported, OK
        x %= -2;                 // Warning reported, OK
        bres = bx / -2;          // signed, OK
        bres = (bx + 1) % -2;    // signed, OK    
        
        i = -3U;                // OK
        y = -14UL;              // OK   
        by = -(32U);            // OK 
        
        // Comparison warnings -- currently commented, #271
//        res = i + u;            // Warning reported
//        res = i + 42U;          // Warning reported
//        b = i < u;              // Warning reported   
//        b = i < 42U;            // Warning reported
//        b = u < 42;             // OK
        
//        int k = 0;
//        for (int i = -4; i < -1; ++i) {k++;}
//        for (int i = 0; i < 42U; ++i) {k++;}
//        for (int i = 0; i < u; ++i) {k++;}
    }
    
    // Warning reported for signed operands of shift
    void shift_signed_warnings() {
        int i = 2;
        unsigned u = 2;
        sc_uint<16> x = 2;
        sc_int<16> y = 2;
        sc_biguint<15> bx = 2;
        sc_bigint<33> by = 2;
        unsigned long ures;
        
        ures = i >> u;                  // Warning
        ures = unsigned(i) >> u;
        ures = y >> x;                  // Warning
        ures = unsigned(y) >> x;
    }
    
    
    void unary_cast() 
    {
        int i = 13;
        unsigned short ut = 13;
        unsigned u = 13;
        sc_uint<15> x = 13;
        sc_biguint<15> bx = 13;
        sc_int<15> y = 13;
        sc_bigint<15> by = 13;
        long l = 13; long long ll = 13;
        unsigned long ul = 13; unsigned long long ull = 13;
        long res;
        sc_bigint<66> bres;
        
        // Using unary in assignment and binary
        res = -10;                  // signed, OK     
        res = -u;                   // unsigned, OK
        res = -u + u;               // unsigned, OK
        res = -x;                   // unsigned, OK
        bres = -bx;                 // signed, OK
        bres = -bx + 1;             // signed, OK
        bres = -bx + u;             // signed, OK
        bres = -bx + x;             // signed, OK
        bres = -bx + y;             // signed, OK
        bres = -bx + by;            // signed, OK
        bres = bx--;                // unsigned, OK  
        bres = bx-- + by--;         // unsigned--, signed+, OK  
        
        // Complex expressions with unary
        bres = y - (++bx);          // signed, OK    
        bres = y - bx++;            // signed, OK
        bres = x + y - bx++;        // unsigned + and signed -, OK
        
        res = int(u);   
        res = +(int(u));   
        res = -(int(u));            // signed, OK 

        res = i + int(u);           // signed, OK
        res = i + (+(int(u)));      // signed, OK
        res = i + (-(int(u)));      // signed, OK
        
        res = y + int(u);           // signed, OK
        res = y + (+(int(u)));      // signed, OK
        res = y + (-(int(u)));      // signed, OK

        bres = by + int(u);         // signed, OK
        bres = by + (+(int(u)));    // signed, OK
        bres = by + (-(int(u)));    // signed, OK

        res = ~u + y;               // signed, OK
        bres = by - (~x);           // signed, OK
        res = (++i) + (u++);        // unsigned, OK
        bres = (--bx) + (by++);     // signed, OK
        
        res = -u;           // unsigned, OK
        res = i + (-u);     // Warning reported, unsigned, OK
        ul = -u;            // unsigned, OK
        ul = u + (-u);      // unsigned, OK

        by += -bx;      // signed, OK
        by = -bx;       // signed, OK
        bx += -bx;      // signed, OK
        bx = -bx;       // signed, OK
    }
    
    void binary_comparison_pos() 
    {
        int i = 13;
        unsigned short ut = 13;
        unsigned u = 13;
        sc_uint<15> x = 13;
        sc_biguint<15> bx = 13;
        sc_int<15> y = 13;
        sc_bigint<15> by = 13;
        long l = 13; long long ll = 13;
        unsigned long ul = 13; unsigned long long ull = 13;
        long res;
        sc_bigint<66> bres;
        bool b;
        
        b = u == 0;
        CHECK (!b);
        b = x != 13;
        CHECK (!b);
        b = x < 42LL;
        CHECK (b);
        b = bx > 10;
        CHECK (b);

        b = u == i;                 // Warning reported
        CHECK (b);
        b = x == y;                 // Warning reported
        CHECK (b);
        b = ut == u;
        CHECK (b);
        b = bx == by;
        CHECK (b);
        b = bx == ll;
        CHECK (b);
        b = x == by;
        CHECK (b);

        // comparison with cast
        b = u == unsigned(i);       // Warning reported
        CHECK (b);
        b = x == unsigned(y);       // Warning reported
        CHECK (b);
        b = x == sc_uint<15>(y);    // Warning reported
        CHECK (b);
        b = int(u) == i;            // OK
        CHECK (b);
        b = int(x) == y;            // OK
        CHECK (b);
        b = sc_int<15>(x) == y;     // OK
        CHECK (b);
        
        u++;
        if (u < i) {CHECK (false);}
        CHECK (u != by);
        CHECK (ll != u);
        CHECK (bx != u);
        
        by--;
        if (bx <= by) {CHECK (false);}
        if (x <= by) {CHECK (false);}
        CHECK (by != bx);
        CHECK (ul != by);
        CHECK (y != by);

        // Signed in comparison, not required for positive values
        u = 13; y = 13; ll = 13;
        b = u == y;                 // OK
        CHECK (b);
        b = u == ll;                // OK      
        CHECK (b);
        unsigned uu = u + y;
        CHECK(uu == 26);
    }

    void binary_comparison_neg() 
    {
        int i = -13;
        unsigned short ut = -13;
        unsigned u = -13;
        sc_uint<15> x = -13;
        sc_biguint<15> bx = -13;
        sc_int<15> y = -13;
        sc_bigint<15> by = -13;
        long l = -13; long long ll = -13;
        unsigned long ul = -13; unsigned long long ull = -13;
        long res;
        sc_bigint<66> bres;
        bool b;
        
        b = u == -0;                
        //CHECK (!b);
        b = x != -13;               
        //CHECK (!b);
        b = x > -42LL;              
        //CHECK (b);
        b = bx < -10;               
        //CHECK (b);
        
        b = u == i;                 // Warning reported 
        CHECK (b);
        b = x == i;                 // Warning reported 
        b = x == y;                 // Warning reported 
        b = x == by;                
        b = u == i;                 // Warning reported
        b = u == by;                
        b = ut == u;                // OK
        CHECK (!b);
        b = bx == by;               
        b = bx == ll;               // OK
        CHECK (!b);
        b = x == by;                
        
        // comparison with cast
        b = u == unsigned(i);       // OK
        CHECK (b);
        b = x == unsigned(y);       // Warning reported
        CHECK (!b);
        b = x == sc_uint<15>(y);    // Warning reported
        CHECK (b);
        b = int(u) == i;            // OK
        CHECK (b);
        b = int(x) == y;            // OK
        CHECK (!b);               
        b = sc_int<15>(x) == y;     // OK
        CHECK (b);
        
        u++;
        if (u < i) {CHECK (false);}
        CHECK (u != by);
        CHECK (ll != u);
        CHECK (bx != u);
        
        by--;
        if (bx <= by) {CHECK (false);}
        if (x <= by) {CHECK (false);}
        CHECK (by != bx);
        CHECK (ul != by);
        CHECK (y != by);
        
        // Signed in comparison
        ll = -13;
        sc_int<32> yy = -13;
        u = 0xFFFFFFF3;              // -13 in 32bits
        b = u == yy;                 // Signed, signed' in SV required here, OK
        CHECK (!b);
        b = u == ll;                 // Signed, OK      
        CHECK (!b);
        
        u = 0xFFFFFFFC;
        b = u == 0xFFFFFFFC;            // casted to unsigned, OK
        CHECK (b);
        b = u == 4294967292;            // Casted to long, signed' added, OK
        CHECK (b);
    }

    void binary_shift() 
    {
        int i = 3;
        unsigned u = 3;
        sc_uint<15> x = 3;
        sc_biguint<15> bx = 3;
        sc_int<15> y = 3;
        sc_bigint<15> by = 3;
        long res; long res2;
        
        res = 1U << u;              // OK
        res = 1U << x;              // OK
        res = 1U << bx.to_uint();   // OK
        res = 1U << unsigned(i);    // OK
        res = u << by.to_uint();    // OK
        res = unsigned(i) << u;     // OK

        // Signed shift 
        res = 1 << i;               // Warning reported
        res = 1 << u;               // Warning reported
        res = (1 << i) + u;         // Warning reported
        res = (1 << u) + u;         // Warning reported

        // Signedness in shift 
        u = 31;
        res = 1 << u;               // Negative number
        res2 = 1U << u;             // Positive number
        //cout << res << " " << res2 << endl;
        //CHECK (res != res2);      // Both positive in SV

        int ires = 1 << u;          // Negative number
        int ires2 = 1U << u;        // Positive number
        //cout << ires << " " << ires2 << endl;
        CHECK (ires == ires2);      // Both negative in SC and SV
    }
    
    
    // For #274
    void explicit_cast_required() 
    {
        long res;
        unsigned long ures;

        // Currently prohibited but could be supported with adding 'signed 
        unsigned u = 0x1F000A001UL;
        res = (int)u;                           // Add 'signed
        cout << hex << res << endl;
        //CHECK (res == -0xFFF5FFF);              // Error in SV

        int i = -10;
        ures = (unsigned)i;                     // Add 'unsigned
        cout << hex << ures << endl;
        //CHECK (ures == 0xFFFFFFF6);             // Error in SV
        
        ures = (unsigned)i + u;                 // Add 'unsigned
        res = i + (int)u;                       // OK
    }    
    
    
    sc_signal<int> is;
    sc_signal<unsigned> us;
    sc_signal<sc_uint<15>> xs;
    sc_signal<sc_biguint<15>> bs;
    void cast_to_unsigned() 
    {
        // Negative value in unsigned variable is prohibited
        int i = -13;
        unsigned short ut = 12;
        unsigned u = -12;               
        sc_uint<15> x = 12;
        sc_biguint<15> bx = 12;
        sc_int<15> y = -13;
        sc_bigint<15> by = -11;
        long l = -112; long long ll = -112;
        unsigned long ul = 113; unsigned long long ull = 113;
        long res;
        unsigned long ures;
        sc_bigint<66> bres;
        
        // Signed/unsigned cast                             (#266.2)
        // Cast to signed/unsigned in assignment
        res = (unsigned)i;                      // OK
        res = (sc_uint<12>)i;                   // OK
        res = int(u);                           // OK
        res = sc_int<6>(ul);                    // OK
        
        res = (unsigned)i;                      // OK, '32
        res = (unsigned)y;                      // OK, '32
        res = (unsigned)i + (unsigned)y;        // OK, '32 + '32
        
        res = (int)x;                           // OK, '32
        ures = (unsigned)i;                     // OK, '32
        ures = sc_uint<4>(y);                   // OK, '4
        res = (int)u + (int)x;                  // OK, 'signed + 'signed
        res = sc_int<16>(x) + long(y);          // OK, '16 + '64    
        
        ures = (int)i;                          // OK, '32
        ures = (int)u;                          // OK, '32
        ures = sc_int<16>(x);                   // OK, '16
        ures = sc_int<16>(y);                   // OK, '16
        
        // Cast to unsigned in binary
        // Unsigned arithmetic                              (#266.3)
        res = x + (unsigned)i;                    // OK 
        res = y + (unsigned)i;                    // OK 
        res = i + (unsigned)i;                    // OK
        res = u + (unsigned)i;                    // OK 
        res = ut + (unsigned)i;                   // OK 
        res = ll + (unsigned)i;                   // OK
        
        // Cast to signed in binary
        // Signed required for both arguments               (#266.3)
        res = int(u) + x;                         // OK
        res = u + int(x);                         // OK
        res = int(u) + y;                         // OK
        res = i + int(x);                         // OK
        
        res = x.to_int() + x;                     // OK
        res = x.to_int() + i;                     // OK
        res = xs.read().to_int() + xs.read();     // OK
        res = xs.read().to_int() + is.read();     // OK
        
        // Signed casted to signed
        res = sc_int<12>(i) + ut;                 // OK
        res = sc_int<12>(i) + u;                  // OK
        res = sc_int<12>(i) + ul;                 // OK
        res = sc_int<12>(y) + x;                  // OK  
        res = is.read() + us.read();              // OK
        res = sc_int<12>(is.read()) + us.read();  // OK
        res = sc_int<12>(is.read()) + xs.read();  // OK
        res = int(i) + ut;      // OK
        res = int(i) + u;       // OK
        res = (long int)i + u;  // OK
        res = (long int)i + x;  // OK
        
        res = x + i;            // OK
        res = x & i;            // OK
        
        // Unintended signed arithmetic for @sc_biguint    (#266.4)
        res = 10 - x;                           // OK
        CHECK (res == -2);
        bres = 10 - bx;                         // OK, signed used here
        CHECK (bres == -2);
        bres = -10 * bx;                        // OK
        CHECK (bres == -120);
        bres = 10 + by;                         // OK           
        CHECK (bres == -1);
        bres = 10 - x;                          // OK
        sct_assert (bres.range(63,0) == 0xFFFFFFFFFFFFFFFE);
    }
    
    void cast_to_unsigned32() 
    {
        // Negative value in unsigned variable is prohibited
        int i = -13;
        unsigned short ut = 12;
        unsigned u = 12;
        sc_uint<15> x = 12;
        sc_biguint<15> bx = 12;
        sc_int<15> y = -13;
        sc_bigint<15> by = -11;
        long l = -112; long long ll = -112;
        unsigned long ul = 113; unsigned long long ull = 113;
        int res;
        sc_bigint<32> bres;
        
        // Signed/unsigned cast                             (#266.2)
        res = (unsigned)i;                      // OK, 32'
        res = (sc_uint<12>)i;                   // OK, 12'
        res = sc_int<6>(ul);                    // OK, '6

        res = i + 1;                            // OK
        res = i + int(1);                       // OK, 32'sd
        res = i + sc_int<4>(1);                 // OK, 4'sd
        res = y + unsigned(1);                  // OK    
        res = y + sc_uint<4>(1);                // OK, 4'd
        bres = y + sc_bigint<4>(1);             // OK, 4'sd
        bres = y + sc_biguint<4>(1);            // OK, 4'sd
        
        // Unintended signed arithmetic for @sc_biguint    (#266.4)
        res = 10 - x;                           // OK
        CHECK (res == -2);
        bres = 10 - bx;                         // OK, signed used here
        CHECK (bres == -2);
        bres = -10 * bx;                        // OK
        CHECK (bres == -120);
        bres = 10 + by;                         // OK           
        CHECK (bres == -1);
        bres = 10 - x;                          // OK
        sct_assert (bres == -2);
    }    
    
    void cast_to_unsigned_compound() 
    {
        int i = -13;
        unsigned short ut = 12;
        unsigned u = 12;
        sc_uint<15> x = 12;
        sc_biguint<15> bx = 12;
        sc_int<15> y = -13;
        sc_bigint<15> by = -11;
        long l = -112; long long ll = -112;
        unsigned long ul = 113; unsigned long long ull = 113;
        
        x += i;                             // unsigned, OK
        CHECK (x == 32767);
        bx *= i;                            // unsigned, OK
        CHECK (bx == 32612);
        
        // Cast to unsigned in binary
        // Unsigned arithmetic                              (#266.3)
        x += (unsigned)i;                   // unsigned, OK 
        y += (unsigned)i;                   // signed, OK 
        i += (unsigned)i;                   // signed, OK
        u += (unsigned)i;                   // unsigned, OK 
        ut += (unsigned)i;                  // unsigned, OK 
        ll += (unsigned)i;                  // signed, OK
        bx += (unsigned)i;                  // unsigned, OK
        
        // Cast to signed in binary
        // Signed required for both arguments               (#266.3)
        x += int(u);                        // unsigned, OK
        bx += y;                            // unsigned, OK
        bx += x;                            // unsigned, OK
        bx += int(y);                       // unsigned, OK
        bx += int(x);         // 'signed added for explicit cast, unsigned OK
        u -= x.to_int();                    // unsigned, OK
        bx -= x.to_int();     // 'signed added for explicit cast, unsigned OK
        
        // Signed casted to signed
        u += sc_int<12>(i);                 // unsigned, OK
        i += sc_int<12>(i);                 // signed, OK
        x += sc_int<12>(i);                 // unsigned, OK
        y += sc_int<12>(i);                 // signed, OK
        bx += sc_int<12>(i);                // OK
        by += sc_int<12>(i);                // OK
        bx -= (long int)i;                  // unsigned, OK
        by -= (long int)i;                  // unsigned, OK
        
        bx -= i;                            // unsigned, OK
        bx = bx - i;                        // signed, OK    
        bx *= by;                           // unsigned, OK
        bx = bx * by;                       // signed, OK    
        bx /= ll;                           // unsigned, OK
        
        by += u;                            // signed
        by += bx;                           // signed, cast for sc_biguint required
        
        x |= i;                             // OK
        u |= i;                             // OK
        bx &= ll;                           // OK
    }    
    
    void cast_to_unsigned_unary() 
    {
        int i = -13;
        unsigned short ut = 12;
        unsigned u = 12;
        sc_uint<15> x = 12;
        sc_biguint<15> bx = 12;
        sc_int<15> y = -13;
        sc_bigint<15> by = -11;
        long l = -112; long long ll = -112;
        unsigned long ul = 113; unsigned long long ull = 113;
        long res;
        sc_bigint<66> bres;
        
        res = +i;           
        res = u--;
        res = (++i) + x;                // unsigned, OK
        bres = (x++) + ll;              // unsigned, OK
        bres = (u--) + by;              // signed, OK
        bres = (bx--) + by;             // signed, OK
        
        // Cast to unsigned in unary
        res = +(unsigned)i;             // OK 
        res = -(unsigned)i;             // OK
        bres = (-(int)u) + bx;          // signed, OK
        bres = (-(int)u) + by;          // signed, OK
        
    }    
    
    // Example for #268
    void unsigned_binary_overflow() 
    {
        int i = -13;
        unsigned u = 12;
        sc_uint<15> x = 12;
        sc_biguint<15> bx = 12;
        sc_int<15> y = -13;
        sc_bigint<15> by = -11;
        long l = -112; long long ll = -112;
        unsigned long ul = 113; unsigned long long ull = 113;
        long res;
        sc_bigint<66> bres;
        
        res = i + ll;               // OK
        res = i + u;                // Warning reported, OK
        CHECK(res == 4294967295);
        res = 10 - u;               // OK 
        cout << hex << res << endl;
        res = i + u + 100;          // Warning reported, OK
        res = i * u;                // Warning reported, OK
        // No sign bit extended in generated SV
        sc_uint<20> res_ = x + y;   // Warning reported, OK

        // Comparison, probably cast required 
        x = -13; bx = -13;
        y = -13; by = -13;
        bool b;
        b = x == y;                 // Warning reported 
        b = bx == by;               // Warning reported 
        b = x == by;                // Warning reported 
    }    
    
    // @const_cast<> to remove constantness
    sc_signal<int> r1;
    void const_cast_for_variable() {
        CHECK (M1 == 43);
        CHECK (M2 == 45);
        
        // That is OK
        int i = *(const_cast<int*>(pm));
        // Error reported, M1 is @localparam cannot be changed
        //*(const_cast<int*>(pm)) = 46;
        
        auto j = (const int)i;
        r1 = j+1;
    }
    
    void cond_const() {
        bool b;
        b = (CONST_Z) ? 1 : 2;
        CHECK(b);
        b = (CONST_Z) ? bool(1) : bool(2);
        
        unsigned char c;
        c = (CONST_A) ? 258 : 259;
        CHECK(c == 2);
        c = (CONST_A) ? (unsigned char)258 : (unsigned char)259;
    }
    
    void bool_cast() {
        bool b = 1;
        b = !b;

        int i = 2;
        i = ~i;
        
        b = i;
    }

    // Implicit and explicit cast for bool type
    void const_bool_type_cast() {
        bool b;
        // Implicit cast
        unsigned char c;
        c = 2;
        b = c;
        CHECK(b);
        
        b = 257;
        CHECK(b);
        
        c = 0;
        b = c;
        CHECK(!b);
        
        // Explicit cast
        unsigned int i;
        i = 65536;
        b = (bool)i;
        CHECK(b);
        
        b = (bool)65536;
        CHECK(b);
        
        i = 0;
        b = (bool)i;
        CHECK(!b);
    }
    
    // Implicit and explicit cast for CPP types
    void const_cpp_type_cast() {
        // Implicit cast
        unsigned char c;
        c = 257;
        CHECK(c == 1);
        
        unsigned short s = 257;
        c = s;
        CHECK(c == 1);
        
        unsigned int i;
        unsigned long l = (unsigned long)1 << 32;
        CHECK(l == 4294967296);
        i = l + 1;
        CHECK(i == 1);
        
        // Explicit cast
        i = (unsigned char)257;
        CHECK(i == 1);
        i = (unsigned short)65537;
        CHECK(i == 1);
        
        i = (unsigned char)s + 1;
        CHECK(i == 2);
    }
    
    void var_cpp_type_cast() {
        // Implicit cast
        unsigned char c;
        unsigned short s;
        unsigned int i;
        unsigned long l = 0xAAAABBBBCCCCDDEEULL;

        i = l;
        s = i;
        c = s;
        
        CHECK(l == 0xAAAABBBBCCCCDDEEULL);
        CHECK(i == 0xCCCCDDEE);
        CHECK(s == 0xDDEE);
        CHECK(c == 0xEE);
    }

    // Cast for CPP types in references
    void const_cpp_ref_impl_cast() {
        unsigned int i = 65537;
        unsigned short s = 257;
        unsigned short& rs = s; 
        
        unsigned char c = rs;
        CHECK(c == 1);
        CHECK(s == 257);
        
        c = rs + 1;
        CHECK(c == 2);
    }

    void const_cpp_ref_expl_cast() {
        unsigned int i = 65537;
        unsigned short s = 257;
        unsigned short& rs = s; 
        
        unsigned int j = (unsigned char)rs;
        CHECK(j == 1);
        CHECK(s == 257);
        
        j = (unsigned char)rs + 1;
        CHECK(j == 2);
    }
    
    void var_cpp_expl_cast() {
        unsigned int u;
        unsigned short s;
        sc_uint<33> ux = 0x1C0000000; 
        sc_biguint<4> bu; 
        sc_int<4> ix; 
        sc_bigint<4> bi; 
        
        int i;
        i = ux.to_int();
        cout << hex << " i " << i << endl;
        CHECK(i == 0xC0000000);
        i = ux.to_int()+1;
        cout << hex << " i " << i << endl;
        CHECK(i == 0xC0000001);
        
        i = (int)s + 1;
        i = (int)u + 1;
        i = (int)ux + ux.to_int() + bu.to_int();
        i = ux.to_int();
        i = bu.to_int();
        i = (int)ix + ix.to_int() + bi.to_int();
        
        i = ix.to_int();
        i = ix.to_long();
        i = ix.to_int64();
        i = ix.to_uint();
        i = ix.to_ulong();
        i = ix.to_uint64();

        i = bi.to_int();
        i = bi.to_long();
        i = bi.to_int64();
        i = bi.to_uint();
        i = bi.to_ulong();
        i = bi.to_uint64();
    }
    
    // Implicit and explicit cast for SC types
    void const_sc_type_cast() {
        sc_uint<4> x = 12;
        sc_uint<3> y = x;
        CHECK(y == 4);
        
        sc_int<5> sx = -7;
        sc_int<3> sy = sx;
        //cout << "sy " << sy << endl;
        CHECK(sy == 1);    // It is really equals to 1
        
        sc_uint<5> z = (sc_uint<3>)x;
        CHECK(z == 4);
        
        z = (sc_uint<3>)(x+1);
        CHECK(z == 5);

        z = (sc_uint<3>)(13);
        CHECK(z == 5);

        z = (sc_uint<2>)15;
        CHECK(z == 3);
        
        unsigned int i = 14;
        z = (sc_uint<2>)i;
        CHECK(z == 2);
        
        sc_int<5> sz;
        sz = (sc_uint<3>)sx;
        cout << "sz " << sz << endl;
        CHECK(sz == 1);
        
        sz = (sc_uint<3>)(-13);
        cout << "sz " << sz << endl;
        CHECK(sz == 3);
    }    
    
    void var_sc_type_cast() {
        // Implicit cast
        sc_uint<8> u1;
        sc_uint<16> u2;
        sc_biguint<32> b1;
        sc_biguint<48> b2; 
        unsigned int i = 0xAAAABBCD;
        unsigned long l = 0xAAAABBBBCCCCDDEEULL;

        u1 = i; 
        CHECK(u1 == 0xCD);
        u1 = l;
        CHECK(u1 == 0xEE);
        u2 = i; 
        CHECK(u2 == 0xBBCD);
        u2 = l;
        CHECK(u2 == 0xDDEE);
        b1 = i; 
        CHECK(b1 == 0xAAAABBCD);
        b1 = l;
        CHECK(b1 == 0xCCCCDDEE);
        b2 = i; 
        CHECK(b2 == 0xAAAABBCD);
        b2 = l;
        CHECK(b2 == 0xBBBBCCCCDDEEULL);
    }

    // Multiple casts for SC types
    static const unsigned CC = 42;
    static const int SC = -42;
    void multi_sc_type_cast() {
        sc_uint<4> x = s;
        sc_uint<8> y = (sc_uint<8>)((sc_uint<6>)x);
        y = (sc_uint<6>)((sc_uint<8>)x);
        y = (sc_uint<6>)((sc_uint<2>)x);
        y = (sc_uint<2>)((sc_uint<3>)x);
        
        sc_uint<16> z = ((sc_uint<8>)((sc_uint<3>)y), 
                         (sc_uint<8>)((sc_uint<3>)0x11));
        z = ((sc_uint<8>)((sc_uint<3>)y), 
              (sc_uint<8>)((sc_uint<3>)CC));
        
        sc_int<16> sz = ((sc_int<8>)(-(sc_int<3>)y), 
                         (sc_int<8>)((sc_int<3>)-0x11));
        sz = ((sc_int<8>)((sc_int<3>)-y), 
              (sc_int<8>)((sc_int<3>)SC));
    }
    
    sc_in<sc_uint<38>>          a{"a"};
    sc_in<sc_uint<42>>          b{"b"};
    sc_in<sc_bigint<67>>        c{"c"};
    sc_out<sc_biguint<111>>     d{"d"};
    
    void read_to_int() {
        int l;
        l = a.read().to_int();
        l = b.read().to_int();
        l = c.read().to_int();
        d.write(a.read().to_int() + b.read().to_int() + c.read().to_int() / 4);
    }
};

class B_top : public sc_module 
{
public:
    sc_signal<sc_uint<38>>      a{"a"};
    sc_signal<sc_uint<42>>      b{"b"};
    sc_signal<sc_bigint<67>>    c{"c"};
    sc_signal<sc_biguint<111>>  d{"d"};

    A a_mod{"a_mod"};

    SC_CTOR(B_top) {
        a_mod.a(a);
        a_mod.b(b);
        a_mod.c(c);
        a_mod.d(d);
    }
};

int sc_main(int argc, char* argv[])
{
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}

