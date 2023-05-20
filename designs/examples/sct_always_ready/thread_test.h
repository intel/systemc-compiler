/*
 * Always ready target (@sct_comb_target) in THREAD to check if sync is required
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

    sct_comb_target<T>          run{"run"};
    sct_fifo<T, 2>              fifo{"fifo"};
    sct_initiator<T>            resp{"resp"};
  
    SC_HAS_PROCESS(A);
    
    explicit A(const sc_module_name& name) : sc_module(name)
    {
        run.clk_nrst(clk, nrst);
        resp.clk_nrst(clk, nrst);
        fifo.clk_nrst(clk, nrst);
        
        SC_THREAD(inThreadProc);
        sensitive << run << fifo.PUT;
        async_reset_signal_is(nrst, 0);

        SC_THREAD(outThreadProc);
        sensitive << resp << fifo.GET;
        async_reset_signal_is(nrst, 0);
    }
    
    void inThreadProc() {
        run.reset_get();
        fifo.reset_put();
        wait();
        
        while (true) {
            auto l = run.b_get();
            cout << sc_time_stamp() << " " << sc_delta_count() << " get from targ " << l << endl;
            assert (fifo.ready());
            fifo.put(l);
            wait();
        }
    }
    
    void outThreadProc() {
        fifo.reset_get();
        resp.reset_put();
        wait();
        
        while (true) {
            if (fifo.request()) {
                auto l = fifo.get();
                resp.put(l);
                //cout << sc_delta_count() << " get from fifo " << l << endl;
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
    sct_comb_target<T>  resp{"resp"};

    A<T>        a{"a"};

    SC_HAS_PROCESS(simple_test);

    explicit simple_test(const sc_module_name& name) : sc_module(name)
    {
        cout << "always_ready_method" << endl;
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
    
    const unsigned N = 4;
    const unsigned M = 5;
    void init_thread()
    {
        T data; int j;
        run.reset_put();
        resp.reset_get();
        wait();

        run.put(42);
        wait();
        while (!resp.get(data)) wait();
        cout << sc_time_stamp() << " " << sc_delta_count() << " : data " << data << endl;
        assert (data == 42); wait();
        cout << sc_time_stamp() << " " << sc_delta_count() << " : #1 test " << endl;

        j = 0;
        for (int i = 0; i < N; ++i) {
            cout << sc_delta_count() << " put to targ " << i << endl;
            run.put(i);
            if (resp.get(data)) {
                cout << "data " << data << endl; assert (data == j); j++;
            }
            wait();
        }
        while (j < N) {
            if (resp.get(data)) {
                cout << "data " << data << endl; assert (data == j); j++;
            }
            wait();
        }

        cout << endl;
        cout << sc_time_stamp() << " " << sc_delta_count() << " : #2 test " << endl;

        j = 0;
        for (int i = 0; i < M; ++i) {
            assert (run.ready());
            run.put(i);
            if (resp.get(data)) {
                cout << "tb data " << data << endl; assert (data == j); j++;
            }
            wait();
        }
        while (j < M) {
            if (resp.get(data)) {
                cout << "tb data " << data << endl; assert (data == j); j++;
            }
            wait();
        }
        cout << sc_time_stamp() << " " << sc_delta_count() << " : all tests done " << endl;
        
        while (true) {
            wait();
        }
    }
};

#endif /* SIMPLE_THREAD_TEST_H */

