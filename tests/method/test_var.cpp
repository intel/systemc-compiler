/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

using namespace sc_core;

// Local variables in method process body analysis
class A : public sc_module {
public:
    sc_in<bool>     a{"a"};
    sc_out<bool>    b{"b"};
    sc_out<bool>    c{"c"};
    sc_in<int>      d{"d"};
    sc_out<int>     e{"e"};

    sc_signal<int>      si{"si"};
    sc_signal<bool>     sb{"sb"};
    sc_signal<int>     *sip;

    sc_out<int>        *oip;
    sc_in<int>         *iip;

    int m = 11;
    int k;
    int n;
    const int* p = nullptr;
    int ci = 42;
    const int* q = &ci;
    
    sc_signal<bool> dummy{"dummy"};

    SC_CTOR(A) {
        iip = new sc_in<int>("iip");
        oip = new sc_out<int>("oip");
        sip = new sc_signal<int>("sip");
        
        SC_METHOD(unknown_ref); sensitive << sig;
        
        SC_METHOD(var1);
        sensitive << a;
        SC_METHOD(var2);
        sensitive << a;
        SC_METHOD(var3);
        sensitive << a;
        SC_METHOD(var4);
        sensitive << a;

        SC_METHOD(signal1);
        sensitive << si << d;
        SC_METHOD(signal2);
        sensitive << si << sb;
        SC_METHOD(signal3);
        sensitive << *sip << *oip << *iip;

        SC_METHOD(port1);
        sensitive << a;
        SC_METHOD(port2);
        sensitive << a << d;
        SC_METHOD(port3);
        sensitive << *oip;
        
        // Waiting for support in elaborator
        SC_METHOD(pointer_compare); 
        sensitive << dummy;
        
        SC_METHOD(pointer_compare2); 
        sensitive << dummy;
    }

    sc_signal<int> sig;
    
    void unknown_ref() 
    {
        int i = 1;
        int& ref = i; 
        ref = sig.read();
    }
    
    
   // Local variables declaration
    void var1() {
	int k;
        int i = 3;
        int j = i+1+k;
        b.write(j);
    }

    // Local variables with the same name in different scopes
    void var2() {
        int i = a.read();
        {
           int i;
           i = a.read()+1;
           b.write(i);
        }
        i = i+2;
        b.write(i);
   }

    // Local variables with the same name in if scopes
    void var3() {
        int i = a.read();
        if (m > 0) {
           int i = 1;
        } else {
           int i = a.read();
           b.write(i);
        }
        b.write(i);
    }

    // Local variables in loop body
    void var4() {
        int i = 0;
        while (i < 2) {
           i++;
           int j = i;
           int k = i + j;
           b.write(k);
        }
        b.write(i);
    }

    // Signal with read/write
    void signal1() {
        int i;
        i = si.read();
        si.write(i);
        
        e.write(si.read());
        si.write(d.read());
    }

    // Signal with assignment operator
    void signal2() {
        int i;
        // int to int
        i = si;
        si = i;
        // bool to int
        i = sb;
        sb = i;
        // bool to bool
        bool f = sb;
        sb = f;
        // int to bool
        f = si;
        si = f;
        // bool initialized by int
        bool f1 = si;
        // int initialized by bool
        int i1 = sb;
    }
    
    // Signal pointer de-reference
    void signal3() {
        int i;
        i = sip->read();
        sip->write(i);

        i = (*sip).read();
        (*sip).write(i);

        i = *sip;
        *sip = i;
        
        *oip = *sip;
        *sip = *oip;
        *sip = *iip;  
        *oip = *iip;
        
        *sip = *sip;
        *oip = *oip;
    }
    
    // Port with read/write
    void port1() {
        int i;
        i = a.read();
        b.write(i);
    }

    // Port with assignment operator
    void port2() {
        int i;
        // int to int
        i = d;
        e = i;
        // bool to int
        i = a;
        b = i;
        // bool to bool
        bool f = a;
        b = f;
        // int to bool
        f = d;
        e = f;
    }
    
    // Port pointer de-reference
    void port3() {
        int i;
        i = oip->read();
        oip->write(i);

        i = (*oip).read();
        (*oip).write(i);
        
        *oip = i;
        i = *oip;
        i = (*oip).read();
        (*oip).write(i);
    }    
    
    // Pointer comparison with 0, NULL, nullptr
    void pointer_compare() {
        const bool b1 = (p == nullptr);
        if (b1) {
            int i;
        }
        sct_assert_const(b1);
        
        bool b2 = (p == nullptr);
        sct_assert_const(b2);
    }    
    
    void pointer_compare2() {
        const bool b1 = (q != nullptr);
        if (b1) {
            int i;
        }
        sct_assert_const(b1);
        
        bool b2 = (q != nullptr);
        sct_assert_const(b2);
    }    
   
};

class B_top : public sc_module {
public:
    sc_signal<bool> a{"a"};
    sc_signal<bool> b{"b"};
    sc_signal<bool> c{"c"};
    sc_signal<int>  e{"e"};
    sc_signal<int>  d{"d"};
    sc_signal<int>  p1{"p1"};
    sc_signal<int>  p2{"p2"};

    A a_mod{"a_mod"};

    SC_CTOR(B_top) {
        a_mod.a(a);
        a_mod.b(b);
        a_mod.c(c);
        a_mod.d(d);
        a_mod.e(e);

        a_mod.oip->bind(p1);
        a_mod.iip->bind(p2);
    }
};

int sc_main(int argc, char *argv[]) {
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}

