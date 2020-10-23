#include "systemc.h"
#include "sct_assert.h"


// C++ type unary operations
class A : public sc_module 
{
public:
    sc_in<bool>         a{"a"};
    sc_out<bool>        b{"b"};
    sc_out<bool>        c{"c"};
    sc_out<bool>*       p;
    
    int                 m;
    int                 k;

    sc_signal<bool> dummy{"dummy"};

    SC_CTOR(A)
    {
        SC_METHOD(narrowCastNeg); sensitive << dummy;
        SC_METHOD(negativeLiterals); sensitive << dummy;
        
        SC_METHOD(increment1); sensitive << dummy;
        SC_METHOD(increment2); sensitive << a;
        SC_METHOD(plus1); sensitive << a;
        SC_METHOD(not1); sensitive << a << b;
        
        SC_METHOD(not2); sensitive << a << b;
    }
    
    static const int NEG = -2;
    static const long int NEGL = -211;

    void narrowCastNeg()
    {
        // -29
        sc_int<8> k1 = -541;
        cout << "k1 " << k1 << endl;
        sct_assert_const(k1 == -29);
        sct_assert(k1 == -29);

        // -48
        sc_bigint<8> k2 = (sc_int<8>)NEGL * 14 + NEGL + (sc_bigint<16>)NEGL;
        cout << "k2 " << k2 << endl;
        sct_assert_const(k2 == -48);
        sct_assert(k2 == -48);
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
        sct_assert_const(i == 2);
        sct_assert(i == 2);
        
        sc_bigint<8> j;
        j = (sc_bigint<8>)NEGL;
        sct_assert_const(j == 45);
        sct_assert(j == 45);
    }
    
    // Increment/decrement variable with known value
    void increment1() {
        int i = 1;
        int j = 2;
        
        i--;
        j++;
        int k1 = ++i;
        int k2 = --j;
        
        b.write(k1);
        b.write(k2);
        
        b.write(i++);
        b.write(--j);
    }
    
    // Increment/decrement variable 
    void increment2() {
        int i = a.read();
        int j = a.read();
        
        i--;
        j++;
        int k1 = ++i;
        int k2 = --j;
        
        b.write(k1);
        b.write(k2);
        
        b.write(i++);
        b.write(--j);
    }    
    
    // Plus/minus
    void plus1() {
        int i = -a.read();
        int j = +a.read();
        
        int k1 = i + (-j);
        int k2 = (+j) + i;
        
        b.write(-k1);
        b.write(+k2);
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
        b.write(!l2 + ~ll2);

        unsigned x = 43;
        unsigned y = ~x;
        unsigned z = ~y;
        sct_assert_const(z == 43);
    }            

    template<typename T1, typename T2> 
    void not_test(T1 par1, T2 par2) {
        T1 a = par1;
        T2 b = par2;
    }
    
    // Not for SC types
    void not2() {
        not_test(sc_uint<10>(41), sc_uint<12>(42));
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
    }
};

int sc_main(int argc, char* argv[])
{
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}
