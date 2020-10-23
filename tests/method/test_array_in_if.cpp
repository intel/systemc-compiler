#include "systemc.h"

using namespace sc_core;

// Functions and arrays in IF condition
struct C : public sc_module 
{
    SC_CTOR(C) {
    }
    
    static const unsigned BLOCK_NUM = 3;
    sc_signal<bool>         block_access[BLOCK_NUM];
};

class A : public C {
public:
    
    SC_HAS_PROCESS(A);

    sc_signal<bool> dummy{"dummy"};

    A(const sc_module_name& name) : C(name) {
        SC_METHOD(func_in_if1); sensitive << dummy;
        SC_METHOD(func_in_if2); sensitive << dummy;

        SC_METHOD(func_in_if_const1); sensitive << dummy;
        SC_METHOD(func_in_if_const2); sensitive << dummy;
        SC_METHOD(func_in_if_const3); sensitive << dummy;
        SC_METHOD(func_in_if_const4); sensitive << dummy;

        SC_METHOD(chan_array_in_if1); sensitive << dummy;
        SC_METHOD(chan_array_in_if2); sensitive << dummy;
    }
    
    static const unsigned BLOCK_NUM = 3;
    static const unsigned SLEEP_COUNTER_WIDTH = 2;
    static const unsigned SLEEP_COUNTER_MAX = (1<<SLEEP_COUNTER_WIDTH)-1;

    sc_signal<sc_uint<SLEEP_COUNTER_WIDTH> > sleep_idle_cntr[BLOCK_NUM];
    sc_signal<bool>         ms_pwrin_nenable[BLOCK_NUM];
    
    bool k;
    
    bool f() {
        return k;
    }

    bool g(int i) {
        return i;
    }

    void func_in_if1() 
    {
        int i;
        if (f() || i) {
            i = 1;
        }
    }

    void func_in_if2() 
    {
        int i;
        if (f() || g(i)) {
            i = 1;
        }
    }

    void func_in_if_const1() 
    {
        int i;
        if (false && g(i)) {
            i = 1;
        }
    }

    void func_in_if_const2() 
    {
        int i;
        if (true || f()) {
            i = 1;
        }
    }

    void func_in_if_const3() 
    {
        int i;
        if (f() || true) {
            i = 1;
        }
    }

    void func_in_if_const4() 
    {
        int i;
        if (false || g(i)) {
            i = 1;
        }
    }
    
    void chan_array_in_if1() 
    {
        int i;
        if (ms_pwrin_nenable[i] || i) {
            i = 1;
        }
    }
    
    // Access channel array in base module in IF condition
    // BUG in SMEM KVG -- fixed
    void chan_array_in_if2() 
    {
       for (int i = 0; i < BLOCK_NUM; i++) {
            if (this->block_access[i] || ms_pwrin_nenable[i]) {
                sleep_idle_cntr[i] = 0;
            }    
        }
    }

 };

class B_top : public sc_module {
public:
    
    A a_mod{"a_mod"};

    SC_CTOR(B_top) {
    }
};

int sc_main(int argc, char *argv[]) {
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}
