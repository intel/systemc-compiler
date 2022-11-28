/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"

using namespace sc_core;

// Simple clocked threads and DvCon 2019 examples
class A : public sc_module {
public:

    sc_in<bool>     clk{"clk"};
    sc_in<bool>     rst{"rst"};
    sc_signal<bool> d;
    sc_signal<bool>* ps;
    sc_signal<bool> o;
    
    SC_CTOR(A) {
        ps = new sc_signal<bool>("ps");
        
        SC_CTHREAD(simple_thread, clk.pos());
        async_reset_signal_is(rst, true);

        SC_CTHREAD(simple_reg, clk.pos());
        async_reset_signal_is(rst, false);
        
        SC_CTHREAD(simple_thread_wo_reset1, clk.pos());

        SC_METHOD(concat_meth);
        sensitive << (*ps);
        
        SC_CTHREAD(simple_concat, clk.pos());
        async_reset_signal_is(rst, false);

        SC_CTHREAD(simple_reg_ro, clk.pos());
        async_reset_signal_is(rst, false);

        // DvCon 2019 examples
        /*SC_CTHREAD(thread1, clk.pos());
        async_reset_signal_is(rst, false);

        SC_CTHREAD(thread2, clk.pos());
        async_reset_signal_is(rst, false);
                 
        SC_CTHREAD(thread_loop, clk.pos());
        async_reset_signal_is(rst, false);   
                
        SC_CTHREAD(thread_break, clk.pos());
        async_reset_signal_is(rst, false);        
        
        SC_METHOD(method_proc);
        sensitive << in << s;*/
    }
    
    void simple_thread() {
        wait();
        while (true) {
            wait();
        }
    }
    
    void simple_reg() {
        o = 0;
        sc_uint<4> l = 0;
        wait();
        
        while (true) {
            l = 1;
            o = l + d + 1;
            l = o;
            wait();
        }
    }

    void simple_thread_wo_reset1() {
        while (true) {
            int i = 0;
            wait();
        }
    }
    
    void concat_meth() {
        sc_uint<2> c = ((sc_uint<1>)ps->read(), (sc_uint<1>)0);
        //sc_uint<2> c = ((sc_uint<1>)ps->read());
    }

    void simple_concat() {
        wait();
        
        while (true) {
            sc_uint<2> c = ((sc_uint<1>)ps->read(), (sc_uint<1>)0);
            wait();
        }
    }
    
    void simple_reg_ro() {
        wait();
        
        while (true) {
            bool b = d;
            wait();
        }
    }

// --------------------------------------------------------------------------
    
    // DvCon19 paper examples
    sc_in<bool>       in;
    sc_out<bool>      out;
    sc_signal<bool>   s;
    
    void method_proc () {
       bool a = in; 
       if (s != 0) { 
          out = a;
       } else {
          out = 0;
       }
    }


    sc_in<unsigned>  a;
    sc_out<unsigned> b;
    
    void thread1 () {
       unsigned i = 0;
       b = 0;
       while (true) {
          wait();
          b = i;
          i = i + a;
       }
    }
    
    void thread2() {
       sc_uint<8> i = 0;
       b = 0;              
       wait();                    // STATE 0
       while (true) {
          auto j = a.read(); 
          i = j + 1;
          wait();                 // STATE 1
          b = i;   
    }}
    
    static const unsigned N = 10;
    uint16_t k[N], n[N], m[N];

    void thread_loop() {
      wait();                // STATE 0
      while (true) {
        for (int i = 0; i < 10; i++) {
          k[i] = n[i] / m[i];
          wait();          // STATE 1
        }
        wait();             // STATE 2
      }
    }
    
    sc_signal<bool> enabled;
    sc_signal<bool> stop;
    sc_signal<bool> ready;
    
    void thread_break() {
      wait();                // STATE 0
      while (true) {
        wait();             // STATE 1
        while (!enabled) {
           if (stop) break;
           wait();          // STATE 2
        }
      ready = false;
      }
    }


};

class B_top : public sc_module {
public:
    sc_in<bool>         clk{"clk"};
    sc_signal<bool>     rst{"rst"};
    sc_clock clk_gen{"clk", sc_time(1, SC_NS)};

    sc_signal<bool>  in;
    sc_signal<bool>  out;
    sc_signal<unsigned>  a;
    sc_signal<unsigned>  b;
    
    A a_mod{"a_mod"};

    SC_CTOR(B_top) {
        clk (clk_gen);
        a_mod.clk(clk);
        a_mod.rst(rst);
        
        a_mod.in(in);
        a_mod.out(out);
        a_mod.a(a);
        a_mod.b(b);
        
        //SC_HAS_PROCESS(B_top);
        //SC_CTHREAD(reset_proc);
    }
    
    /*void reset_proc() {
        rst = 1;
        wait();
        
        rst = 0;
    }*/
};

int sc_main(int argc, char *argv[]) {

    B_top b_mod{"b_mod"};
//    b_mod.clk(clk);
    
    sc_start();
    return 0;
}

