#include "systemc.h"
#include <sct_assert.h>

using namespace sc_core;

// while general cases
class A : public sc_module 
{
public:
    sc_in<bool>         a{"a"};
    sc_out<bool>        b{"b"};
    sc_out<bool>        c{"c"};
    sc_out<bool>*       p;
    
    int                 m;
    int                 k;
    int                 n;
    int*                q;

    sc_signal<bool> dummy{"dummy"};

    SC_CTOR(A) {
        SC_METHOD(while_stmt_empty); sensitive << dummy;
        SC_METHOD(while_stmt1); sensitive << dummy;
        SC_METHOD(while_stmt2); sensitive << dummy;
        SC_METHOD(while_stmt3); sensitive << dummy;
        SC_METHOD(while_stmt4); sensitive << dummy;
        SC_METHOD(while_stmt5); sensitive << dummy;
        SC_METHOD(while_stmt6); sensitive << dummy;
        SC_METHOD(while_stmt7); sensitive << dummy;
        SC_METHOD(while_const); sensitive << dummy;
        
        SC_METHOD(while_sc_type); sensitive << a;
    }
    
    // Empty While
    void while_stmt_empty() {
        int i = 0;
        while (i < 2) {
            k = i;
            i++;
        }
        sct_assert_level(0);
    }

    // Simple while
    void while_stmt1() {
        k = 0; 
        int i = 0;
        while (i < 2) {
            k = k + 1; 
            i++;
        }
        sct_assert_level(0);
    }
  
    // While with IF inside
    void while_stmt2() {
        int j = 1;
        int i = 0;
        while (i < 3) {
            i++;
            if (m > 0) {
                j = 2;
            } else {
                j = 3;
            }
            sct_assert_level(1);
        }
        sct_assert_level(0);
        j = 4;
    }

    // While with several inputs from outside
    void while_stmt3() {
        int i = 0;
        int j = 1; k = 0;
        if (m > 0) {
            j = 2;
        } else {
            j = 3;
        }
        while (i < 2) {   // 2 inputs whilem outside
            k++; i++;
        }
        j = 4;
        sct_assert_level(0);
    }    

    // While with several inputs from outside
    void while_stmt4() {
        int i = 0;
        int j = 1; k = 0;
        if (m > 0) {
            j = 2;
        }
        while (i < 2) {   // 2 inputs whilem outside
            k++;
            i++;
        }
        sct_assert_level(0);
        j = 3;
    }    
    
    // While with inner while
    void while_stmt5() {
        k = 0;
        int i = 0;
        while (i < 2) {   
            int j = 0; 
            i++;
            while (j < 3) {
                k = k + 1; j++;
                sct_assert_level(2);
            }
        }
        sct_assert_level(0);
    }     
    
    // While in IF branch
    void while_stmt6() {
        k = 0;
        int i = 0;
        if (m > 0) {
            while (i < 2) {   
                k = k + 1; i++;
            }
        } else {
            k = 3;
        }
        sct_assert_level(0);
    }         

    // While with some index calculations
    void while_stmt7() {
        k = 0; n = 0;
        int i = 0;
        while (i < 3) {   
            k = k + i; 
            i++;
            n = m++;
        }
    }     

    // While with false and true constant condition
    void while_const() {
        k = 0;
        while (false) {
            k = k + 1;
        }
        b.write(k+1);
        
        k = 1;
//        while (true) {
//            k = k + 1;
//        }
        b.write(k+2);
    }
    
    void while_sc_type() {
        k = 0; 
        sc_uint<3> i = 0;
        while (i < 2) {
            k = k + 1; 
            i++;
        }
        sct_assert_const(k == 2);
    }

    
};

class B_top : public sc_module 
{
public:
    sc_signal<bool> a{"a"};
    sc_signal<bool> b{"b"};
    sc_signal<bool> c{"c"};

    A a_mod{"a_mod"};

    SC_CTOR(B_top) {
        a_mod.a(a);
        a_mod.b(b);
        a_mod.c(c);
    }
};

int sc_main(int argc, char* argv[])
{
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}
