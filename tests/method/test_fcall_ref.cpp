/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"

using namespace sc_core;

struct C : public sc_module 
{
    SC_CTOR(C) {
    }
    
    void ref_par(sc_uint<3>& par1) {
        const sc_uint<2> a = par1.range(1,0);
    }
};

// References and pointers in function parameters, channel passed via 
// constant reference
template <unsigned N>
class A : public C {
public:
    SC_HAS_PROCESS(A);

    sc_in_clk clk;
    sc_signal<bool> nrst;
    sc_signal<bool> dummy{"dummy"};

    sc_out<bool>        out{"out"};
    sc_out<bool>*       p;
    
    int                 m;
    int                 k;
    sc_uint<3>          n;
    int*                q;
    int*                q1;
    int*                q2;
    bool*               r;
    sc_uint<3>*         s;
    sc_uint<3>*         s1;
    sc_uint<3>*         s2;
    
    sc_signal<sc_uint<4>>  sig;
    
    A(const sc_module_name& name) : C(name) 
    {
        p = new sc_out<bool>("p");
        q = sc_new<int>();
        q1 = sc_new<int>();
        q2 = &m;
        r = sc_new<bool>();
        s = sc_new<sc_uint<3>>();
        s1 = sc_new<sc_uint<3>>();
        s2 = &n;
        
        SC_METHOD(const_reference); 
        sensitive << dummy;

        SC_METHOD(const_reference_sc); 
        sensitive << dummy;
        
        SC_METHOD(const_reference_noninit); 
        sensitive << si;
        
        SC_METHOD(const_reference_expr); 
        sensitive << si;

        SC_METHOD(const_reference_sig); 
        sensitive << sig << sig2 << sig3;

        SC_METHOD(const_reference_sig_arr);
        sensitive << sig << sig_arr[0] << sig_arr2[0];
        
        SC_METHOD(scint_reference); 
        sensitive << dummy;

        SC_METHOD(reference_inner_call); 
        sensitive << dummy;
        
        SC_METHOD(pointer_param);
        sensitive << (*p);
        
        SC_METHOD(pointer_param_modfied);
        sensitive << (*p);
        
        SC_METHOD(pointer_inner_call);
        sensitive << out;
        
        SC_METHOD(recursive_ref);
        sensitive << sig;

        SC_METHOD(recursive_ref_const);
        sensitive << sig;
        
        SC_METHOD(reference_init_func);
        sensitive << sig;
        
    }
    
    void ref(sc_uint<3>& val) {
        sc_uint<2> k = val.range(2,1);
        val = k+1;
    }

    template <class T>
    T const_ref(const T& val) {
        T j = val;
        return j;
    }

    sc_uint<2> const_ref2(const sc_uint<4>& val) {
        sc_uint<2> j = val.range(1,0);
        return j+1;
    }
    
    int const_ref3(const int& val) {
        const int j = val;
        if (j) {        // 1
            return 1;
        } else {
            return 0;
        }
    }
    
     // Constant reference parameter
    void const_reference() 
    {
        const_ref3(1);
    }

    // Constant reference parameter SC type
    void const_reference_sc() 
    {
        sc_uint<2> i = 1;
        i = const_ref(i);
        i = const_ref2(2);

        i = const_ref2(1);
        i = const_ref2(2);
        
        sc_uint<4> j = 5;
        i = const_ref2(j);
        i = const_ref2(j+1);
    }
    
    // Constant reference initialized with non-initialized variable, 
    // warning reported
    void const_reference_noninit() 
    {
        int k;
        const_ref(k);   // warning
        int m = 1;
        const_ref(m);
    }
        
    // Constant reference initialized with expression, check no warning
    sc_signal<sc_uint<4>> si;
    void const_reference_expr() 
    {
        const_ref(si.read());
        const_ref((sc_uint<2>)si.read().range(1,0));
    }
    
// ----------------------------------------------------------------------------    
    // Signal and signal array element passed via constant reference
    void const_ref_sig(const sc_signal<sc_uint<4>>& par) {
        sc_uint<4> l = par;
    }
    
    sc_signal<sc_uint<8>> sig2;
    sc_signal<sc_biguint<4>> sig3;
    void const_reference_sig() 
    {
        const_ref2(sig);
        const_ref2(sig.read());
        const_ref2(sig.read()+1);
        
        // Another type signal, no reference
        const_ref2(sig2.read());
        const_ref2(sig3.read());
        
        // Signal reference
        const_ref_sig(sig);
    }
    
    sc_signal<sc_uint<4>> sig_arr[3];
    sc_signal<sc_uint<8>> sig_arr2[3];
    void const_reference_sig_arr() 
    {
        const_ref2(sig_arr[1]);
        const_ref2(sig_arr[sig.read()]);

        // Another type signal, no reference
        const_ref2(sig_arr2[1].read());
        const_ref2(sig_arr2[sig.read()].read());
    }
    
// ----------------------------------------------------------------------------
    // Method with @sc_int reference parameter
    // BUG from real design -- fixed
    void scint_reference() 
    {
        sc_uint<3> x = 1;
        ref(x);
        
        sc_uint<3>& xr = x;
        xr.range(2,1) = 1;
        int j = xr.bit(0);
        
        this->ref_par(x);
    }
    
    // Reference parameter used in inner call
    sc_uint<2> unused_return() 
    {
        return 1;
    }
    
    void ref_with_call(sc_uint<3>& par) 
    {
        ref(par);
    }

    sc_uint<2> const_ref_with_call(const sc_uint<4>& par) 
    {
        sc_uint<2> a = const_ref2(par) + 1;
        return a;
    }
    
    void reference_inner_call() 
    {
        // Check there is no extra statement printed
        unused_return();
        
        sc_uint<3> x = 2;
        ref_with_call(x);
        
        const_ref_with_call(3);
    }
  
    // ---------------------------------------------------------------------
    
    // Pointer parameter
    int ptr1(int* val) {
        return (*val + 1);
    }

    // Pointer parameters
    int ptr2(int* val1, bool* val2) {
        int i;
        if (*val2) {
            i = *val1;
        } else {
            i = 0;
        }
        return i;
    }

    // Pointer parameters SC type
    int ptr3(sc_uint<3>* val) {
        int i = val->range(2,1);
        *val = 1;
        return i;
    }

    // Pointer parameter modified inside
    void ptr4(int* val) {
        int i = *val + 1;
        *val = i;
    }

    // Not supported yet
    bool port_ptr(sc_out<bool>* val) {
        bool b = val->read();
        return b;
    }

    
    // Function with pointer parameter
    void pointer_param()
    {
        int i;
        
        *q = 1;
        i = ptr1(q);
        *q = i;
        i = ptr2(q, r);
        
        i = ptr3(s1);
        
        //bool b = port_ptr(p);
    }

    // Function with pointer parameter modified
    void pointer_param_modfied()
    {
        *q2 = 1;
        ptr4(q2);
        
        *q1 = 1;
        ptr4(q1);
    }    

    // Pointer parameter used in inner call
    void ptr_with_call(sc_uint<3>* par) 
    {
        int i = *par;
        ptr3(par);
        
    }

    void pointer_inner_call()
    {
        *s = 2;
        ptr_with_call(s);

        *s2 = 2;
        ptr_with_call(s2);
    }
    
// ----------------------------------------------------------------------------
    // Recursive reference
    
    void rec_ref_(sc_uint<4>& par_) 
    {
        par_++;
    }
    
    sc_uint<4> rec_ref(sc_uint<4>& par) 
    {
        rec_ref_(par);
        return par + 1;
    }
    
    void recursive_ref() 
    {
        sc_uint<4> x = sig.read();
        int res = rec_ref(x);
    }
    
// ----------------------------------------------------------------------------
    // Recursive constant reference
    
    int rec_ref_const_(const sc_uint<4>& par_) 
    {
        return par_ + 1;
    }
    
    sc_uint<4> rec_ref_const(const sc_uint<4>& par) 
    {
        return rec_ref_const_(par) + 1;
    }
    
    void recursive_ref_const() 
    {
        int res = rec_ref_const(sig.read());
    }

// ----------------------------------------------------------------------------    
    // Constant reference initialized with operator or function
    int f1(int i ) {
        return (i+1);
    }
    
    int ref_call(int& par__) 
    {
        int loc =  par__ + 1;
        return loc;
    }
    
    int ref_const_call(const int& par__) 
    {
        int loc =  par__ + 1;
        return loc;
    }
    
    int sum(int &xr, int yr)
    {
        int sum = xr + yr;
        yr = sum;
        xr = sum;
        return sum;
    }

    void reference_init_func() {
        int m = 1;
        //int n = ref_call(++m);    // #251
        //int l = sum(++m, n--);
        
        ref_const_call(m++);
        ref_const_call(--m);
        int x = ref_const_call(f1(m++));
        
        // return by reference not supported yet
    }
    
};

class B_top : public sc_module {
public:
    sc_clock clk{"clk", 1, SC_NS};
    sc_signal<bool>      s1;
    sc_signal<bool>      s2;

    A<1> a_mod{"a_mod"};
    
    SC_CTOR(B_top) {
        a_mod.clk(clk);
        a_mod.p->bind(s1);
        a_mod.out(s2);
    }
};

int sc_main(int argc, char *argv[]) {
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}

