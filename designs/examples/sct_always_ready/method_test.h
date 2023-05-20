/*
 * Always ready target (@sct_comb_target) in METHOD process
 */

#ifndef SIMPLE_METHOD_TEST_H
#define SIMPLE_METHOD_TEST_H

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
    sct_initiator<T>            resp{"resp", 1};
  
    SC_HAS_PROCESS(A);
    
    explicit A(const sc_module_name& name) : sc_module(name)
    {
        run.clk_nrst(clk, nrst);
        resp.clk_nrst(clk, nrst);
        fifo.clk_nrst(clk, nrst);
        
        SC_METHOD(methProc);
        sensitive << run << fifo.PUT;
        
        SC_THREAD(threadProc);
        sensitive << resp << fifo.GET;
        async_reset_signal_is(nrst, 0);
    }
    
    void methProc() {
        run.reset_get();
        fifo.reset_put();
        
        if (run.request()) {
            assert (fifo.ready());
            fifo.put(run.get());
            //cout << sc_time_stamp() << " " << sc_delta_count() << " put " << run.peek() << endl;
        }
    }
    
    void threadProc() {
        fifo.reset_get();
        resp.reset_put();
        wait();
        
        while (true) {
            if (fifo.request()) {
                assert (resp.ready());
                resp.put(fifo.get());
                //cout << sc_time_stamp() << " " << sc_delta_count() << " tresp put " << fifo.peek() << endl;
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

    sct_initiator<T>    trun{"trun"};
    sct_comb_target<T>  tresp{"tresp"};

    A<T>        a{"a"};

    SC_HAS_PROCESS(simple_test);

    explicit simple_test(const sc_module_name& name) : sc_module(name)
    {
        cout << "always_ready_method" << endl;
        trun.clk_nrst(clk, nrst);
        tresp.clk_nrst(clk, nrst);
        a.clk(clk);
        a.nrst(nrst);
        a.run.bind(trun);
        a.resp.bind(tresp);
        
        SC_THREAD(init_thread);
        sensitive << trun << tresp;
        async_reset_signal_is(nrst, 0);
    }
    
    const unsigned N = 3;
    const unsigned M = 4;
    void init_thread()
    {
        T data; int j;
        trun.reset_put();
        tresp.reset_get();
        wait();

        trun.put(42);
        wait();
        data = tresp.b_get();
        cout << sc_time_stamp() << " " << sc_delta_count() << " : data " << data << endl;
        assert (data == 42); wait();
        cout << sc_time_stamp() << " " << sc_delta_count() << " : #1 test " << endl;
        cout << endl;

        j = 0;
        for (int i = 0; i < N; ++i) {
            cout << sc_delta_count() << " put to targ " << i << endl;
            trun.put(i);
            if (tresp.get(data)) {
                cout << "data " << data << endl; assert (data == j); j++;
            }
            wait();
        }
        while (j < N) {
            if (tresp.get(data)) {
                cout << "data " << data << endl; assert (data == j); j++;
            }
            wait();
        }
        
        cout << sc_time_stamp() << " " << sc_delta_count() << " : #2 test " << endl;
        cout << endl;
        
        j = 0;
        for (int i = 0; i < M; ++i) {
            trun.put(i);
            //cout << sc_time_stamp() << " " << sc_delta_count() << " put " << i << endl;
            if (tresp.get(data)) {
                cout << sc_time_stamp() << " " << sc_delta_count() << " data " << data << endl; assert (data == j); j++;
            }
            wait();
        }
        while (j < M) {
            //cout << sc_time_stamp() << " " << sc_delta_count() << " ... " << j << endl;
            if (tresp.get(data)) {
                cout << sc_time_stamp() << " " << sc_delta_count() << " data " << data << endl; assert (data == j); j++;
            }
            wait();
        }
        cout << sc_time_stamp() << " " << sc_delta_count() << " : all tests done " << endl;
        wait();

        while (true) {
            wait();
        }
    }
};

#endif /* SIMPLE_METHOD_TEST_H */

