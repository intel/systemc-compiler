/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"
#include "sct_assert.h"

using namespace sc_core;

// Bit and range select special cases
class A : public sc_module {
public:
    // Define variables
    sc_signal<bool> dummy;
    sc_int<8> x;
    sc_bigint<8> y;
    sc_uint<8> ux;
    sc_biguint<8> uy;

    sc_signal<sc_uint<32>> inp;
    sc_signal<sc_uint<32>> outp;
    
    sc_signal<bool> s;

    SC_CTOR(A) {
        
        SC_METHOD(bit_range_rhs); 
        sensitive << inp;
        
        SC_METHOD(bit_in_loop); 
        sensitive << s;
        
        // No bit/range passed as reference to function is possible
        //SC_METHOD(bit_in_fcall); 
        //sensitive << s;

        SC_METHOD(sc_uint_wide); 
        sensitive << as;

        SC_METHOD(sc_uint_ctor_bit); 
        sensitive << as << inp;
        
        SC_METHOD(sc_uint_ctor_range); 
        sensitive << as << inp;

        SC_METHOD(signal_array_part_sel); 
        sensitive << ssig[0] << ssig[1] << inp;

        SC_METHOD(to_int); 
        sensitive << as << inp;

        SC_METHOD(to_int_bit_range); 
        sensitive << as << inp;
        
        SC_METHOD(sc_uint_to_bool); 
        sensitive << as << inp;
        
        SC_METHOD(int_to_bool); 
        sensitive << as << isig;

        SC_METHOD(zero_zero_range); 
        sensitive << as << bs;

        SC_METHOD(zero_zero_range_big); 
        sensitive << cs << ds;

        SC_METHOD(zero_zero_bit); 
        sensitive << as << bs;

        SC_METHOD(zero_zero_bit_big); 
        sensitive << cs << ds;

        SC_METHOD(bit_range_sel); 
        sensitive << inp;
        
        SC_METHOD(bit_range_array); 
        sensitive << inp;
    }
    
    #define CHECK(ARG) sct_assert(ARG); sct_assert_const(ARG);

    void bit_range_rhs() 
    {
        sc_uint<3> i = 3;
        bool b;
        b = i.bit(0);             
        CHECK(b);
        
        b = i.bit(1);
        CHECK(b);
    }
    
    void bit_in_loop() 
    {
        sc_uint<4> xw;
        xw = 0;
        for (int i = 0; i < 4; i++) {
            xw.bit(i) = s.read();
        }
        bool xwZero = (xw == 0);
        sct_assert_unknown(xwZero);    
    }
    
    template<class T>
    void f(T& b) {
        b = 1;
    }
    
    void bit_in_fcall() 
    {
        sc_uint<4> xw;
        xw = 0;
        f(xw.bit(1));       // Not correct
        
        bool xwZero = (xw == 0);
        sct_assert_unknown(xwZero); 
    }

    // Check no outside of variable width part/bit selection
    void sc_uint_wide() 
    {
        bool b;
        sc_uint<1> k;
        //b = ((sc_uint<2>)k).bit(1);              -- fatal error
        // sc_uint<2> x = ((sc_uint<3>)k).range(2,1);  -- fatal error
    }
   
    // Check construct @sc_uint with/without typecast with @bit()
    void sc_uint_ctor_bit() 
    {
        sc_uint<3> i = 1;
        sc_uint<3> ii = 2;
        sc_uint<2> j = i;
        sc_uint<1> k = i;
        sc_uint<2> y;
        sc_uint<1> z;
        
        bool b;
        b = i;
        CHECK(b);
        b = ((sc_uint<1>)i).bit(0);             
        CHECK(b);
        b = ((sc_uint<2>)i).bit(0); 
        CHECK(b);
        b = ((sc_uint<1>)k).bit(0); 
        CHECK(b);
        b = ((sc_uint<2>)k).bit(0);
        CHECK(b);
        b = ((sc_uint<1>)inp.read()).bit(0);    
        b = (sc_uint<1>)(inp.read().bit(1));
        
        b = ii;
        CHECK(b);
        b = ii.bit(0);             
        CHECK(!b);
        b = (sc_uint<1>)ii;             
        CHECK(!b);
        b = (sc_uint<2>)ii;  
        CHECK(b);
        b = ((sc_uint<1>)ii).bit(0);             
        CHECK(!b);
        
        
        y = ((sc_uint<1>)i).bit(0);  
        CHECK(y == 1);
        y = ((sc_uint<2>)i).bit(0); 
        CHECK(y == 1);
        y = ((sc_uint<1>)k).bit(0); 
        CHECK(y == 1);
        y = ((sc_uint<2>)k).bit(0);
        CHECK(y == 1);
        //y = ((sc_uint<2>)k).bit(1); -- fatal error reported
        // y = ((sc_uint<1>)k).bit(1); -- fatal error reported
        
        y = ii.bit(0);  
        CHECK(y == 0);
        y = ii.bit(1);  
        CHECK(y == 1);
        y = (sc_uint<1>)ii;  
        CHECK(y == 0);
        y = (sc_uint<2>)ii;  
        CHECK(y == 2);
        y = ((sc_uint<1>)ii).bit(0);  
        CHECK(y == 0);

        z = ((sc_uint<1>)i).bit(0);  
        CHECK(z == 1);
        z = ((sc_uint<2>)i).bit(0); 
        CHECK(z == 1);
        z = ((sc_uint<1>)k).bit(0); 
        CHECK(z == 1);
        z = ((sc_uint<2>)k).bit(0);
        CHECK(z == 1);
        
        z = ii;
        CHECK(z == 0);
        
        k = ii;
        CHECK(k == 0);
    }
    
     // Check construct @sc_uint with/without typecast with @range()
    void sc_uint_ctor_range() 
    {
        sc_uint<3> i = 1;
        sc_uint<3> ii = 2;
        sc_uint<1> k = i;
        sc_uint<2> y;
        sc_uint<1> z;
        
        bool b;
        b = i;
        CHECK(b);
        b = ((sc_uint<1>)i).range(0,0);             
        CHECK(b);
        b = ((sc_uint<2>)i).range(0,0); 
        CHECK(b);
        b = ((sc_uint<1>)k).range(0,0);
        CHECK(b);
        b = ((sc_uint<2>)k).range(0,0);
        CHECK(b);
        b = ((sc_uint<1>)inp.read()).range(0,0);    
        b = (sc_uint<1>)(inp.read().range(1,1));
        
        b = ii;
        CHECK(b);
        b = ii.range(0,0);
        CHECK(!b);
        b = (sc_uint<1>)ii;             
        CHECK(!b);
        b = (sc_uint<2>)ii;  
        CHECK(b);
        b = ((sc_uint<2>)ii).range(0,0);  
        CHECK(!b);
        b = ((sc_uint<2>)ii).range(1,1);  
        CHECK(b);
        b = ((sc_uint<2>)ii).range(1,0);  
        CHECK(b);
        b = ((sc_uint<1>)ii).range(0,0);
        CHECK(!b);
        
        
        y = ((sc_uint<1>)i).range(0,0);
        CHECK(y == 1);
        y = ((sc_uint<2>)i).range(0,0);
        CHECK(y == 1);
        y = ((sc_uint<1>)k).range(0,0);
        CHECK(y == 1);
        y = ((sc_uint<2>)k).range(0,0);
        CHECK(y == 1);
        //y = ((sc_uint<2>)k).range(1, 0); -- fatal error reported
        //y = ((sc_uint<2>)k).range(1, 1); -- fatal error reported
        //y = ((sc_uint<1>)k).range(1, 1); -- fatal error reported
        //y = ((sc_uint<1>)k).range(1, 0); -- fatal error reported
        
        y = ii.range(0,0);
        CHECK(y == 0);
        y = ii.range(1,1);
        CHECK(y == 1);
        y = ii.range(1,0);
        CHECK(y == 2);
        y = (sc_uint<1>)ii;  
        CHECK(y == 0);
        y = (sc_uint<2>)ii;  
        CHECK(y == 2);
        y = ((sc_uint<2>)ii).range(0,0);  
        CHECK(y == 0);
        y = ((sc_uint<2>)ii).range(1,1);  
        CHECK(y == 1);
        y = ((sc_uint<2>)ii).range(1,0);  
        CHECK(y == 2);
        y = ((sc_uint<1>)ii).range(0,0);
        CHECK(y == 0);

        z = ((sc_uint<1>)i).range(0,0);
        CHECK(z == 1);
        z = ((sc_uint<2>)i).range(0,0);
        CHECK(z == 1);
        z = ((sc_uint<1>)k).range(0,0);
        CHECK(z == 1);
        z = ((sc_uint<2>)k).range(0,0);
        CHECK(z == 1);
        
        z = ii;
        CHECK(z == 0);
        
        k = ii;
        CHECK(k == 0);
    }
    
    sc_signal<sc_uint<8>> ssig[2];
    void signal_array_part_sel() 
    {
        bool b = ssig[1].read().bit(2);
        sc_uint<5> x = ssig[1].read().range(4,3);
        
        int i = inp.read();
        x = ssig[i].read().range(i+1,i);
    }

    // SC types to_int(), to_uint(), ...
    void to_int() 
    {
        int a = -3;
        sc_uint<39> u = (0x11UL << 32) + 0x1;
        sc_int<41> i = (-0x22L << 32) - 0x2;
        sc_biguint<47> bu = 0x23;
        sc_bigint<67> bi = 0x104;
        sc_bigint<90> z;
        
        z = u;
        cout << hex << "z " << z << endl;
        CHECK(z == (0x11UL << 32) + 0x1);
        z = u.to_int();
        cout << "z " << z << endl;
        CHECK(z == 0x1);
        z = u.to_int();
        CHECK(z == 0x1);
        z = u.to_int64();
        CHECK(z == (0x11UL << 32) + 0x1);

        z = i;
        cout << "z " << z << endl;
        CHECK(z == (-0x22L << 32) - 0x2);
        z = i.to_int();
        cout << "z " << z << endl;
        CHECK(z == -0x2);
        z = i.to_long();
        cout << "z " << z << endl;
        CHECK(z == (-0x22L << 32) - 0x2);

        z = u.to_int() + u.to_int64() + u.to_long();
        z = u.to_uint() + u.to_uint64() + u.to_ulong();
        
        z = i.to_int() + i.to_int64() + i.to_long();
        z = i.to_uint() + i.to_uint64() + i.to_ulong();     // warning reported
        
        z = bu.to_int() + bu.to_int64() + bu.to_long();
        z = bu.to_uint() + bu.to_uint64() + bu.to_ulong();

        z = bi.to_int() + bi.to_int64() + bi.to_long();
        z = bi.to_uint() + bi.to_uint64() + bi.to_ulong();  // warning reported
    }
    
    // SC bit/range to_int(), to_uint(), ...
    void to_int_bit_range() 
    {
        sc_uint<45> u = (0x12UL << 32) + 0x1;
        sc_int<41> i = (-0x22L << 32) - 0x2;
        sc_biguint<47> bu = 0x23;
        sc_bigint<67> bi = 0x104;
        sc_bigint<90> z;
        
        z = u.range(37,0);
        cout << "z " << z << endl;
        CHECK(z == (0x12UL << 32) + 0x1);
        z = u.range(37,0).to_int();
        cout << "z " << z << endl;
        CHECK(z == 0x1);
        z = u.range(37,1).to_int();
        cout << "z " << z << endl;
        CHECK(z == 0);
    }
    
    sc_signal<bool> sig;
    void sc_uint_to_bool() 
    {
        sc_uint<2> i = 1;
        sc_int<5> j = 1;
        sc_biguint<17> bi = (unsigned)inp.read();
        bool b = as.read();
        b = j;
        b = j.range(3,1);
        CHECK(!b);
        j = 2;        
        b = j.bit(1);
        CHECK(b);
        
        j = 4;        
        b = j.bit(2).to_bool();
        CHECK(b);
        b = j.bit(1).to_bool();
        CHECK(!b);
        
        sig = i;
        sig = bi.to_uint();
        sig.write(as.read());
        sig = as.read();
        
        sig = i.range(1,0);
        sig = j.range(3,1);
        sig = j.bit(4);
        sig = bi.bit(16).to_bool();
        sig = bi.range(3,1).to_int();
    }
    
    sc_signal<bool> bsig;
    sc_signal<int> isig;
    void int_to_bool() 
    {
        int i = 1;
        bool b = i;
        const int c = 1;
        b = c;
        b = 2-c;
        b = isig.read();
        b = (sc_uint<1>)i;
        
        sc_uint<2> x = 1;
        b = x;
        b = (sc_uint<1>)x;
        b = (bool)x;
        
        bsig = i;
        bsig.write(isig.read());
        bsig = isig.read();
    }
    
    // Bug from JBL MD, [0 : 0] range for signal read
    static const unsigned WIDTH = 1;
    typedef sc_uint<WIDTH> A_t;
    typedef sc_int<WIDTH> B_t;
    typedef sc_biguint<WIDTH> C_t;
    typedef sc_bigint<WIDTH> D_t;

    sc_signal<A_t> as;
    sc_signal<B_t> bs;
    sc_signal<C_t> cs;
    sc_signal<D_t> ds;

    void zero_zero_range() 
    {
        A_t a;
        A_t aa = a.range(WIDTH-1, 0);
        a.range(WIDTH-1, 0) = 1;
        bool ab = as.read().range(WIDTH-1, 0);
        sc_uint<5> j = as.read().range(WIDTH-1, 0);
        sc_biguint<5> jj = (sc_biguint<5>)as.read().range(WIDTH-1, 0);
        
        B_t b;
        B_t bb = b.range(WIDTH-1, 0);
        b.range(WIDTH-1, 0) = 1;
        int i = bs.read().range(WIDTH-1, 0);
        i = bs.read().range(WIDTH-1, 0) + 1;
    }
    
    void zero_zero_range_big() 
    {
        C_t c;
        C_t cc = c.range(WIDTH-1, 0);
        c.range(WIDTH-1, 0) = 1;
        bool ab = cs.read().range(WIDTH-1, 0) != 0;
        sc_uint<5> j = (sc_uint<5>)cs.read().range(WIDTH-1, 0);
        sc_biguint<5> jj = cs.read().range(WIDTH-1, 0) + j;
        
        D_t d;
        D_t dd = d.range(WIDTH-1, 0);
        d.range(WIDTH-1, 0) = 1;
        int i = ds.read().range(WIDTH-1, 0).to_int();
        i = ds.read().range(WIDTH-1, 0).to_long() + 1;
    }

    void zero_zero_bit() 
    {
        A_t a;
        bool aa = a.bit(0);
        a.bit(WIDTH-1) = 1;
        bool ab = as.read().bit(WIDTH-1);
        int j = as.read().bit(WIDTH-1);
        long jj = as.read().bit(WIDTH-1);
    }
    
    void zero_zero_bit_big() 
    {
        C_t c;
        bool cc = c.bit(WIDTH-1);
        c.bit(WIDTH-1) = 1;
        bool ab = cs.read().bit(WIDTH-1) != 0;
        sc_uint<5> j = (sc_uint<5>)cs.read().bit(WIDTH-1);
        sc_biguint<5> jj = cs.read().bit(WIDTH-1) + j;
    }
    
    void bit_range_sel() 
    {
        x[1] = y[0];
        ux[1] = uy[0];

        x(3,2) = y(2,1);
        x(5,3) = y(4,2);

        ux(3,2) = uy(2,1);
        ux(5,3) = uy(4,2);
    }
    
    const unsigned MC = 2;
    static const unsigned MSC = 3;
    void bit_range_array() 
    {
        const unsigned LC = 4;
        sc_uint<4>      uarr[3];
        sc_biguint<65>  barr[3];
        
        uarr[0].range(2,1) = 1;
        bool b = uarr[MC].bit(0) || uarr[MSC-1].bit(3);
        b = uarr[LC-MSC].range(1,0);
        barr[MC-1].range(64,63) = uarr[MC-2].range(1,0);
    }
    
};

int sc_main(int argc, char *argv[]) {
    A a_mod{"a_mod"};
    sc_start();
    return 0;
}

