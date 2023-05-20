/*
 * FIFO bound to target which used in THREAD/METHOD
 */

#ifndef TARGET_FIFO_TEST_H
#define TARGET_FIFO_TEST_H

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
  
    SC_HAS_PROCESS(A);
    
    explicit A(const sc_module_name& name) : sc_module(name)
    {
        run.clk_nrst(clk, nrst);
        resp.clk_nrst(clk, nrst);

        run.template add_fifo<2>(1, 1);

        SC_THREAD(threadProc);
        sensitive << resp << run;
        async_reset_signal_is(nrst, 0);
        
        cout << "fifo_bind_thread" << endl;
    }
    
    void threadProc() {
        run.reset_get();
        resp.reset_put();
        wait();
        
        while(true) {
            if (run.request() && resp.ready()) {
                T data = run.get();
                resp.put(data);
                cout << sc_time_stamp() << " " << sc_delta_count() 
                     << " : Get from FIFO " << data << endl;
            }
            wait();
        }
    }
};

template<class T>
struct B : public sc_module 
{
    sc_in<bool>         clk{"clk"};
    sc_in<bool>         nrst{"nrst"};

    sct_target<T>       run{"run"};
    sct_initiator<T>    resp{"resp"};
  
    SC_HAS_PROCESS(B);
    
    explicit B(const sc_module_name& name) : sc_module(name)
    {
        run.clk_nrst(clk, nrst);
        resp.clk_nrst(clk, nrst);

        run.template add_fifo<2>(1, 1);

        SC_METHOD(methProc);
        sensitive << run << resp;
        async_reset_signal_is(nrst, 0);
        cout << "fifo_bind_meth" << endl;
    }
    
    void methProc() {
        run.reset_get();
        resp.reset_put();
        
        if (run.request() && resp.ready()) {
            T data = run.get();
            resp.put(data);
            cout << sc_time_stamp() << " " << sc_delta_count() 
                 << " : Get from FIFO " << data  << endl;
        }
    }
};


class simple_test : public sc_module 
{
public:
    using T = sc_uint<16>;

    sc_in<bool>         clk{"clk"};
    sc_in<bool>         nrst{"nrst"};

#ifdef SYNC_INIT
    sct_initiator<T>    run{"run", 1};
#else
    sct_initiator<T>    run{"run"};
#endif
    sct_target<T>       resp{"resp"};

// These pipeline numbers are maximal one for RTL and TLM mode
#ifdef THREAD
    A<T>        a{"a"};
    const unsigned N = 6;
#endif
#ifdef METHOD
    B<T>        a{"a"};
    #ifdef SYNC_INIT
        const unsigned N = 6;  // 7 for TLM 
    #else
        const unsigned N = 5;  // 6 for TLM
    #endif
#endif

    SC_HAS_PROCESS(simple_test);

    explicit simple_test(const sc_module_name& name) : sc_module(name)
    {
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
    
    void init_thread()
    {
        T data; 
        run.reset_put();
        resp.reset_get();
        wait();

        // Scenario #1. Multiple push w/o pop
        for (int i = 0; i < N; ++i) {
            while (!run.put(42+i)) wait();
            cout << sc_time_stamp() << " " << sc_delta_count() << " : put " << (42+i) << endl;
            wait();
        }
        cout << sc_time_stamp() << " " << sc_delta_count() << " : #1, all put done " << endl;

        for (int i = 0; i < N; ++i) {
            while (!resp.get(data)) wait();
            assert (data == 42+i);
            wait();
        }
        cout << sc_time_stamp() << " " << sc_delta_count() << " : #1 all get done " << endl << endl;
        
        // Scenario #2. Push mixed with pop
        while (!run.put(42)) wait();
        wait();
        
        while (!resp.get(data)) wait();
        assert (data == 42); wait();
        
        while (!run.put(43)) wait();
        wait();
        while (!run.put(44)) wait();
        wait();
        
        while (!resp.get(data)) wait();
        assert (data == 43); wait();
        while (!resp.get(data)) wait();
        assert (data == 44); wait();

        while (!run.put(45)) wait();
        wait();
        while (!run.put(46)) wait();
        wait();
        while (!run.put(47)) wait();
        wait();

        while (!resp.get(data)) wait();
        assert (data == 45); wait();
        while (!resp.get(data)) wait();
        assert (data == 46); wait();
        while (!resp.get(data)) wait();
        assert (data == 47); wait();
        cout << sc_time_stamp() << " " << sc_delta_count() << " : #2 all get done " << endl;

        wait();

        while (true) {
            wait();
        }
    }
};

#endif /* TARGET_FIFO_TEST_H */

