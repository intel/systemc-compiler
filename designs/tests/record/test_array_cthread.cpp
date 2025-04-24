/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

using namespace sc_core;

// Local record arrays including array element as function value/reference parameter
template <unsigned N>
class A : public sc_module {
public:
    sc_in<bool>     clk;
    sc_signal<bool> nrst{"nrst"};
    sc_signal<sc_uint<4>> sig{"sig"};

    SC_CTOR(A) 
    {   
        SC_CTHREAD(rec_loc_arr_declare, clk.pos());  
        async_reset_signal_is(nrst, 0);
        
        SC_CTHREAD(rec_loc_arr0, clk.pos());  
        async_reset_signal_is(nrst, 0);
        
        SC_CTHREAD(rec_loc_arr0a, clk.pos());  
        async_reset_signal_is(nrst, 0);
        
        SC_CTHREAD(rec_loc_arr1, clk.pos());  
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(rec_loc_arr2, clk.pos());  
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(code_scope_state_clean, clk.pos());  
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(code_scope_state_clean1, clk.pos());  
        async_reset_signal_is(nrst, 0);
        
        SC_CTHREAD(code_scope_state_clean2, clk.pos());  
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(rec_loc_arr_multistate, clk.pos());  
        async_reset_signal_is(nrst, 0);
        
        SC_CTHREAD(rec_loc_arr3, clk.pos());  
        async_reset_signal_is(nrst, 0);
        
        SC_CTHREAD(rec_loc_arr4, clk.pos());  
        async_reset_signal_is(nrst, 0);
        
        SC_CTHREAD(rec_loc_arr5, clk.pos());  
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(rec_loc_arr6, clk.pos());  
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(rec_loc_arr7, clk.pos());  
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(rec_loc_arr8, clk.pos());  
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(rec_loc_arr9, clk.pos());  
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(rec_loc_arr10, clk.pos());  
        async_reset_signal_is(nrst, 0);

        // #141
        //SC_CTHREAD(rec_arr_elem_assign, clk.pos());  
        //async_reset_signal_is(nrst, 0);
        
        SC_CTHREAD(rec_arr_elem_field_assign, clk.pos());  
        async_reset_signal_is(nrst, 0);
        
        
        SC_CTHREAD(rec_arr_elem_const_val1, clk.pos());  
        async_reset_signal_is(nrst, 0);
        
        SC_CTHREAD(rec_arr_elem_const_val2, clk.pos());  
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(rec_arr_elem_const_val3, clk.pos());  
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(rec_arr_elem_const_val4, clk.pos());  
        async_reset_signal_is(nrst, 0);


        SC_CTHREAD(rec_arr_elem_func_param_val, clk.pos());  
        async_reset_signal_is(nrst, 0);
        
        SC_CTHREAD(rec_arr_elem_func_param_val2, clk.pos());   
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(rec_arr_elem_func_param_val3, clk.pos());   
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(rec_arr_elem_func_param_ref, clk.pos());   
        async_reset_signal_is(nrst, 0);
        
        SC_CTHREAD(rec_arr_elem_func_param_ref2, clk.pos());   
        async_reset_signal_is(nrst, 0);
        
        SC_CTHREAD(rec_arr_elem_func_param_ref3, clk.pos());   
        async_reset_signal_is(nrst, 0);
        
        SC_CTHREAD(rec_arr_func_param_val, clk.pos());  
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(rec_arr_func_param_val2, clk.pos());  
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(rec_arr_elem_func_param_cref1, clk.pos());   
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(rec_arr_elem_func_param_cref2, clk.pos());   
        async_reset_signal_is(nrst, 0);
        
        SC_CTHREAD(rec_arr_elem_func_param_cref3, clk.pos());   
        async_reset_signal_is(nrst, 0);
    }
   
//---------------------------------------------------------------------------    
    
    struct Simple {
        sc_int<2>  a;
        sc_uint<4> b;
    };
    
    // Record array declaration leads to extra registers, but not a big problem 
    sc_signal<int> t0;
    void rec_loc_arr_declare() 
    {
        wait(); 
        while (true) {
            Simple ad[2];           // reg even could be comb
            auto i = ad[0].b;
            t0 = i;
            
            wait();
        }
    }

    // Add name collision 
    int ap_b = 11;
    
    // Simple record array
    sc_signal<int> t1;
    void rec_loc_arr0() 
    {
        wait(); 
        while (true) {
            Simple ap[2];

            ap[1].b = 1;
            ap[0].b = ap[1].b + ap_b;
            int i = ap[0].b;
            t1 = i;
            
            sct_assert_array_defined(ap[0].b);
            sct_assert_array_defined(ap[1].b);
            wait();
        }
    }
    
    // Add name collision 
    int ar_b;
    sc_signal<int> t2;
    void rec_loc_arr0a() 
    {
        wait(); 
        while (true) {
            Simple ar[2];
            
            ar[1].b = sig.read()-1;
            ar_b = sig.read()+1;
            wait();

            ar[0].b = ar[1].b - ar_b;
            t2 = ar[0].b;
        }
    }

    // Record array access at unknown index
    sc_signal<int> t3;
    void rec_loc_arr1() {
        
        wait(); 
        while (true) {
            Simple br[2];            // @br is comb
            int i = sig.read();

            br[i].a = 1;
            br[i+1].b = br[i].b - 1;
            t3 = br[i].b;
            wait();
        }
    }
    
    // Several record arrays
    sc_signal<int> t4;
    void rec_loc_arr2() 
    {
        wait(); 
        while (true) {
            Simple cr[2];       // @cr is comb
            Simple crr[2][3];   // @crr is comb

            cr[1].a = 2;
            crr[1][2].a = cr[1].a;
            cr[0].b = 42;
            int c = crr[1][0].b + cr[0].b;
            t4 = c;
            wait();
        }
    }
    
    // Simple code scope, check local variable removed from state by level 
    sc_signal<int> t5;
    void code_scope_state_clean() 
    {
        int k = sig.read();
        wait(); 
        
        while (true) {
            {
                Simple dd[2];           // comb
                dd[k].a = 1;
            }
            int j = k;
            t5 = j;
            wait();
        }
    }
    
    // Simple code scope, check local variable removed from state by level 
    void code_scope_state_clean1() 
    {
        int k = sig.read();
        wait(); 
        
        while (true) {
            {
                Simple dd[2];           // comb
                dd[k].a = 1;

                wait();
                
                k++;
                dd[k].a = 2;
            }
            int j = k;
            
            wait();
        }
    }
    
    // Simple code scope, check local variable removed from state by level 
    void code_scope_state_clean2() 
    {
        int k = sig.read();
        wait(); 
        
        while (true) {
            {
                Simple dd[2];           // comb
                dd[k].a = 1;

                wait();
                
                k++;
                dd[k].a = 2;
            }
        }
    }
    
    
    // Record in multi-state thread
    void rec_loc_arr_multistate() 
    {
        int k;
        wait(); 
        while (true) {
            Simple dd[2];           // comb
            
            k = sig.read();
            dd[k].a = 2;
            
            wait();
            
            dd[k].a = 3;
        }
    }
    
    // Several record arrays at unknown index
    void rec_loc_arr3() 
    {
        wait(); 
        while (true) {
            Simple dr[2];           // comb
            Simple drr[2][3];       // reg
            int i = sig.read();     // reg 

            dr[i].a = 2;
            drr[1][i+1].a = dr[1].a;
            wait();
            
            i = drr[1][i].b;
        }
    }
    
    // Multidimensional array with various unknown indices
    sc_signal<int> t6;
    void rec_loc_arr4() 
    {
        wait(); 
        while (true) {
            Simple err[2][3][4];    // reg
            int i = sig.read();
            int j = sig.read();

            err[1][2][3].a = j;
            err[1][j][1].b = 42;
            err[i][j+1][2].b = err[1][j][i].b;
            wait();
            
            i = err[i][j+1][2].b + err[0][1][j].b;
            t6 = i;
        }
    }
    
//---------------------------------------------------------------------------    
    // Check register created for unknown index for multidimensional array
    sc_signal<int> t7;
    void rec_loc_arr5() 
    {
        wait(); 
        while (true) {
            Simple frr[2][3];       // @frr is register
            int i = sig.read();  

            wait();
            
            frr[i][1].b = 42;
            int j = frr[1][1].b;
            t7 = j;
        }
    }

    sc_signal<int> t8;
    void rec_loc_arr6() 
    {
        wait(); 
        while (true) {
            Simple grr[2][3];       // @grr is register
            int i = sig.read();  

            wait();
            
            grr[1][i].b = 42;
            int j = grr[1][1].b;
            t8 = i + j;
        }
    }
    
    sc_signal<int> t9;
    void rec_loc_arr7() 
    {
        wait(); 
        while (true) {
            Simple hrr[2][3];       // @hrr is register
            int i = sig.read();  

            wait();
            
            hrr[1][1].b = 42;
            int j = hrr[i][1].b;
            t9 = i+j;
        }
    }
    
    sc_signal<int> t10;
    void rec_loc_arr8() 
    {
        wait(); 
        while (true) {
            Simple jrr[2][3];       // @jrr is register
            int i = sig.read();  

            wait();
            
            jrr[1][1].b = 42;
            int j = jrr[1][i].b;
            t10 = i+j;
        }
    }
    
    sc_signal<int> t11;
    void rec_loc_arr9() 
    {
        wait(); 
        while (true) {
            Simple irr[2][3];       // @irr is register
            int i = sig.read();  

            wait();
            
            irr[i][i].b = 42;
            int j = irr[i][i].b;
            t11 = i+j;
        }
    }

    sc_signal<int> t12;
    void rec_loc_arr10() 
    {
        wait(); 
        while (true) {
            Simple krr[2][3];       // @krr is comb
            int i = sig.read();  

            wait();
            
            krr[1][2].b = 42;
            int j = krr[1][2].b;
            t12 = i+j;
        }
    }
    
//---------------------------------------------------------------------------    
    
    // Record array element assign
    void rec_arr_elem_assign()
    {
        wait(); 
        while (true) {
            Simple mr[3];
            Simple lr[3];

            for (int i = 0; i < 2; i++) {
                lr[i] = mr[i+1];
                mr[i+1] = mr[i];
            }
            wait();
        }
    }    

    // Record array element field assign
    void rec_arr_elem_field_assign()
    {
        wait(); 
        while (true) {
            Simple pr[3];
            Simple rr[3];

            for (int i = 0; i < 2; i++) {
                rr[i].b = pr[i+1].b;
                pr[i+1].a = rr[i].a;
            }
            wait();
        }
    }    

//---------------------------------------------------------------------------    
    // Record array element as function parameter by value 
    sc_signal<int> t13;
    void f1(Simple par) {
        int i = par.b;
        t13 = i;
    }

    sc_signal<int> t13a;
    void f1a(Simple par) {
        int i = par.b;
        t13a = i;
    }

    sc_signal<int> t13b;
    void f1b(Simple par) {
        int i = par.b;
        t13b = i;
    }

    // Record as function parameter by value
    void rec_arr_elem_func_param_val()
    {
        wait(); 
        while (true) {
            Simple sr;
            f1(sr);
            
            wait();
        }
    }
    
    // Record array element as function parameter by value
    void rec_arr_elem_func_param_val2()
    {
        wait(); 
        while (true) {
            Simple tr[3];
            
            f1a(tr[1]);
            wait();
        }
    }
    
    
    // Record array element as function parameter at unknown index by value
    void rec_arr_elem_func_param_val3()
    {
       int i = sig.read();
        wait(); 
        while (true) {
            Simple tr[3];
            f1b(tr[i]);
            
            wait();
        }
    }
    
//---------------------------------------------------------------------------    
    // Record array element as function parameter by constant value 
    
    sc_signal<int> t14;
    void ff1(const Simple par) {
        wait();
        int i = par.a + par.b;
        t14 = i;
    }

    sc_signal<int> t14a;
    void ff1a(const Simple par) {
        wait();
        int i = par.a + par.b;
        t14a = i;
    }

    sc_signal<int> t14b;
    void ff1b(const Simple par) {
        wait();
        int i = par.a + par.b;
        t14b = i;
    }

    sc_signal<int> t14c;
    void ff1c(const Simple par) {
        wait();
        int i = par.a + par.b;
        t14c = i;
    }

    void rec_arr_elem_const_val1()
    {
        wait(); 
        while (true) {
            Simple sr;      // comb
            sr.b = sig.read();
            ff1(sr);
            
            wait();
        }
    }
    
    void rec_arr_elem_const_val2()
    {
        wait(); 
        while (true) {
            Simple sr;      // reg
            wait();
            
            ff1a(sr);
        }
    }
    
    void rec_arr_elem_const_val3()
    {
        Simple crra[3];   // reg
        wait(); 
        while (true) {
            ff1b(crra[1]);
            wait();
        }
    }
    
    void rec_arr_elem_const_val4()
    {
        Simple crrb[3];   // reg
        wait(); 
        while (true) {
            wait();
            int i = sig.read();
            ff1c(crrb[i]);
        }
    }
    
//---------------------------------------------------------------------------    
    // Record array element as function parameter by reference
    sc_signal<int> t15;
    void f2(Simple& par) {
        int k = par.b;
        t15 = k;
    }
    
    void rec_arr_elem_func_param_ref()
    {
        wait(); 
        while (true) {
            Simple vr[3];   // reg
            f2(vr[1]);
            wait();

            int i = sig.read();
            f2(vr[i]);
        }
    }
    
    // Used for debug
    /*void rec_arr_elem_func_param_ref_M()
    {
        Simple vr[3];
        f2(vr[1]);
    }
    
    
    void f4(int& par) {
        par = 1;
    }
    
    void arr_elem_func_param_ref()
    {
        int vr[3]; 
        wait(); 
        while (true) {
            f4(vr[1]);
            wait();
            int i = vr[1];
        }
    }*/
    
    
    sc_signal<int> t16;
    void f2_two(Simple& par1, Simple& par2) {
        int k = par1.b + par2.b;
        t16 = k;
    }

    
    void rec_arr_elem_func_param_ref2()
    {
        Simple wrr[2];              // reg
        int i = sig.read();
        wait(); 
        
        while (true) {
            Simple wr[3];           // reg
            f2_two(wr[1], wrr[i]);
            f2_two(wr[i], wr[1]);
            
            wait();
            Simple w;               // comb
            f2_two(w, wrr[0]);
            f2_two(wr[i-1], w);
        }
    }
    
    sc_signal<int> t17;
    void f3(Simple& par) {
        par.b = 1;
    }
    
    void rec_arr_elem_func_param_ref3()
    {
        int i = sig.read();
        wait(); 
        while (true) {
            Simple vr[3];       // comb
            f3(vr[1]);
            f3(vr[1]);
            f3(vr[i+1]);

            wait();

            f3(vr[i-1]);
            t17 = vr[i].a;
        }
    }
    
//---------------------------------------------------------------------------    
    // Record array element as function parameter by constant reference
    
    sc_signal<int> t18;
    void cref_sum(const Simple& par) {
        int res = par.a + par.b;
        t18 = res;
    }

    void rec_arr_elem_func_param_cref1()
    {
        int indx = 0;
        wait(); 
        
        while (true) {
            Simple cvr[3];
            cref_sum(cvr[1]);
            wait();

            indx = sig.read();
            Simple cwr[3];
            cref_sum(cwr[indx]);
            wait();
        }
    }
    
    // Function with wait()
    int cref_wait(const Simple& par) {
        int res = par.a;
        wait();                             // 1, 3
        res = res + par.b;
        return res;
    }

    sc_signal<int> t19;
    void rec_arr_elem_func_param_cref2()
    {
        int indx = 0;
        wait(); 
        
        while (true) {
            Simple cvrr[3];
            if (sig.read()) {
                int res = cref_wait(cvrr[2]);
                t19 = res;
            }
            wait();                         // 2
        }
    }
    
    sc_signal<int> t20;
    void rec_arr_elem_func_param_cref3()
    {
        int indx = 0;
        wait(); 
        
        while (true) {
            if (sig.read() < 3) {
                indx = sig.read();
                Simple cwrr[3];
                cref_wait(cwrr[indx]);
                t20 = cwrr[indx].a;
            }
            wait();                         // 2
        }
    }
    

//---------------------------------------------------------------------------    
    // Record array as function parameter 
    sc_signal<int> t21;
    void f5(Simple par[2]) {
        int indx = par[1].b;
        bool c = par[indx].a == 2;
        t21 = c;
    }

    sc_signal<int> t21a;
    void f5a(Simple par[2]) {
        int indx = par[1].b;
        bool c = par[indx].a == 2;
        t21a = c;
    }

    void rec_arr_func_param_val()
    {
        wait(); 
        while (true) {
            Simple ar[2];   // reg
            f5(ar);
            wait();
        }
    }
    
    void rec_arr_func_param_val2()
    {
        wait(); 
        while (true) {
            Simple ar[2];   // reg
            int i = sig.read();
            ar[i].a = 0; ar[i].b = 1;
            
            wait();
            
            f5a(ar);
        }
    }    
    
};

class B_top : public sc_module {
public:
    sc_in<bool>     clk;
    A<1> a_mod{"a_mod"};

    SC_CTOR(B_top) {
        a_mod.clk(clk);
    }
};

int sc_main(int argc, char *argv[]) 
{
    sc_clock clk("clk", 1, SC_NS);
    B_top b_mod{"b_mod"};
    b_mod.clk(clk);
    
    sc_start();
    return 0;
}

