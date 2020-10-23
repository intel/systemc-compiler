#include <sct_assert.h>
#include <systemc.h>
#include <stdint.h>
#include <iostream>
#include <string>

using namespace sc_core;

// Constant evaluation in binary statements
class A_mod : public sc_module {
public:
    
    SC_HAS_PROCESS(A_mod);
    const unsigned A = 1;
    static const unsigned B = 2;
    static const unsigned Z = 0;
    const int C = -1;
    static const long D = -2;
    
    const sc_uint<8> K = 0x11;
    const sc_int<8> L = 0x11;
    const sc_int<8> M = -42;
    const sc_biguint<64> N = 0x555;
    const sc_bigint<64> P = -0x555;

    sc_signal<sc_uint<4>> s{"s"};
    
    A_mod(const sc_module_name& name) : sc_module(name) 
    {
        SC_METHOD(big_shift); sensitive << s;
        
        SC_METHOD(concat_const); sensitive << s;
        SC_METHOD(concat_const_var); sensitive << s;
        
        SC_METHOD(concat_const_cast); sensitive << s;
    }

    void big_shift() {
        sc_biguint<131> a = (sc_biguint<131>)1 << 130;
        sc_biguint<142> b = (a + 3) << 10;
        sc_biguint<139> c = b - a + 1;
        sc_uint<20> d = c >> 120;
        
        sct_assert_const(d == 0x07fc00);
        
        /*
        std::cout << hex;
        a.print(std::cout); std::cout << std::endl;
        b.print(std::cout); std::cout << std::endl;
        c.print(std::cout); std::cout << std::endl;
        d.print(std::cout); std::cout << std::endl;*/
    }    
    
    // Constant and constant
    void concat_const() 
    {
        sc_uint<4> a = ((sc_uint<2>)1, sc_uint<2>(2));
        sct_assert_const(a == 6);
        
        a = ((sc_uint<2>)(A+1), sc_uint<2>(B));
        sct_assert_const(a == 10);

        // Narrowing and widening
        a = ((sc_uint<3>)1, sc_uint<1>(2));
        sct_assert_const(a == 2);
        a = ((sc_uint<3>)(A), sc_uint<1>(B));
        sct_assert_const(a == 2);

        sc_uint<16> b = (K, L);
        sct_assert_const(b == 0x1111);
        
        b = (K, (sc_uint<4>)L);
        sct_assert_const(b == 0x111);
        
        b = (sc_uint<4>(K), L);
        std::cout << hex << b << std::endl;
        sct_assert_const(b == 0x111);

        b = (sc_uint<2>(K), sc_uint<10>(L));
        std::cout << hex << b << std::endl;
        sct_assert_const(b == 0x411);

        sc_biguint<72> d = (N, K);
        std::cout << hex << d << std::endl;
        sct_assert_const(d == 0x55511);
    }

    // Constant and variable
    void concat_const_var() 
    {
        sc_uint<4> a = s.read();
        sc_uint<8> b = ((sc_uint<2>)1, a);
        b = (a, (sc_uint<4>)0xFF);
        b = (a, (sc_uint<4>)(254 + 1));
        sc_uint<16> c = (b, K);
        c = (K, a);
        
        sc_uint<16> d = (sc_uint<11>)K;
        d = (sc_uint<11>)a;
        d = ((sc_uint<11>)K, (sc_uint<3>)a);
        d = ((sc_uint<3>)K, (sc_uint<5>)a);
        
        
        sc_uint<20> e = (sc_uint<11>)(K + a*B);
        e = (sc_uint<11>)((sc_uint<3>)K + (B >> 2));
        e = (sc_uint<7>)((sc_uint<7>)a + sc_uint<14>(K));
    }
    
    // Constant with multiple casts
    void concat_const_cast() 
    {
        sc_uint<16> a = ((sc_uint<8>)((sc_uint<4>)0x55), 
                        (sc_uint<8>)((sc_uint<4>)0x11));
        std::cout << hex << a << std::endl;
        sct_assert_const(a == 0x501);
        
        a = ((sc_uint<1>)1, (sc_uint<6>)((sc_uint<4>)(K+1)), 
             (sc_uint<6>)(sc_uint<4>(L)));
        std::cout << hex << a << std::endl;
        sct_assert_const(a == 0x1081);
        
        sc_uint<7> b = (sc_uint<8>)N;
        std::cout << hex << b << std::endl;
        sct_assert_const(b == 0x55);
        
        b = (sc_uint<6>)N;
        std::cout << hex << b << std::endl;
        sct_assert_const(b == 0x15);
    }
};


int sc_main(int argc, char *argv[]) {
    A_mod a_mod{"a_mod"};
    sc_start();
    return 0;
}
