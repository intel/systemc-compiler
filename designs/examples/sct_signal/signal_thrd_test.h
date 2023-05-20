/*
 * Test for @sct_signal used in threads and methods 
 */

#ifndef SIMPLE_TEST_H
#define SIMPLE_TEST_H

#include "sct_common.h"
#include "sct_assert.h"
#include <systemc.h>


template<unsigned N>
struct A : public sc_module 
{
    using T = sc_uint<N>;
    sc_in<bool>         clk{"clk"};
    sc_in<bool>         nrst{"nrst"};

    sct_target<T>       run{"run"};
    sct_initiator<T>    resp{"resp"};
    
    SC_HAS_PROCESS(A);
    
    explicit A(const sc_module_name& name) : sc_module(name)
    {
        run.clk_nrst(clk, nrst);
        resp.clk_nrst(clk, nrst);
        
        SCT_THREAD(runThrd, clk);
        sensitive << run << busy;
        async_reset_signal_is(nrst, false);
        
        SCT_THREAD(compThrd, clk);
        sensitive << resp << a_enbl << a << c;
        async_reset_signal_is(nrst, false);

        SC_METHOD(compMeth);
        sensitive << a_enbl << busy_d << b << b_enbl;
    }
    
    sct_signal<bool>    busy{"busy"};
    sct_signal<bool>    busy_d{"busy_d"};
    sct_signal<T>       a{"a"};
    sct_signal<bool>    a_enbl{"a_enbl"};
    sct_signal<T>       b{"b"};
    sct_signal<bool>    b_enbl{"b_enbl"};
    sct_signal<T>       c{"c"};
  
    void runThrd() {
        run.reset_get();
        a = 0; a_enbl = 0; 
        wait();
        
        while (true) {
            a_enbl = 0;
            if (run.request() && !busy) {
                a = run.get();
                a_enbl = 1;
            }
            wait();
        }
    }
    
    /// This thread activated by changed signals: @a_enbl, @c
    void compThrd() {
        T tmp;
        b = 0; b_enbl = 0;
        busy_d = 0;
        resp.reset_put();
        wait();
        
        while (true) {
            if (a_enbl) {
                assert (!busy_d);
                busy_d = 1;
                tmp = a;
                wait();     // @a_enbl: 1 -> 0
              
                b = tmp+1;
                b_enbl = 1;
                wait();     // @c: 0 -> some value
                
                b_enbl = 0;
                resp.b_put(c);
                busy_d = 0;
            }
            wait();         // @c: some value -> 0 or @a_enbl: 0 -> 1
        }
    }
    
    void compMeth() {
         busy = busy_d || a_enbl;
         c = b_enbl ? b.read()+1 : 0;
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
    
    A<16>        a{"a"};

    SC_HAS_PROCESS(simple_test);

    explicit simple_test(const sc_module_name& name) : sc_module(name)
    {
        cout << "sct_signal thread test" << endl;
        cout << "SCT_CMN_TLM_MODE " << SCT_CMN_TLM_MODE << endl;
        cout << "CLOCK " << SCT_CMN_TRAITS::CLOCK << endl;
        cout << "RESET " << SCT_CMN_TRAITS::RESET << endl;

        a.clk(clk);
        a.nrst(nrst);
        run.clk_nrst(clk, nrst);
        resp.clk_nrst(clk, nrst);
        
        a.run.bind(run);
        a.resp.bind(resp);
        
        SC_THREAD(init_thread);
        sensitive << run << resp;
        async_reset_signal_is(nrst, false);
    }
    
    void init_thread()
    {
        T data = 0;
        run.reset_put();
        resp.reset_get();
        wait();

        for (int i = 0; i < 3; i++) {
            run.b_put(i);
            cout << sc_time_stamp() << " " << sc_delta_count() << " : TB thread, PUT data = " << i << endl;
            wait();
            
            data = resp.b_get();
            cout << sc_time_stamp() << " " << sc_delta_count() << " : TB thread, data = " 
                 << data << " / " << (i+2) << endl;
            assert (data == i+2);
            wait();
        }
        
        cout << sc_delta_count() << " : TB thread, done " << endl;
        wait();

        while (true) {
            wait();
        }
    }
};

#endif /* SIMPLE_TEST_H */

