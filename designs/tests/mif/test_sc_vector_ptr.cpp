/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"

// MIF accessed through a pointer to element of sc_vector of MIF

struct Get_if : sc_interface {
    virtual unsigned get() = 0;
    virtual void addTo(sc_sensitive& sen) = 0;
};

struct A : sc_module, Get_if {

    sc_signal<unsigned>  s{"s"};
    
    A(sc_module_name) 
    {}
    
    unsigned get() override {
        return s.read();
    }
    
    void addTo(sc_sensitive& sen) override {
        sen << s;
    }
    
};

struct B : sc_module, sc_interface {

    //Get_if*  p;
    A* p;
    
    SC_HAS_PROCESS(B);
    
    B(sc_module_name, A* a = nullptr) 
    {
        SC_METHOD(methProc);
        put_fifo_handle = new sc_process_handle(sc_get_current_process_handle());
    }
    
    sc_process_handle*      put_fifo_handle = nullptr;    
    
    void bind() {
        this->sensitive << *put_fifo_handle;
        p->addTo(this->sensitive);
    }
    
    template<class Module>
    void bind(Module& module) {
        p = &module;
        bind();
    }
    
    sc_signal<unsigned> res{"res"};
    void methProc() {
        res = 0;
        if (p) res = p->get();
    }
};

SC_MODULE(Top) {

    sc_in<bool>  clk;
    sc_signal<bool>  rstn;

    const static unsigned M = 1;
    
    sc_vector<A> a_vec;
    A   a{"a"};
    B   b;
    
    SC_CTOR(Top) : a_vec("a_vec", M), b("b"/*, &a_vec[0]*/)
    {
        // No pointer to external MIF instance
        b.p = new A("a");   // OK
        //b.p = &a;         // @p is zero, #283
        //b.p = &a_vec[0];  // @p is zero, #283
        b.bind();
        //b.bind(a_vec[0]);
    }
        
};

int sc_main(int argc, char **argv) {

    sc_clock clk("clk", 1, SC_NS);
    Top top("top");
    top.clk(clk);
    
    sc_start();

    return 0;
}
