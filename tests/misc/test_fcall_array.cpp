/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Port/signal pointers, arrays and pointer arrays as function parameter
struct Top : public sc_module
{
    sc_in<bool>         clk{"clk"};
    sc_in<bool>         rst{"rst"};
    
    sc_signal<bool>     s;
    
    sc_in<sc_uint<8>>       iarr[3];
    sc_out<sc_uint<8>>      oarr[3];

    SC_HAS_PROCESS(Top);
    Top (const sc_module_name& name) : sc_module(name) 
    {
        pi = sc_new<sc_uint<4>>();
        ps = new sc_signal<sc_uint<4>>;
        parr = new sc_signal<bool>[3];
        for (int i = 0; i < 3; ++i) {
            marrp[i] = sc_new<sc_uint<4>>();
            rarrp[i] = new sc_signal<sc_uint<4>>("parr");
            psarr[i] = new sc_signal<sc_uint<4>>("parr");
        }
        
        SC_METHOD(methProc);
        for (int i = 0; i < 3; ++i) {
            sensitive << sarr[i] << parr[i] << *psarr[i] << iarr[0];
        }

        SC_CTHREAD(intArrThread, clk.pos());
        async_reset_signal_is(rst, 0);

        SC_CTHREAD(intArrThread2, clk.pos());
        async_reset_signal_is(rst, 0);

        SC_CTHREAD(intPtrArrThread, clk.pos());
        async_reset_signal_is(rst, 0);

        SC_CTHREAD(sigPtrThread, clk.pos());
        async_reset_signal_is(rst, 0);
        
        SC_CTHREAD(sigArrThread, clk.pos());
        async_reset_signal_is(rst, 0);

        SC_CTHREAD(sigLocRefThread, clk.pos());
        async_reset_signal_is(rst, 0);
        
    }
    
    void f1 (sc_signal<bool> val[]) {
        s = val[0] ^ val[1];
    }
    
    void f2 (sc_signal<bool>* val, int i) {
        val[i] = true;
        s = val[i+1];
    }

    void f3 (sc_signal<sc_uint<4>>* val[], int j, sc_uint<4> newval) {
        val[j]->write(newval);
    }

    void f4 (sc_signal<bool>* sig, sc_out<sc_uint<8>> prt[3], int k) {
        prt[k] = sig[k+1] ? 42 : 0;
    }
    
    template<class SIG_t, class PORT_t>
    void f5 (SIG_t* sig, PORT_t* prt) {
        for (int j = 0; j < 3; j++) {
            sig[j] = prt[j].read();
        }
    }

    sc_signal<bool>         sarr[3];
    sc_signal<bool>*        parr;
    sc_signal<sc_uint<4>>*  psarr[3];
    
    void methProc() 
    {
        f1(sarr);
        f1(parr);
        f2(sarr, 0);
        f2(parr, 1);
        f3(psarr, 1, 5);
        f4(sarr, oarr, 1);
        f5(sarr, iarr);
    }
    
//----------------------------------------------------------------------------
// Threads with non-channel array and pointer

    void initIntArr(sc_uint<4> val[3], sc_uint<4> mval[3][3]) {
        for (int i = 0; i < 3; i++) {
            val[i] = 0;
            for (int j = 0; j < 3; j++) {
                mval[i][j] = 0;
            }
        }
    }
    
    void intArrFunc(sc_uint<4>* val, size_t SIZE) {
        for (int j = 0; j < SIZE; j++) {
            val[j] = j+1;
        }
    }

    void intMultArrFunc (sc_uint<4> val[], sc_uint<4> mval[][3], unsigned k) {
        mval[0][k] = val[k];
    }

    sc_uint<4>      arr[3];         // register
    sc_uint<4>      marr[3][3];     // comb
    
    // Array and multi-dimensional array registers
    void intArrThread() 
    {
        initIntArr(arr, marr);
        sc_uint<4> x = marr[1][1];
        wait();
        
        while(true) {
            intArrFunc(arr, 3);
            
            wait();
            
            intMultArrFunc(arr, marr, 1);
        }
    }
    
    sc_uint<4>      narr[3];         // register
    sc_uint<4>      nnarr[3][3];     // comb
    sc_signal<sc_uint<4>> ns;
    
    // Array and multi-dimensional array registers, reset value used after reset
    void intArrThread2() 
    {
        ns = 0;
        initIntArr(narr, nnarr);
        wait();
        
        while(true) {
            
            int j = ns.read();
            ns = nnarr[0][j];
            wait();
            
            intMultArrFunc(narr, nnarr, 1);
            ns = narr[j];
        }
    }
    
    template<class VAL1_t, class VAL2_t>
    void intPtrArrFunc(VAL1_t* val1, VAL2_t* val2[]) {
        *val1 = 1;
        *val2[*val1] = 0;
    }

    sc_uint<4>*     pi;             // register
    sc_uint<4>*     marrp[3];       // register
    
    // Pointer and pointer array registers
    void intPtrArrThread() 
    {
        wait();
        
        while(true) {
            intPtrArrFunc(pi, marrp);
            
            wait();
            
            int l = *pi + *marrp[1];
        }
    }


//----------------------------------------------------------------------------
// Threads with signal/port array

    // Channel reference and channel pointer
    template<class SIG1_t, class SIG2_t>
    void sigRefPtrFunc (SIG1_t& sig1, SIG2_t* sig2) {
        sig1 = 0;
        auto a1 = sig1.read();
        *sig2 = 0;
        auto a2 = sig2->read();
    }
    
    sc_signal<sc_uint<4>>   ss;
    sc_signal<sc_uint<4>>*  ps;
    
    // Pointer to channel
    void sigPtrThread() 
    {
        wait();
        
        while(true) 
        {
            sigRefPtrFunc(ss, ps);
            wait();
        }
    }
    
    template<class SIG_t>
    void initSigArr(SIG_t val[], size_t SIZE) {
        for (int i = 0; i < SIZE; i++) {
            val[i] = 0;
        }
    }

    template<class SIG_t>
    void initSigPtrArr(SIG_t* val[], size_t SIZE) {
        for (int i = 0; i < SIZE; i++) {
            *val[i] = 0;
        }
    }
    
    // Two arrays
    template<class SIG1_t, class SIG2_t>
    void sigArrPtrFunc(SIG1_t* sig1, SIG2_t* sig2[], size_t SIZE) {
        for (int j = 0; j < SIZE; j++) {
            sig1[j] = *sig2[j];
        }
        *sig2[0] = sig1[SIZE-1];
        sig2[1]->write(sig1[SIZE-2]);
    }
    
    sc_signal<sc_uint<4>>   rarr[3];
    sc_signal<sc_uint<4>>*  rarrp[3];

    // Array of channels and channel pointers
    void sigArrThread() 
    {
        initSigArr(rarr, 3);
        initSigPtrArr(rarrp, 3);
        wait();
        
        while(true) {
            sigArrPtrFunc(rarr, rarrp, 3);
            wait();
        }
    }
    
//----------------------------------------------------------------------------
    
    // Local reference to signal, no local pointer to signal allowed
    void sigLocRefThread() 
    {
        wait(); 
        
        while(true) 
        {
            sc_signal<sc_uint<4>>& rs = ss;
            rs = 0;
            sc_uint<4> l = rs.read();
            
            wait();
        }
    }

};

int sc_main(int argc, char **argv) {

    sc_clock clock_gen{"clock_gen", 10, SC_NS};
    sc_signal<bool> rst{"rst"};
    sc_signal<sc_uint<8>> t[3];
    
    Top top{"top"};
    
    top.clk(clock_gen);
    top.rst(rst);
    for (int i = 0; i < 3; ++i) {
        top.iarr[i](t[i]);
        top.oarr[i](t[i]);
    }
    sc_start();

    return 0;
}

