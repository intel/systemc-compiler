/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"
#include <sct_assert.h>

using namespace sc_core;

// Defined and read not defined values tests
class A : public sc_module
{
public:
    sc_in<bool>         clk{"clk"};
    sc_in<bool>         nrst{"nrst"};
    sc_in<bool>         a{"a"};
    sc_out<bool>        b{"b"};

    int                 m;
    int                 k;
    int                 n;
    
    int*                pi;
    int*                pa[3];
    int*                pb[3];
    sc_uint<3>*         ppa[3][2];
    sc_uint<3>*         ppb[3][2];
    sc_uint<3>*         ppc[3][2];
    sc_uint<3>*         ppd[3][2];

    sc_signal<bool>*    pc;
    sc_signal<bool>*    pca[3];
    sc_signal<bool>*    ppca[3][2];

    SC_CTOR(A)
    {
        // Dynamically allocated array
        pc = sc_new_array< sc_signal<bool> >(2);
        pi = sc_new<int>();
        
        for (int i = 0; i < 3; i++) {
            pa[i] = sc_new<int>();
            pb[i] = sc_new<int>();
            pca[i] = new sc_signal<bool>("pca");
            for (int j = 0; j < 2; j++) {
                ppa[i][j] = sc_new<sc_uint<3>>(); 
                ppb[i][j] = sc_new<sc_uint<3>>(); 
                ppc[i][j] = sc_new<sc_uint<3>>(); 
                ppd[i][j] = sc_new<sc_uint<3>>(); 
                ppca[i][j] = new sc_signal<bool>("ppca");
            }
        }
        
        
        SC_METHOD(read_pointer); sensitive << a;
        
        SC_METHOD(read_pointer_array_unknown1); sensitive << a << *ppca[0][0] << *pca[0];
        SC_METHOD(read_pointer_array_unknown2); sensitive << a << *ppca[0][0] << *pca[0];
        SC_METHOD(read_pointer_array_unknown3); sensitive << a << *ppca[0][0] << *pca[0];
         
        SC_METHOD(define_pointer_array_unknown1); sensitive << a;

        SC_METHOD(read_array_unknown1); sensitive << a;
        SC_METHOD(read_array_unknown2); sensitive << a;
        SC_METHOD(read_array_unknown3); sensitive << a;
        SC_METHOD(read_array_unknown4); sensitive << a;
        SC_METHOD(read_array_unknown_sc_type); sensitive << a;
        
        SC_METHOD(define_array_unknown1); sensitive << a;
        SC_METHOD(define_array_unknown2); sensitive << a;
        SC_METHOD(define_array_unknown_sc_type); sensitive << a;
        
    }

    void read_pointer()
    {
        int i = *pi;
        sct_assert_read(*pi);
    }
    
    void read_pointer_array_unknown1()
    {
        int i;
        i = *pa[a]; 
        sct_assert_read(pa);
        sct_assert_defined(pa, false);
        
        sc_uint<3> x = *ppa[a][0];
        sct_assert_read(ppa);
        sct_assert_defined(ppa, false);
        
        bool b = pca[a]->read();
        sct_assert_read(pca);
        sct_assert_defined(pca, false);
        
        b = ppca[a][0]->read();
        sct_assert_read(ppca);
        sct_assert_defined(ppca, false);
    }
    
    void read_pointer_array_unknown2()
    {
        sc_uint<3> x = *ppd[1][a];
        sct_assert_read(ppd);
        sct_assert_defined(ppd, false);

        bool b = *pca[a];
        sct_assert_read(pca);
        sct_assert_defined(pca, false);

        b = *ppca[1][a];
        sct_assert_read(ppca);
        sct_assert_defined(ppca, false);
    }

    void read_pointer_array_unknown3()
    {
        sc_uint<3> x = *ppb[0][a];
        sct_assert_read(ppb);
        sct_assert_defined(ppb, false);

        bool b = (*pca[a]).read();
        sct_assert_read(pca);
        sct_assert_defined(pca, false);
        
        b = (*ppca[0][a]).read();
        sct_assert_read(ppca);
        sct_assert_defined(ppca, false);
    }
    
    void define_pointer_array_unknown1()
    {
        *pb[a] = 1;
        sct_assert_read(pb, false);
        sct_assert_defined(pb, false);
        
        sc_uint<3> x;
        *ppc[a][0] = x;
        *ppc[a][a] = x;
        sct_assert_read(ppc, false);
        sct_assert_defined(ppc, false);
        
        pca[a]->write(1);
        sct_assert_read(pca, false);
        sct_assert_defined(pca, false);
        
        *ppca[a][0] = 1;
        (*ppca[1][a]).write(2);
        sct_assert_read(ppca, false);
        sct_assert_defined(ppca, false);
    }
    
    
    void read_array_unknown1()
    {
        int arr[2];
        int arr_[2][3];
        int i;
        i = arr[a];
        i = arr_[a][0];
        
        sct_assert_read(arr);
        sct_assert_read(arr_);
    }

    void read_array_unknown2()
    {
        int arr[2];
        int arr_[2][3];
        int i = arr[a];
        int j = arr_[1][a];
        
        sct_assert_read(arr);
        sct_assert_read(arr_);
    }
    
    void read_array_unknown3()
    {
        int arr[2];
        int arr_[2][3];
        arr[a]++;
        arr_[a][0]++;
        
        sct_assert_read(arr);
        sct_assert_read(arr_);
    }
    
    void read_array_unknown4()
    {
        int arr[2];
        int arr_[2][3];
        arr[a] -= 1;
        arr_[1][a] -= 1;
        
        sct_assert_read(arr);
        sct_assert_read(arr_);
    }
    
    void read_array_unknown_sc_type()
    {
        sc_uint<3> arr1[2];
        int i;
        i = arr1[a];
        
        sc_uint<3> arr2[2];
        int j = arr2[a];
        
        sc_uint<3> arr3[2];
        arr3[a]++;
        
        sc_uint<3> arr4[2];
        arr4[a] += 1;
        
        sct_assert_read(arr1);
        sct_assert_read(arr2);
        sct_assert_read(arr3);
        sct_assert_read(arr4);
    }

    void define_array_unknown1()
    {
        int arr[2];
        
        arr[a] = 0;
        
        sct_assert_read(arr, false);
    }

    void define_array_unknown2()
    {
        int arr[2][2];
        
        arr[a][1] = 0;
        arr[0][a] = 0;
        
        sct_assert_read(arr, false);
    }
    
    void define_array_unknown_sc_type()
    {
        sc_int<2> arr[2];
        
        arr[a] = 0;
        
        // Always defined at initialization
        sct_assert_defined(arr);
        sct_assert_read(arr, false);
    }
};

class B_top : public sc_module
{
    sc_signal<bool> a{"a"};
    sc_signal<bool> b{"b"};
    sc_signal<bool> clk{"clk"};
    sc_signal<bool> nrst{"nrst"};

public:
    A a_mod{"a_mod"};

    SC_CTOR(B_top) {
        a_mod.clk(clk);
        a_mod.nrst(nrst);
        a_mod.a(a);
        a_mod.b(b);
    }
};

int sc_main(int argc, char* argv[])
{
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}

