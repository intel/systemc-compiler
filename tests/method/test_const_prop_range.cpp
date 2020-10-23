#include "systemc.h"
#include <sct_assert.h>


// Constant evaluation for range(), concat() and bit() 
class A : public sc_module
{
public:
    sc_in<bool>         clk{"clk"};
    sc_in<bool>         nrst{"nrst"};
    sc_in<sc_uint<8>>   a{"a"};
    sc_uint<4>          x;
    
    SC_CTOR(A)
    {
        SC_METHOD(bitConstProp); sensitive << a;
        SC_METHOD(rangeConstProp); sensitive << a;
        SC_METHOD(concatConstProp); sensitive << a;
        SC_METHOD(concatConstPropRHS); sensitive << a;

        SC_METHOD(constPropLHS); sensitive << a;
    }
    
    void bitConstProp() 
    {
        x = 11;
        sct_assert_const(x.bit(3) == 1);
        x = 11;
        sct_assert_const(x.bit(2) == 0);
        x = 11;
        sct_assert_const(x[1] == 1);
        x = 11;
        sct_assert_const(x[0] == 1);

        x = 11;
        sc_uint<4> y = 5;
        sct_assert_const(y[1] == x[2]);
        x = 11;
        y = 5;
        sct_assert_const(y[0] == x[0]);
    }
    
    void rangeConstProp() 
    {
        x = 11;
        sct_assert_const(x.range(3,0) == 11);
        x = 11;
        sct_assert_const(x.range(2,0) == 3);
        x = 11;
        sct_assert_const(x.range(1,0) == 3);
        x = 11;
        sct_assert_const(x.range(0,0) == 1);
        x = 11;
        sct_assert_const(x(3,2) == 2);
        x = 11;
        sct_assert_const(x(3,1) == 5);
        x = 11;
        sct_assert_const(x(2,1) == 1);

        x = 11;
        sc_uint<4> y = 5;
        sct_assert_const(x(3,2) == y(2,1));
    }
    
    void concatConstProp() 
    {
        sct_assert_const(concat((sc_uint<3>)0x3,(sc_uint<4>)0xA) == 0x3A);
        sct_assert_const(concat(sc_uint<3>(0xA),sc_uint<4>(0x28)) == 0x28);
        sct_assert_const(concat(sc_biguint<9>(0x1AB), sc_uint<4>(0x2)) == 0x1AB2);
        
        sct_assert_const(concat(sc_int<4>(0x3), sc_uint<3>(0x2)) == 0x1A);
        sct_assert_const(concat(sc_int<4>(0x3), sc_int<3>(0x2)) == 0x1A);

        sct_assert_const( (sc_uint<4>(0xA), sc_uint<4>(0xB)) == 0xAB);
        sct_assert_const( (sc_uint<4>(0xA), sc_uint<4>(0xB), sc_uint<4>(0xC)) == 0xABC);
        sct_assert_const( ((sc_uint<4>(0xA), sc_uint<4>(0xB)), sc_uint<4>(0xC)) == 0xABC);
        sct_assert_const( (sc_uint<4>(0xA), sc_uint<4>(0xB), 
                     sc_uint<4>(0xC), sc_uint<4>(0xD)) == 0xABCD);
    }

    void concatConstPropRHS() 
    {
        x = 0xB;
        
        sc_uint<5> y = 0x2;
        sct_assert_const(concat(y,x) == 0x2B);
        sct_assert_const(concat(x,y) == 0x162);

        sc_int<4> z = 3;
        sct_assert_const(concat(z,x) == 0x3B);
        sct_assert_const((z,x) == 0x3B);
        
        sc_uint<8> t = concat(y,x);
        sct_assert_const(t == 0x2B);
        t = (y,x);
        sct_assert_const(t == 0x2B);
        
        sc_uint<12> t2 = (x, sc_uint<4>(y), sc_uint<4>(0xA));
        sct_assert_const(t2 == 0xB2A);
        sct_assert_const((sc_uint<8>(0xCD), t2) == 0xCDB2A);
    }
    
    // bit() and range() in LHS, value is cleared in CPA
    void constPropLHS() 
    {
        x = 2;
        x.bit(0) = 1;
        sct_assert_unknown(x);
        
        x = 3;
        x[0] = 1;
        sct_assert_unknown(x);

        x = 4;
        x.range(2,2) = 1;
        sct_assert_unknown(x);

        x = 5;
        x(1,0) = 1;
        sct_assert_unknown(x);
    }

};

class B_top : public sc_module
{
    sc_signal<sc_uint<8>> a{"a"};
    sc_signal<bool> clk{"clk"};
    sc_signal<bool> nrst{"nrst"};

public:
    A a_mod{"a_mod"};

    SC_CTOR(B_top) {
        a_mod.clk(clk);
        a_mod.nrst(nrst);
        a_mod.a(a);
    }
};

int sc_main(int argc, char* argv[])
{
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}
