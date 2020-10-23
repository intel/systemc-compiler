#include "systemc.h"
#include <iostream>

using namespace sc_core;

// Concatenation operations for SC and C++ types

// sc_int< 8 > x ;
// sc_bigint< 8 > y ;
// x[3] = y[2] // Legal
// (x+x)[3] = 0 ; // Illegal, as x+x is promoted to a native C++ type
// (y+y)[3] = 0 ; // Legal as y+y is still a sc_bigint
// (y,y)[3] = 0 ; // Illegal as concatenation doesn’t support bitref

class A : public sc_module {
public:
    // Define variables
    sc_signal<bool> dummy;
    sc_uint<8> x1;
    sc_uint<8> x2;
    sc_uint<8> y1;
    sc_uint<8> y2;

    sc_in<sc_uint<32>> i1;
    sc_in<sc_uint<32>> i2;
    sc_in<sc_uint<32>> i3;
    sc_in<sc_uint<32>> i4;
    sc_out<sc_uint<32>> o1;
    sc_out<sc_uint<32>> o2;
    sc_out<sc_uint<32>> o3;
    sc_out<sc_uint<32>> o4;
    sc_signal<sc_int<32>> o5;
    sc_signal<sc_int<32>> o6;
    sc_signal<sc_int<32>> o7;
    sc_signal<sc_int<32>> o8;

    sc_int<4> t;
    sc_bigint<4> tb;
    sc_int<4> x;
    sc_bigint<4> xb;

    SC_CTOR(A) {
        SC_METHOD(concat_cpp); 
        sensitive<< s1 << s2 << s3 << s4 << s5 << s6;

        SC_METHOD(concat_compl); 
        sensitive<< s1 << s2 << s3 << s4 << s5 << s6;
        
        SC_METHOD(bit_range_sel); 
        sensitive<< i1 << i2;

        SC_METHOD(bit_range_sel2); 
        sensitive<< i1;

        SC_METHOD(bit_range_sel3); 
        sensitive<< i1;
    }
    
    sc_signal<bool>     s1;
    sc_signal<char>     s2;
    sc_signal<int>      s3;
    sc_signal<unsigned> s4;
    sc_signal<long>     s5;
    sc_signal<sc_biguint<4>>  s6;

    // Concat and non-intended comma for SC and CPP types mix
    void concat_cpp() 
    {
        bool b = s1.read();
        bool c = s1.read();
        int i = s3.read();
        sc_uint<1> y = s2.read();
        sc_biguint<33> by = s5.read();
        sc_uint<16> z;
        sc_biguint<40> bz;
        
        // Concat SC and bool
        z = concat(b, y);
        z = (b, sc_uint<1>(0));
        z = (y, 1, b);
        
        // Concat SC and int, warning reported
        z = (y, i);
        z = (i, y);
        z = (s3, y); 
        z = (y, s3.read());
        z = (y, s4);
        z = (s5.read(), y, s4);
        
        // Non-intended comma, C++ compiler warning
        z = (b, 1); 
        z = (b, c);         
        z = (i, c); 
        z = (s1, s2);
        z = (i, s2);
        z = (s3, s4, s5, y);
        
        // Big types
        bz = concat(by, 11);
        bz = (by, s5);
        bz = (by, y);
        bz = (by, i);
        bz = (b, by);
        bz = (s4.read(), by);
        bz = (4, by, i);
    }
    
    // Multiple concat in one expression
    void concat_compl() 
    {
        bool b = s1.read();
        bool c = s1.read();
        int i = s3.read();
        sc_uint<1> y = s2.read();
        sc_uint<12> yy = s2.read();
        sc_biguint<33> by = s5.read();
        sc_uint<16> z;
        sc_biguint<40> bz;
        
        z = ((c, y), (s1, yy));
        z = ((i, yy), yy);
        
        bz = (yy++, by);
        bz = (by, sc_uint<5>(11));
    }
    
    void bit_range_sel() 
    {
        t = 1;
        x = 2;
        tb = 1;
        xb = 2;

        y1=i1.read();
        y2=i2.read();

        (x1,x2) = (y1,y2);

        o1 = x1;
        o2 = x2;
    }
    
    void bit_range_sel2() 
    {
        t = 1;
        x = 2;

        sc_uint<8> a = (t, x);
        // Type conversion required, else it is casted to boolean in Clang AST
        o3 = (t, sc_uint<4>(t*x)); 
        o4 = (sc_uint<4>(t),sc_uint<4>(t*x));
    }

    void bit_range_sel3() 
    {
        t = 1;
        x = 2;
        tb = 1;
        xb = 2;

        o6 = (t, sc_uint<4>(tb*xb));
        o7 = (sc_uint<3>(x >> 1), t);
        o8 = (t, sc_biguint<5>(xb >> 1));
    }
    
};

class B_top : public sc_module {
public:
    sc_signal<sc_uint<32>> i1;
    sc_signal<sc_uint<32>> i2;
    sc_signal<sc_uint<32>> i3;
    sc_signal<sc_uint<32>> i4;
    sc_signal<sc_uint<32>> o1;
    sc_signal<sc_uint<32>> o2;
    sc_signal<sc_uint<32>> o3;
    sc_signal<sc_uint<32>> o4;

    A a_mod{"a_mod"};
    
    SC_CTOR(B_top) {
        a_mod.i1(i1);
        a_mod.i2(i2);
        a_mod.i3(i3);
        a_mod.i4(i4);
        a_mod.o1(o1);
        a_mod.o2(o2);
        a_mod.o3(o3);
        a_mod.o4(o4);
    }
};

int sc_main(int argc, char *argv[]) 
{
    B_top bmod{"b_mod"};
    sc_start();
    return 0;
}
