/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

// Check unused variables/statements remove for local variables 
struct A : public sc_module 
{
    sc_in<bool>         clk;
    sc_signal<bool>     nrst;
    sc_signal<sc_uint<4>> s;

    sc_signal<int>  t0;
    sc_signal<int>  t1;
    sc_signal<int>  t2;
    sc_signal<int>  t3;
    sc_signal<int>  t4;
    sc_signal<int>  t5;
    sc_signal<int>  t6;
    sc_signal<int>  t7;
    
    SC_HAS_PROCESS(A);

    A(const sc_module_name& name) : 
        sc_module(name) {
        
        SC_METHOD(remove0); sensitive << s;
        SC_METHOD(remove0a); sensitive << s;
        SC_METHOD(remove0b); sensitive << s;
        SC_METHOD(remove0c); sensitive << s;
        SC_METHOD(remove1); sensitive << s;
        SC_METHOD(remove1a); sensitive << s;
        SC_METHOD(remove_if); sensitive << s;
        SC_METHOD(remove_loops); sensitive << s;
        SC_METHOD(remove_switch); sensitive << s;
        SC_METHOD(remove_binoper); sensitive << s;
        SC_METHOD(remove_ref); sensitive << s;
        SC_METHOD(remove_constref); sensitive << s;
        SC_METHOD(remove_ptr); sensitive << s;
        
        SC_CTHREAD(remove0_thread, clk.pos());
        async_reset_signal_is(nrst, 0);
        SC_CTHREAD(remove0a_thread, clk.pos());
        async_reset_signal_is(nrst, 0);
        SC_CTHREAD(remove0b_thread, clk.pos());
        async_reset_signal_is(nrst, 0);
        SC_CTHREAD(remove0c_thread, clk.pos());
        async_reset_signal_is(nrst, 0);
        SC_CTHREAD(remove1_thread, clk.pos());
        async_reset_signal_is(nrst, 0);
        SC_CTHREAD(remove1a_thread, clk.pos());
        async_reset_signal_is(nrst, 0);
        SC_CTHREAD(remove_if_thread, clk.pos());
        async_reset_signal_is(nrst, 0);
        SC_CTHREAD(remove_loops_thread, clk.pos());
        async_reset_signal_is(nrst, 0);
        SC_CTHREAD(remove_switch_thread, clk.pos());
        async_reset_signal_is(nrst, 0);
        SC_CTHREAD(remove_binoper_thread, clk.pos());
        async_reset_signal_is(nrst, 0);
        SC_CTHREAD(remove_ref_thread, clk.pos());
        async_reset_signal_is(nrst, 0);
        SC_CTHREAD(remove_constref_thread, clk.pos());
        async_reset_signal_is(nrst, 0);
    }
    
    
    void f(int& l) {
        l++;
    }
    
    int g(int l) {
        return l+1;
    }
    
    // All the variables removed
    void remove0() 
    {
        int x; x = 1;  
        sc_uint<4> y; y = 2; 
        
        int xn; 
        sc_uint<4> yn;

        int i = 1;
        int j = i + 1;
    }
    
    void remove0_thread() 
    {
        int x; x = 1;  
        sc_uint<4> y; y = 2; 
        
        int xn; 
        sc_uint<4> yn;

        int i = 1;
        int j = i + 1;
        wait();

        while (true) {
            int x; x = 1;  
            sc_uint<4> y; y = 2; 

            int xn; 
            sc_uint<4> yn;

            int i = 1;
            int j = i + 1;
            wait();
        }
    }

    // Multi-assign not removed
    sc_uint<3> ms;
    void remove0a() 
    {
        int x; x = x = 1;  
        int z; sc_uint<4> y; y = z = 1;
        sc_uint<3> t; t = ms = 1;
        
        sc_uint<10> w; sc_uint<10> v;
        w.bit(3) = v = 1;
        
        sc_uint<10> ww; sc_uint<10> vv;
        ww = vv.range(x+1, x) = 1;
    }
    
    sc_uint<3> mst;
    void remove0a_thread() 
    {
        int x; x = x = 1;  
        int z; sc_uint<4> y; y = z = 1;
        sc_uint<3> t; t = mst = 1;
        
        sc_uint<10> w; sc_uint<10> v;
        w.bit(3) = v = 1;
        
        sc_uint<10> ww; sc_uint<10> vv;
        ww = vv.range(x+1, x) = 1;
        wait();

        while (true) {
            int x; x = x = 1;  
            int z; sc_uint<4> y; y = z = 1;
            sc_uint<3> t; t = mst = 1;

            sc_uint<10> w; sc_uint<10> v;
            w.bit(3) = v = 1;

            sc_uint<10> ww; sc_uint<10> vv;
            ww = vv.range(x+1, x) = 1;
            wait();
        }
    }
    
    
    // Comma not removed
    sc_uint<3> mt;
    void remove0b() 
    {
        sc_uint<5> x; 
        sc_biguint<4> y; 
        (x,y) = 42;
        sc_uint<3> t; 
        (t, mt) = s.read();
        
        int xx; int yy; int z;
        z = xx+1, yy++;            
    }
    
    sc_uint<3> mtt;
    void remove0b_thread() 
    {
        wait();

        while (true) {
            sc_uint<5> x; 
            sc_biguint<4> y; 
            (x,y) = 42;
            sc_uint<3> t; 
            (t, mtt) = s.read();

            int xx; int yy; int z;
            z = xx+1, yy++;            
            wait();
        }
    }
    
    
    // Index of array and bit/range not removed
    void remove0c() {
        int i = s.read();
        sc_uint<5> x;
        x = 2;
        t0 = x.bit(i);
        
        int j = s.read();
        t0 = x.range(j, j-1);
        
        bool aa[3];
        unsigned k; k = s.read();
        t0 = aa[k];
        
        bool aa_[3];                // removed   
        unsigned kk; kk = 1;        // removed
        bool a = aa_[kk];           // removed
        
        bool aaa[3][2];
        sc_uint<3> ii; unsigned jj;
        ii = 1; jj = 1;
        t0 = aaa[ii+1][jj];
    }
    
    void remove0c_thread() 
    {
        wait();

        while (true) {
            int i = s.read();
            sc_uint<5> x;
            x = 2;
            t0 = x.bit(i);

            int j = s.read();
            t0 = x.range(j, j-1);

            bool aa[3];
            unsigned k; k = s.read();
            t0 = aa[k];

            bool aa_[3];                // removed   
            unsigned kk; kk = 1;        // removed
            bool a = aa_[kk];           // removed

            bool aaa[3][2];
            sc_uint<3> ii; unsigned jj;
            ii = 1; jj = 1;
            t0 = aaa[ii+1][jj];
            wait();
        }
    }
    
    // All removed
    void remove1() 
    {
        int x; x = 1;  
        sc_uint<4> y; y = x;    // removed
        
        --x;
        
        int i = 1;
        int j = i + 1;
        
        int l = j++;
    }
    
    // All removed
    void remove1_thread() 
    {
        int x; x = 1;  
        sc_uint<4> y; y = x;    // removed
        wait();

        while (true) {
            --x;

            int i = 1;
            int j = i + 1;

            int l = j++;
            wait();
        }
    }
    
    
    // All removed
    void remove1a() 
    {
        int x; x = 1;  
        sc_uint<4> y = s.read(); 
        y += s.read();
        x /= 1;
        
        int i = 1;
        int j = i + 1;
        int l;
        l += j;
    }
    
    // All removed
    void remove1a_thread() 
    {
        int x; x = 1;  
        int i = 1;
        wait();

        while (true) {
            sc_uint<4> y = s.read(); 
            y += s.read();
            x /= 1;

            int j = i + 1;
            int l;
            l += j;
            wait();
        }
    }
    
    // All the variables removed
    void remove1b() 
    {
        int x; x = 1;  
        sc_uint<4> y; y = x; 
        
        int i = 1;
        int j = i + 1;
        
        int l = -j;
    }
    
    void remove1b_thread() 
    {
        int x; x = 1;  
        sc_uint<4> y; y = x; 
        wait();

        while (true) {
            int i = 1;
            int j = i + 1;

            int l = -j;
            wait();
        }
    }
    
    // IF and conditional operators not removed
    void remove_if() 
    {
        t1 = 0;
        
        int i = 1;
        if (i == s.read()) {
            t1 = 1;
        }
        
        int j = 1;              // not removed 
        if (j == s.read()) {}   // removed

        sc_uint<4> y; sc_int<4> x;
        y = s.read(); x = s.read();
        if (x > y) {
            t1 = 2;
        }
        
        sc_uint<10> z; sc_int<4> v; sc_int<4> w;
        z = s.read(); v = 1; w = s.read();
        t1 = z ? v : w;
        
        sc_uint<10> zz; sc_int<4> vv; sc_int<4> ww;
        zz = s.read();          // not removed
        vv = 1; ww = s.read();  // removed
        auto l = zz ? vv : ww;  // removed
    }
    
    void remove_if_thread() 
    {
        t1 = 0;
        int i = 1;
        int j = 1;              // not removed 
        wait();

        while (true) {

            if (i == s.read()) {
                t1 = 1;
            }

            if (j == s.read()) {}   // removed

            sc_uint<4> y; sc_int<4> x;
            y = s.read(); x = s.read();
            if (x > y) {
                t1 = 2;
            }

            sc_uint<10> z; sc_int<4> v; sc_int<4> w;
            z = s.read(); v = 1; w = s.read();
            t1 = z ? v : w;

            sc_uint<10> zz; sc_int<4> vv; sc_int<4> ww;
            zz = s.read();          // not removed
            vv = 1; ww = s.read();  // removed
            auto l = zz ? vv : ww;  // removed
            wait();
        }
    }
    
    
    // Loops, side effect in loop increment, not removed
    void remove_loops() 
    {
        for (int i = 0; i < 2; i++) {
            t2 = i;
        }

        unsigned jj = s.read();
        for (int j = 0; j < 2; j = j + jj) {
            t2 = j;
        }

        unsigned kk = s.read();                 // not removed
        for (unsigned k = 0; k < 2; k += kk) {} // removed
        
        unsigned n = s.read();
        while (n != 0) {
            n = n - 1;
            t2 = n;
        }
        
        sc_uint<4> m = s.read();
        while (m) {
            m = m - 1;
        }
        
        sc_int<4> p; 
        p = s.read();
        do {
            p = p - 1;
        } while (p > 0);
    }
    
    void remove_loops_thread() 
    {
        sc_uint<4> m = s.read();
        wait();

        while (true) {
            for (int i = 0; i < 2; i++) {
                t2 = i;
            }

            unsigned jj = s.read();
            for (int j = 0; j < 2; j = j + jj) {
                t2 = j;
            }

            unsigned kk = s.read();                 // not removed
            for (unsigned k = 0; k < 2; k += kk) {} // removed

            unsigned n = s.read();
            while (n != 0) {
                n = n - 1;
                t2 = n;
            }

            while (m) {
                m = m - 1;
            }

            sc_int<4> p; 
            p = s.read();
            do {
                p = p - 1;
            } while (p > 0);
            wait();
        }
    }
    
    
    // switch, not removed
    void remove_switch() 
    {
        int i; i = s.read();
        switch (i) {
            case 1: t3 = 1; break;
            default: t3 = 2;
        }

        unsigned j; j = s.read();
        switch (j) {
            case 3: t3 = 1; break;
            case 2: ;
            case 1: t3 = 2;
        }
    }
    
    void remove_switch_thread() 
    {
        wait();

        while (true) {
            int i; i = s.read();
            switch (i) {
                case 1: t3 = 1; break;
                default: t3 = 2;
            }

            unsigned j; j = s.read();
            switch (j) {
                case 3: t3 = 1; break;
                case 2: ;
                case 1: t3 = 2;
            }
            wait();
        }
    }
    

    // && and || binary operators
    void remove_binoper() 
    {
        bool b; b = (s.read() == 1);
        int i; i = s.read();
        
        t4 = 0;
        if (b || i == 2) {
            t4 = 1;
        }
        
        unsigned j; j = s.read();   // removed
        if (true || j) {            // removed
            t4 = 2;
        }
        
        sc_uint<4> m = 1;           // not removed
        sc_int<4> p; p = s.read();  // removed
        if (m != 1 && p == 2) {     // removed
            t4 = 3;                 // removed
        }
    }
    
    void remove_binoper_thread() 
    {
        wait();

        while (true) {
            bool b; b = (s.read() == 1);
            int i; i = s.read();

            t4 = 0;
            if (b || i == 2) {
                t4 = 1;
            }

            unsigned j; j = s.read();   // removed
            if (true || j) {            // removed
                t4 = 2;
            }

            sc_uint<4> m = 1;           // not removed
            sc_int<4> p; p = s.read();  // removed
            if (m != 1 && p == 2) {     // removed
                t4 = 3;                 // removed
            }
            wait();
        }
    }
    
    
    // references, not removed
    void remove_ref() 
    {
        int i; i = 1;
        int& ri = i;
        t5 = ri;
        
        int j; j = 1;               // removed
        const int& rj = j;          // removed
        int l = rj;                 // removed
        
        sc_uint<4> x; x = s.read();
        sc_uint<4>& rx = x;
        t5 = 1 + rx;

        sc_uint<4> y; y = s.read();
        sc_uint<4>& ry = y;
        sc_uint<4>& rry = ry;
        t5 = rry / 2;
    }
    
    void remove_ref_thread() 
    {
        int i; i = 1;
        int j; j = 1;               // removed
        const int& rj = j;          // removed
        int l = rj;                 // removed
        wait();

        while (true) {
            int& ri = i;
            t5 = ri;


            sc_uint<4> x; x = s.read();
            sc_uint<4>& rx = x;
            t5 = 1 + rx;

            sc_uint<4> y; y = s.read();
            sc_uint<4>& ry = y;
            sc_uint<4>& rry = ry;
            t5 = rry / 2;
            wait();
        }
    }
    
    // constant references, not removed
    void remove_constref() 
    {
        int i; i = 1;
        const int& ri = i;
        t5 = ri;
        
        const int& rc = 42;         
        t5 = rc;

        int j; j = 1;               // removed
        const int& rj = j;          // removed
        int l = rj;                 // removed

        sc_uint<4> x; x = s.read();
        const sc_uint<4>& rx = x+1;
        t5 = rx;

        sc_uint<4> y; y = s.read();
        const sc_uint<4>& ry = y;
        const sc_uint<4>& rry = ry;
        t5 = rry;
    }
    
    void remove_constref_thread() 
    {
        int i; i = 1;
        const int& ri = i;
        sc_uint<4> y; 
        const sc_uint<4>& ry = y;
        const sc_uint<4>& rry = ry;
        wait();

        while (true) {
            t5 = ri;

            const int& rc = 42;         
            t5 = rc;

            int j; j = 1;               // removed
            const int& rj = j;          // removed
            int l = rj;                 // removed

            sc_uint<4> x; x = s.read();
            const sc_uint<4>& rx = x+1;
            t5 = rx;

            t5 = rry;
            wait();
        }
    }
    

    // pointers
    int m;
    int* mp = &m;
    int k = 42;                     // not removed    
    int* kp = &k;
    int n = 43;                     // not removed
    int* np = &n;
    void remove_ptr() 
    {
        *mp = -42;
        t6 = *mp;
        
        t6 = *kp;
        
        bool b = np;
        t6 = b;
    }
};

int sc_main(int argc, char *argv[]) 
{
    sc_clock clk{"clk", 1, SC_NS};
    A a_mod{"a_mod"};
    a_mod.clk(clk);

    sc_start();
    return 0;
}

