/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

struct Simple {
    int         a;
    sc_uint<3>  b;
    
    Simple() = default;
    Simple(sc_uint<3> par) : b(par) {a = 42;}

    Simple& operator=(const Simple& other)
    {a = other.a; b = other.b; return *this;}
    inline friend bool operator==(const Simple& lhs, const Simple& rhs)
    {return (lhs.a == rhs.a && lhs.b == rhs.b);}
    inline friend std::ostream& operator<< (std::ostream& os, const Simple &obj)
    {return os;}
    inline friend void sc_trace(sc_trace_file*& f, const Simple& val, std::string name) 
    {}
};

// Check unused variables/statements remove for record signals/ports and their arrays
struct A : public sc_module 
{
    sc_in<bool>         clk;
    sc_signal<bool>     nrst;
    sc_signal<sc_uint<4>> s;
    sc_signal<sc_uint<4>> r;

    // Ports never removed
    sc_in<Simple>   i1;
    sc_out<Simple>  o1;
    sc_vector<sc_in<Simple>> ivec1{"ivec1", 3};
    sc_vector<sc_vector<sc_out<Simple>>> ovec1{"ovec1", 3};
    
    sc_signal<Simple>  t0;
    sc_signal<Simple>  t1;
    sc_signal<Simple>  t2;
    sc_signal<Simple>  t3;             // removed

    sc_vector<sc_signal<Simple>> vec0{"vec0", 3};
    sc_vector<sc_signal<Simple>> vec1{"vec1", 3};
    sc_vector<sc_signal<Simple>> vec2{"vec2", 3};   // removed    
    sc_vector<sc_signal<Simple>> vec3{"vec2", 3};
    sc_vector<sc_vector<sc_signal<Simple>>> vvec0{"vvec0", 3};
    sc_vector<sc_vector<sc_signal<Simple>>> vvec1{"vvec1", 3};
    sc_vector<sc_vector<sc_signal<Simple>>> vvec2{"vvec2", 3};  // removed    
    sc_vector<sc_vector<sc_signal<Simple>>> vvec3{"vvec3", 3}; 
    sc_vector<sc_vector<sc_signal<Simple>>> vvec4{"vvec4", 3};  // removed    

    sc_vector<sc_signal<Simple>> vec0t{"vec0", 3};
    sc_vector<sc_signal<Simple>> vec1t{"vec1", 3};
    sc_vector<sc_signal<Simple>> vec2t{"vec2", 3};   // removed    
    sc_vector<sc_signal<Simple>> vec3t{"vec2", 3};
    sc_vector<sc_vector<sc_signal<Simple>>> vvec0t{"vvec0", 3};
    sc_vector<sc_vector<sc_signal<Simple>>> vvec1t{"vvec1", 3};
    sc_vector<sc_vector<sc_signal<Simple>>> vvec2t{"vvec2", 3};  // removed    
    sc_vector<sc_vector<sc_signal<Simple>>> vvec3t{"vvec3", 3}; 
    sc_vector<sc_vector<sc_signal<Simple>>> vvec4t{"vvec4", 3};  // removed    

    SC_HAS_PROCESS(A);

    A(const sc_module_name& name) : 
        sc_module(name) {
        
        for (int i = 0; i < 3; i++) {
            ovec1[i].init(2);
            vvec0[i].init(2);
            vvec1[i].init(2);
            vvec2[i].init(2);
            vvec3[i].init(2);
            vvec4[i].init(2);

            vvec0t[i].init(2);
            vvec1t[i].init(2);
            vvec2t[i].init(2);
            vvec3t[i].init(2);
            vvec4t[i].init(2);
        }
        
        SC_METHOD(remove_port); 
        sensitive << s << r << i1 << o1;

        SC_METHOD(remove_sig); 
        sensitive << s << r << t0;
        
        SC_METHOD(remove_sig_vec); sensitive << s << r;
        for (int j = 0; j < 3; j++) {
            sensitive << vec0[j];
            for (int k = 0; k < 2; k++) {
                sensitive << vvec0[j][k] << vvec3[j][k] << vvec4[j][k];
            }
        }
        
        SC_CTHREAD(remove_sig_vec_thread, clk.pos());
        async_reset_signal_is(nrst, 0);
    }
    
    
    // Ports, port array and vectors
    void remove_port() {
        // Do nothing, ports not removed
    }
    
    // Signal and pointer to signal
    void remove_sig() 
    {
        Simple ss;
        t1 = ss;
        t2 = t0.read();
    }
    
    // Signal 1D and 2D vectors
    void remove_sig_vec() 
    {
        const int& l = vec0[1].read().a;
        vec1[l] = Simple{};
        vec3[r.read()] = vvec0[1][1].read();
        Simple ss;
        vvec1[1][l].write(ss);
        Simple rr;
        rr = vvec3[s.read()][l];                        // not removed
        auto ll = vvec4[s.read()][r.read()].read().a;   // removed
    }
    
    
    void remove_sig_vec_thread() 
    {
        wait();
        while (true) {
            const int& l = r.read();
            vec1t[l] = Simple{};
            vec3t[r.read()] = vvec0t[1][1].read();
            Simple ss;
            vvec1t[1][l].write(ss);
            Simple rr;
            rr = vvec3t[s.read()][l];                        // not removed
            
            wait();
        }
    }

    
};

int sc_main(int argc, char *argv[]) 
{
    sc_signal<Simple>   i1;
    sc_signal<Simple>   o1;
    sc_signal<Simple>   ivec1[3];
    sc_signal<Simple>   ovec1[3][2];
    
    A a_mod{"a_mod"};
    sc_clock clk{"clk", 1, SC_NS};
    a_mod.clk(clk);

    a_mod.i1(i1);
    a_mod.o1(o1);
    for (int i = 0; i < 3; i++) {
        a_mod.ivec1[i](ivec1[i]);
        for (int j = 0; j < 2; j++) {
            a_mod.ovec1[i][j](ovec1[i][j]);
        }
    }
    
    sc_start();
    return 0;
}

