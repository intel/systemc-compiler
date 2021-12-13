/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Function call in loop condition
struct A : public sc_module 
{
    sc_in<bool>         clk;
    sc_signal<bool>     nrst;

    SC_HAS_PROCESS(A);

    A(const sc_module_name& name) : 
        sc_module(name)
    {
//        SC_METHOD(meth_dowhile1);
//        sensitive << s;
        
        SC_CTHREAD(thread_for1, clk.pos());
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(thread_for2, clk.pos());
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(thread_for3, clk.pos());
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(thread_for4, clk.pos());
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(thread_for5, clk.pos());
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(thread_for6, clk.pos());
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(thread_for7, clk.pos());
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(thread_for8, clk.pos());
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(thread_for9, clk.pos());
        async_reset_signal_is(nrst, 0);
        
        
        SC_CTHREAD(thread_while1, clk.pos());
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(thread_while2, clk.pos());
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(thread_while3, clk.pos());
        async_reset_signal_is(nrst, 0);
        
        
        SC_CTHREAD(thread_dowhile1, clk.pos());
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(thread_dowhile2, clk.pos());
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(thread_dowhile3, clk.pos());
        async_reset_signal_is(nrst, 0);
    }
    
    sc_signal<int> s;
    
    unsigned h() {
        return s.read() ? 4 : 5;
    }
    
    void meth_dowhile1() 
    {
        unsigned i = 0;
        do {
           i++;
        } while (f(i));
    }
    
    void mem_init_for() 
    {
        bool a[3];
        for (unsigned i = 0; i < 3; i++) {
            a[i] = i % 2;
            wait();
        }
        wait();
        
        while (true) {
            wait();
        }
    }
    
    void thread_for1() 
    {
        wait();
        for (unsigned i = 0; i < h(); i++) wait();
        wait();
        
        while (true) {
            unsigned i = 0;
            for (; i < h()+1; ) {
               i++;
               wait();
            }
            wait();
        }
    }
    
    unsigned f2(unsigned par) {
        par--;
        return (par+1);
    }
    void thread_for2() 
    {
        wait();
        
        while (true) {
            for (unsigned i = 0; i < f2(i); ++i) {
               wait();
            }
            wait();
        }
    }
    
    unsigned f3(unsigned par) {
        return s.read() ? par+1 : par+2;
    }
    void thread_for3() 
    {
        const int N = 4;
        wait();
        
        while (true) {
            for (unsigned i = 0; i < f3(N); ++i) {
                wait();
                if (s.read()) continue;
                i++;
            }
            wait();
        }
    }
    
    unsigned f4(unsigned par1, sc_uint<5> par2) {
        for (int j = 0; j < par2; ++j) {
            par1++;
        }
        return par1;
    }
    void thread_for4() 
    {
        wait();
        
        while (true) {
            for (unsigned k = 0; k < 3; ++k) {
                for (unsigned i = 0; i < f4(k, i); ++i) {
                    wait();
                }
                wait();
            }
        }
    }
    
    // Function call in function in for condition 
    unsigned f5a(unsigned par2) {
        return (par2+1);
    }
    unsigned f5(unsigned par1) {
        unsigned j = f5a(par1+1);
        return j;
    }
    void thread_for5() 
    {
        wait();
        
        while (true) {
            for (unsigned i = 0; i < f5(s.read()); ++i) {
                wait();
            }
            wait();
        }
    }
    
    // While loop in function in for condition
    unsigned f6a(unsigned par3) {
        return (par3+1);
    }
    unsigned f6(unsigned par1, sc_uint<5> par2) {
        int j = 0; 
        while (j < par2) {
            par1 += f6a(++j);
        }
        return par1;
    }
    void thread_for6() 
    {
        wait();
        
        while (true) {
            for (unsigned i = 0; i < f6(i, 3); ++i) {
                wait();
            }
            wait();
        }
    }  
    
    // Two functions in for condition
    unsigned f7a(unsigned par1) {
        return (par1+3);
    }
    unsigned f7b(sc_uint<5> par2) {
        unsigned j = par2 * 2; 
        return j;
    }
    void thread_for7() 
    {
        wait();
        
        while (true) {
            for (unsigned i = 0; f7a(i) > f7b(i); ++i) {
                wait();
            }
            wait();
        }
    } 
    
    // Two functions in for condition
    unsigned f8a(int par1, int par2) {
        return (par1>par2);
    }
    unsigned f8b(unsigned par3) {
        return par3 * 2;
    }
    void thread_for8() 
    {
        wait();
        
        while (true) {
            for (unsigned i = 0; f8a(i, s.read()) || f8b(s.read()) > 10; ++i) {
                wait();
            }
            wait();
        }
    } 

    bool f9(unsigned par1) {
        if (par1 > 3) {
            return s.read();
        } else {
            return par1-1;
        }
    }
    void thread_for9() 
    {
        wait();
        
        while (true) {
            if (s.read()) {
                wait();
            } else {
                for (unsigned i = 0; f9(i+1); ++i) {
                    wait();
                }
                wait();
            }
        }
    }
    
//----------------------------------------------------------------------------
    
    bool f(unsigned par) {
        return (par < 3);
    }
    
    bool g() {
        return s.read();
    }
    
    void thread_while1() 
    {
        wait();
        
        while (!g()) wait();
        wait();
        
        while (true) {
            unsigned i = 0;
            while (f(i)) {
               i++;
               wait();
            }
            wait();
        }
    }
    
    void thread_while2() 
    {
        wait();
        
        while (true) {
            unsigned i = 0;
            unsigned k = 1;
            while (f(i)) {
               for (int j = 0; j < 3; ++j) k++;
               i += k;
               wait();
               if (s.read()) continue;
            }
            wait();
        }
    }
    
     void thread_while3() 
    {
        wait();
        
        while (true) {
            unsigned i = 0;
            while (s.read() && f(i)) {
               while (true) {
                   wait();
                   if (s.read()) break;
                   i--;
               }
               i++;
            }
            wait();
        }
    }
    
    void thread_dowhile1() 
    {
        wait();
        
        do {
            wait();
        } while (!g());
        wait();
        
        while (true) {
            unsigned i = 0;
            do {
               i++;
               wait();
               i = i + 1;
            } while (f(i));
        }
    }
    
    void thread_dowhile2() 
    {
        wait();
        
        while (true) {
            unsigned i = 0;
            do {
                do {
                   i++;
                   wait();
                } while (g());
                
                wait();
            } while (s.read());
            wait();
        }
    }
    
    void thread_dowhile3() 
    {
        wait();
        
        while (true) {
            unsigned i = 0;
            do {
               i++;
               wait();
            } while (f(i) || g());
            
            do {
               i--;
               wait();
            } while (f(i) && g());
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

