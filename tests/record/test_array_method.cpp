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
    sc_signal<bool> sig{"sig"};

    SC_CTOR(A) 
    {
        SC_METHOD(rec_loc_arr0);  
        sensitive << sig;
        
        SC_METHOD(rec_loc_arr1);  
        sensitive << sig;

        SC_METHOD(rec_loc_arr2);  
        sensitive << sig;

        SC_METHOD(rec_loc_arr3);  
        sensitive << sig;

        SC_METHOD(rec_arr_elem_assign);  
        sensitive << sig;
        
        SC_METHOD(rec_arr_elem_const_val1);  
        sensitive << sig;
   
        SC_METHOD(rec_arr_elem_const_val2);  
        sensitive << sig;
   
        SC_METHOD(rec_arr_elem_const_val3);  
        sensitive << sig;
   
        SC_METHOD(rec_arr_elem_const_val4);  
        sensitive << sig;

        
        SC_METHOD(rec_arr_elem_func_param_val);  
        sensitive << sig;
        
        SC_METHOD(rec_arr_elem_func_param_val2); 
        sensitive << sig;
        
        SC_METHOD(rec_arr_elem_func_param_ref);  
        sensitive << sig;

        SC_METHOD(rec_arr_elem_func_param_ref2); 
        sensitive << sig;
        
        SC_METHOD(rec_arr_elem_field_func_param_val);  
        sensitive << sig;

        SC_METHOD(rec_arr_elem_field_func_param_ref);  
        sensitive << sig;

        SC_METHOD(rec_arr_func_param);  
        sensitive << sig;
        
        
        SC_METHOD(rec_arr_elem_func_param_cref1);  
        sensitive << sig;
        
        SC_METHOD(rec_arr_elem_func_param_cref2);  
        sensitive << sig;
        
        SC_METHOD(rec_arr_elem_func_param_cref3);  
        sensitive << sig;
    }
   
    
    struct Simple {
        bool a;
        int b;
    };
    
    // Add name collision 
    int ap_b;
    
    // Simple record array
    void rec_loc_arr0() 
    {
        //Simple ar;
        Simple ap[2];
        ap[0].a = true;
        ap[1].b = 12;
        ap_b = 43;
        
        ap[0].b = ap[1].b + 1;
        if (ap[1].a) {
            int i = ap[0].b >> ap[1].b;
        }
        
    }
    
    // Record array access at unknown index
    void rec_loc_arr1() {
        Simple ar[2];
        int i = sig.read();
        
        ar[i].a = false;
        ar[i+1].b = ar[i].b - 1;
    }
    
    // Several record arrays
    void rec_loc_arr2() 
    {
        Simple ar[2];
        Simple arr[2][3];
        
        ar[1].a = false;
        arr[1][2].a = !ar[1].a;
        int c = arr[1][0].b + ar[0].b;
        
        sct_assert_const(!ar[1].a);
        sct_assert_const(arr[1][2].a);
        sct_assert_unknown(arr[0][2].a);
        
        sct_assert_array_defined(ar[1].a);
        sct_assert_read(ar[1].a);
        sct_assert_array_defined(arr[1][2].a);
    }
    
    // Record array access in loop
    void rec_loc_arr3() {
        Simple ar[2];
        for (int i = 0; i < 2; i++) {
            ar[i].a = i % 2;
            ar[i].b = i;
        }
        
        int k = 0;
        for (int i = 0; i < 2; i++) {
            k += ar[i].a ? ar[i].b : 0;
        }
    }    
    
    void rec_arr_elem_assign()
    {
        Simple ar[3];
        ar[1] = ar[2];

        Simple br[2];
        br[1] = ar[0];
    }    

//---------------------------------------------------------------------------    
    // Record array element as function parameter by constant value 
    
     void ff1(const Simple par) {
        int i = par.a + par.b;
    }

    void rec_arr_elem_const_val1()
    {
        Simple sr;      // comb
        sr.b = sig.read();
        ff1(sr);
    }
    
    void rec_arr_elem_const_val2()
    {
        Simple sr;      // reg
        ff1(sr);
    }
    
    void rec_arr_elem_const_val3()
    {
        Simple crra[3];   // reg
        ff1(crra[1]);
    }
    
    void rec_arr_elem_const_val4()
    {
        Simple crrb[3];   // reg
        int i = sig.read();
        ff1(crrb[i]);
    }
    
//---------------------------------------------------------------------------    
    // Record array element as function parameter
    void f1(Simple par) {
        int i = par.b;
    }

    void f2(Simple& par) {
        int i;
        i = par.b;
    }
    
    void f3(int par1, bool par2) {
        int i = par2 ? par1 : 1;
    }

    void f4(int& par1, bool& par2) {
        int i = par2 ? par1 : 1;
    }

    void rec_arr_elem_func_param_val()
    {
        Simple ar[3];
        f1(ar[1]);
    }
    
    void rec_arr_elem_func_param_val2()
    {
        Simple ar[3][2];
        f1(ar[2][1]);
    }

    void rec_arr_elem_func_param_ref()
    {
        Simple ar[3];
        f2(ar[1]);
    }
    
    void rec_arr_elem_func_param_ref2()
    {
        Simple ar[3][2];
        f2(ar[2][1]);
    }
    
    void rec_arr_elem_field_func_param_val()
    {
        Simple ar[3];
        f3(ar[2].b, ar[1].a);
    }
    
    void rec_arr_elem_field_func_param_ref()
    {
        Simple ar[3];
        f4(ar[2].b, ar[1].a);
    }

//---------------------------------------------------------------------------    
    // Record array element as function parameter by constant reference
    
    void cref_sum(const Simple& par) {
        int res = par.a + par.b;
    }

    void rec_arr_elem_func_param_cref1()
    {
        int indx = 0;
        Simple cvr[3];
        cref_sum(cvr[1]);
        
        indx = sig.read();
        Simple cwr[3];
        cref_sum(cwr[indx]);
    }
    
    void rec_arr_elem_func_param_cref2()
    {
        int indx = 0;
        Simple cvrr[3];
        if (sig.read()) {
            cref_sum(cvrr[2]);
        }
    }
    
    void rec_arr_elem_func_param_cref3()
    {
        int indx = 0;
        indx = sig.read();
        Simple cwrr[3];
        cref_sum(cwrr[indx]);
    }
    
//---------------------------------------------------------------------------    
    // Record array as function parameter
    void f5(Simple par[2]) {
        int i = par[1].b;
        bool c = par[i].a;
    }

    // Array passed by pointer
    void rec_arr_func_param()
    {
        Simple ar[2];
        ar[0].b = 0;
        
        f5(ar);
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

