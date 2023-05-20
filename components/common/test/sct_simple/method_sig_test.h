/******************************************************************************
 * Copyright (c) 2023, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 *
 *****************************************************************************/

/*
 * One target/initiator pair in METHOD/THREAD controlled by signals to simplify debug.
 */

#ifndef SIMPLE_METHOD_TEST_H
#define SIMPLE_METHOD_TEST_H

#include "sct_common.h"
#include "sct_assert.h"
#include <systemc.h>

template<class T>
struct B : public sc_module 
{
    sc_in<bool>         clk{"clk"};
    sc_in<bool>         nrst{"nrst"};

    sct_target<T>       run{"run", 0};
    sc_signal<bool>     resp_ready;
    sc_signal<bool>     resp_req;
    sc_signal<T>        resp_data;
  
    SC_HAS_PROCESS(B);
    
    explicit B(const sc_module_name& name) : sc_module(name)
    {
        run.clk_nrst(clk, nrst);

        SC_METHOD(methProc);
        //SC_THREAD(threadProc);
        //async_reset_signal_is(nrst, SCT_CMN_TRAITS::RESET);
        sensitive << run; 
    }
    
    void methProc() {
        run.reset_get();
        resp_req = run.request();
        resp_data = 0;
        
        if (run.request() && resp_ready) {
            T data = run.get();
            resp_data = data;
            cout << sc_time_stamp() << " " << sc_delta_count()
                 << " : Get " << data << endl;
        } else {
            cout << sc_time_stamp() << " " << sc_delta_count()
                 << " : NO get " << endl;
        }
    }
    
     void threadProc() {
        run.reset_get();
        resp_req  = 0;
        resp_data = 0;
        wait();
        
        while (true) {
            resp_req = run.request();
            if (run.request() && resp_ready) {
                T data = run.get();
                resp_data = data;
                cout << sc_time_stamp() << " " << sc_delta_count()
                     << " : Get " << data << endl;
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

    sct_initiator<T>    run{"run", 0};

    B<T>        a{"a"};

    SC_HAS_PROCESS(simple_test);

    explicit simple_test(const sc_module_name& name) : sc_module(name)
    {
        cout << "simple_method_sig" << endl;
        run.clk_nrst(clk, nrst);
        a.clk(clk);
        a.nrst(nrst);
        a.run.bind(run);
        
        SC_THREAD(init_thread);
        sensitive << run << clk.pos();
        async_reset_signal_is(nrst, SCT_CMN_TRAITS::RESET);

        //SC_METHOD(test_meth); sensitive << clk.pos();
        //SC_METHOD(test_meth2); sensitive << event;
    }
    
    /// Test to establish clk.pos() event is delayed 1DC over timed event
//    const sc_time ONE_NS = sc_time(1, SC_NS);
//    sc_event event{"event"};
//    void test_meth() {
//        cout << sc_time_stamp() << " " << sc_delta_count() << " : Test meth " << endl;
//        event.notify(ONE_NS);
//    }
//    
//    void test_meth2() {
//        cout << sc_time_stamp() << " " << sc_delta_count() << " : Test meth 2" << endl;
//    }
    
    void init_thread()
    {
        run.reset_put();
        a.resp_ready = 1;
        wait();

        while (!run.put(42)) wait();
        //run.put(42);
        cout << sc_time_stamp() << " " << sc_delta_count() << " : Put " << endl;
        wait();

        cout << sc_time_stamp() << " " << sc_delta_count() << " : Wait resp " << endl;
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

#endif /* SIMPLE_METHOD_TEST_H */

