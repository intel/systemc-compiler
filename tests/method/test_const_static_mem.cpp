#include <iostream>
#include <sct_assert.h>
#include "systemc.h"


// Constant static members

struct Simple {
    bool a;
    sc_uint<4> b;

    Simple(bool a, sc_uint<4> b) : a(a), b(b) 
    {}
};

class A : public sc_module 
{
public:
    const static sc_uint<16> B; 
    const static sc_bigint<65> BB; 
    const static sc_uint<16> BBB;
    const static unsigned C;  
    const static int ARR[3]; 
    const static sc_int<8> ARRI[3]; 
    
    sc_in<bool>         clk;
    sc_signal<bool>     nrst;

    sc_signal<bool> s;

    SC_CTOR(A) 
    {
        SC_METHOD(const_method); 
        sensitive << s;
        
        SC_METHOD(const_record_method); 
        sensitive << s;
        
        SC_CTHREAD(const_record_thread, clk.pos());
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(const_record_thread2, clk.pos());
        async_reset_signal_is(nrst, 0);
    }

    // Constant member variable and array
    static unsigned getvar(void) {
        return C;
    }

    void const_method() {
        int a = B;
        a = BB.to_int();
        a = BBB;
        a = ARR[1];
        a = ARRI[1];
        a = getvar();
        unsigned b = a + C;
     }

// ----------------------------------------------------------------------------
    
    // Constant record member and local
    const Simple mrec1{false, 4};
    const Simple mrec2{false, 5};

    void const_record_method() 
    {
        const Simple rec(false, 1);
        int j = rec.b + mrec1.b;
    }
    
    void const_record_thread() 
    {
        const Simple rec(false, 1);
        wait();
        
        while (true) {
            int j = rec.b + mrec2.b;
            wait();
        }
    } 
    
    // Check name conflict with @rec from @const_record_thread() 
    void const_record_thread2() 
    {
        const Simple rec(true, 2);
        wait();
        
        while (true) {
            int j = rec.a ? rec.b : (sc_uint<4>)0;
            wait();
        }
    } 
    
};

const sc_uint<16> A::B = 42;
const sc_bigint<65> A::BB = -43;
const sc_uint<16> A::BBB = A::B+1;
const unsigned A::C = 1;
const int A::ARR[3] = {1,2,3};
const sc_int<8> A::ARRI[3] = {4,-5,6};

int sc_main(int argc, char *argv[])
{
    sc_clock clk{"clk", 1, SC_NS};
    A top_inst{"top_inst"};
    top_inst.clk(clk);
    
    sc_start(100, SC_NS);
    return 0;
}
