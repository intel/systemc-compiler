/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

// Created by Huzaifa/mmoiseev on 07/18/19

#include <systemc.h>

// module A has child B, and B has child C. A has pointer/port on C and run 
// its function, and C run function of A

template<typename T>
struct my_if : public sc_interface {
    virtual sc_uint<32>  f(T fval) = 0;
    virtual sc_uint<32> access_var() = 0;
};


// CHILD C
struct C : sc_module, my_if<sc_uint<4>>  {

    // Declare some variables
    const sc_uint<4> cvar = 8;
    const sc_uint<3> num1 = 1;
    sc_signal<bool> access;
    
    // Declare a port
    // sc_port<my_if<sc_uint<4>>> sc_port_b;


    SC_CTOR(C){
        SC_METHOD(c_proc);
        sensitive << access;
    }
    void c_proc () {
    }
    sc_uint<32> access_var(){
        return cvar;
    }

    sc_uint<32> f(sc_uint<4> c_fvar){
       return (num1 + c_fvar);
   }
};

// Parent of C, and Child of A
struct B : sc_module, my_if<sc_uint<4>> {
    C c{"cname"};
    // Declare some variables
    const sc_uint<4> bvar = 4;
    const sc_uint<3> num1 = 6;
    sc_signal<bool> dummy;

    // Declare a port
    sc_port<my_if<sc_uint<4>>> sc_port_c;
    // my_if<sc_uint<4>>>* sc_port_c;

    SC_CTOR(B){
        sc_port_c(c);
        SC_METHOD(b_proc);
        sensitive << dummy;
    }
    void b_proc () {
        sc_port_c->f(4);

    }
    sc_uint<32> f(sc_uint<4> b_fvar){
        return (sc_port_c->access_var() + num1 - b_fvar);
    }
    sc_uint<32> access_var(){
        return (sc_port_c->access_var());
    }
};

// Grandparent of C, Parent of B
struct A : sc_module, my_if<sc_uint<4>> {
    B b{"bname"};

    // Declare a port
    sc_port<my_if<sc_uint<4>>> sc_port_b;

    // Declare some variables
    const sc_uint<4> bvar = 4;
    const sc_uint<3> num1 = 6;
    sc_signal<bool> dummy;

    SC_CTOR(A){
        sc_port_b(b);
        
        SC_METHOD(a_proc);
        sensitive << dummy;
    }
    
    void a_proc () {
        sc_port_b->f(4);
    }
    
    sc_uint<32> f(sc_uint<4> b_fvar){
        return (sc_port_b->access_var() + num1 - b_fvar);
    }
    
    sc_uint<32> access_var(){
        return (sc_port_b->access_var());
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

