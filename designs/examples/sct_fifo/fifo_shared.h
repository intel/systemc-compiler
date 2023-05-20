/******************************************************************************
 * Copyright (c) 2023, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 *
 *****************************************************************************/

/*
 * FIFO used for process to process interconnect
 */

#ifndef FIFO_SHARED_H
#define FIFO_SHARED_H

#include "sct_fifo.h"
#include "sct_assert.h"
#include <systemc.h>

using namespace sct;

template<class T>
struct A : public sc_module 
{
    sc_in<bool>         clk{"clk"};
    sc_in<bool>         nrst{"nrst"};

    sc_out<bool>        run_ready;
    sc_in<bool>         run_req;
    sc_in<T>            run_data;
    sc_in<bool>         resp_ready;
    sc_out<bool>        resp_req;
    sc_out<T>           resp_data;
    
    sct_fifo<T, 2>      fifo{"fifo", 1};
  
    SC_HAS_PROCESS(A);
    
    explicit A(const sc_module_name& name) : sc_module(name)
    {
        fifo.clk_nrst(clk, nrst);
        
        SC_METHOD(putMeth);
        sensitive << run_req << run_data << fifo.PUT;
        //SC_CTHREAD(putThread, clk.pos());
        //async_reset_signal_is(nrst, 0);

        SC_METHOD(getMeth);
        sensitive << resp_ready << fifo.GET;
        //SC_CTHREAD(getThread, clk.pos());
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

    sc_signal<bool>     run_ready;
    sc_signal<bool>     run_req;
    sc_signal<T>        run_data;
    sc_signal<bool>     resp_ready;
    sc_signal<bool>     resp_req;
    sc_signal<T>        resp_data;
    
    A<T>        a{"a"};

    SC_HAS_PROCESS(simple_test);

    explicit simple_test(const sc_module_name& name) : sc_module(name)
    {
        cout << "fifo_shared meth" << endl;
        a.clk(clk);
        a.nrst(nrst);
        
        a.run_ready(run_ready);
        a.run_req(run_req);
        a.run_data(run_data);
        a.resp_ready(resp_ready);
        a.resp_req(resp_req);
        a.resp_data(resp_data);
        
        SCT_CTHREAD(init_thread, clk, SCT_CMN_TRAITS::CLOCK);
        async_reset_signal_is(nrst, SCT_CMN_TRAITS::RESET);
    }
    
    const unsigned N = 1;
    void init_thread()
    {
        run_req = 0;
        run_data = 0;
        resp_ready = 1;
        wait();

        run_req = 1;
        run_data = 42;
        while (!run_ready || !run_req) wait();
        run_req = 0;
        run_data = 0;
        
        cout << sc_time_stamp() << " " << sc_delta_count() 
             << " : TB thread, put done " << endl;

        while (!resp_req || !resp_ready) wait();
        T data = resp_data;
        resp_ready = 0;
        
        cout << sc_time_stamp() << " " << sc_delta_count() 
             << " : TB thread, get done = " << data << endl;
        wait();
        
        while (true) {
            wait();
        }
    }
};

#endif /* FIFO_SHARED_H */

