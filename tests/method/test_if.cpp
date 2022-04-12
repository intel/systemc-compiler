/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"
#include <sct_assert.h>

// if in method process body analysis
template <
    class TRAITS
>
class A : public sc_module
{
public:
    sc_in<bool>         a{"a"};
    sc_out<bool>        b{"b"};
    sc_out<bool>        c{"c"};
    sc_out<bool>*       p;

    int                 m;
    int                 k;
    int                 n = 11;
    int                 j;
    int*                q;

    sc_signal<bool> s{"s"};

    SC_CTOR(A) : j(1) {
        SC_METHOD(if_const_and_signal); sensitive << s1 << s2;
        
        SC_METHOD(if_empty1); sensitive << s;
        SC_METHOD(if_empty2); sensitive << s;
        SC_METHOD(if_empty3); sensitive << s;
        SC_METHOD(if_empty4); sensitive << a << s;
        SC_METHOD(if_stmt1); sensitive << a << b << s;
        SC_METHOD(if_stmt2); sensitive << s;
        SC_METHOD(if_stmt2a); sensitive << s;
        SC_METHOD(if_stmt3);
        sensitive << a << s;
        SC_METHOD(if_stmt4);
        sensitive << a << b << s;
        SC_METHOD(if_stmt5);
        sensitive << a << s;
        SC_METHOD(if_compl_cond1); sensitive << s;
        SC_METHOD(if_compl_cond2); sensitive << s;
        SC_METHOD(if_compl_cond3); sensitive << s;
        SC_METHOD(if_compl_cond4); sensitive << s;
        
        SC_METHOD(if_const); sensitive << s;
    }
    

    //------------------------------------------------------------------------ 
    // Extra @conf_read_buf_enable generated before if -- bug in real design
    // Important that @READ_BUFFER_ENABLE is need to be calculated 
    static const bool READ_BUFFER_ENABLE = TRAITS::CACHE_FEATURES & 2;
    
    sc_signal<bool>     s1;
    sc_signal<bool>     s2;

    void if_const_and_signal()
    {
        if (READ_BUFFER_ENABLE && s1) {
        }

        if (!READ_BUFFER_ENABLE || s1) {
        }
        
        if (READ_BUFFER_ENABLE && (s1 || s2)) {
        }

        if (!READ_BUFFER_ENABLE || (s1 && s2)) {
        }
        int i  =0;
    }

    //------------------------------------------------------------------------ 

    
    // IF with empty else branch
    void if_empty1() {
        int i;
        if (n > 0) {
            i = 3;
        } else {
        }
        sct_assert_level(0);
        i = 5;
    }

    // IF with empty then branch
    void if_empty2() {
        int i; int n = s.read();
        if (n > 0) {
        } else {
            i = 3;
        }
        sct_assert_level(0);
        i = 5;
    }
    
    // IF with both empty branches
    void if_empty3() {
        int n = s.read();
        if (n > 0) {
        } else {
        }
        sct_assert_level(0);
        b.write(1);
    }    

    // Several IF`s with empty branches
    void if_empty4() {
        int n = s.read();
        if (n > 0) {
            if (n > 1) {
                m = 1;
            }
        } else {
        }
        if (n > 2) {
        } else {
            m = 2;
            if (a.read()) {
            }
        }
        sct_assert_level(0);
        m = 3;
    }   

    // One IF with both non-empty branches
    void if_stmt1() {
        int i;
        int j = a.read();
        if (j > 0) {
            i = 1;
        } else {
            i = 2;
        }
        sct_assert_level(0);
        b.write(i);
    }

    // Inner IF`s
    void if_stmt2() {
        int i;
        int m = s.read();
        int n = s.read();
        if (m > 0) {
            i = 1;
            if (n > 0) {
                i = 3;
            } else {}
            i = 5;
        } else {
            i = 2;
            if (n > 1) {
                i = 4;
            }
        }
        sct_assert_level(0);
        i = 0;
    }
    
    // Two inner IF`s
    void if_stmt2a() 
    {
        int k = 0;              // B7
        int m = s.read();
        int n = s.read();
        if (m > 0) {        
            k = 1;              // B6

            if (n > 0) { 
                k = 2;          // B5
            } else {
                k = 3;          // B4
            }

        } else {
            if (n > 1) {        // B3
                k = 4;          // B2
            }
        }
        sct_assert_level(0);
        k = 6;                  // B1    
    }           

    // Double inner IF`s
    void if_stmt3() {
        int i;
        int n = s.read();
        int k = s.read();
        if (a.read()) {
            i = 1;
            if (n > a.read()) {
                if (k == a.read()) {
                    i++;
                } else {
                    i--;
                }
                i = 3;
            } else {}
            i = 5;
        }
        sct_assert_level(0);
        b.write(i);
    }
    
    // Inner IF with two level up next block
    void if_stmt4() {
        int i;
        if (a.read()) {
            if (b.read()) {
                i = 1;
            }
        }
        sct_assert_level(0);
        b.write(i);
    }
    
    // Sequential IF and general statements
    void if_stmt5() {
        int i;
        int m = s.read();
        int k = s.read();
        int n = s.read();
        if (m > 0) {
            i = 1;
            i = 2;
        }
        if (k < 0) {
            i = 3;
            if (n > 0) {
                i = 4;
            }
        }
        i = 5;
        if (k == 0) {
            i = 6;
        }
        sct_assert_level(0);
    }
    
    // Complex condition in IF statement with ||
    void if_compl_cond1() {
        int i;
        int k = s.read();
        int m = s.read();
        if (m == 1 || k == 1) {
            i = 1;
        } else {
            i = 2;
        }
        if (m > 1 || k < 1) {
            i = 1;
        }
        if (m != k || k != 1) {
        } else {
            i = 2;
        }
        if (m > 1 || k > 1 || k > m) {
        } else {
        }
        sct_assert_level(0);
        b.write(i);
    }

    // Complex condition in IF statement with &&
    void if_compl_cond2() {
        int i;
        int k = s.read();
        int m = s.read();
        if (m == 1 && k == 1) {
            i = 1;
        } else {
            i = 2;
        }
        if (m > 1 && k < 1) {
            i = 1;
        }
        if (m != k && k != 1) {
        } else {
            i = 2;
        }
        if (m > 1 && k > 1 && k > m) {
        } else {
        }
        sct_assert_level(0);
        b.write(i);
    }

    // Complex condition in IF statement with && and ||
    void if_compl_cond3() {
        int i;
        int k = s.read();
        int m = s.read();
        if (m == 1 && k == 2 || k < m) {
            i = 1;
        } else {
            i = 2;
        }
        if (m == 1 || k == 2 && k < m) {
            i = 1;
        }
        if (m == 1 && k == 1 || m == 2 && k == 2) {
        } else {
            i = 2;
        }
        if (m == 1 || k == 1 && m == 2 || k == 2) {
        } else {
        }
        sct_assert_level(0);
        b.write(i);
    }

    // Complex condition in IF statement with && and || and ()
    void if_compl_cond4() {
        int i;
        int k = s.read();
        int m = s.read();
        if (m == 1 && (k == 2 || k < m)) {
            i = 1;
        } else {
            i = 2;
        }
        if ((m == 1 || k == 2) && k < m) {
            i = 1;
        }
        if (m == 1 && (k == 1 || m == 2) && k == 2) {
        } else {
            i = 2;
        }
        if ((m == 1 || (k == 1 && m == 2)) || k == 2) {
        } else {
        }
        sct_assert_level(0);
        b.write(i);
    }
    
    // IF with one of block reachable
    void if_const() {
        int i;
        int k = s.read();
        int m = s.read();
        if (false) {
            i = k;
        } else {
            i = m;
        }
        if (true) {
            i = m;
        } else {
            i = k;
        }
        
        if (false) {
        } else {
            i = k;
        }
        
        if (true) {
            i = m;
        }
        sct_assert_level(0);
    }    
};


struct CTRAITS {
    static const unsigned CACHE_FEATURES  = 15;
};

class B_top : public sc_module
{
    sc_signal<bool> a{"a"};
    sc_signal<bool> b{"b"};
    sc_signal<bool> c{"c"};

public:
    A<CTRAITS> a_mod{"a_mod"};

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

