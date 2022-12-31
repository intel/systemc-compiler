/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"
#include <sct_assert.h>
#include <iostream>
#include <string>

using namespace sc_core;

// Function calls general cases
void globFunc1() {
    int a = 0;
}

int globFunc2(int i) {
    return i;
}

struct C : public sc_module, sc_interface
{
    sc_out<sc_uint<2> > out_port;
    int m;
    
    C(const sc_module_name& name) : sc_module(name) {
    }
    
    void f1() {
        int k = 1; 
        m = 2;
    }
    
    int f2(sc_uint<3> a, sc_uint<3>& b) {
        return (a + b);
    }
    
    void f3(sc_uint<2> val) {
        out_port = val;
    }
};

class A : public sc_module {
public:
    sc_in<bool>         a;
    sc_out<bool>        b;
    sc_out<bool>        c;
    sc_out<bool>*       p;
    sc_in<sc_uint<3> >* ip;
    
    sc_signal<sc_uint<2> > s1[2];
    sc_signal<sc_uint<3> >* sp;
    
    int                 m;
    int                 k;
    int                 n;
    int*                q;
    int*                q2;
    bool*               r;
    sc_uint<3>*         s;
    
    C                   struct_c; 
    C                   struct_c2;

    sc_signal<bool> dummy{"dummy"};

    SC_HAS_PROCESS(A);

    A(const sc_module_name& name) : 
        sc_module(name), 
        struct_c("struct_c"),
        struct_c2("struct_c2")
    {
        struct_c.out_port(s1[0]);
        struct_c2.out_port(s1[1]);
        
        p = new sc_out<bool>("p");
        ip = new sc_in<sc_uint<3> >("ip");
        q = sc_new<int>();
        q2 = &m;
        r = sc_new<bool>();
        s = sc_new<sc_uint<3>>();
        sp = new sc_signal<sc_uint<3> >("sp");

        SC_METHOD(std_funcs); sensitive << dummy;
        
        SC_METHOD(read_channel_in_cout); sensitive << b << *p << s1[0];

        SC_METHOD(func_call_params); sensitive << dummy;
        SC_METHOD(child_record_call); sensitive << dummy;
        SC_METHOD(port_access_call); sensitive << a << c << s1[1];
        SC_METHOD(included_func_calls); sensitive << a;
        SC_METHOD(multiple_returns); sensitive << dummy;
        SC_METHOD(return_in_loop); sensitive << dummy;

        SC_METHOD(glob_func_call); sensitive << dummy;
        SC_METHOD(static_func_call); sensitive << dummy;
        
        SC_METHOD(included_funcs1);
        sensitive << a;

        SC_METHOD(included_funcs2);
        sensitive << a;
        
        SC_METHOD(func_double_used); sensitive << dummy;
        
        SC_METHOD(func_chan_pointer); sensitive << *sp << *p << *ip;
        SC_METHOD(func_var_pointer); sensitive << dummy;
        
        SC_METHOD(special_funcs); sensitive << dummy;
        
        //SC_METHOD(code_after_return); -- not passed yet
    }
    
    // @cout with channel access -- BUG in real design fixed
    void read_channel_in_cout() 
    {
        cout << b.read() << p->read() << s1[0].read() << endl;
    }
    
    void f1() {
        m = m + 1;
    }
        
    int f2(int i, bool b) {
        return (b) ? i : i+1;
    }
    
    sc_uint<3> f3(int i) {
        i = i + 1;
        return i;
    }

    void f4(int& i) {
        i = i + 1;
    }
    
    void f4const(const int& i) {
        int m = i + 1;
    }
    
    int f5(int arr_par[3]) {
        int result = 0;
        for (int i = 0; i < 3; i++) {
            result = result + arr_par[i];
        }
        return result;
    }
    
    sc_uint<3> f5_sc(sc_uint<3> arr_par[2]) {
        sc_uint<3> result = 0;
        for (int i = 0; i < 2; i++) {
            result = result + arr_par[i];
        }
        return result;
    }
    
    void f6() {
        b = !a.read();
    }

    bool f7(sc_out<bool>& port) {
        port = a.read();
        return !a.read();
    }
    
    bool f8_() {
        return a.read();
    }
    
    sc_uint<3> f8__(int val) {
        return (val+1);
    }
    
    sc_uint<4> f8(sc_uint<3> val) {
        if (f8_()) {
            return 0;
        } else {
            return (a ? val + 1 : val -1);
        }
    }
    
    void f9_(unsigned& v) {
        v++;
    }
    
    unsigned f9(unsigned val) {
        f9_(val);
        f9_(val);
        return val;
    }
    
    void f10(int& val) {
        int m;
        if (val) {
            m = 1;
            return;
        } else {
            m = 2;
            return;
        }
    }

    unsigned f11(unsigned val) {
        switch (val) {
            case 1: return 1;
            case 2: return 2;
            default : return 3;
        }
    }

    // Code after return 
    // Not passed as no delayed branch for val = 2 is not ready
    int f12(int val) {
        if (val) {
            return 0;
            val = 1;
        }
        val = 2;
        return val;
    }

    static void staticFunc1() {
        int i = 0;
    }
    
    int staticFunc2(int i) {
        return i; 
    }
    
    // ----------------------------------------------------------------------

    // Global functions
    void glob_func_call() 
    {
        globFunc1();
        
        int i = globFunc2(1);
        globFunc2(i);
    }

    // Static functions
    void static_func_call() 
    {
        staticFunc1();
        
        int i = staticFunc2(1);
        staticFunc2(i);
    }
    
    // Function call with different kinds of parameters and returned value
    void func_call_params() {
        int j;

        // Function with global variable update
        f1();
        
        // Function with returned value not used
        f2(1, true);

        // Function with returned value used
        j = f3(1);

        // Function with value by reference
        f4(j);
        f4const(j);

        // Function with array parameter
        int arr_arg[3] = {1,2,3};
        j = f5(arr_arg);
        
        sc_uint<3> arr_arg2[2] = {1,2};
        j = f5_sc(arr_arg2);
    }

    // Child record method call
    void child_record_call() {
        int j;
        sc_uint<3> x = 1;
        sc_uint<3> y = 2;
        
        // Record member access
        struct_c.m = 1;   
        
        // Function call without parameters
        struct_c.f1();
        
        // Function call with references
        j = struct_c.f2(2, x);
        
        // Several instances call
        j = struct_c.f2(1, x) - struct_c2.f2(2, y);
    }

    // Function call with port access
    void port_access_call() {
        bool b1;
        int i = s1[1].read();
        // Port access no parameters
        f6();
        // Port passed by reference
        b1 = f7(c) + i;
        
        // Port access in child module
        struct_c.f3(3);
    }
    
    // Function with included function calls
    void included_func_calls() {
        // Different function calls
        int j = f8(f8__(1));
        
        // The same function multiple calls
        j = f9(2);
    }    
    
    // Function with multiple returns
    void multiple_returns() {
        int j = 1;
        // No returned value
        f10(j);
        
        // Returned value
        j = f11(j+1);
    }    
    
    // Function with return in switch called from loop
    void return_in_loop() {
        for (int i = 0; i < 3; i++) {
            int j = f11(i);
        }
        sct_assert_level(0);
    }      
    
    // Included function with non-zero scope level 
    void g3() {
        int k = 3;
    }
    
    void g2() {
        int k = 2;
        g3();
    }
    
    void g1() {
        if (a) {
            g2();
        } else {
            g3();
        }
        int k = 4;
    }
    
    void included_funcs1() 
    {
        int k = 1;
        if (a) {
            g2();
        }
        sct_assert_level(0);
    }    
    
    void included_funcs2() 
    {
        int k = 1;
        g1();
    } 

    // The same function call used double
    bool ff(int i) {
        return (i == 1);
    }
    
    void func_double_used() 
    {
        bool b = (ff(1)) ? false : ff(1);
    }
    
    // Function with channel pointer parameter
    template <typename ChanType>
    int f_ch_ptr(ChanType chan) {
        int i = chan->read();
        return i;
    }
    
    int f_ch_ptr2(sc_signal<sc_uint<3> >* chan) {
        int i = chan->read();
        return i;
    }

    void func_chan_pointer()
    {
        int j;
        // sc_signal<>*
        j = f_ch_ptr(sp);
        j = f_ch_ptr2(sp);
        
        // sc_out<>*
        j = f_ch_ptr(p);
        //j = f_ch_ptr(&b);

        // sc_in<>*
        j = f_ch_ptr(ip);
        //j = f_ch_ptr(&a);
    }
    
    // Function with pointer parameter, accessed via "->"
    template <typename PtrType>
    bool f_ptr(PtrType ptr) {
        bool b = ptr->bit(2);
        return b;
    }
    
    void func_var_pointer()
    {
        int j;
        j = f_ptr(s);
    }
    
    // Check there is no code generated for special function parameters
    void special_funcs() {
        int y = 0;
        sct_assert_const (y == 0);
        sct_assert_level (0);
        SC_REPORT_ERROR("msg type", "some msg");
    }

    // Check there is no code generated for special function parameters
    void std_funcs() {
        std::cout << "A";
        std::cout << "A" << "B" << std::endl;
        unsigned a = 2;
        std::cout << "A" << 1 << "B" << a << std::endl;
    }
    
    // Code after return
    // TODO: not passed yet
    void code_after_return() {
        int i = 1;
        int j = f12(i);
    }     
};


int sc_main(int argc, char *argv[]) {
    sc_signal<bool>      s1;
    sc_signal<bool>      s2;
    sc_signal<bool>      s3;
    sc_signal<sc_uint<3> > s4;

    A a_mod{"a_mod"};
    a_mod.a(s1);
    a_mod.b(s1);
    a_mod.c(s2);
    a_mod.p->bind(s3);
    a_mod.ip->bind(s4);

    sc_start();
    return 0;
}

