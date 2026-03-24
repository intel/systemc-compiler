/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Register next assignment optimization if it is assigned before used

struct Simple {
    static const unsigned A = 2;
    int a;
    sc_uint<3> b;

    Simple& operator=(const Simple& other)
    {a = other.a; b = other.b; return *this;}
    inline friend bool operator==(const Simple& lhs, const Simple& rhs)
    {return (lhs.a == rhs.a && lhs.b == rhs.b);}
    inline friend std::ostream& operator<< (std::ostream& os, const Simple &obj)
    {return os;}
    inline friend void sc_trace(sc_trace_file*& f, const Simple& val, std::string name) 
    {}
};

struct A : public sc_module
{
    sc_in_clk           SC_NAMED(clk);
    sc_in<bool>         SC_NAMED(rst);

    sc_signal<int>      SC_NAMED(s);

    SC_CTOR(A) 
    {
        for (int i = 0; i < 3; ++i) {
            ap[i] = new sc_signal<unsigned>("name");
            aa[i].init(2);
        }
        w4 = new sc_signal<unsigned>("w4");
        w5 = new sc_signal<unsigned>("w5");
        w6 = new sc_signal<unsigned>("w6");
        
        SC_CTHREAD(var_assign, clk.pos());
        async_reset_signal_is(rst, true);

        SC_CTHREAD(sig_assign, clk.pos());
        async_reset_signal_is(rst, true);

        SC_CTHREAD(sig_assign_for, clk.pos());
        async_reset_signal_is(rst, true);
        
        SC_CTHREAD(sig_assign2, clk.pos());
        async_reset_signal_is(rst, true);

        SC_CTHREAD(sig_assign3, clk.pos());
        async_reset_signal_is(rst, true);
        
        SC_CTHREAD(sig_arr_assign, clk.pos());
        async_reset_signal_is(rst, true);

        SC_CTHREAD(sig_rec_assign, clk.pos());
        async_reset_signal_is(rst, true);
        
        SC_CTHREAD(sig_ref_ptr_assign, clk.pos());
        async_reset_signal_is(rst, true);
    }

    sc_signal<int> t0;
    void var_assign() {
        int l1;
        int l2;
        wait();
        
        while (true) {
            l1 = s.read();
            wait();
            l2 = 1;
            t0 = l1;
        }
    }

    sc_signal<int> t1;
    sc_signal<int> s1;
    sc_signal<int> s2;
    sc_signal<int> s3;
    sc_signal<sc_uint<16>> s4;
    sc_signal<sc_bigint<16>> s5;
    sc_signal<sc_uint<16>> s6;
    sc_signal<sc_uint<16>> s7;      // not defined in this process     
    void sig_assign() {
        s1 = 0;
        s2 = 0;
        s3 = 0;
        s4 = 0;
        s5 = 0;
        s6 = 0;
        s7 = 0;
        t1 = 0;
        wait();
        
        while (true) {
            if (s.read()) {
                s1 = s.read();    
                t1 = s1;
            } else {
                s2 = s.read();    
                s6 = 1;
            }
            s1 = 1;                 // No @s1_next = s1
            
            if (s.read()) {
                s3 = 42;
                s5 = 1;
                s6 = 2;
            } else {
                s4 = 0;
                s5 = 2;             // No @s5_next = s5
            }
            t1 = s7.read();         // No @s7_next = s7
            wait();                 // No @t1_next = t1
        }
    }
    
    sc_signal<int> e1;
    sc_signal<int> e2;
    sc_signal<int> e3;
    sc_signal<int> e4;
    sc_signal<int> e5;
    void sig_assign_for() {
        e1 = 0;                     // No @e1_next = e1
        e2 = 0;                     // No @e2_next = e2
        e3 = 0; 
        e4 = 0;
        e5 = 0;
        wait();
        
        while (true) {
            for (int i = 0; i < 100; ++i) {
                e1 = 1;
                for (int j = 0; j < 100; ++j) {
                    e2 = 2;
                    if (j == s.read()) {
                        e3 = 3;
                        continue;
                    }
                    e4 = 4;
                }
                e5 = 5;
                wait();
            }
            e1 = 0;
            e2 = 0;
            e3 = 0;
            e4 = 0;
            while (s.read()) {e5 = 0;}
            wait();                
        }
    }
    
    sc_signal<int> t3;
    sc_signal<int> r1;
    sc_signal<unsigned> r2;
    sc_signal<unsigned> r3;
    sc_signal<unsigned> r4;
    sc_signal<sc_uint<16>> r5;
    sc_signal<sc_uint<16>> r6;
    void sig_assign2() {
        r1 = 0;
        r2 = 1;
        r3 = 0;
        r4 = 0;
        r5 = 0;
        r6 = 0;
        t3 = 0;
        wait();
        
        while (true) {
            r1 = 1;             
            r2 = 2;                         // No @r2_next = r2
            r3 = 3; 
            r4 = 4;
            if (r1) r5 = 1; else r5 = 2;    // No @r5_next = r2
            t3 = r1 + r2;
            wait();
            
            r1 = 1; 
            t3 = r3;
            while (!s.read()) {
                r2 = 2; 
                r5 = 5;
                r6 = 6;
                if (s.read()) r3 = 3;
                wait();
                r4 = s.read();
            }
            r6 = 6;
        }
    }
    
    sc_signal<int> t4;
    sc_signal<unsigned> v1;
    sc_signal<unsigned> v2;
    sc_signal<unsigned> v3;
    sc_signal<sc_uint<16>> v4;

    void f1() {
        v1 = 0;
        t4 = v1;
    }
    void f2() {
        v3 = 1;
        wait();
        v1 = 1;
        t4 = v2;
    }

    void sig_assign3() {
        v1 = 0;
        v2 = 1;
        v3 = 0;
        v4 = 0;
        t4 = 0;
        wait();
        
        while (true) {
            f1();           // No @v1_next = v1
            v2 = 1;         // No @v3_next = v3
            f2();
            v3 = 1;
            v4 = 1;
            t4 = 2;
            wait();
        }
    }
    
    sc_signal<unsigned> t5;
    sc_signal<unsigned> a1[3];
    sc_vector<sc_signal<unsigned>> SC_NAMED(a2, 3);
    sc_vector<sc_signal<sc_uint<16>>> SC_NAMED(a3, 3);
    sc_signal<unsigned> a4[3];
    sc_signal<unsigned> a5[3];
    sc_vector<sc_vector<sc_signal<sc_uint<16>>>> SC_NAMED(aa, 3);
    sc_signal<unsigned>* ap[3];
    void sig_arr_assign() {
        for (int i = 0; i != 3; ++i) {
            a1[i] = 0;
        }
        a3[1] = 0;
        ap[2]->write(42);
        *ap[0] = 1; *ap[1] = 1; *ap[2] = 1;
        aa[0][0] = 0;
        wait();
        
        while (true) {
            for (int i = 0; i != 3; ++i) {
                a1[i] = 0;
                a2[i] = i;
            }
            t5 = a1[0] + a2[0];
            a3[s.read()] = 1;
            if (a4[2] == s.read()) {
                t5 = 0;
                a5[s.read()] = 0;
                aa[0][0].write(1);
            }
            aa[0][a5[s.read()]] = 2;
            a4[1] = 0;
            *ap[s.read()] = 1;
            wait();
        }
    }
    
    
    sc_signal<Simple> t2;
    sc_signal<Simple> rec1;
    sc_signal<Simple> rec2;
    sc_signal<Simple> rec3;
    sc_signal<Simple> recarr[2];
    void sig_rec_assign() {
        Simple lrec;
        rec1 = lrec;
        rec2 = lrec;
        rec3 = lrec;
        recarr[0] = lrec;
        wait();
        
        while (true) {
            rec1 = lrec;     // No @rec1_next = rec1
            if (s.read()) {  // @rec2_next = rec2
                rec3 = lrec; // @rec3_next = rec3 
            }
            t2 = rec1;       // No @t2_next = t2
            t2 = rec2;
            t2 = rec3;
            
            recarr[1] = rec3;// @recarr_next = recarr  
            t2 = recarr[1];
            wait();
        }
    }
    
    sc_signal<unsigned> t6;
    sc_signal<unsigned> w1;
    sc_signal<unsigned> w2;
    sc_signal<unsigned> w3;
    sc_signal<unsigned>* w4;
    sc_signal<unsigned>* w5;
    sc_signal<unsigned>* w6;
    void sig_ref_ptr_assign() {
        w1 = 0;
        w2 = 1;
        *w4 = 3;
        w5->write(4);
        t6 = 0;
        auto& r3 = w3;
        *w6 = 0;
        wait();
        
        while (true) {
            auto& r1 = w1;          
            auto& r2 = w2;
            r1.write(1);                // No @w1_next = w1
            if (s.read()) r2 = 2;
            t6 = r1 + r2 + r3;          // No @t6_next = t6
            r3 = r2;
            
            w4->write(s.read());        // No @w4_next = w4
            t6 = *w4 + w5->read() + *w6;
            wait();
            *w6 = s.read() ? 1 : 2;
        }
    }
    
};

int sc_main(int argc, char **argv) 
{
    sc_clock clk {"clk", sc_time(1, SC_NS)};
    sc_signal<bool> rst;
    
    A a_mod{"a_mod"};
    a_mod.clk(clk);
    a_mod.rst(rst);

    sc_start();

    return 0;
}
