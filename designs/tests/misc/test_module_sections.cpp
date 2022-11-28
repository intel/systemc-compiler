/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <sct_assert.h>
#include <systemc.h>

// Constants and static constants in global scope
const bool CC = true;
static const bool C = true;

const int DD = 2;
static const int D = 42;

const sc_int<4> EE = -2;
static const sc_uint<4> E = 3;

struct MyStruct : public sc_module, sc_interface
{
    SC_CTOR(MyStruct) {
    }
    
    virtual bool mb_read()
    {
        int i = D + DD;
        return (i == 1);
    }
};

SC_MODULE(MyChild) 
{
    sc_in_clk       clk;
    sc_in<bool>     f[2];
    SC_CTOR(MyChild) {
        SC_METHOD(proc);
        sensitive << f[0];
    }
    
    void proc() {
        bool b = f[0];
    }
    
};

SC_MODULE(MyModule) 
{
    sc_in_clk       clk;
    sc_signal<bool> rst;
    sc_signal<int>  sig;
    
    sc_signal<bool> s1;
    sc_signal<bool> s2;
    
    MyStruct        struct1;
    
    MyChild         m{"m"};
    
  
    struct MyRec {
        int i;
        MyRec(int i_) : i(i_) {};
    };

    sc_signal<bool>* ap;
    bool* bp;
    sc_uint<8>* vp;
    MyRec* mp;
    sc_in<int>** ports;
    sc_signal<int>* signals;

    SC_CTOR(MyModule) : struct1("struct1") {
        
        ap = new sc_signal<bool>("a");    // OK, signal is sc_object
        //bp = new bool;                    // ERROR, non-sc_object
        bp = sc_new<bool>();              // OK
        //vp = new sc_uint<8>();            // ERROR, non-sc_object
        vp = sc_new<sc_uint<8>>();        // OK
        //mp = new MyRec(42);               // ERROR, new for non-sc_object
        mp = sc_new<MyRec>(42);           // OK, using sc_new
        ports = new sc_in<int>* [10];    // ERROR, new for pointer, non-sc_object 
        //ports = sc_new_array<sc_in<int>*>(10); // OK, using sc_new_array
        signals = new sc_signal <int>[10];     // OK, array of sc_objects 

        m.clk(clk);
        m.f[0](s1);
        m.f[1](s2);
        
        SC_CTHREAD(newProc, clk.pos());
        async_reset_signal_is(rst, 0);

        SC_CTHREAD(proc, clk.pos());
        async_reset_signal_is(rst, 0);
        
        SC_CTHREAD(threadProc, clk.pos());
        async_reset_signal_is(rst, 0);
        
        SC_METHOD(methodProc);
        sensitive << a << b;

        SC_CTHREAD(multiStateProc, clk.pos());
        async_reset_signal_is(rst, 0);
        
        SC_CTHREAD(waitn, clk.pos());
        async_reset_signal_is(rst, 0);
        
        SC_CTHREAD(loopProc, clk.pos());
        async_reset_signal_is(rst, 0);

        SC_CTHREAD(breakProc, clk.pos());
        async_reset_signal_is(rst, 0);
        
        SC_METHOD(uniqProc);
        sensitive << b;
    }    
    
    void newProc() 
    {
        wait();
        while (true) {
            auto var = mp->i;
            wait();
        }
    }
    
    void proc() 
    {
        int i = (C) ? D : DD;
        wait();
        while (true) {
            bool b = struct1.mb_read();
            sig = E;
            sig = EE;
            wait();
        }
    }
    
    sc_out<int>    a;   // Member channel
    sc_signal<int> b;   // Member channel
    int c;              // Member non-channel
    int d;              // Member non-channel

    void threadProc() {
        int j = 0;          
        a = 0;
        c = 0;
        d = 1;
        wait();

        while (true) {
            a = j;                  // Channel a is defined
            int i = 0;              // Local j is defined before use
            j = b + i;              // Local j is used before define
            c = b;                  // Non-channel c is defined before use
            i = a.read() + c + d;   // Local i is not used
            d = a.read();           // Non-channel d is used before define
            wait();
        }
    }
    
    void methodProc() {
        bool x;
        int i;
        i = a.read();
        x = i == b.read();
        sig = (x) ? i : 0;
    }
    
   // Thread process example
   void multiStateProc() {
       sc_uint<16> x = 0;
       sig = 1;              
       wait();                    // STATE 0

       while (true) {
          sc_uint<8> y = a.read(); 
          x = y + 1;
          wait();                 // STATE 1
          sig = x;           
       }
    }
   
    void waitn () {
        wait();
        while (true) {
            wait(3);
        }
    }

    // Loop with wait    
    sc_signal<bool> enable;
    sc_signal<bool> ready;
    sc_signal<bool> stop;
    
    void loopProc() {
        enable = 0;
        wait();                     // STATE 0
        while (true) {
            for (int i = 0; i < 3; ++i) {
                enable = 0;
                wait();             // STATE 1
            }
            enable = 1;
        }
    } 

    void breakProc() {
        ready = 0;
        wait();                     // STATE 0
        while (true) {
            wait();                 // STATE 1
            while (!enable) {
                if (stop) break;
                ready = 1;
                wait();             // STATE 2
            }
            ready = 0;
        }
    }   
    
    sc_signal<int> out;
    void uniqProc() {
        out = b;		
        bool b;            // b_1
        {
            bool b;        // b_2 
            out = b;
        }
        out = b;
    }
    
    SCT_ASSERT(s1, SCT_TIME(1), s2, clk.pos());
  
};


SC_MODULE(tb) {

    sc_clock clk{"clk", 1, SC_NS};
    sc_signal<int> a;
    
    MyModule top_mod{"top_mod"};

    SC_CTOR(tb) {
        top_mod.clk(clk);
        top_mod.a(a);
    }

};


int sc_main(int argc, char **argv) {

    cout << "test_promote_ports\n";

    tb tb_inst{"tb_inst"};
    sc_start();

    return 0;
}


