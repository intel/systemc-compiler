#include "systemc.h"

using namespace sc_core;

// Complex conditions with side effect
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

    sc_signal<bool> dummy{"dummy"};

    SC_CTOR(A) : m(1) 
    {
        SC_METHOD(if_side_effect1); sensitive << a;
        SC_METHOD(if_side_effect2); sensitive << a;
    }

    bool f() {
        return k;
    }
    
    // IF condition with function call with side effect
    // This test has incorrect RTL as function body is inlined
    void if_side_effect1() {
        int i = 0;
        if (a.read() && f()) {  // inlined body and temporary variable used here
            i = 1;
        }
        i = 2;
    }
    
    // Expression with side effect
    void if_side_effect2() {
        int i = 0;
        if (a.read() && m++) {  
            i = m;
        }
        i = 2;
    }
};

class B_top : public sc_module
{
    sc_signal<bool> a{"a"};
    sc_signal<bool> b{"b"};
    sc_signal<bool> c{"c"};

public:
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
