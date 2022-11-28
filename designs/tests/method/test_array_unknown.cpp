/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"
#include "sct_assert.h"

using namespace sc_core;

// Array access at non-determinable index, write leads to all elements unknown
class A : public sc_module {
public:
    int     m[3];
    int*    mm[3];
    sc_uint<3>* kk[3];
    int     n[2][2]; 
    sc_signal<bool>* ch[2][2];

    sc_signal<int> a;
    
    SC_CTOR(A) {
        for (int i = 0; i < 3; ++i) {
            mm[i] = sc_new<int>();
            kk[i] = sc_new<sc_uint<3>>();
        }
        for (int i = 0; i < 2; ++i) {
            for (int j = 0; j < 2; ++j) {
                ch[i][j] = new sc_signal<bool>("ch");
            }
        }
        
        // Not supported in GEN yet, passed in CPA
        //SC_METHOD(comp_assign_array); sensitive << a;

        SC_METHOD(array_init); sensitive << a;
        SC_METHOD(multi_array_of_channel_pointers); sensitive << a << *ch[0][0];
        
        SC_METHOD(write_unknown_index1); sensitive << a;
        SC_METHOD(write_unknown_index2); sensitive << a;
        SC_METHOD(write_unknown_index3); sensitive << a;
        SC_METHOD(write_unknown_index_sc_type); sensitive << a;
        
        SC_METHOD(pointer_unknown_index1); sensitive << a;
        SC_METHOD(pointer_unknown_index_sc_type); sensitive << a;
        
        SC_METHOD(unary_array_unknown1); sensitive << a;
        SC_METHOD(unary_array_unknown2); sensitive << a;
        SC_METHOD(unary_array_unknown_sc_type); sensitive << a;
        
        SC_METHOD(comp_assign_array_unknown1); sensitive << a;
        SC_METHOD(comp_assign_array_unknown2); sensitive << a;
        SC_METHOD(comp_assign_array_unknown3); sensitive << a;
        
        SC_METHOD(read_unknown_index); sensitive << a;
    }
    
    void comp_assign_array() 
    {
        int arr[2] = {1,2};
        int* p = &arr[0];
        p += 1;
        sct_assert_const (p == &arr[1]);
    }
    
    
    void array_init() 
    {
        int n[2][2];
        for (int i = 0; i < 2; i++) 
            for (int j = 0; j < 2; j++) 
                n[i][j] = i+j+1;
        
        sct_assert_const(n[0][0] == 1);
        sct_assert_const(n[0][1] == 2);
        sct_assert_const(n[1][0] == 2);
        sct_assert_const(n[1][1] == 3);
    }    
    
    void multi_array_of_channel_pointers() 
    {
        bool b = ch[1][0]->read();
    }
    
    void write_unknown_index1() 
    {
        int m[3];
        m[0] = 1; m[1] = 2; m[2] = 3;

        m[a] = 1;
        
        sct_assert_unknown(m[0]);
        sct_assert_unknown(m[1]);
        sct_assert_unknown(m[2]);
    }
    
    void write_unknown_index2() 
    {
        int n[2][2];
        n[0][0] = 0; n[0][1] = 1; n[1][0] = 2; n[1][1] = 3;
        
        n[a][1] = 4;
        
        sct_assert_unknown(n[0][0]);
        sct_assert_unknown(n[0][1]);
        sct_assert_unknown(n[1][0]);
        sct_assert_unknown(n[1][1]);
    }

    void write_unknown_index3() 
    {
        int n[2][2];
        n[0][0] = 0; n[0][1] = 1; n[1][0] = 2; n[1][1] = 3;
        
        n[1][a] = 2;
        
        sct_assert_const(n[0][0] == 0);
        sct_assert_const(n[0][1] == 1);
        sct_assert_unknown(n[1][0]);
        sct_assert_unknown(n[1][1]);
    }
    
    void write_unknown_index_sc_type() 
    {
        sc_int<3> k[3];

        k[a] = 1;
        
        sct_assert_unknown(k[0]);
        sct_assert_unknown(k[1]);
        sct_assert_unknown(k[2]);
    }
    
    void pointer_unknown_index1() 
    {
        *mm[0] = 1; *mm[1] = 2; *mm[2] = 3;

        *mm[a] = 1;
        
        sct_assert_unknown(*mm[0]);
        sct_assert_unknown(*mm[1]);
        sct_assert_unknown(*mm[2]);
    }
    
    void pointer_unknown_index_sc_type() 
    {
        *kk[0] = 1; *kk[1] = 2; *kk[2] = 3;

        *kk[a] = 1;
        
        sct_assert_unknown(*kk[0]);
        sct_assert_unknown(*kk[1]);
        sct_assert_unknown(*kk[2]);
    }
    
    void unary_array_unknown1() 
    {
        int m[3];
        m[0] = 1; m[1] = 2; m[2] = 3;

        m[a]++;
        
        sct_assert_unknown(m[0]);
        sct_assert_unknown(m[1]);
        sct_assert_unknown(m[2]);
    }

    void unary_array_unknown2() 
    {
        int m[3];
        m[0] = 1; m[1] = 2; m[2] = 3;

        --m[a];
        
        sct_assert_unknown(m[0]);
        sct_assert_unknown(m[1]);
        sct_assert_unknown(m[2]);
    }
    
    void unary_array_unknown_sc_type() 
    {
        sc_uint<3> k[3];
        k[0] = 1; k[1] = 2; k[2] = 3;

        k[a]++;
        
        sct_assert_unknown(k[0]);
        sct_assert_unknown(k[1]);
        sct_assert_unknown(k[2]);
    }

    void comp_assign_array_unknown1() 
    {
        int m[3];
        m[0] = 1; m[1] = 2; m[2] = 3;

        m[a] += 1;
        
        sct_assert_unknown(m[0]);
        sct_assert_unknown(m[1]);
        sct_assert_unknown(m[2]);
    }

    void comp_assign_array_unknown2() 
    {
        int n[2][2];
        n[0][0] = 0; n[0][1] = 1; n[1][0] = 2; n[1][1] = 3;
        
        n[a][1] -= 4;
        
        sct_assert_unknown(n[0][0]);
        sct_assert_unknown(n[0][1]);
        sct_assert_unknown(n[1][0]);
        sct_assert_unknown(n[1][1]);
    }
    
    void comp_assign_array_unknown3() 
    {
        int n[2][2];
        n[0][0] = 0; n[0][1] = 1; n[1][0] = 2; n[1][1] = 3;
        
        n[1][a] -= 2;
        
        sct_assert_const(n[0][0] == 0);
        sct_assert_const(n[0][1] == 1);
        sct_assert_unknown(n[1][0]);
        sct_assert_unknown(n[1][1]);
    }    
     
    // Check read at unknown index does not clear value
    void read_unknown_index()
    {
        int m[3];
        m[0] = 1; m[1] = 2; m[2] = 3;
        
        int i = m[a];
        i = m[a.read()+1];
        
        if (m[a]) {
            int k = 1;
        }
        
        sct_assert_const(m[0] == 1);
        sct_assert_const(m[1] == 2);
        sct_assert_const(m[2] == 3);
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

