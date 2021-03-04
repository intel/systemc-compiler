/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"

using namespace sc_core;

// Local variables in method process body analysis
template <unsigned N>
class A : public sc_module {
public:
    int m;
    int k = 2;
    int n = 3;
    sc_uint<2>*         t;
    sc_uint<2>*         t1;
    sc_uint<2>*         t2;
    sc_uint<4>*         u;
    sc_uint<4>*         u1;
    sc_uint<4>*         u2;
    
    sc_in<sc_uint<2> >      in1;
    sc_out<sc_uint<2> >     out1;
    sc_signal<sc_uint<2> >  sig1;
    
    typedef sc_uint<4> MyUInt4;
    sc_in<MyUInt4 >         in2;
    sc_out<MyUInt4 >        out2;
    sc_signal<MyUInt4 >     sig2;
    sc_signal<sc_uint<3> >  sig_arr[3];
    
    static const unsigned CONST_A = N;
    static const unsigned CONST_B = CONST_A << 1;
    static const unsigned CONST_C = CONST_A + CONST_B;
    
    static constexpr unsigned CEXPR_A = CONST_A;
    static constexpr unsigned CEXPR_B = CEXPR_A << 1;
    static constexpr unsigned CEXPR_C = CEXPR_A + CEXPR_B;
    
    sc_signal<bool> dummy{"dummy"};

    SC_CTOR(A) {
        t = sc_new<sc_uint<2>>(0);
        t1 = sc_new<sc_uint<2>>(0);
        t2 = sc_new<sc_uint<2>>(0);
        u = sc_new<sc_uint<4>>(0);
        u1 = sc_new<sc_uint<4>>(0);
        u2 = sc_new<sc_uint<4>>(0);
        
        SC_METHOD(partial_select_for_type_cast);
        sensitive << in1;
        
        SC_METHOD(loop_range_double); sensitive << dummy;

        SC_METHOD(sc_types); sensitive << dummy;
        SC_METHOD(sc_uint_local); sensitive << dummy;
        SC_METHOD(sc_uint_pointers); sensitive << dummy;
        SC_METHOD(sc_uint_channels); sensitive << in1 << in2 << sig1 << sig2;
        SC_METHOD(sc_uint_bit); sensitive << dummy;
        SC_METHOD(sc_uint_range); sensitive << dummy;
        SC_METHOD(sc_uint_concat); sensitive << dummy;
        SC_METHOD(sc_uint_reduce); sensitive << sig1;

        SC_METHOD(sc_variable_cast); sensitive << dummy;
        SC_METHOD(sc_literal_cast); sensitive << dummy;
        SC_METHOD(sc_channel_cast); sensitive << in1 << out1 << sig1 << sig_arr[2];
    }

    // SC type range/bit for variable under type cast --
    // BUG from real design fixed
    void partial_select_for_type_cast() 
    {
        sc_uint<4> a = 3;
        bool b = ((sc_uint<3>)a).bit(1);
        b = ((sc_uint<1>)in1.read()).bit(0);
        b = (sc_uint<1>)in1.read();
        
        int i = ((sc_uint<4>)a).range(2,0);
        i = ((sc_uint<2>)in1.read()).range(1,0);
        i = (sc_uint<1>)in1.read();
        
        sc_uint<6> c = ((sc_uint<4>)a).range(3,0);
        i = ((sc_uint<5>)(c)).range(2,1) + a.range(1,0);
    }
    
    // Double expression in conditional statement assignment to channel range --
    // BUG from real design fixed
    void loop_range_double() 
    {
        sc_uint<32> val;
        sc_uint<32> wval;
        sc_uint<32> rval;
        sc_uint<4> byteEnable;
        bool b;
        
        for (int i = 0; i < 4; i++) {
            val.range(i * 8 + 7, i * 8) = 
                    (byteEnable.bit(i)) ? wval.range(i * 8 + 7, i * 8) : 
                                          rval.range(i * 8 + 7, i * 8);
        }
    }
    
    // C++ types
    void cpp_types() {
        bool b;
        unsigned char uc;
        unsigned short us;
        unsigned u;
        unsigned int u1;
        unsigned long ul;
        unsigned long long ull;

        char c;
        short s;
        int i;
        signed int i1;
        long l;
        long long ll;
        
        uint8_t  u8;
        uint16_t u16;
        uint32_t u32;
        uint64_t u64;

        int8_t  i8;
        int16_t i16;
        int32_t i32;
        int64_t i64;
    }

    // @sc_uint
    void sc_types() {
        int i;
        sc_uint<1>     x = 1;
        sc_uint<2>     y = 2;
        sc_biguint<64> z = 3;
        sc_int<3>      t = 4;
        sc_bigint<64>  w = 5;
    }

    // @sc_uint local variables
    void sc_uint_local() {
        int i;
        sc_uint<2> x;
        i = x;
        x = i + 1;
        
        sc_uint<2>  y = x + i;
        x = y;
        x = y + i;
    }

    // @sc_uint pointers
    void sc_uint_pointers() {
        int i;
        bool b;
        sc_uint<2> x;
        
        x = *t;
        x = *t + 1;
        *t = x;
        *t = x + 1;
        
        // With type cast
        i = *t;
        *t = i;
        
        *t = *u2;
        *u2 = *t;

        *t = b;
        b = *t;
    }
   
    // @sc_uint pointers
    void sc_uint_channels() {
        int i;
        bool b;
        sc_uint<2> x;
        MyUInt4 y;
        
        x = in1.read();
        x = in1;
        
        out1.write(x);
        out1.write(y);
        out1 = x;
        out1 = y;
        
        y = in2;
        y = x + in2.read();
        
        out2.write(y);
        out2.write(x);
        out2.write(in1.read() + y);
        out2 = y;
        out2 = y + in1.read();
        
        sig1 = sig2.read();
        out2 = sig1.read();
        sig2.write(in1.read());
    }    

    // @sc_uint special methods : bit() and operator []
    void sc_uint_bit() {
        int i; bool b;
        sc_uint<2> x; MyUInt4 y;
        
        b = x.bit(0);
        i = t1->bit(1);
        x = (*t1).bit(1);
        
        x.bit(0) = b;
        t1->bit(1) = i;
        (*t1).bit(1) = x;
        
        for (int j = 0; j < 2; j++) {
            x.bit(j) = i;
            y[j+1] = i;
        }

        y[3] = i;
        y[CEXPR_B+1] = x[4-CEXPR_C];
    }
    
    // @sc_uint special methods : range() and operator ()
    void sc_uint_range() {
        int i; bool b;
        sc_uint<2> x; sc_uint<32> y;
        
        i = x.range(1, 0);
        i = u->range(3, 0);
        (*u).range(3,0) = i;
        u->range(3,2) = (*t2).range(1, 0);
        y = u->range(3, 0);

        i = x(1, 0);
        y = u->range(3, 3);
        y = u->operator() (3, 3);
        
        for (int j = 0; j < 2; j++) {
            y.range(3,0) = i;
            y.range(j+1,j) = i;
            y.range(j+CONST_C,j) = i;
            y.range(CONST_C,CONST_B) = i;
            y.range(8*j+CONST_C,8*j) = i;
            y.range(8*(j+1),8*j) = i;
            
            i = y.range(CEXPR_B, CEXPR_A);
            i = y.range(CEXPR_B+j, CEXPR_A+j);
            i = y.range(CEXPR_B*j+CEXPR_C, 2*j);
            i = y.range(5*j+3, (CEXPR_B+CEXPR_C)*j);
            i = y.range(3*j+3+4, (2+1)*j+5);
            i = y.range(3*j+3, 3*j+2);

            y(8*j+1, 8*j) = i;
            i = y(4-j, 3-j);
        }
    }
    
    // @sc_uint special methods : concat() and operator ,
    void sc_uint_concat() {
        int i; bool b;
        sc_uint<2> x; sc_uint<2> z; 
        MyUInt4 y;
        
        y = concat(x, z);
        concat(x, z) = y;
        
        y = (x, z);
        i = (x, y, z);
        (x, z) = i;
        (x, y, z) = y;

        // Removing brackets test
        (i) = (y + (z + (x)));
        ((i)) = ((y) + (z + x));
    }     
    
    // @sc_uint special methods : and_reduce() and or_reduce()
    void sc_uint_reduce() {
        int i; 
        sc_uint<2> x; 
        MyUInt4 y;
        
        bool b = and_reduce(x);
        b = or_reduce(x);
        b = xor_reduce(x) || and_reduce(y);
        b = nand_reduce(*u1) | nor_reduce(x);
        
        b = sig1.read().and_reduce();
        b = sig1.read().or_reduce();
        b = sig1.read().xor_reduce();
    }   
    
    // type cast to get bit/range of argument
    void sc_variable_cast() {
        int i; bool b;
        sc_uint<2> x;
        sc_uint<4> y; 

        // Cast in assignment
        x = (sc_uint<2>)y;
        x = sc_uint<2>(y);
        x = static_cast<int>(y);
        x = (unsigned)(y) + long(y) + short(y) + char(y) + bool(y);
        
        // Cast in operations
        x = (sc_uint<2>)y + 2*(-(sc_uint<2>)y);
        x = (sc_uint<2>)(x+y);
        x = (sc_uint<2>)y.range(3,1);
        x = (sc_uint<2>)((sc_uint<3>)(y));
    }   
    
    // type cast for literals
    void sc_literal_cast() {
        int i; bool b;
        sc_uint<2> x = (sc_uint<2>)5;
        x = (sc_uint<2>)3 + (bool)x;
        sc_uint<4> y = 1+(sc_uint<2>)(3+1); 
    }

    // type cast for ports/signals
    void sc_channel_cast() {
        sc_uint<2> x;
        sig1 = (sc_uint<3>)in1.read() + (sc_uint<1>)out1.read();
        sig1.write(sc_uint<2>(sig1.read()));
        x = (bool)sig1.read() + static_cast<int>(in1.read());
        
        sig1.write((sc_uint<2>)sig_arr[2].read());
    }    
};

class B_top : public sc_module {
public:
    sc_signal<sc_uint<2> >      s1;
    sc_signal<sc_uint<4> >      s2;

    A<1> a_mod{"a_mod"};

    SC_CTOR(B_top) {
        a_mod.in1(s1);
        a_mod.out1(s1);
        a_mod.in2(s2);
        a_mod.out2(s2);
    }
};

int sc_main(int argc, char *argv[]) {
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}

