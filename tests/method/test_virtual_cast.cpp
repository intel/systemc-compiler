#include "systemc.h"

using namespace sc_core;

// Virtual and non-virtual unction call with cast to base module type

struct A : public sc_module
{
    char                 m;

    A(const sc_module_name& name) : sc_module(name)
    {}
    
    void f() {
        m += 1;
    }
};


class C : public A 
{
public:
    short                 m;
    sc_signal<bool> dummy{"dummy"};

    SC_HAS_PROCESS(C);
    C(const sc_module_name& name) : A(name) {
        SC_METHOD(proc_func); sensitive << dummy;
    }
    
    void f() {
        m += 2;
    }

    void proc_func() {
        f();
        C::f();
        A::f();
    }
};

class D : public C 
{
public:
    int                 m;
    sc_signal<bool> dummy{"dummy"};

    SC_HAS_PROCESS(D);
    D(const sc_module_name& name) : C(name) {
        SC_METHOD(proc_func); sensitive << dummy;
    }

    void f() {
        m += 3;
    }

    void proc_func() {
        f();
        D::f();
        C::f();
        A::f();
    }
};


class B_top : public sc_module 
{
public:
    C   c_mod{"c_mod"};
    D   d_mod{"d_mod"};

    B_top(const sc_module_name& name) : sc_module(name) {
    }
};

int sc_main(int argc, char *argv[]) {
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}