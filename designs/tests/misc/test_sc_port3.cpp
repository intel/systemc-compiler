/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by Huzaifa/mmoiseev on 07/18/19

#include <systemc.h>

// 1) two child modules A and B of the same parent C have pointer/port to each 
// other and run its function, i.e. in A process function of B is called and 
// visa versa.

template<typename T>
struct my_if : public sc_interface 
{
    virtual sc_uint<32>  f(T fval) = 0;
    virtual sc_uint<32> access_var() = 0;
    virtual void g(T& gval) = 0;
};

struct B : sc_module, my_if<sc_uint<4>> 
{
    sc_port<my_if<sc_uint<4>>> c_port;
    const sc_uint<4> bvar = 4;
    const sc_uint<3> num1 = 6;
    
    sc_signal<bool> dummy;
    
    SC_CTOR(B){
        SC_METHOD(b_proc);
        sensitive << dummy;
    }
    void b_proc() {
        c_port->f(4);

    }
    
    sc_uint<32> f(sc_uint<4> b_fvar){
        return (c_port->access_var() + num1 - b_fvar);
    }
    
    void g(sc_uint<4>& c_gvar) {
        c_gvar++;
    }
    
    sc_uint<32> access_var() {
        return bvar;
    }
};

struct C : sc_module, my_if<sc_uint<4>>
{
    sc_port<my_if<sc_uint<4>>> b_port;
    sc_uint<4> cvar;
    const sc_uint<3> num1 = 1;
    sc_signal<bool> access;
    
    SC_CTOR(C) {
        SC_METHOD(c_proc);
        sensitive << access;
    }
    
    void c_proc () {
        b_port->f(8);
        cvar = 1;
        b_port->g(cvar);
    }
    
    sc_uint<32> access_var(){
        cvar = 0;
        return cvar;
    }

    sc_uint<32> f(sc_uint<4> c_fvar){
       return (b_port->access_var() + num1 + c_fvar);
    }

    void g(sc_uint<4>& c_gvar){
        c_gvar++;
    }
};

SC_MODULE(A) {
    B b{"bname"};
    C c{"cname"};
    SC_CTOR(A){
        b.c_port(c); // (SAME AS b.c_port = &c; if B has my_if<sc_uint<4>>* c_port;)
        c.b_port(b);
    }
};

int sc_main(int argc, char **argv)
{
    A a{"a_module"};
    //sc_clock clk("clk", 1, SC_NS);
    //dut.clk(clk);
    sc_start();
    return 0;
}

