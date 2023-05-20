/******************************************************************************
 * Copyright (c) 2023, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 *
 *****************************************************************************/

/*
 * Multiple sequential puts/gets to FIFO done from THREAD 
 * to check there no hangs up.
 */

#ifndef SIMPLE_THREAD_TEST_H
#define SIMPLE_THREAD_TEST_H

#include "sct_common.h"
#include "sct_assert.h"
#include <systemc.h>

template<class T>
struct A : public sc_module 
{
    sc_in<bool>         clk{"clk"};
    sc_in<bool>         nrst{"nrst"};

    sct_target<T>       run{"run"};
    sct_initiator<T>    resp{"resp"};
    sct_fifo<T, 4>      fifo{"fifo", 0, 0, 1};
  
    SC_HAS_PROCESS(A);
    
    explicit A(const sc_module_name& name) : sc_module(name)
    {
        run.clk_nrst(clk, nrst);
        resp.clk_nrst(clk, nrst);
        fifo.clk_nrst(clk, nrst);

        SC_THREAD(threadA);
        sensitive << run << fifo.PUT;
        async_reset_signal_is(nrst, 0);

        SC_THREAD(threadB);
        sensitive << fifo.GET << resp;
        async_reset_signal_is(nrst, 0);
    }
    
    const unsigned N = 4;
    void threadA() {
        run.reset_get();
        fifo.reset_put();
        wait();
        
        while(true) {
            T data = run.b_get();

            unsigned i = 0;
            while (fifo.ready() && i < N) {
                fifo.put(data + i);
                cout << sc_time_stamp() << " " << sc_delta_count() 
                     << " : Put to fifo " << (data+i) << endl;
                i++;
                wait();
            }
            wait();
        }
    }
    
    void threadB() {
        fifo.reset_get();
        resp.reset_put();
        T data;
        wait();
        
        while(true) {
            if (fifo.elem_num() >= N) {
                unsigned i = 0;
                while (fifo.request() && i < N) {
                    data = fifo.get();
                    cout << sc_time_stamp() << " " << sc_delta_count() 
                         << " : Get from fifo " << data << endl;
                    i++;
                    wait();

                }
                resp.b_put(data-N+1);
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

    sct_initiator<T>    run{"run"};
    sct_target<T>       resp{"resp"};

    A<T>        a{"a"};

    SC_HAS_PROCESS(simple_test);

    explicit simple_test(const sc_module_name& name) : sc_module(name)
    {
        cout << "shared fifo multi_put_test" << endl;
                
        run.clk_nrst(clk, nrst);
        resp.clk_nrst(clk, nrst);
        a.clk(clk);
        a.nrst(nrst);
        a.run.bind(run);
        a.resp.bind(resp);
        
        SC_THREAD(init_thread);
        sensitive << run << resp;
        async_reset_signal_is(nrst, 0);
    }
    
    const unsigned N = 3;
    void init_thread()
    {
        T data; 
        run.reset_put();
        resp.reset_get();
        wait();

        while (!run.put(42)) wait();
        cout << sc_time_stamp() << " " << sc_delta_count() << " : TB put 42" << endl;
        wait();
        
        while (!resp.get(data)) wait();
        cout << sc_time_stamp() << " " << sc_delta_count() << " : TB get " << data << endl;
        assert (data == 42); wait();
        wait();
        
        
        while (!run.put(43)) wait();
        cout << sc_time_stamp() << " " << sc_delta_count() << " : Put 43" << endl;
        wait();
        while (!run.put(44)) wait();
        cout << sc_time_stamp() << " " << sc_delta_count() << " : Put 44" << endl;
        wait();
        
        while (!resp.get(data)) wait();
        assert (data == 43); wait();
        while (!resp.get(data)) wait();
        assert (data == 44); wait();

        cout << sc_time_stamp() << " " << sc_delta_count() << " : #2 all get done " << endl;
        wait();

        while (true) {
            wait();
        }
    }
};

#endif /* SIMPLE_THREAD_TEST_H */

