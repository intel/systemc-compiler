/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

using namespace sc_core;

// Non-channel array/array of pointers parameters passed to function
class A : public sc_module {
public:
  
    sc_signal<int> s;

    int* ap[3];
    int** pp;

    SC_CTOR(A) {
        
        pp = sc_new_array<int*>(3);
        for (int i = 0; i < 3; i++) {
            ap[i] = sc_new_array<int>(2);
            pp[i] = sc_new_array<int>(2);
        }
        
        SC_METHOD(array_in_func); sensitive << s;
        SC_METHOD(array_2d_in_func); sensitive << s;
        
        SC_METHOD(array_pointers_in_func); sensitive << s;
        SC_METHOD(double_pointers_in_func); sensitive << s;
    }
   
    // 1D array passed to pointer and array function parameters
    void arr_func1(int* par) {
        sct_assert_const (par[0] == 1);
        sct_assert_const (par[1] == 2);
        sct_assert_const (par[2] == 3);

        int i;
        i = par[0];
        sct_assert_const (i == 1);
        i = par[s.read()];
    }
    
    void arr_func2(int par[]) {
        sct_assert_const (par[0] == 1);
        sct_assert_const (par[1] == 2);
        sct_assert_const (par[2] == 3);
        
        int i;
        i = par[0];
        sct_assert_const (i == 1);
        i = par[s.read()];
    }

    void arr_func3(int par[3]) {
        sct_assert_const (par[0] == 1);
        sct_assert_const (par[1] == 2);
        sct_assert_const (par[2] == 3);
        
        int i;
        i = par[0];
        sct_assert_const (i == 1);
        i = par[s.read()];
    }
    
    void arr_sc_func(sc_uint<3> par[]) {
        sct_assert_const (par[0] == 1);
        sct_assert_const (par[1] == 2);
        sct_assert_const (par[2] == 3);
        
        sc_uint<3> i;
        i = par[0];
        sct_assert_const (i == 1);
    }

    void array_in_func() {
        int arr[3];
        arr[0] = 1; arr[1] = 2; arr[2] = 3;
        arr_func1(arr);
        arr_func2(arr);
        arr_func3(arr);
        
        sc_uint<3> arr_sc[3];
        arr_sc[0] = 1; arr_sc[1] = 2; arr_sc[2] = 3;
        arr_sc_func(arr_sc);
    }

    
    // 2D array passed to pointer and array function parameters
    void arr_2D_func1(int par[3][2]) {
        sct_assert_const (par[0][0] == 1);
        sct_assert_const (par[2][1] == 3);

        int i;
        i = par[1][1];
        sct_assert_const (i == 2);
        i = par[s.read()][s.read()+1];
    }
    
    void arr_2D_func2(int par[][2]) {
        sct_assert_const (par[0][0] == 1);
        sct_assert_const (par[2][1] == 3);

        int i;
        i = par[1][1];
        sct_assert_const (i == 2);
        i = par[s.read()][s.read()+1];
    }

    void array_2d_in_func() {
        int arr[3][2];
        arr[0][0] = 1; arr[1][1] = 2; arr[2][1] = 3;
        arr_2D_func1(arr);
        arr_2D_func2(arr);
    }

    // Array of pointers to arrays passed to function parameter
    void arr_ptr_func1(int* par[]) {
        sct_assert_const (par[0][0] == 1);
        sct_assert_const (par[2][1] == 3);

        int i;
        i = par[1][1];
        sct_assert_const (i == 2);
        i = par[s.read()][s.read()+1];
    }
    
    void arr_ptr_func2(int* par[3]) {
        sct_assert_const (par[0][0] == 1);
        sct_assert_const (par[2][1] == 3);

        int i;
        i = par[1][1];
        sct_assert_const (i == 2);
        i = par[s.read()][s.read()+1];
    }
    
    void array_pointers_in_func() {
        ap[0][0] = 1; ap[1][1] = 2; ap[2][1] = 3;
        sct_assert_const (ap[0][0] == 1);
        
        arr_ptr_func1(ap);
        arr_ptr_func2(ap);
    }
    
    // Double pointers passed to function parameter
    void dbl_ptr_func1(int** par) {
        sct_assert_const (par[0][0] == 1);
        sct_assert_const (par[2][1] == 3);

        int i;
        i = par[1][1];
        sct_assert_const (i == 2);
        i = par[s.read()][s.read()+1];
    }
    
    void double_pointers_in_func() {
        pp[0][0] = 1; pp[1][1] = 2; pp[2][1] = 3;
        sct_assert_const (pp[2][1] == 3);
        
        dbl_ptr_func1(pp);
    }
    
};

class B_top : public sc_module {
public:
    
    A a_mod{"a_mod"};

    SC_CTOR(B_top) {
    }
};

int sc_main(int argc, char *argv[]) {
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}

