#include "systemc.h"
#include <sct_assert.h>

using namespace sc_core;

// Incorrect range. Test should fail
class A : public sc_module
{
public:
    sc_signal<bool>     a{"a"};
    sc_uint<4>          x;
    
    SC_CTOR(A)
    {
        SC_METHOD(extraRange); sensitive << a;
    }
    
    // Lo higher than hi
    void extraRange() 
    {
        sc_uint<4> y = 3;
        x = y.range(1,2);
    }
};

class B_top : public sc_module
{

public:
    A a_mod{"a_mod"};

    SC_CTOR(B_top) {
    }
};

int sc_main(int argc, char* argv[])
{
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}
