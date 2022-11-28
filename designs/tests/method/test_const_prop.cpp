/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"
#include <sct_assert.h>

using namespace sc_core;

// Constant propagation general case
class A : public sc_module
{
public:
    sc_in<bool>         clk{"clk"};
    sc_in<bool>         nrst{"nrst"};
    sc_in<bool>         a{"a"};
    sc_out<bool>        b{"b"};

    sc_signal<bool>     c{"c"};
    
    static const unsigned CTRL_NUM = 0;
    sc_signal<bool>* ctrl_interrupt_sig[CTRL_NUM];

    SC_CTOR(A) : useWriteResp0(1), useWriteResp1(1)
    {
        for (int i = 0; i < 3; i++) {
            arr1[i] = new sc_signal<bool>("arr1");
            arr2[i] = new sc_signal<bool>("arr2");
            arr3[i] = new sc_signal<bool>("arr3");
        }
        
        SC_METHOD(complex_if_level); sensitive << a;
        SC_METHOD(mstrResponseMuxProc); sensitive << a << *arr3[0] << *arr1[0];

        SC_METHOD(NoReturnProc); sensitive << a;
        SC_METHOD(NoReturnProc2); sensitive << a;

        SC_METHOD(not_test); sensitive << a;

        SC_METHOD(intrControlProc); sensitive << a;
        SC_METHOD(chooseRequestProc); sensitive << a;
        SC_METHOD(ackProc2R2Wcache4); sensitive << a << rr_first_indx
                            << port_req[0] << port_oper[0] << port_bindx[0];
         
        SC_METHOD(simple_if1); sensitive << a;
        SC_METHOD(simple_if2); sensitive << a;
        SC_METHOD(if_in_func1); sensitive << a;   
        SC_METHOD(if_in_func2); sensitive << a;        
        SC_METHOD(if_in_func3); sensitive << a;   
        SC_METHOD(const_param_func); sensitive << a;
        SC_METHOD(if_in_func5); sensitive << a;   
        
        SC_METHOD(simple_switch1); sensitive << a;
        SC_METHOD(simple_switch2); sensitive << a;
        SC_METHOD(simple_switch3); sensitive << a;
        SC_METHOD(simple_binary1); sensitive << a;
        SC_METHOD(simple_binary2); sensitive << a << c;
        SC_METHOD(simple_binary3); sensitive << a << c;
        SC_METHOD(simple_cond1); sensitive << a;
        

        SC_METHOD(double_if1); sensitive << a << b;
        SC_METHOD(double_if2); sensitive << a << b;
        SC_METHOD(double_if3); sensitive << a << b;
        SC_METHOD(double_if4); sensitive << a << b << c;
        SC_METHOD(double_if5); sensitive << a << b << c;
        SC_METHOD(double_if6); sensitive << a << b << c;
        SC_METHOD(seq_if); sensitive << a << b << c;
        
        SC_METHOD(double_if_for1); sensitive << a << b << c;
        SC_METHOD(double_if_for2); sensitive << a << b << c;
        SC_METHOD(double_if_while); sensitive << a << b << c;
        SC_METHOD(double_if_break); sensitive << a << b << c;
        SC_METHOD(false_if_break); sensitive << c;
        
        SC_METHOD(multiple_calls1); sensitive << a;
        SC_METHOD(multiple_calls2); sensitive << a;
        
        SC_METHOD(local_array_if); sensitive << a;

        SC_CTHREAD(local_array, clk.pos());
        async_reset_signal_is(nrst, false);

        SC_METHOD(return_const_in_if); sensitive << a;
        
        SC_METHOD(return_const_in_for); sensitive << a;
    }
    
    //----------------------------------------------------------------------
    // One IF at level 2 and several IF at level 4, no IF with level 3
    // Bug in real design -- fixed
    void complex_if_level() {
        int k; int m; int n;
        if (k) {
            if (m) {
                if (n) {
                    if (k > n) {
                        int aa = 0;
                    }
                } else {
                    if (true) {
                        int bb = 1;
                    } else {
                    }
                }
            }
            sct_assert_level(1);
        }
    }

    //----------------------------------------------------------------------
    // Bug in real design cache module
    sc_signal<bool>* arr1[3];
    sc_signal<bool>* arr2[3];
    sc_signal<bool>* arr3[3];

    void mstrResponseMuxProc() 
    {
        int k; int m; int n;
        int i = a.read();   
        if ( ( (*arr1[i]) &&                // B5 
               (true || !(*arr2[i]))) ||    // B4, B3
             (*arr3[i]) )                   // B2
        {
            m = 1;    // B1
        }
    }


    //----------------------------------------------------------------------
    // No return in some branch processes
    const unsigned useWriteResp0;
    const unsigned useWriteResp1;
    
    bool useWriteResp(unsigned portId) 
    {
        switch (portId) {
            case 0: return useWriteResp0;
            case 1: return useWriteResp1;
            default: assert(false); return useWriteResp0;
        }
    }
    
    void NoReturnProc() 
    {
        int k; int m; int n;
        assert (m > k);
        if (a.read()) {
            assert (m > k);
            assert (m > k && "message");
        }
    }

    void NoReturnProc2() 
    {
        useWriteResp(a.read());
    }
    
    //----------------------------------------------------------------------
    // Unary not and logic not
    void not_test()
    {
        sc_uint<3> x = 3;
        sc_uint<3> y = ~x;
        sct_assert_const(y == 4);
        
        bool b = x == y;
        sct_assert_const(!b);
    }
    
    //----------------------------------------------------------------------
    // Check level after complex IF
    
    void double_if1() 
    {
        int k; int m; int n;
        if (a.read()) {
            if (b.read()) {
                m = 1;
            } else {
                m = 2;
            }
        } else {
            if (b.read()) {
                m = 3;
            } else {
                m = 4;
            }
        }
        
        sct_assert_level(0);
        m = 0;
    }
    
    void double_if2() 
    {
        int k; int m; int n;
        if (a.read()) {
            if (b.read()) {
                m = 1;
            } else {
                m = 2;
            }
        }
        
        sct_assert_level(0);
        m = 0;
    }
    
    void double_if3() 
    {
        int k; int m; int n;
        if (a.read()) {
            if (b.read()) {
                m = 1;
            } else {
                m = 2;
            }
        } else {
            if (b.read()) {
                m = 3;
            }
        }
        
        sct_assert_level(0);
        m = 0;
    }
    
    void double_if4() 
    {
        int k; int m; int n;
        if (a.read()) {
            if (b.read()) {
                m = 1;
            } else 
            if (c.read()) {
                m = 2;
            } else {    
                m = 3;
            }
        } else {
            if (b.read()) {
                m = 4;
            } else 
            if (c.read()) {
                m = 5;
            } else {    
                m = 6;
            }
        }
        
        sct_assert_level(0);
        m = 0;
    }
    
    int f() {
        return 5;
    }
    
    void double_if5() 
    {
        int k; int m; int n;
        if (a.read()) {
            
            if (a.read()) {
                if (b.read()) {
                    m = 1;

                } else 
                if (c.read()) {
                    if (true) {
                        m = f(); 
                    }            
                }
            }

            sct_assert_level(1);
        }
    }
    
    void double_if6() 
    {
        int k; int m; int n;
        if (a.read()) {
            if (b.read()) {
                m = 1;  // Min level
                
            } else {
                if (c.read()) {
                    m = 2;
                } else {
                    m = 3;
                }
            }
        } else {
            if (b.read()) {
                if (c.read()) {
                    m = 4;
                } else {
                    m = 5;
                }
            } else {
                if (c.read()) {
                    m = 6;
                } else {
                    m = 7;
                }
            }
        }

        sct_assert_level(0);
    }
    
    void double_if_for1() 
    {
        int k; int m; int n;
        if (a.read()) {
            if (b.read()) {
                for (int i = 0; i < 2; i++) {
                }
            } else 
            if (c.read()) {
                m = 2;
            } else {    
                m = 3;
            }
        } else {
            if (b.read()) {
                m = 4;
            } else 
            if (c.read()) {
                for (int i = 0; i < 2; i++) {
                    if (a.read()) break;
                }
            } else {    
                m = 6;
            }
        }
        
        sct_assert_level(0);
        m = 0;
    }
    
    void double_if_for2() 
    {
        int k; int m; int n;
        if (a.read()) {
            if (b.read()) {
                for (int i = 0; i < 2; i++) {
                    if (c.read()) {
                        m = 1;
                    }
                }
            } else 
            if (c.read()) {
                m = 2;
            } else {    
                m = 3;
            }
        } else {
            if (b.read()) {
                m = 4;
            } else 
            if (c.read()) {
                for (int i = 0; i < 2; i++) {
                    if (c.read()) {
                        m = 1;
                    } else {
                        m = 2;
                    }
                }
            } else {    
                m = 6;
                if (c.read()) {
                    m = 1;
                } else {
                    m = 2;
                }
            }
        }
        
        sct_assert_level(0);
        m = 0;
    }
    
    void double_if_while() 
    {
        int k; int m; int n;
        if (a.read()) {
            int i = 0;
            if (b.read()) {
                while (i < 2) {
                    i++;
                }
            } else 
            if (c.read()) {
                m = 2;
            } else {    
                m = 3;
            }
        } else {
            int i = 0;
            while (i < 2) {
                if (b.read()) {
                    m = 4;
                } else {
                    int j = 1;
                    while (j < 3) {
                        j++;
                        if (c.read()) break;
                    }
                }
                i++;
            }
            if (c.read()) {
                for (int i = 0; i < 2; i++) {
                    if (c.read()) {
                        m = 1;
                    } else {
                        m = 2;
                    }
                }
            }
        }
        
        sct_assert_level(0);
        m = 0;
    }
    
    void double_if_break() 
    {
        int k; int m; int n;
        for (int i = 0; i < 2; i++) {
            if (a.read()) {
                m = 1;
                if (b.read()) {
                    m = 2;
                    break;
                }
                break;
            }
        }
        sct_assert_level(0);
    }
    
    void false_if_break() 
    {
        for (int i = 0; i < 3; ++i) {
            if (false) {
                if (c.read()) break;
            }
            sct_assert_level(1);
        }
    }
    
    void seq_if() 
    {
        int k; int m; int n;   
        if (a.read()) {
            sct_assert_level(1);
            m = 1;
        } else 
        if (b.read()) {
            sct_assert_level(2);
            m = 2;
        } else
        if (c.read()) {
            sct_assert_level(3);
            m = 3;
        } else {
            m = 4;
        }
        
        sct_assert_level(0);
        m = 0;
    }
        
    //----------------------------------------------------------------------
    
    // Bug in real design -- fixed
    void intrControlProc()
    {
        // Zero iteration loop
        bool b = false;
        for (int i = 0; i < CTRL_NUM; i++) {
            b = b || *ctrl_interrupt_sig[i];
        }
    }

    // Bug in real design -- fixed
    void chooseRequestProc() 
    {
        if (false) 
        {                                    
            for (int i = 0; i < 1; i++) {
                if (a.read()) {
                    break;
                }
            }
        } else {
        }    
    }
    
    // Bug in real design -- fixed
    static const unsigned BLOCK_NUM = 3;
    static const unsigned PORT_NUM = 3;
    
    sc_signal<bool> port_req[PORT_NUM];
    sc_signal<bool> port_oper[PORT_NUM];
    sc_signal<sc_uint<2> > rr_first_indx;
    sc_signal<sc_uint<2> > port_bindx[PORT_NUM];
    
    sc_uint<2> getNextPortIndex(unsigned iter, sc_uint<2> portIndx) 
    {
        if (true) {
            if (iter < 1) {
                // High priority ports
                return (portIndx+1);  // No wrap up allowed for them
            } else 
            if (iter == 1) {
                return rr_first_indx.read();
            } else {
                return ((portIndx == 0) ? 2 :
                        ((portIndx == PORT_NUM-1) ? 0 : (portIndx+1)));
            }
        } else {
            return ((portIndx == PORT_NUM-1) ? 0 : (portIndx+1));
        }   
    }
    
    sc_uint<2> getFirstPortIndx() {
        return rr_first_indx.read();
    }

    void ackProc2R2Wcache4()
    {
        bool    readFirstAccess_flat[BLOCK_NUM];
        bool    readSecndAccess_flat[BLOCK_NUM];
        bool    writeFirstAccess_flat[BLOCK_NUM];
        bool    writeSecndAccess_flat[BLOCK_NUM];

        for (int i = 0; i < BLOCK_NUM; i++) {
            readFirstAccess_flat[i]  = a.read();
            readSecndAccess_flat[i]  = 0;
            writeFirstAccess_flat[i] = a.read();
            writeSecndAccess_flat[i] = 0;
        }

        sc_uint<2> portIndx = getFirstPortIndx();

        for (unsigned i = 1; i < PORT_NUM; i++) {
            portIndx = getNextPortIndex(i, portIndx);

            sc_uint<2> blockIndx = port_bindx[portIndx].read();
            bool accessPermit = port_req[portIndx];

            if (!port_oper[portIndx]) {
                if (!readFirstAccess_flat[blockIndx]) {
                    readFirstAccess_flat[blockIndx] = accessPermit;
                } else 
                if (!readSecndAccess_flat[blockIndx]) {
                    readSecndAccess_flat[blockIndx] = accessPermit;
                } else {
                    accessPermit = 0;
                }
            } else {
                if (!writeFirstAccess_flat[blockIndx]) {
                    writeFirstAccess_flat[blockIndx] = accessPermit;
                } else
                if (!writeSecndAccess_flat[blockIndx]) {
                    writeSecndAccess_flat[blockIndx] = accessPermit;
                } else {
                    accessPermit = 0;
                }
            }
            
            sct_assert_level(1);
        }
    }
    

    //----------------------------------------------------------------------
    
    // One IF with constant condition 
    void simple_if1() {
        int i;
        i = 1;
        
        if (i > 0) {    // termCond 1
            i = 2;
        }
        sct_assert_const(i == 2);
    }
    
    // Two IFs with constant condition
    void simple_if2() {
        int i;
        i = 1;
        int m = i+1;
        
        if (a.read()) {
            if (i < 0) { // termCond 0
                i = 2;
            } else {
                i = 3;
            }
            sct_assert_const(i == 3);
        } else {
            if (m > 0) { // termCond 1
                i = 4;
            }
            sct_assert_const(i == 4);
        }
    }
    
    // IFs in function call
    void f1() {
        int j = 1;
        if (j > 0) {    // termCond 1
            j = 2;
        }
    }
    
    void if_in_func1() {
        int i = 1;
        if (i < 0) {    // termCond 0
            i = 2;
        }
        
        f1();
    }
    
    // Constant propagation from function
    void f2() {
        int m = 3;
    }
    
    void if_in_func2() {
        f2();
    }
    
    // Constant propagation to function
    void f3() {
        int m;
        if (m == 4) {
            int ll = 1;
        }
    }
    
    void if_in_func3() {
        int m = 4;
        
        f3();
    }
    
    // Function with constant parameter by reference
    void f4_(unsigned& ref) {
        ref++;
    }
    void f4(unsigned val) {
        f4_(val);
        sct_assert_const(val == 2);    
    }
    void const_param_func() {
        f4(1);
    } 
    
    // Function returns constant value
    int f5() {
        return 5;
    }
    
    void if_in_func5() {
        int i = f5();
        sct_assert_const (i == 5);
        if (i == 5) {}
    }
    
    // One SWITCH with constant condition 
    void simple_switch1() {
        int i = 2;
        switch (i) {
            case 1: i = 2; break;
            case 2: i = 3; break;
            default: ;
        }
        sct_assert_const (i == 3);
    }
    
    // One SWITCH w/o constant condition 
    void simple_switch2() {
        int i = a.read();
        switch (i) {
            case 1: i = 2; break;
            case 2: i = 3; break;
            default: ;
        }
    }
    
    // One SWITCH with default true
    void simple_switch3() {
        int i = 3;
        switch (i) {
            case 1: i = 2; break;
            case 2: i = 3; break;
            default: i = 4;
        }
        sct_assert_const (i == 4);
    }
    
    // Two binary operators in condition
    void simple_binary1() {
        int i = 1;
        int m;
       
        if (a.read() && i < 0) {
            m = 2;
        }

        if (i > 0 || a.read()) {
            m = 3;
        }
    }

    void simple_binary2() {
        int i = 0;
        int m = -1;
       
        if (i == 0 || a.read()) {
            m = 0;
        }
        sct_assert_const (m == 0);

        if ((i == 1 || i == 2) && a.read()) {
            m = 1;
        }

        if ((i == 0 || i == 1) && a.read()) {
            m = 2;
        }

        if ((i == 1 || i == 0) && a.read()) {
            m = 3;
        }

        if ((i == 1 || a.read()) && c.read()) {
            m = 4;
        }

        if ((i == 0 || a.read()) && c.read()) {
            m = 5;
        }

        if ((a.read() || i == 0) && c.read()) {
            m = 6;
        }

        if ((a.read() || i == 1) && c.read()) {
            m = 7;
        }
    }

    void simple_binary3() {
        int i = 0;
        int m;
        if ((i == 0 && i == 1) && a.read()) {
            m = 1;
        }

        if ((i == 0 && i == 1) || a.read()) {
            m = 2;
        }
        
        if ((i == 0 && a.read()) || c.read()) {
            m = 3;
        }

        if ((i == 1 && a.read()) || c.read()) {
            m = 4;
        }

        if ((i == 0 && a.read()) && c.read()) {
            m = 5;
        }
    }

    // Conditional operator
    void simple_cond1() {
        int i = 1;
       
        int m = (i < 0) ? 1 : 2;
        
        sct_assert_const(m == 2);
    }
    
    void simple_var() {
        int k = 1;
        int m = a.read();
        int i = m;
    } 
    
    //---------------------------------------------------------------------------    
    // IF with local array
    void local_array_if() {
        if (a.read()) {
            bool arr[3];
        }
    }

    // Local array in main loop
    void local_array() 
    {
        wait();
        
        while (true) {
            bool arr[3];
            
            wait();
        }
    }    
    
    
    // -----------------------------------------------------------------------
    // Multiple function calls at different levels
    void g() {
        int k = 0;
    }

    void multiple_calls1()
    {
        if (a.read()) {
            g();
        }

        g();
    }
    
    void multiple_calls2()
    {
        g();

        if (a.read()) {
            g();
        }
    }

    // -----------------------------------------------------------------------
    // Constant returned from function
    int getConst() {
        return 2;
    }
    int getConst_(int i) {
        return i+1;
    }

    void return_const_in_if() {
        int i = a.read();
        if (i < getConst()) {
            int ll = 1;
        }
    }

    void return_const_in_for() {
        int x = 0;
        int N = getConst();
        sct_assert_const (N==2);
        for (int i = 0; i < N; ++i) {
            x++;
        }
        sct_assert_const (x==2);
    }

    // Function call in loop not supported
    void return_const_in_for2() {
        int x = 0;
        for (int i = 0; i < getConst(); ++i) {
            x++;
        }
        sct_assert_const (x==2);
    }

    void return_const_in_for3() {
        int x = 0;
        for (int i = getConst(); i < 4; ++i) {
            x++;
        }
        sct_assert_const (x==2);
    }

    void return_const_in_while() {
        int i = 0;
        while (i < getConst()) {
            i++;
        }
        sct_assert_const (i == 2);
    }

    void return_const_in_while2() {
        int i = 0;
        while (i < getConst_(1)) {
            i++;
        }
        sct_assert_const (i == 2);
    }

    void return_const_in_dowhile() {
        int i = 0;
        do {
            i++;
        } while (i < getConst()); 
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

