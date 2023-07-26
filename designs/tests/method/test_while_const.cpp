/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"
#include <sct_assert.h>

using namespace sc_core;

// while with complex condition with &&/|| constant LHS/RHS
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

    sc_signal<int> s{"s"};

    SC_CTOR(A) {
        SC_METHOD(while_with_binary_oper1); sensitive << s;
        SC_METHOD(while_with_binary_oper1a); sensitive << s;
        SC_METHOD(while_with_binary_oper1b); sensitive << s;
        SC_METHOD(while_with_binary_oper1c); sensitive << s;
        SC_METHOD(while_with_binary_oper1d); sensitive << s;
        SC_METHOD(while_with_binary_oper1e); sensitive << s;
        SC_METHOD(while_with_binary_oper1f); sensitive << s;
        SC_METHOD(while_with_binary_oper1g); sensitive << s;
        
        SC_METHOD(while_with_binary_oper2); sensitive << s;
        SC_METHOD(while_with_binary_oper2a); sensitive << s;
        SC_METHOD(while_with_binary_oper2b); sensitive << s;
        SC_METHOD(while_with_binary_oper2c); sensitive << s;
        SC_METHOD(while_with_binary_oper2d); sensitive << s;
        SC_METHOD(while_with_binary_oper2e); sensitive << s;
        SC_METHOD(while_with_binary_oper2f); sensitive << s;
        SC_METHOD(while_with_binary_oper2g); sensitive << s;
        
        SC_METHOD(while_with_binary_oper3); sensitive << s;
        SC_METHOD(while_with_binary_oper4); sensitive << s;
    }

    sc_signal<int> t0;
    void while_with_binary_oper1()
    { 
        bool b1 = 1, b2 = s.read();
        int k = 0;
        
        while (b1 || b2) { // B5, B4
            k = 1;      // B3
            b1 = 0;
            b2 = s.read();
        }               // B2
        sct_assert_level(0);
        k = 2;          // B1
        t0 = k;
    }
    
    sc_signal<int> t1;
    void while_with_binary_oper1a()
    { 
        bool b1 = 0, b2 = s.read();
        int k = 0;
        
        while (b1 || b2) { // B5, B4
            k = 1;      // B3
            b1 = 0;
            b2 = s.read();
        }               // B2
        sct_assert_level(0);
        k = 2;          // B1
        t1 = k;
    }

    sc_signal<int> t2;
    void while_with_binary_oper1b()
    { 
        bool b1 = 0, b2 = 0;
        int k = 0;
        
        while (b1 || b2) { // B5, B4
            k = 1;      // B3
        }               // B2
        sct_assert_level(0);
        k = 2;          // B1
        t2 = k;
    }
    
    sc_signal<int> t3;
    void while_with_binary_oper1c()
    { 
        bool b1 = 1, b2 = 1;
        int k = 0;
        
        while (b1 || b2) { // B5, B4
            k = 1;      // B3
            b1 = 0;
            b2 = 0;
        }               // B2
        sct_assert_level(0);
        k = 2;          // B1
        t3 = k;
    }

    sc_signal<int> t4;
    void while_with_binary_oper1d()
    { 
        bool b1 = 0, b2 = 1, b3 = 0;
        int k = 0;
        
        while (b1 || b2 || b3) { // B5, B4
            k = 1;      // B3
            b1 = 0;
            b2 = 0;
            b3 = 0;
        }               // B2
        sct_assert_level(0);
        k = 2;          // B1
        t4 = k;
    }

    sc_signal<int> t5;
    void while_with_binary_oper1e()
    { 
        bool b1 = 0, b2 = 0, b3 = 0;
        int k = 0;
        
        while (b1 || b2 || b3) { // B5, B4
            k = 1;      // B3
        }               // B2
        sct_assert_level(0);
        k = 2;          // B1
        t5 = k;
    }

    sc_signal<int> t6;
    void while_with_binary_oper1f()
    { 
        bool b1 = 0, b2 = 0, b3 = 1;
        int k = 0;
        
        while (b1 || b2 || b3) { 
            k = 1;      
        }               
        k = 2;          
        t6 = k;
    }
    
    sc_signal<int> t7;
    void while_with_binary_oper1g()
    { 
        bool b1 = s.read();
        int k = 0;
        
        while (b1 || false) { 
            k = 1;      
        }               
        k = 2;          
        t7 = k;
    }
    
// ----------------------------------------------------------------------------    

    sc_signal<int> t8;
    void while_with_binary_oper2()
    { 
        bool b1 = 1, b2 = s.read();
        int k = 0;
        
        while (b1 && b2) { 
            k = 1;      
            b1 = s.read();
        }               
        k = 2;          
        t8 = k;
    }

    sc_signal<int> t9;
    void while_with_binary_oper2a()
    { 
        bool b1 = 0, b2 = s.read();
        int k = 0;
        
        while (b1 && b2) { 
            k = 1;      
            b1 = s.read();
        }               
        k = 2;          
        t9 = k;
    }
    
    sc_signal<int> t10;
    void while_with_binary_oper2b()
    { 
        bool b1 = 0, b2 = 0;
        int k = 0;
        
        while (b1 && b2) { 
            k = 1;      
            b1 = 0;
        }
        k = 2;          
        t11 = k;
    }
    
    sc_signal<int> t11;
    void while_with_binary_oper2c()
    { 
        bool b1 = 1, b2 = 1;
        int k = 0;
        
        while (b1 && b2) { 
            k = 1;      
            b1 = 0;
        }               
        k = 2;          
        t11 = k;
    }

    sc_signal<int> t12;
    void while_with_binary_oper2d()
    { 
        bool b1 = 1, b2 = 0, b3 = 1;
        int k = 0;
        
        while (b1 && b2 && b3) { 
            k = 1;      
        }               
        k = 2;          
        t12 = k;
    }

    sc_signal<int> t13;
    void while_with_binary_oper2e()
    { 
        bool b1 = 1, b2 = 1, b3 = 1;
        int k = 0;
        
        while (b1 && b2 && b3) { 
            k = 1;      
            b2 = 0;
        }               
        k = 2;          
        t13 = k;
    }

    sc_signal<int> t14;
    void while_with_binary_oper2f()
    { 
        bool b1 = 1, b2 = 1, b3 = 1;
        int k = 0;
        
        while (b1 && b2 & b3) { 
            k = 1;      
        }               
        k = 2;          
        t14 = k;
    }
    
    sc_signal<int> t15;
    void while_with_binary_oper2g()
    { 
        bool b1 = s.read();
        int k = 0;
        
        while (b1 && true) { 
            k = 1;      
        }               
        k = 2;          
        t15 = k;
    }
    
// ---------------------------------------------------------------------------    
    
    sc_signal<int> t16;
    void while_with_binary_oper3()
    { 
        bool b1 = 1, b2 = 1, b3 = 1;
        int k = 0;
        
        while (b1 && (b2 || b3)) { 
            k = 1;      
            b1 = 0;
        }               
        sct_assert_level(0);
        k = 2;          
        t16 = k;
    }

    sc_signal<int> t17;
    void while_with_binary_oper4()
    { 
        bool b1 = 1, b2 = 1, b3 = 1;
        int k = 0;
        
        while ((b1 && b2) || b3) { 
            k = 1;      
            b1 = 0;
            b2 = 0;
            b3 = 0;
        }               
        sct_assert_level(0);
        k = 2;      
        t17 = k;
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

