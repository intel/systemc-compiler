#include "sct_assert.h"
#include <systemc.h>

// Constant initialization in constructor body
// Error reported as non defined variables @j and @z are used
struct A : public sc_module 
{
    sc_in<bool> clk{"clk"};
    sc_signal<sc_uint<32>> s;
    
    const bool b = 0;
    const int  i = 0;
    const sc_uint<16> x = 0;
    const sc_bigint<64> y = 0;
    
    static const size_t N = 3;
    const unsigned arr[N] = {0,0,0};

    // Non-constant fields have no initialization value after elaboration
    int j;
    sc_uint<16> z;
    
    SC_HAS_PROCESS(A);
    
    explicit A(const sc_module_name& name, bool b_, int i_) : 
        sc_module(name) 
    {
        const_cast<bool&>(b) = b_;
        const_cast<int&>(i)  = i_+1;
        const_cast<sc_uint<16>&>(x) = i << 2;
        const_cast<sc_bigint<64>&>(y) = i - x;
        
        for (int i = 0; i < N; i++) {
            const_cast<unsigned&>(arr[i]) = i+1;
        }

        j = 10;
        z = 11;
        
        SC_METHOD(proc);
        sensitive << clk.pos();
    }
    
    void proc() 
    {
        sct_assert_const(b);
        sct_assert_const(i == 12);
        sct_assert_const(x == 48);
        sct_assert_const(y == -36);

        // Non-constant fields have no initialization value after elaboration
        // No @j declaration generated as @sct_assert_unknown is debug function
        sct_assert_unknown(j);
        sct_assert_unknown(z);
        
        if (b) {
            s = i + x;
        }

        for (int i = 0; i < N; i++) {
            sct_assert_const(arr[i] == i+1);
        }
    }

};

struct Top : public sc_module 
{
    sc_in<bool>     clk{"clk"};
    A               modA{"modA", 1, 11};

    SC_CTOR(Top)  
    {
        modA.clk(clk);
    }
};

int sc_main(int argc, char **argv) {

    sc_clock clock_gen{"clock_gen", 10, SC_NS};
    Top mod{"mod"};
    mod.clk(clock_gen);
    sc_start();

    return 0;
}
