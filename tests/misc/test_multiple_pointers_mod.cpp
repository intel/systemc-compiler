#include <systemc.h>

using namespace sc_core;

// Multiple pointers to module, 4 warnings reported
class B : public sc_module {
public:
    SC_CTOR(B) {
    }
};

class A : public sc_module {
public:
    B    b{"b"};

    B*   pb;
    B*   qb;

    B*   dpb;
    B*   dqb;
    
    sc_signal<bool>         dummy;
    
    SC_CTOR(A) 
    {
        pb = &b;
        qb = &b;
        
        dpb = new B("bb");
        dqb = dpb;
        
        SC_METHOD(meth); 
        sensitive << dummy;
    }
    
    void meth() {}
};

int sc_main(int argc, char *argv[]) {
    A a_mod{"a_mod"};
    sc_start();
    return 0;
}

