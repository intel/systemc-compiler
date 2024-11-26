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

// Empty IF, loops and function calls, empty call with string parameter
struct A : public sc_module 
{
    sc_in<bool>         clk;
    sc_signal<bool>     nrst;

    sc_signal<sc_uint<4>> s;

    SC_HAS_PROCESS(A);

    A(const sc_module_name& name) : 
        sc_module(name)
    {
        SC_METHOD(no_sens_method);

        SC_METHOD(empty_if_method);
        sensitive << s;

        SC_METHOD(empty_loop_method);
        sensitive << s;
        
        SC_CTHREAD(empty_loop_thread, clk.pos());
        async_reset_signal_is(nrst, 0);

        SC_METHOD(fcall_method);
        sensitive << s;
        
        SC_METHOD(trace_method); sensitive << s;

        SC_CTHREAD(trace_thread, clk.pos());
        async_reset_signal_is(nrst, 0);
          
        SC_CTHREAD(fcall_thread, clk.pos());
        async_reset_signal_is(nrst, 0);
    }
    
    sc_signal<int> t0;
    void no_sens_method() {
        int a = 1;
        t0 = a;
    }

    sc_signal<int> t1;
    void empty_if_method() {
        int a = 0;
        
        if (s.read()) {
        }

        if (s.read()) {
        } else {
        }

        if (s.read()) a = 1;
            
        if (s.read()) {
        } else {
            a = 2;
        }

        if (s.read()) {
            a = 3;
        } else {
        }
        t1 = a;
    }

    sc_signal<int> t2;
    void empty_loop_method() 
    {
        int a = 0;
        
        for (int i = 0; i < 2; ++i) {
        }

        for (int i = 0; i < 3; ++i) {
            if (a) {}
        }

        int j = 0;
        while (j < 3) {
            ++j;
        }
        t2 = a + j;
    }

    void empty_loop_thread() 
    {
        wait();
        
        while (true) {
            int a = 0;
            while (s.read()) {wait();}
            wait();
        }
    }
    
    void f() {}
    
    void fcall_method() {
        f();
    }
    
    struct Flit_t {
        static const unsigned B = 2;
        sc_int<2> a;
        sc_uint<3> b;
        
        inline friend bool operator==(const Flit_t& lhs, const Flit_t& rhs)
        {return (lhs.a == rhs.a && lhs.b == rhs.b);}
        
        inline friend std::ostream& operator<< (std::ostream& os, const Flit_t &obj)
        {return os;}
        
        inline friend void sc_trace(sc_trace_file*& f, const Flit_t& val, std::string name) 
        {}
        
        inline Flit_t getFlitClone() const {
             Flit_t res; res.a = 42;
             return res;
         }
    };
    
    void traceFlit(const Flit_t& flit, const std::string &tag)
    {
        if (flit.a == flit.b) {
            cout << "_TR " << sc_time_stamp() << ", " << name()
                 << ", " << flit.getFlitClone() << ", " << tag << endl;
        }
    }
    
    void traceFlit2(std::string tag, const Flit_t& flit)
    {
        if (flit.a == flit.b) {
            cout << "_TR " << sc_time_stamp() << ", " << name()
                 << ", " << flit.getFlitClone() << ", " << tag << endl;
        }
    }

    void trace_method() 
    {
        Flit_t flit;
        traceFlit(flit, "IS nACK entered drop buff");
        traceFlit2("IS nACK entered drop buff", flit);
    }

    void trace_thread() {
        Flit_t flit;
        traceFlit(flit, "IS nACK entered drop buff");
        wait();
        
        while (true) {
            traceFlit2("IS nACK entered drop buff", flit);
            wait();
        }
    }    

    void fcall_thread() {
        
        f();
        wait();
        
        while (true) {
            f();
            wait();
        }
    }    
};

int sc_main(int argc, char *argv[]) 
{
    A a_mod{"a_mod"};
    sc_clock clk{"clk", 1, SC_NS};
    a_mod.clk(clk);
    
    sc_start();
    return 0;
}

