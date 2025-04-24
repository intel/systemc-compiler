/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"

using namespace sc_core;

// Array and pointer single/multi-dimensional member/local arrays
class A : public sc_module {
public:
    sc_in_clk clk{"clk"};
    sc_signal<bool> rst;
    sc_signal<sc_uint<2>> sig;

    SC_CTOR(A) 
    {
        for (int i = 0; i < 2; i++) {
            zp[i] = sc_new<int>();
            yp[i] = sc_new<int>();
            xp[i] = sc_new<int>();
            qp[i] = sc_new<int>();
            
            for (int j = 0; j < 2; j++) {
                zpp[i][j] = sc_new<int>();
                ypp[i][j] = sc_new<int>();
                xpp[i][j] = sc_new<int>();
                qpp[i][j] = sc_new<int>();
            }
        }
        
        SC_METHOD(array_in_method1);
        sensitive << sig;

        SC_METHOD(array_in_method2);
        sensitive << sig;

        SC_CTHREAD(array_in_thread1, clk.pos());
        async_reset_signal_is(rst, true);

        SC_CTHREAD(array_in_thread2, clk.pos());
        async_reset_signal_is(rst, true);

        SC_CTHREAD(array_use_def0, clk.pos());
        async_reset_signal_is(rst, true);
        
        SC_CTHREAD(array_use_def1, clk.pos());
        async_reset_signal_is(rst, true);

        SC_CTHREAD(array_use_def2, clk.pos());
        async_reset_signal_is(rst, true);
        
        SC_CTHREAD(array_use_def_ptr1, clk.pos());
        async_reset_signal_is(rst, true);

        SC_CTHREAD(array_use_def_ptr2, clk.pos());
        async_reset_signal_is(rst, true);

        SC_CTHREAD(array_use_def_sig, clk.pos());
        async_reset_signal_is(rst, true);

        SC_CTHREAD(array_elem_trhead1, clk.pos());
        async_reset_signal_is(rst, true);

        SC_CTHREAD(array_elem_trhead2, clk.pos());
        async_reset_signal_is(rst, true);

        //SC_METHOD(array_elem_trhead1);
        //sensitive << sig;
    }

    // 1D array access
    sc_uint<3> r[3];
    sc_signal<int> t0;
    void array_in_method1()
    {
        r[0] = 1; r[1] = 2;
        int i = sig.read();
        r[i] = 3;
        int j = r[i+1] + r[2];
        t0 = j;
    }
    
    // 2D array access
    sc_uint<4> rr[3][2];
    sc_signal<int> t1;
    void array_in_method2()
    {
        for (int k = 0; k < 3; k++) {
            rr[k][0] = k; 
        }
        int i = sig.read();
        rr[1][i] = 2;
        int j = rr[2][1];
        rr[i][j] = 3;
        j = rr[i+1][j-1] + 1;
        t1 = j;
    }
    
    // 1D array access in CTHREAD
    bool s[3];
    sc_signal<int> t2;
    void array_in_thread1()
    {
        s[1] = false;
        wait();
        
        while (true) {
            int i = sig.read();
            s[i] = i == 1;
            bool b = s[i+1] || s[i];
            t2 = b;
            wait();
        }
    }
    
    // 2D array access in CTHREAD
    int ss[2][3];
    void array_in_thread2()
    {
        for (int k = 0; k < 2; k++) {
            for (int l = 0; l < 3; l++) {
                ss[k][l] = k+l; 
            }
        }
        int i = 0;
        wait();
        
        while (true) {
            i = sig.read();
            ss[i+1][0] = 0;
            int j = ss[i][i] * i;
            
            if (ss[i][j]) {
                j = sig.read();
            }
            wait();
            
        }
    }

// ----------------------------------------------------------------------------    
    
    sc_signal<int> t3;
    void array_use_def0() 
    {
        bool  v[3];
        bool  w[3][3];
        int j = sig.read();
        wait();
        
        while (true) {
            w[1][1] = 1;
            bool c = w[1][1];
            t3 = c;

            wait();
        }
    }
    
    // UseDef checks for arrays
    sc_signal<int> t4;
    void array_use_def1() 
    {
        wait();
        
        while (true) {
            // @z is not register
            int  z[2];
            z[1] = 2;
            int i = 1 + z[1];
            z[0] = 1;
            i = z[0] + z[1];
            
            //@u is not register
            int u[2];
            u[1] = 0;
            i = u[0];
            
            // @y is not register
            int y[2];
            int j = sig.read();
            y[j] = 0;
            i = y[0];
            
            // @x is not register
            int x[2];
            x[j] = 0;
            i = x[j];

            // @w & @ww are register
            int w[2];
            int ww[2] = {};
            w[j] = 0;

            t4 = i;
            wait();

            t4 = w[j] + ww[j];
        }
    }
    
    
    // UseDef checks for arrays through wait()    
    sc_signal<int> t5;
    void array_use_def2() 
    {
        wait();
        
        while (true) {
            // @z is register
            int  z[2];
            //@u is register
            sc_uint<3> u[2];
            // @y is not register
            sc_int<4> y[2];

            wait();
            z[1] = 2;
            int i = z[0];
            
            u[0] = 3;
            i = u[0];
            
            y[1] = 4;
            
            wait();
            i = u[0];
            
            y[1] = 5;
            i = y[1];
            t5 = i;
            wait();
        }
    }
    
// ----------------------------------------------------------------------------    
    
    // UseDef checks for pointer arrays
    int*  zp[2];    // reg
    int*  yp[2];    // reg
    int*  xp[2];    // reg
    int*  qp[2];    // comb
    
    sc_signal<int> t6;
    void array_use_def_ptr1() 
    {
        int j = sig.read();
        wait();
        
        while (true) {
            *zp[1] = 3;
            int i = *zp[0];
            
            *yp[j] = 4;
            i = *yp[1];

            *xp[0] = 5; *xp[1] = 6;
            i = *xp[j];

            *qp[1] = 7;
            i = *qp[1];
            t6 = i;
            wait();
        }
    }
    
    // UseDef checks for pointer arrays
    int*  zpp[2][2];    // reg
    int*  ypp[2][2];    // reg
    int*  xpp[2][2];    // reg
    int*  qpp[2][2];    // comb
    
    sc_signal<int> t7;
    void array_use_def_ptr2() 
    {
        int j = sig.read();
        wait();
        
        while (true) {
            *zpp[1][0] = 3;
            int i = *zpp[1][1];
            
            *ypp[j][1] = 4;
            i = *ypp[0][1];

            *xpp[0][0] = 5; *xpp[1][0] = 6; *xpp[0][1] = 6; *xpp[1][1] = 6;
            i = *xpp[j][0];

            *qpp[1][1] = 7;
            i = *qpp[1][1];
            t7 = i;

            wait();
        }
    }    
    
// ----------------------------------------------------------------------------    
    
    // Channels written in process are register there anyway
    sc_signal<bool>  ws[3][3];
    sc_signal<bool>  vs[3][3];
    sc_signal<bool>  ys[3][3];
    
    sc_signal<int> t8;
    void array_use_def_sig() 
    {
        int j = sig.read();
        wait();
        
        while (true) {
            ws[1][j] = 1;
            vs[j][1] = 1;
            ys[0][1] = 1;
            
            bool c = ws[1][1];
            c = vs[1][0];
            c = ys[0][1];
            t8 = c;

            wait();
        }
    }    
    
// ----------------------------------------------------------------------------    

    // Array elements accessed in two threads, warning/error reported
    sc_signal<bool>  sa[3];
    bool sb[3];

    sc_signal<int> t9;
    void array_elem_trhead1() 
    {
        wait();
        
        while (true) {
            sa[1] = 0;            
            sb[1] = 0;              
            t9 = sa[1];
            //t9 = sb[1];           // Error reported, otherwise @sb optimized
            wait();
        }
    }

    sc_signal<int> t10;
    void array_elem_trhead2() 
    {
        wait();
        
        while (true) {
            //sa[2] = 0;            // Error reported   
            sb[2] = 0;              
            t10 = sb[2];            
            wait();
        }
    }
};

int sc_main(int argc, char *argv[]) {
    sc_clock clk("clk", 1, SC_NS);
    A a{"a"};
    a.clk(clk);
    sc_start();
    return 0;
}
 
