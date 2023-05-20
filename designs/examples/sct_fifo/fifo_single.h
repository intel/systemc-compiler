/******************************************************************************
 * Copyright (c) 2023, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 *
 *****************************************************************************/

/*
 * FIFO used inside of single process
 */

#ifndef FIFO_SINGLE_H
#define FIFO_SINGLE_H

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

    sct_fifo<T, 2>      fifo{"fifo", 1, 1};
  
    SC_HAS_PROCESS(A);
    
    explicit A(const sc_module_name& name) : sc_module(name)
    {
        fifo.clk_nrst(clk, nrst);

        SCT_CTHREAD(threadProc, clk, SCT_CMN_TRAITS::CLOCK);
        sensitive << fifo;
        async_reset_signal_is(nrst, SCT_CMN_TRAITS::RESET);
    }
    
    void threadProc() {
        run_ready = 0;
        resp_req  = 0;
        resp_data = 0;
        fifo.reset();
        wait();
        
        while(true) {
            run_ready = fifo.ready();
            if (run_req && fifo.ready()) {
                T data = run_data.read();
                fifo.put(data);
                cout << sc_time_stamp() << " " << sc_delta_count() 
                     << " : Put into FIFO " << data << endl;
            } else 
            if (fifo.request() && resp_ready) {
                T data = fifo.get();
                resp_req = 1;
                resp_data = data;
                cout << sc_time_stamp() << " " << sc_delta_count() 
                     << " : Get from FIFO " << data << endl;
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
        cout << "fifo single thread test" << endl;
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
    
    const unsigned N = 4;
    void init_thread()
    {
        run_req = 0;
        run_data = 0;
        resp_ready = 1;
        wait();

        for (int i = 0; i < N; ++i) {
            while (!run_ready) wait();
            run_req = 1; run_data = 32+i;
            wait();
        }
        cout << sc_time_stamp() << " " << sc_delta_count() << " : #1, all put done " << endl;

        T data; 
        for (int i = 0; i < N; ++i) {
            while (!resp_req) wait();
            assert (resp_data.read() == 32+i);
            wait();
        }
        cout << sc_time_stamp() << " " << sc_delta_count() << " : #1 all get done " << endl << endl;
        wait();
        

        while (true) {
            wait();
        }
    }
};

#endif /* FIFO_SINGLE_H */

