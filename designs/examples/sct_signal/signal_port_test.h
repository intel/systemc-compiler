/*
 * Test for @sct_in/@sct_out
 */

#ifndef SIMPLE_TEST_H
#define SIMPLE_TEST_H

#include "sct_common.h"
#include <systemc.h>


template<unsigned N>
struct A : public sc_module 
{
    using T = sc_uint<N>;
    sc_in<bool>         clk{"clk"};
    sc_in<bool>         nrst{"nrst"};

    sct_in<T>           run_port{"run_port"};
    sct_out<T>          resp_port{"resp_port"};
    sct_signal<T>       s{"s"};
    
    
    SC_HAS_PROCESS(A);
    
    explicit A(const sc_module_name& name) : sc_module(name)
    {
        SC_METHOD(methProc);
        sensitive << run_port << s;

        SCT_THREAD(thrdProc, clk, nrst);
        sensitive << run_port << resp_port;
        async_reset_signal_is(nrst, false);
    }
    
    void methProc() {
        // All assignment forms supported
        resp_port.write(run_port.read() + s.read());
        resp_port.write(run_port);
        resp_port = run_port.read();
        resp_port = run_port;
    }
    
    void thrdProc() {
        s = 0;
        wait();
        
        while (true) {
            s = run_port.read() + resp_port.read();
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

    sct_signal<T>       run{"run"};
    sct_signal<T>       resp{"resp"};
    sct_fifo<T, 4>      fifo{"fifo"};
    
    A<16>               a{"a"};

    SC_HAS_PROCESS(simple_test);

    explicit simple_test(const sc_module_name& name) : sc_module(name)
    {
        cout << "sct_signal port test, MODE = " << SCT_CMN_TLM_MODE << endl;
        a.clk(clk);
        a.nrst(nrst);
        fifo.clk_nrst(clk, nrst);
        
        a.run_port(run);
        a.resp_port(resp);
        
        SC_THREAD(init_thread);
        sensitive << fifo << resp;
        async_reset_signal_is(nrst, false);
    }
    
    void init_thread()
    {
        T data;
        run.write(0);
        fifo.reset();
        wait();

        for (int i = 1; i < 4; i++) {
            run.write(i);
            fifo.put(i);
            cout << sc_time_stamp() << " " << sc_delta_count() << " : TB thread, PUT data = " << i << endl;
            wait();
            
            data = resp.read();
            fifo.get();
            cout << sc_time_stamp() << " " << sc_delta_count() << " : TB thread, data = " 
                 << data << " / " << i << endl;
            assert (data == i);
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

