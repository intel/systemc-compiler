/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"

using namespace sc_core;

// Logical expression and conditional operator (?) in method process body analysis
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
    int                 j;
    int*                q;

    sc_signal<bool> s{"s"};
    SC_CTOR(A) : j(1) 
    {
        SC_METHOD(dead_cond_error); sensitive << s0;
        
        SC_METHOD(cond_oper_for_channels);
        sensitive << s1 << s2 << s;
        
        SC_METHOD(logic_expr1); sensitive << s;
        SC_METHOD(logic_expr2); sensitive << s;
        SC_METHOD(logic_expr3); sensitive << s;
        SC_METHOD(logic_expr4); sensitive << s;

        SC_METHOD(cond_oper_compl); sensitive << s;
        SC_METHOD(cond_oper1); sensitive << s;
        SC_METHOD(cond_oper2); sensitive << s;
        // TODO; Fix me, #142
        //SC_METHOD(cond_oper3); sensitive << s;
        
//        SC_METHOD(cond_oper4); sensitive << s;
//        SC_METHOD(cond_oper5); sensitive << s;
//        SC_METHOD(cond_oper6); sensitive << s;
        
        SC_METHOD(cond_oper_const); sensitive << s;
        SC_METHOD(cond_oper_fcall); sensitive << s;
    }
    
    sc_signal<bool>  s0;
    
    // "No term in sub-statement" error reported for dead sub-condition, #68
    // BUG in real design
    void dead_cond_error() {
        if (true || !s0) {
        }
    }
    
    sc_signal<bool>  s1;
    sc_signal<bool>  s2;

    // Conditional operator for channels
    // BUG in real design -- fixed
    void cond_oper_for_channels()
    {
        bool b;
        int m = s.read();
        int k = s.read();
        b = (k) ? s1 : s2;
        b = k ? s1 : s2;
    }

    
    // Logic expression with ||
    void logic_expr1() {
        int m = s.read();
        int k = s.read();
        bool l0 = !(m==k);
        bool l1 = m == 1 || k == 1;
        bool l2 = (m > 1 || k < 1);
        bool l3 = m != k || k != 1;
        bool l4 = (m > 1 || !(k > 1) || k > m || k < m);
    }    

    // Logic expression with ||, && and mixed
    void logic_expr2() {
        int m = s.read();
        int k = s.read();
        bool l1 = m == 1 && k == 1;
        bool l2 = (m > 1 && k < 1);
        bool l3 = m != k && k != 1;
        bool l4 = (m > 1 && k > 1 && k > m && !(m == k));
    }

    // Logic expression with || and && mixed
    void logic_expr3() {
        int m = s.read();
        int k = s.read();
        bool l1 = m == 1 && k == 1 || k < m;
        bool l2 = m > 1 || k < 1 && k != 1;
        bool l3 = m != k && k != 1 || !(m < k) && k < 1;
        bool l4 = m != k || !(k==m) && m < k || k < 1;
    }

    // Logic expression with || and && mixed with ()
    void logic_expr4() {
        int m = s.read();
        int k = s.read();
        bool l1 = m == 1 && (k == 1 || (k < m));
        bool l2 = (m > 1 || k < 1) && k != 1;
        bool l3 = m != k && ((k != 1 || !(m < k)) && k < 1);
        bool l4 = m != k || !(k==m) && (m < k || k < 1);
    }
    
    // Condition operator with complex condition
    void cond_oper_compl() {
        int i;
        int m = s.read();
        int k = s.read();
        i = (m + 1) ? m + 1 : m + 2;
        i = (m + 1 > k -1) ? 1 : 2;
    }
 
    // Condition operator in assignment
    void cond_oper1() {
        int i;
        int m = s.read();
        int k = s.read();
        i = (m == 1) ? 1 : 2;
        i = (m > k) ? m+1 : k+1;
        i = (m < k || m == 1) ? m++ : ++k;
        i = (m < k && !(m == 1)) ? ((++i) + m) : (m * k);
        i = (m == k) ? m : ((m == 1) ? 1 : 2);
        i = (m != k) ? ((m == 1) ? m++ : --k) : (m - k);
    }    
    
    // Condition operator in IF branches
    void cond_oper2() {
        int i;
        int m = s.read();
        int k = s.read();
        if (m > 1) {
            i = (m == 1) ? m : k;
        } else {
            i = (m == 2) ? k : m;
        }
        if (m > 1) {
        } else {
            i = (m == 2) ? k : m;
        }
        if (m > 1) {
            i = (m == 1) ? m : k;
        } else {
        }
        i = (m == 1) ? m : k;
    }    

    // Condition operator in IF condition
    void cond_oper3() {
        int i;
        int m = s.read();
        int k = s.read();
        if (((m == 1) ? m : k) > 1) {
            i = 1;
        } else {
            i = 2;
        }
        if (i = ((m == 1) ? m : k) > 1) {
            i = 1;
        } else {
            i = 2;
        }
        if (m == ((m == 1) ? m : k)) {
            i = 1;
        } else {
            i = 2;
        }
    }    
    
    // Condition operator in loop with continue
    void cond_oper4() {
        int m = s.read();
        int k = s.read();
        for (int i = 0; i < 2; i++) {
            i = (m == 1) ? m : k;
        }
        
        for (int i = 0; i < 2; i++) {
            if (m > k) {
                i = (m == 1) ? m : k;
            } else {
                continue;
            }    
        }
        for (int i = 0; i < 2; i++) {
            if (m > k) {
                continue;
            } else {
                i = (m == 1) ? m : k;
            }    
        }
        for (int i = 0; i < 2; i++) {
            if (m > k) {
                continue;
            }
            i = (m == 1) ? m : k;
        }
        for (int i = 0; i < 2; i++) {
            if (m > k) {
                i = (m == 1) ? m : k;
                continue;
            }
        }
    }        
    
    // Condition operator in loop with break
    void cond_oper5() {
        int m = s.read();
        int k = s.read();
        for (int i = 0; i < 2; i++) {
            if (m > k) {
                i = (m == 1) ? m : k;
            } else {
                break;
            }    
        }
        for (int i = 0; i < 2; i++) {
            if (m > k) {
                break;
            } else {
                i = (m == 1) ? m : k;
            }    
        }
        for (int i = 0; i < 2; i++) {
            if (m > k) {
                break;
            }
            i = (m == 1) ? m : k;
        }
        for (int i = 0; i < 2; i++) {
            if (m > k) {
                i = (m == 1) ? m : k;
                break;
            }
        }
    }            
    
    // Condition operator in loop with break exit only
    void cond_oper6() {
        int i;
        int m = s.read();
        int k = s.read();
        for (; ; ) {
            if (m > k) {
                i = (m == 1) ? m : k;
            } else {
                break;
            }    
        }
        for (; ; ) {
            if (m > k) {
                break;
            } else {
                i = (m == 1) ? m : k;
            }    
        }
        for (; ; ) {
            if (m > k) {
                break;
            }
            i = (m == 1) ? m : k;
        }
        for (; ; ) {
            if (m > k) {
                i = (m == 1) ? m : k;
                break;
            }
        }
    }        
    
    // Condition operator with one of block reachable
    void cond_oper_const() {
        int i;
        int m = s.read();
        int k = s.read();
        i = (false) ? m : k;
        i = (true) ? m : k;
        i = (false || k == m) ? m : k;
        i = (true && k == m) ? m : k;
        i = (false && k == m) ? m : k;
        i = (true || k == m) ? m : k;
    } 

    // Condition operator with constant condition and function call
    int f(int l) {
        return (l+1);
    }
    
    void cond_oper_fcall() {
        int i;
        i = 0 ? f(1) : 1;
        i = 1 ? f(2) : f(3);
        i = (1 || f(1)) ? f(2) : f(3);
        i = (0 && f(4)) ? f(5) : f(6);
    }    

};

class B_top : public sc_module
{
    sc_signal<bool> a{"a"};
    sc_signal<bool> b{"b"};
    sc_signal<bool> c{"c"};

public:
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
