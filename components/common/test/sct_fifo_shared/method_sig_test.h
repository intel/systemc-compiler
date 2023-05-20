/******************************************************************************
 * Copyright (c) 2023, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 *
 *****************************************************************************/

/*
 * One FIFO for METHOD/THREAD interconnect controlled by signals to simplify debug.
 */

#ifndef SHARED_METH_TEST_H
#define SHARED_METH_TEST_H

#include "sct_common.h"
#include "sct_assert.h"
#include <systemc.h>

template<class T>
struct A : public sc_module 
{
    sc_in<bool>         clk{"clk"};
    sc_in<bool>         nrst{"nrst"};

    sc_signal<bool>     run_ready;
    sc_signal<bool>     run_req;
    sc_signal<T>        run_data;
    sc_signal<bool>     resp_ready;
    sc_signal<bool>     resp_req;
    sc_signal<T>        resp_data;
    
    sct_fifo<T, 2>      fifo{"fifo", 1};
    //sct_fifo<T, 2>      fifo{"fifo", 1, 1};
  
    SC_HAS_PROCESS(A);
    
    explicit A(const sc_module_name& name) : sc_module(name)
    {
        SC_METHOD(putMeth);
        //SC_THREAD(putThread);
        sensitive << run_req << run_data << fifo.PUT;
        //async_reset_signal_is(nrst, 0);

        SC_METHOD(getMeth);
        //SC_THREAD(getThread);
        sensitive << resp_ready << fifo.GET;
        //async_reset_signal_is(nrst, 0);
    }
    
    void putMeth() {
        run_ready = fifo.ready();
        fifo.reset_put();
        
        if (run_req && fifo.ready()) {
            T data = run_data;
            fifo.put(data);
            cout << sc_time_stamp() << " " << sc_delta_count() << " : Put into FIFO " << data << " run_req " << run_req.read() << endl;
        } else {
            cout << sc_time_stamp() << " " << sc_delta_count() << " : NO put" << endl;
        }
    }
    void putThread() {
        run_ready = 0;
        fifo.reset_put();
        wait();
        
        while (true) {
            run_ready = fifo.ready();
            if (run_req && fifo.ready()) {
                T data = run_data;
                fifo.put(data);
                cout << sc_time_stamp() << " " << sc_delta_count() << " : Put into FIFO " << data << " run_req " << run_req.read() << endl;
            }
            wait();
        }
    }
    
    void getMeth() {
        resp_req = 0;
        resp_data = 0;
        fifo.reset_get();
        
        if (fifo.request() && resp_ready) {
            T data = fifo.get();
            resp_req = 1;
            resp_data = data;
            //cout << sc_delta_count() << " : " << fifo.elem_num() << endl;
            cout << sc_time_stamp() << " " << sc_delta_count() << " : Get from FIFO " << data << endl;
        } else {
            cout << sc_time_stamp() << " " << sc_delta_count() << " : NO get" << endl;
        }
    }
    
    void getThread() {
        resp_req = 0;
        resp_data = 0;
        fifo.reset_get();
        wait();
        
        while (true) {
            if (fifo.request() && resp_ready) {
                T data = fifo.get();
                resp_req = 1;
                resp_data = data;
                //cout << sc_delta_count() << " : " << fifo.elem_num() << endl;
                cout << sc_time_stamp() << " " << sc_delta_count() << " : Get from FIFO " << data << endl;
            }
            wait();
        }
    }
};

class simple_test : public sc_module 
{
public:
    using T = sc_uint<16>;

    sc_in<bool>         clk{"clk"};
    sc_in<bool>         nrst{"nrst"};

    A<T>        a{"a"};

    SC_HAS_PROCESS(simple_test);

    explicit simple_test(const sc_module_name& name) : sc_module(name)
    {
        cout << "fifo_shared meth" << endl;
        a.clk(clk);
        a.nrst(nrst);
        a.fifo.clk_nrst(clk, nrst);
        
        SC_CTHREAD(init_thread, clk.pos());
        //sensitive << a.run_ready << a.run_req << a.resp_req << a.resp_data;
        async_reset_signal_is(nrst, 0);
    }
    
    const unsigned N = 1;
    void init_thread()
    {
        a.run_req = 0;
        a.run_data = 0;
        //a.resp_ready = 0;
        a.resp_ready = 1;
        wait();

        a.run_req = 1;
        a.run_data = 42;
        while (!a.run_ready || !a.run_req) wait();
        a.run_req = 0;
        a.run_data = 0;
        
        cout << sc_time_stamp() << " " << sc_delta_count() << " : TB thread, put done " << endl;

        //a.resp_ready = 1;
        while (!a.resp_req || !a.resp_ready) wait();
        T data = a.resp_data;
        a.resp_ready = 0;
        
        cout << sc_time_stamp() << " " << sc_delta_count() << " : TB thread, get done = " << data << endl;
        wait();
        
        while (true) {
            wait();
        }
    }
};

#endif /* SHARED_METH_TEST_H */

