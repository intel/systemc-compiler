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

// Binary operation general cases
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

    SC_CTOR(A) 
    {
        SC_CTHREAD(syncProc, clk);
        async_reset_signal_is(rstn, false);
     
        SC_METHOD(multiAssign); sensitive << s << a;
        
        SC_METHOD(enumOperation); sensitive << s << a;
        SC_METHOD(comma); sensitive << s << a;
        SC_METHOD(shift); sensitive << s << a;
        SC_METHOD(sc_type_shift); sensitive << dummy;

        SC_METHOD(sc_type_main); sensitive << dummy;
        
        SC_METHOD(sc_type_main_neg); sensitive << dummy;
        SC_METHOD(sc_type_main_signed); sensitive << dummy;
        
        SC_METHOD(sc_type_logic_bitwise); sensitive << dummy;
        SC_METHOD(logic_bitwise_literal); sensitive << dummy;
        
        SC_METHOD(sc_type_comp); sensitive << dummy;
        SC_METHOD(large_types); sensitive << dummy;

        SC_METHOD(sc_relational_ops); sensitive << dummy;
        SC_METHOD(sc_fp_bitwise_fns); sensitive << dummy;
    }
    
    #define CHECK(ARG) sct_assert(ARG); sct_assert_const(ARG);
    
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

// ---------------------------------------------------------------------------    

    void multiAssign() 
    {
        bool b1, b2;
        b1 = b2 = 1;
        //b1 = !(b2 = 1);            // Error reported, OK     
        
        unsigned u1, u2, u3;
        u1 = u2 = 1;
        u1 = u2 = u3 = 2;
        u1 = u2 += 1;
        u1 = u2 = u3++;
        u1 = u2 = --u3;
        u1 = (u2 = 3);
        u1 *= (u2 = 3);
        //u1 += u2 = u3;
        //u1 = ((u2 *= u3) * 42);   // Error reported, OK
        //u1 = +(u2 = u3);          // Error reported, OK
      
        sc_uint<12> ux1, ux2, ux3;
        ux1 = ux2 = 1;
        ux1 = ux2 = ux3 = 1;
        ux1 = ux2 -= 1;
        ux1 = ux2 == 1;
        ux1 = u1 = (s.read() ? 1 : 2);
        ux1 = (ux2 += 3);
        ux1 += ux2 += ux3+1;
        //(ux1 += u2)++;             // Error reported, OK
        //ux1 = ((ux2 *= ux3) * 42); // Error reported, OK
        
        sc_bigint<66> bx1, bx2, bx3;
        bx1 = bx2 = -11;
        bx1 = bx2 /= 2;
        bx1 = ux2 = ux1+1;
        bx1 = u1 = ux1 = 3;
        ux1 = bx1 = 5;
        
        u1 = (int)(ux1 = 3);
        u1 = (sc_uint<10>)(ux1 = 3);
        ux1 = (sc_uint<10>)(ux2 = 3); 
        ux1= (sc_uint<33>)(sc_uint<10>)(ux2 = 3);
        bx1 = (sc_biguint<33>)(bx1 = 3);
        bx1 -= (sc_biguint<33>)(bx1 = 3);
    }

    // Enum in binary operations 
    enum E1 {
        U1_ENUM=1,
        U2_ENUM=2,
        U3_ENUM=0
    };
    
    enum E2 {
        I1_ENUM=1,
        I2_ENUM=-2,
        I3_ENUM=0
    };
    
    void enumOperation() 
    {
        E1 uvar = U1_ENUM;
        E2 ivar = I2_ENUM;

        bool b = false;
        int i = 11; 
        sc_uint<4> x = 1;
        sc_int<4> y = -1;
        unsigned u = 1;
        sc_biguint<17> bu = 1;
        sc_bigint<24> bi = -2;
        sc_bigint<33> z;
        
        i = b + uvar;

        i = x + ivar;       // Warning reported
        //CHECK(i == -1);
        i = x + uvar;
        CHECK(i == 2);
        i = -2 + uvar;
        CHECK(i == -1);
        
        i = -2;
        i = i + uvar; CHECK(i == -1);
        i = -2;
        i = i + ivar; CHECK(i == -4);
        i = u + uvar; CHECK(i == 2);
        i = u + ivar;        // Warning reported
        z = bu + uvar; CHECK(z == 2);
        z = bu + ivar; CHECK(z == -1);
        z = bi + uvar; CHECK(z == -1);
        z = bi + ivar; CHECK(z == -4);
    }
    
// ---------------------------------------------------------------------------    
    
    // C++ comma, not concat
    int f(int i) {
        return (i+1);
    }
    
    void g(int& arg, int i) {
        arg += i;
    }

    void comma() {
        int i, j, k;
        k = (i++, j++);     // Warning reported
        k = (j=i+k, j);     // Warning reported
        
        k = (j = f(1), j++);    // Warning reported
        CHECK (j == 3);     
        k = (g(j, 2), j);       // Warning reported
        CHECK (j == 5);
        
        i = 0;
        k = (i = 1, i+1);       // Warning reported
        CHECK (i == 1);
        CHECK (k == 2);
    }
    
    // Shift
    void shift() {
        unsigned i;
        sc_uint<4> x;
        sc_uint<8> y;
        int m = 3;
        i = m << 2;
        sc_uint<12> ii = concat(x, y) << 2;
        i = a.read() >> 3;
        s = i << m;
        int k = a.read() << s.read();
        k++;
        b.write(s.read() >> k);
    }

    // Shift for SC data types including more than 64bit 
    void sc_type_shift() 
    {
        sc_uint<37> i = 15;
        sc_uint<39> j = i << 2;
        sc_biguint<72> x = (sc_biguint<70>)i << 68;
        x = x << 70;
        CHECK(x==0);
        sc_biguint<130> y = static_cast< sc_biguint<130> >(i) >> 71;
        CHECK(y==0);
        y = x >> 67;
        CHECK(y==0);
        y = x >> i;
        y = x << (j-1);
        // Cast to SC type
        int ii = 42;
        y = x << sc_uint<12>(ii);
        y = x << sc_biguint<12>(ii);
    }
    
    void sc_type_main_neg() 
    {
        sc_uint<5> x = 1;
        sc_int<5> y = -5;
        sc_bigint<33> bx = 4;
        sc_bigint<42> bz;
        sc_biguint<33> bux = 7;
        sc_int<42> z;
        
       
        // Incorrect as (-3-x) considered as unsigned, no fix here
        z = -3 - x + y;     // Warning reported
        //CHECK(z == -9); // -1 in SV
        
        z = x * y;          // Warning reported
        //CHECK(z == -5);
        // Incorrect as promoted to ULL in SC, no fix here
        z = y / (x + 1);    // Warning reported
        //CHECK(z == -3);  // -2 in SV
        
        bz = (x + y - 1) / bx;  // Warning reported
        cout << bz << endl;
        //CHECK(bz == -2); // -1 in SV
        
        bz = (x - 1 + y) / bx;  // Warning reported
        cout << bz << endl;
        //CHECK(bz == -2); // -1 in SV
    }
    
    // Main binary:: +, -, *, /, %, <<, >>
    void sc_type_main() 
    {
        sc_uint<3> x = 1;
        sc_int<4> y = 5;
        sc_bigint<33> bx = 4;
        sc_biguint<33> bux = 7;
        sc_int<42> z;
        sc_bigint<42> bz;
        
        z = x + y;              // Warning reported
        //CHECK(z == 6);
        z = 3*x + y*2 + 2*3;    // Warning reported
        //CHECK(z == 19);
        z = x + 3 + y + 0;      // Warning reported
        //CHECK(z == 9);
        
        bz = (x - 1 + y) / bx;  // Warning reported
        cout << bz << endl;
        //CHECK(bz == 1);
        bz = (x / y + bx) / bux;    // Warning reported
        //CHECK(bz == 0);
        bz = x * (bux + 1) + y / bx;    
        cout << bz << endl;
        CHECK(bz == 9);   

        z = (y >> 1) + (bx >> x);
        cout << z << endl;
        CHECK(z == 4);
        z = (y << ((x + 1) >> 1)) * bux;
        cout << z << endl;
        CHECK(z == 70);
        z = (x * y) << (bux.to_int() >> 1); // Warning reported
        cout << z << endl;
        //CHECK(z == 40);
        
        bz = (x + y) - (++bux);     // Warning reported
        cout << bz << endl;
        //CHECK(bz == -2);
        bz = (y % 2) * (14 % bx + 1);
        cout << bz << endl;
        CHECK(bz == 3);
        bz = (y % bx) * (x + 10) / bux;
        cout << bz << endl;
        CHECK(bz == 1);
    }
    
    // All types are signed, check no @signed' in SV
    void sc_type_main_signed() 
    {
        sc_int<3> x = 1;
        sc_int<4> y = 5;
        sc_bigint<33> bx = 4;
        sc_bigint<33> by = 7;
        sc_int<42> z;
        sc_bigint<42> bz;
        
        z = x + y;
        CHECK(z == 6);
        z = 3*x + y*2 + 2*3;
        CHECK(z == 19);
        z = x + 3 + y + 0;
        CHECK(z == 9);
        
        bz = (x - 1 + y) / bx;
        cout << bz << endl;
        CHECK(bz == 1);
        bz = (x / y + bx) / by;
        CHECK(bz == 0);
        bz = x * (by + 1) + y / bx;
        cout << bz << endl;
        CHECK(bz == 9);

        bz = (x * y) << (by.to_int() >> 1);
        cout << bz << endl;
        CHECK(bz == 40);
        bz = (x + y) - (++by);
        cout << bz << endl;
        CHECK(bz == -2);
        bz = (y % bx) * (x + 10) / by;
        cout << bz << endl;
        CHECK(bz == 1);
    }
    
    // Bitwise and logic operations: &, |, ^
    void sc_type_logic_bitwise() 
    {
        int i = 10; 
        unsigned uu = 3;
        sc_uint<5> x = 2;
        sc_int<4> y = 3;
        sc_biguint<10> bx = 7;
        sc_bigint<10> by = 5;
        sc_uint<18> z;

        z = i | uu;         // Warning reported
        //CHECK(z == 11);
        z = i & uu;         // Warning reported
        //CHECK(z == 2);
        z = i ^ uu;         // Warning reported
        //CHECK(z == 9);

        z = x | uu + bx & by;
        cout << z << endl;
        CHECK(z == 2);
        z = (x ^ (uu + 1)) + (y | bx | by);
        cout << z << endl;
        CHECK(z == 13);
        z = (x & 11) * (10 | (2*i));    // Warning reported
        cout << z << endl;
        //CHECK(z == 60);
        
        bool b;
        b = x && y;
        CHECK(b);
        b = x && !y;
        CHECK(!b);
        
        b = x || y;
        CHECK(b);
        b = x || false;
        CHECK(b);
        b = i == y && bx.to_int() || (x - 1);
        CHECK(b);
        b = !i || (y + bx.to_int()) && true;
        CHECK(b);
    }
    
    // &, |, ^ with literals including >32bit literals
    void logic_bitwise_literal() 
    {
        int i = 0x123; 
        unsigned uu = 3;
        long j = 0x12300000005;
        sc_biguint<100> z;

        z = i | 0x24;
        CHECK(z == 0x127);
        z = i & 0x24;
        CHECK(z == 0x20);
        z = i ^ 0x24;
        CHECK(z == 0x107);

        z = j | 0x3;
        CHECK(z == 0x12300000007);
        z = j & 0x16;
        CHECK(z == 0x4);
        z = j ^ 0x11;
        CHECK(z == 0x12300000014);

        z = j | 0x4400000002;
        CHECK(z == 0x16700000007);
        z = j & 0x4500000004;
        CHECK(z == 0x100000004);

        z = j | 0x140000000000;
        CHECK(z == 0x152300000005);
        z = j & 0x153000000000;
        CHECK(z == 0x12000000000);
    }
    
    void sc_type_comp() 
    {
        int i = 10; 
        unsigned uu = 3;
        sc_uint<5> x = 2;
        sc_int<4> y = 3;
        sc_biguint<10> bx = 7;
        sc_bigint<10> by = 5;

        bool b = x > y;     // Warning reported
        //CHECK(!b);
        b = x != y;         // Warning reported
        //CHECK(b);
        b = x != 1 && bx != i;
        CHECK(b);
        
        b = x == y-1;       // Warning reported
        //CHECK(b);
        b = 0 == y - uu;
        CHECK(b);
        b = x == 2 && bx == uu + 4;
        CHECK(b);
        
        b = x <= y;         // Warning reported
        //CHECK(b);
        b = x <= y && bx > by && true;  // Warning reported
        //CHECK(b);
        
        b = x < y;          // Warning reported
        //CHECK(b);
        b = x < (y+1) && (uu || false); // Warning reported
        //CHECK(b);
    }
    
    // More than 64bit types
    void large_types() 
    {
        __int128_t i = -10;
        __uint128_t j = 10;
        
        i = 140 / j;
        CHECK(i == 14);
        i = j << 2;
        CHECK(i == 40);
        
        sc_bigint<100> x = -11;
        sc_biguint<100> y = 11;
        sc_bigint<100> z;
        
        z = x + y;
        CHECK(z == 0);
        z = x * y;
        cout << z << endl;
        CHECK(z == -121);
        x = y * 2;
        CHECK(x == 22);
        y = x << 50;
        CHECK(y == 24769797950537728ULL);
        
    }

// ---------------------------------------------------------------------------    

    void sc_relational_ops()
    {
    	unsigned rel_a = 0;
    	unsigned rel_b = rel_a-1;
    	sct_assert_const(rel_a==0);
    	sc_uint<32> rel_c = 0;
    	sc_uint<32> rel_d = rel_c-1;
    	int rel_e = rel_d;
    	int rel_f = rel_a;
    	bool rel_less_than = rel_a < rel_b;
    	//HTR sct_assert_const(rel_less_than==1);
    	bool rel_greater_than = rel_d > rel_c;
    	//HTR sct_assert_const(rel_greater_than==1);

    	bool rel_not_eq = rel_d != rel_c;
    	sct_assert_const(rel_not_eq==1);
    	rel_b=rel_a;
        rel_c=rel_d;

    	 bool rel_less_than_eq = rel_a <= rel_b;
    	 bool rel_greater_than_eq = rel_d >= rel_c;
    	 sct_assert_const(rel_less_than_eq==1);
    	 sct_assert_const(rel_greater_than_eq==1);

    }
    
    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
    void sc_fp_bitwise_fns_types(T1 par1, T2 par2, T3 par3, T4 par4, T5 par5, T6 par6, T7 par7) {
        T1 B = par1;
        T2 C = par2;
        T3 i = par3;
        T4 s2 = par4;
        T5 s3 = par5;
        T6 u2 = par6;
        T7 u3 = par7;
        //HTR cout << "Entered function: #" << iter << endl;
        iter++;
    	T7 A0 = B + C; //Addition with assignment
    	cout << hex << "A0 : " << A0 << ". B : " << B << ". C : " << C << endl;
    	// sct_assert_const(A0==(B+C));
    	T7 A1 = B - C; //Subtraction with assignment
    	//HTR cout << "A1 : " << A1 << ". B : " << B << ". C : " << C << endl;
    	// sct_assert_const(A1==(B-C));
    	T7 A2 = B * C; //Multiplication with assignment
    	//HTR cout << "A2 : " << A2 << ". B : " << B << ". C : " << C << endl;
    	// sct_assert_const(A2==(B*C));
    	T7 A3 = B / C; //Division with assignment
    	//HTR cout << "A3 : " << A3 << ". B : " << B << ". C : " << C << endl;
    	// sct_assert_const(A3==(B/C));
    	T7 A4 = B << i; //Left shift with assignment
    	//HTR cout << "A4 : " << A4 << ". B : " << B << ". i : " << i << endl;
    	// sct_assert_const(A4==(B<<i));
    	T7 A5 = B >> i; //Right shift with assignment
    	//HTR cout << "A5 : " << A5 << ". B : " << B << ". i : " << i << endl;
    	// sct_assert_const(A5==(B>>i));
    	T7 s10 = s2 & s3; //Bitwise and with assignment for signed operands
    	//HTR cout << "s10 : " << s10 << ". s2 : " << s2 << ". s3 : " << s3 << endl;
    	// sct_assert_const(s10==(s2 & s3));
    	T7 s11 = s2 | s3; //Bitwise or with assignment for signed operands
    	//HTR cout << "s11 : " << s11 << ". s2 : " << s2 << ". s3 : " << s3 << endl;
    	// sct_assert_const(s11==(s2 | s3));
    	T7 s12 = s2 ^ s3; //Bitwise exclusive-or with assignment for signed operands
    	//HTR cout << "s12 : " << s12 << ". s2 : " << s2 << ". s3 : " << s3 << endl;
    	// sct_assert_const(s12==(s2 ^ s3));
    	T7 u10 = u2 & u3; //Bitwise and with assignment for unsigned operands
    	//HTR cout << "u10 : " << u10 << ". u2 : " << u2 << ". u3 : " << u3 << endl;
    	// sct_assert_const(u10==(u2 & u3));
    	T7 u11 = u2 | u3; //Bitwise or with assignment for unsigned operands
    	//HTR cout << "u11 : " << u11 << ". u2 : " << u2 << ". u3 : " << u3 << endl;
    	// sct_assert_const(u11==(u2 | u3));
    	T7 u12 = u2 ^ u3; //Bitwise exclusive-or with assignment for unsigned operands
    	//HTR cout << "u12 : " << u12 << ". u2 : " << u2 << ". u3 : " << u3 << endl;
    	// sct_assert_const(u12==(u2 ^ u3));
    }

    void sc_fp_bitwise_fns()
    {
    	sc_fp_bitwise_fns_types(sc_uint<10>(5), sc_uint<12>(1), sc_uint<22>(3), sc_uint<30>(12),
                                sc_uint<16>(90), sc_uint<14>(64), sc_uint<22>(14));
    	sc_fp_bitwise_fns_types(sc_int<13>(15), sc_int<12>(11), sc_int<12>(2), sc_int<12>(22),
                                sc_int<10>(40), sc_int<12>(44), sc_int<32>(24));
    	sc_fp_bitwise_fns_types(sc_bigint<12>(5), sc_bigint<12>(1), sc_bigint<12>(3), sc_bigint<12>(12),
                                sc_bigint<10>(90), sc_bigint<12>(64), sc_bigint<32>(14));
    	sc_fp_bitwise_fns_types(sc_biguint<10>(5), sc_biguint<12>(1), sc_biguint<12>(3), sc_biguint<12>(12),
                                sc_biguint<10>(90), sc_biguint<12>(64), sc_biguint<32>(14));
    	int x0;
    	int x1;
    	int x2;
    	int x3;
    	int x4;
    	int x5;
    	int x6;
    	int x7;
    	x0=5;
    	x1=1;
    	x2=1;
    	x3=12;
    	x4=90;
    	x5=64;
    	x6=14;

        for (int loopcount =0; loopcount < 10; loopcount++) {
        	x0+=5;
        	x1+=1;
        	x2+=1;
        	x3+=12;
        	x4+=90;
        	x5+=64;
        	x6+=14;
        	sc_fp_bitwise_fns_types(sc_biguint<20>(x0), sc_biguint<32>(x1), sc_biguint<52>(x2), sc_biguint<72>(x3),
                                    sc_biguint<20>(x4), sc_biguint<32>(x5), sc_biguint<52>(x6));

        }
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

