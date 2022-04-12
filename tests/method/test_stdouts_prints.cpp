/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"
#include <sct_assert.h>
#include <stdio.h>

using namespace sc_core;

// Check stdout, stderr, cerr, printf, fprintf ignored
class A : public sc_module
{
public:
    int                 m = 11;
    sc_signal<bool>     dummy{"dummy"};

    SC_CTOR(A) 
    {
        SC_METHOD(meth_print); 
        sensitive << dummy;

        SC_METHOD(meth_printf); 
        sensitive << dummy;

        SC_METHOD(meth_cerr); 
        sensitive << dummy;

        SC_METHOD(meth_cout); 
        sensitive << dummy;

        SC_METHOD(meth_cin); 
        sensitive << dummy;

        SC_METHOD(meth_sc_print); 
        sensitive << dummy;

        SC_METHOD(meth_sc_dump); 
        sensitive << dummy;

        SC_METHOD(meth_sc_kind); 
        sensitive << dummy;
    }

    void meth_print() 
    {
        std::printf("hello word");
        int i = m;
        printf("val = %d \n", i);
    }    

    void meth_printf() 
    {
        FILE *fp;
        fp = fopen("file.txt", "w");
        fprintf(fp, "message\n");
        fclose(fp);
    }    
    
    void meth_cerr() {
        bool a = dummy.read();
        cerr << "Some error" << a << endl;
        cerr << !a << endl;
    }    
    
    int f() {
        return 42;
    }
    int g(int par) {
        return (++par);
    }
    

    void meth_cout() {
        int i;
        cout << "Some output" << i << endl;
        if (dummy)  {
            cout << "dummy " << dummy.read() << endl;
        }
        
        cout << "a" << f() << endl;
        cout << "b" << f() << g(f()) << endl;
        
        unsigned computeEn = 1;
        cout << "\tlce_comp_stub "<< this->name() << " WRITE CSR"
            << ", wdata 0x" << hex << this->dummy.read() << dec
            << "; compute_en = " << computeEn << endl;
    }    

    void meth_cin() {
        int i;
        cin >> i;
    }    

    void meth_sc_print() {
        dummy.print();
        print();
    }    

    void meth_sc_dump() {
        dummy.dump();
        this->dump();
    }    

    void meth_sc_kind() {
        dummy.kind();
        this->kind();
    }    
};

class B_top : public sc_module
{
public:
    A a_mod{"a_mod"};

    SC_CTOR(B_top) {
    }
};

int sc_main(int argc, char* argv[])
{
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}

