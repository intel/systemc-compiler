/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Register variables/channels/arrays in CTHREAD
struct A : public sc_module
{
    sc_in_clk               clk;
    sc_in<bool>             rst;

    sc_in<int>               in;     
    sc_out<int>              out;
    sc_signal<bool>          a;
    sc_signal<sc_uint<4>>    b;
    sc_signal<int>           c;
    sc_signal<sc_bigint<33>> d;
    sc_signal<unsigned>      e;    // Register, besides it is defined before use
    sc_signal<sc_uint<4>>    f;
    sc_signal<sc_uint<4>>    g;
    sc_signal<sc_uint<4>>    h; 
    sc_signal<sc_uint<4>>    arr[3];

    SC_CTOR(A) 
    {
        SC_CTHREAD(var_init_reset, clk.pos());
        async_reset_signal_is(rst, true);
        
        SC_METHOD(channels0);
        sensitive << a << b << c;

        SC_CTHREAD(channels1, clk.pos());
        async_reset_signal_is(rst, true);

        SC_CTHREAD(channels2, clk.pos());
        async_reset_signal_is(rst, true);

        SC_CTHREAD(variables1, clk.pos());
        async_reset_signal_is(rst, true);

        SC_CTHREAD(variables2, clk.pos());
        async_reset_signal_is(rst, true);
        
        SC_CTHREAD(variables_in_scope1, clk.pos());
        async_reset_signal_is(rst, true);
        
        SC_CTHREAD(variables_in_scope2, clk.pos());
        async_reset_signal_is(rst, true);
        
        SC_CTHREAD(local_array1, clk.pos());
        async_reset_signal_is(rst, true);

        SC_CTHREAD(local_array2, clk.pos());
        async_reset_signal_is(rst, true);
        
        SC_CTHREAD(member_array1, clk.pos());
        async_reset_signal_is(rst, true);
        
        SC_CTHREAD(local_record_array1, clk.pos());
        async_reset_signal_is(rst, true);
        
    }

    // Variable initialized in reset only
    // The generated code is OK, it needs to use local constant for this case.
    int mi;
    sc_signal<int> r0;
    
    void var_init_reset() {
        const int ci = 42;      // Comb
        int vi = 43;            // Reg
        mi = 44;                // Reg
        wait();
        
        while (true) {
            r0 = mi+vi+ci;
            wait();
        }
    }
    
    void channels0() 
    {
        bool x = a;
        sc_uint<4> y = b.read() + c;
        h = y;
    }
    
    // Channels and ports
    void channels1() 
    {
        a = 1;
        b = 0;
        out = 0;
        wait();
        
        while (true) {
            b = 42;
            c = 43;
            d = 44;
            out = in;
            e = 0;
            wait();
            
            e = 1;
            c = d.read().to_int() + e.read();
        }
    }
    
    // Channels and ports accessed at some paths
    void channels2() 
    {
        f = 0;      // RnD
        g = 0;      // WO
        arr[0] = 1;
        for (int i = 1; i < 3; ++i) {
            arr[i] = i;
        }
        wait();
        
        while (true) {
            if (a) {
                g = f;
            }
            f = arr[1];
            arr[b.read()] = in.read();
            wait();
        }
    }

// ---------------------------------------------------------------------------    
    
    // Local and member variables 
    bool k;                     // assigned in reset only
    unsigned m;                 // RnD at one path only
    sc_uint<4> n;               // not assigned in reset
    sc_uint<4> p;               // WO, not reg
    void variables1() {
        sc_uint<4> x = 1;
        k = false;              
        m = 0; 
        wait();
        
        while (true) {
            sc_uint<4> z = b.read() + n; // RnD at some paths    
            int t = m;                   // RnD 
            p = n;
            
            wait();
            
            if (a) z = 0;   
            n = z + (k ? t : 0);
            m++;
        }
    }
    
    // Variables accessed at some paths
    sc_uint<1> arrv[3];
    sc_signal<int> s1;
    sc_uint<4> w;
    void variables2() {
        arrv[0] = 1;
        sc_uint<2> r = a.read() ? arrv[0] : (sc_uint<1>)0;
        w = 0;
        s1 = 0;
        wait();
        
        while (true) 
        {
            if (b.read()) {
                auto v = r >> arrv[b.read()];
                while (c.read()) {
                    s1 = w + v;
                    wait();
                }

                s1 = v; 
            }
            wait();
        }
    }
    
    // Local variables declared in scope
    void variables_in_scope1() {
        wait();
        
        while (true) {

            if (a.read()) {
                sc_uint<16> lu = b.read();  // reg
                
                while (lu != c.read()) {
                    lu--;
                    wait();             // 1
                }
            }
            
            wait();                     // 2
        }
    }
    
    sc_signal<int> r5;
    void variables_in_scope2() {
        wait();
        
        while (true) {

            for (int i = 0; i < 10; ++i) {
                sc_bigint<8> lbi = sc_bigint<8>(b.read());  // reg
                if (a.read()) {
                    int li = i+1;       // comb
                    r5 = li;
                    wait();             // 1
                    
                    r5 = i + lbi.to_int();
                }
                wait();                 // 2
            }
            
            wait();                     // 3
        }
    }    
    
// --------------------------------------------------------------------------
    // Arrays
    
    // Local array initialized with variables
    sc_signal<int> s6;
    void local_array1()
    {
        int i = 42;                         // reg
        int k = 42;                         // comb in reset only
        bool arr1[3];                       // reg
        sc_uint<4> arr2[3];
        int arr3[3] = {1,2,k}; 
        wait();
        
        while (true) {
            int j = 43;                     // comb
            sc_uint<4> arr4[2] = {i, j+1}; 
            s6 = arr1[0];
            wait();
        }
    }
    
    // Local array registers
    sc_signal<int> s7;
    void local_array2()
    {
        bool arr1[3];                       // comb           
        arr1[0] = 1;
        sc_uint<4> arr2[3];                 // reg
        int arr3[3] = {1,2,arr1[0]};        // reg
        wait();
        
        while (true) {
            sc_uint<4> arr4[2] = {arr3[0], arr3[1]}; 
            wait();
            s7 = arr2[1] + arr4[1];
        }
    }
    
    // Member array registers
    sc_signal<int> s8;
    bool marr1[3];                       // comb           
    sc_uint<4> marr2[20];                // reg
    int marr3[3];                        // reg
    
    void member_array1()
    {
        marr1[0] = true;
        for (int i = 0; i < 20; ++i) {
            marr2[i] = i;
        }
        s8 = marr1[0];
        wait();
        
        while (true) {
            s8 = marr2[1] + marr3[1];
            marr3[s7.read()] = s6.read();
            wait();
        }
    }
    
// --------------------------------------------------------------------------
    
    struct Simple {
        bool a;
        int b;
        Simple() = default;
        Simple(int par) : a(false), b(par) {}
    };
    
    // Local record array 
    sc_signal<int> s9;
    void local_record_array1()
    {
        Simple rarr0[3];        // reg
        Simple rarr1[3];        // reg
        rarr0[0].a = true;
        rarr0[0].b = 0;
        wait();
        
        while (true) {
            Simple rarr2[2];    // comb
            rarr2[1].b = 42;
            s9 = rarr0[0].b + rarr1[0].b + rarr2[1].b;
            rarr1[s8.read()].b = 43;
            wait();
        }
    }
    
};

int sc_main(int argc, char **argv) 
{
    sc_clock clk {"clk", sc_time(1, SC_NS)};
    sc_signal<bool> rst;
    sc_signal<int>  t;      
    
    A a_mod{"a_mod"};
    a_mod.clk(clk);
    a_mod.rst(rst);
    a_mod.in(t);
    a_mod.out(t);
    
    sc_start();

    return 0;
}
