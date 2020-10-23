#include "systemc.h"

using namespace sc_core;

// Bit and range select test
class A : public sc_module {
public:
    // Define variables
    sc_signal<bool> dummy;
    sc_int<8> x;
    sc_bigint<8> y;
    sc_uint<8> ux;
    sc_biguint<8> uy;

    sc_in<sc_uint<32>> inp;
    sc_out<sc_uint<32>> outp;

    SC_CTOR(A) {
        
        SC_METHOD(sc_uint_wide); 
        sensitive << as;

        SC_METHOD(sc_uint_ctor); 
        sensitive << as << inp;

        SC_METHOD(signal_array_part_sel); 
        sensitive << ssig[0] << ssig[1];

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

    // Check no outside of variable width part/bit selection
    void sc_uint_wide() 
    {
        bool b;
        sc_uint<1> k;
        b = ((sc_uint<2>)k).bit(1);                 // b = k;
        sc_uint<2> x = ((sc_uint<3>)k).range(2,1);  // x = k;
    }
    
    // Check construct @sc_uint with/without typecast
    void sc_uint_ctor() 
    {
        sc_uint<3> i = 1;
        sc_uint<2> j = i;
        sc_uint<1> k = i;
        
        bool b = i;
        b = ((sc_uint<1>)i).bit(0);             // b = i; 
        b = ((sc_uint<2>)i).bit(0); 
        
        b = ((sc_uint<1>)k).bit(0); 
        b = ((sc_uint<2>)k).bit(0);

        b = ((sc_uint<1>)inp.read()).bit(0);    // b = inp;
        b = (sc_uint<1>)(inp.read().bit(1));
    }

    sc_signal<sc_uint<8>> ssig[2];
    void signal_array_part_sel() 
    {
        bool b = ssig[1].read().bit(2);
        sc_uint<5> x = ssig[1].read().range(4,3);
    }
    
    sc_signal<bool> sig;
    void sc_uint_to_bool() 
    {
        sc_uint<2> i = 1;
        bool b = as.read();
        sig = i;
        sig.write(as.read());
        sig = as.read();
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

class B_top : public sc_module {
public:
    sc_signal<sc_uint<32>> sig_inp;
    sc_signal<sc_uint<32>> sig_outp;

    // Instantiate  module class A
    A a_mod{"a_mod"};
    // Connect signals to module A
    SC_CTOR(B_top) {
        a_mod.inp(sig_inp);
        a_mod.outp(sig_outp);
    }
};

int sc_main(int argc, char *argv[]) {
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}
