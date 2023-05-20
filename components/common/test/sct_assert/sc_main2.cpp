/******************************************************************************
 * Copyright (c) 2023, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 *
 *****************************************************************************/

#include "sct_common.h"
#include "systemc.h"

// Assertions and SVA generation for SingleSource channels test
template<class T>
struct A : public sc_module 
{
    sc_in<bool>         clk{"clk"};
    sc_in<bool>         nrst{"nrst"};

    sct_target<T>       run{"run"};
    sct_initiator<T>    resp{"resp"};
    sct_fifo<T, 2>      fifo{"fifo", 1};
    sct_register<T>     reg{"reg"};
    
    sct_in<T>           in{"in"};
    sct_signal<T>       ss{"ss"};
    sct_out<T>          out{"out"};
  
    SC_HAS_PROCESS(A);
    
    explicit A(const sc_module_name& name) : sc_module(name)
    {
        run.clk_nrst(clk, nrst);
        resp.clk_nrst(clk, nrst);
        fifo.clk_nrst(clk, nrst);
        reg.clk_nrst(clk, nrst);

        SC_METHOD(reqProc);
        sensitive << run << fifo.PUT << in << reg;

        SC_METHOD(respProc);
        sensitive << fifo.GET << resp << ss;
    }
    
    void reqProc() {
        ss = in.read(); 
        reg.reset();
        run.reset_get();
        fifo.reset_put();
        
        if (reg.read()) {
            if (fifo.ready()) {
                cout << sc_time_stamp() << " " << sc_delta_count()
                     << " : Put to fifo from reg " << reg.read() << endl;
                fifo.put(reg.read());
                reg.write(0);
            }
        } else
        if (run.request()) {
            if (fifo.ready()) {
                cout << sc_time_stamp() << " " << sc_delta_count()
                     << " : Put to fifo from input " << run.peek() << endl;
                fifo.put(run.get());
            } else {
                reg.write(run.get());
                cout << sc_time_stamp() << " " << sc_delta_count()
                     << " : Store to reg" << endl;
            }
        }
    }

    void respProc() {
        out = ss.read(); 
        fifo.reset_get();
        resp.reset_put();
        
        if (resp.ready() && fifo.request()) {
            resp.put(fifo.get());
        }
    }

    // Assertions
    SCT_ASSERT(run.request() && fifo.ready(), SCT_TIME(1), fifo.request(), clk.pos());
    SCT_ASSERT(!run.request() && fifo.ready(), SCT_TIME(1), fifo.ready(), clk.pos());
    SCT_ASSERT(run.request(), SCT_TIME(0,1), fifo.elem_num() > 0, clk.pos());

    SCT_ASSERT(run.request() && !reg.read() && !fifo.ready(), SCT_TIME(1), reg.read(), clk.pos());
    SCT_ASSERT(run.request() && reg.read(), SCT_TIME(1), run.request(), clk.pos());
    SCT_ASSERT(fifo.ready() && !reg.read(), SCT_TIME(1), !reg.read(), clk.pos());

    SCT_ASSERT(in.read(), SCT_TIME(0), out.read(), clk.pos());
};


class simple_test : public sc_module 
{
public:
    using T = sc_uint<16>;

    sc_in<bool>         clk{"clk"};
    sc_signal<bool>     nrst{"nrst"};

    sct_initiator<T>    run{"run"};
    sct_target<T>       resp{"resp"};
    
    sct_signal<T>       in_s{"in_s"};
    sct_signal<T>       out_s{"out_s"};

    A<T>        a{"a"};

    SC_HAS_PROCESS(simple_test);

    explicit simple_test(const sc_module_name& name) : sc_module(name)
    {
        cout << "assert_chan test" << endl;
        run.clk_nrst(clk, nrst);
        resp.clk_nrst(clk, nrst);
        a.clk(clk);
        a.nrst(nrst);
        a.run.bind(run);
        a.resp.bind(resp);
        
        a.in(in_s);
        a.out(out_s);
        
        SC_THREAD(reset_thread);
        
        SC_THREAD(init_thread);
        sensitive << run << resp;
        async_reset_signal_is(nrst, SCT_CMN_TRAITS::RESET);
    }

    void reset_thread() {
        nrst = 0;
        wait(sc_time(2, SC_NS));
        nrst = 1;
    }
    
    const unsigned N = 4;
    void init_thread()
    {
        T data; 
        run.reset_put();
        resp.reset_get();
        in_s = 0;
        wait();

        in_s = 42;
        while (!run.put(42)) wait();
        wait();
        while (!resp.get(data)) wait();
        assert (data == 42); wait();

        in_s = 43;
        while (!run.put(43)) wait();
        wait();
        in_s = 44;
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
        
        cout << endl;
        cout << "--------------------------------" << endl;
        cout << "|       Test passed OK         |" << endl;
        cout << "--------------------------------" << endl;
        sc_stop();
    }
};

int sc_main(int argc, char* argv[])
{
    simple_test st{"st"};
    sc_clock clk{"clock", 1, SC_NS};
    st.clk(clk);

    sc_start();
    return 0;
}
