#include "systemc.h"
#include "sct_assert.h"


// SC type specific unary operations
class A : public sc_module 
{
public:
    sc_in<bool>         a{"a"};
    sc_out<bool>        b{"b"};
    sc_out<bool>        c{"c"};
    sc_out<bool>*       p;
    
    int                 m;
    int                 k;
    int*                q;
    
    sc_uint<3>          z;
    sc_uint<3>*         pz;

    sc_signal<bool> dummy{"dummy"};

    SC_CTOR(A)
    {
        p = new sc_out<bool>("p");
        q = sc_new<int>();
        pz = sc_new<sc_uint<3>>();
        
        SC_METHOD(and_reduce1); sensitive << dummy;
        SC_METHOD(or_reduce1); sensitive << dummy;
        SC_METHOD(xor_reduce1); sensitive << dummy;

        SC_METHOD(sc_types_exclaim); sensitive << dummy;
        SC_METHOD(sc_types_inc_dec); sensitive << dummy;
        SC_METHOD(sc_types_comp_assign); sensitive << dummy;

        SC_METHOD(sc_to_int); sensitive << dummy;
    }
    

     void and_reduce1() {
        sc_uint<4> x1 = 0xC;
        sct_assert_const(nand_reduce(x1));
        sct_assert_const(x1.nand_reduce());

        sc_uint<8> x2 = 0xFF;
        sc_uint<8> x3 = 0xFA;
        bool l1 = x1.and_reduce();
        bool l2 = x2.nand_reduce();
        
        sct_assert_const(!l1);
        sct_assert_const(!l2);
        sct_assert_const(and_reduce(x2));
        sct_assert_const(x3.nand_reduce());
    }     
    
     void or_reduce1() {
        sc_uint<4> x1 = 0xC;
        sct_assert_const(!x1.nor_reduce());
        sct_assert_const(!nor_reduce(x1));

        sc_uint<8> x2 = 0x00;
        sc_uint<8> x3 = 0x10;
        bool l1 = or_reduce(x1);
        bool l2 = x2.nor_reduce();
        
        sct_assert_const(l1);
        sct_assert_const(l2);
        sct_assert_const(!x2.or_reduce());
        sct_assert_const(!x3.nor_reduce());

        bool l3 = ((sc_uint<1>)x2).or_reduce();
        bool l4 = ((sc_uint<2>)l3).or_reduce();
        bool l5 = ((sc_uint<2>)((sc_uint<1>)x2)).or_reduce();
        l5 = ((sc_uint<1>)((sc_uint<2>)x2)).or_reduce();
}     
    
     // Only 0x0 and 0x1 are considered for XOR/XNOR
     void xor_reduce1() {
        sc_uint<4> x1 = 0x1;
        sc_uint<8> x2 = 0x0;
        sct_assert_const(x1.xor_reduce());
        sct_assert_const(!xnor_reduce(x1));

        bool l1 = x2.xor_reduce();
        bool l2 = xnor_reduce(x2);
        
        sct_assert_const(!l1);
        sct_assert_const(l2);
    }     
    
    void sc_types_exclaim() {
        sc_uint<3> x = 1;
        bool b = x;
        sct_assert_const(b);
        x = 0;
        sct_assert_const(!x);
    }
    
    void sc_types_inc_dec() {
        sc_uint<3> x = 1;
        x++;
        sct_assert_const(x == 2);
        ++x;
        sct_assert_const(x == 3);
        --x;
        sct_assert_const(x == 2);
        x--;
        sct_assert_const(x == 1);
    }
    
    void sc_types_comp_assign() {
        sc_uint<3> x = 1;
        x += 2;
        sct_assert_const(x == 3);
        sc_int<8> y = 2;
        y -= x;
        sct_assert_const(y == -1);
        y += x;
        sct_assert_const(y == 2);
        y *= x;
        sct_assert_const(y == 6);
        y = -1;
        x += y;
        sct_assert_const(x == 2);
        x += (y+2);
        sct_assert_const(x == 3);
    }
    
    void sc_to_int() {
        sc_biguint<66> x = 15;
        int i = x.to_int();
        sct_assert_const(i == 15);
        x = -10;
        i = x.to_int64();
        sct_assert_const(i == -10);
        
        x = 12;
        unsigned u = x.to_uint();
        sct_assert_const(u == 12);
        x = 11;
        u = x.to_uint64();
        sct_assert_const(u == 11);
        
        long unsigned ul = x.to_ulong();
        sct_assert_const(ul == 11);
        x = -20;
        long int l = x.to_long();
        sct_assert_const(l == -20);
    }
    
};

class B_top : public sc_module 
{
public:
    sc_signal<bool>  a{"a"};
    sc_signal<bool>  b{"b"};
    sc_signal<bool>  c{"c"};
    sc_signal<bool>  p{"p"};
    
    A a_mod{"a_mod"};

    SC_CTOR(B_top) {
        a_mod.a(a);
        a_mod.b(b);
        a_mod.c(c);
        a_mod.p->bind(p);
    }
};

int sc_main(int argc, char* argv[])
{
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}
