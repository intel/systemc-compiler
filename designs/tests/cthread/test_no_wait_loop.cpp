#include "systemc.h"

struct A : sc_module {
    sc_in_clk clk;
    sc_signal<bool> nrst;
    sc_signal<bool> condition;
    sc_signal<unsigned> result;

    SC_CTOR(A) {
#ifdef NO_WAIT_LOOP_FAIL
        SC_THREAD(continueLoop);
        sensitive << clk.pos();
        async_reset_signal_is(nrst, false);

        SC_THREAD(nestedBreakLoop);
        sensitive << clk.pos();
        async_reset_signal_is(nrst, false);

        SC_THREAD(deadBreakLoop);
        sensitive << clk.pos();
        async_reset_signal_is(nrst, false);

        SC_CTHREAD(no_wait_main_loop, clk.pos());
        async_reset_signal_is(nrst, false);
    
#else
        SC_THREAD(finiteLoops);
        sensitive << clk.pos();
        async_reset_signal_is(nrst, false);
#endif
    }

            
    // Main loop w/o wait(), see #133
    sc_signal<int> s;
    void no_wait_main_loop()
    {
        s = 0;
        wait();
        
        while (true) { 
            s = 1;
        } 
    }

    void pause() { wait(); }

    void finiteLoops() {
        result = 0u;
        wait();
        while (true) {
            unsigned count = 3u;
            while (count != 0u) {
                --count;
                result = count;
            }
            for (unsigned slot = 0; slot < 3u; ++slot) {
                result = slot;
            }
            count = condition.read() ? 3u : 0u;
            while (count != 0u && condition.read()) {
                --count;
            }
            count = condition.read() ? 3u : 0u;
            const unsigned& count_ref = count;
            while (count_ref != 0u && condition.read()) {
                --count;
            }
            while (true) {
                result = 3u;
                break;
            }
            while (false) {
                result = 4u;
            }
            while (condition.read() && false) {
                result = 5u;
            }
            while (true) {
                pause();
                break;
            }
            wait();
        }
    }

    void continueLoop() {
        wait();
        while (true) {
            while (true) {
                continue;
            }
            wait();
        }
    }

    void nestedBreakLoop() {
        wait();
        while (true) {
            while (true) {
                for (unsigned slot = 0; slot < 1u; ++slot) {
                    break;
                }
            }
            wait();
        }
    }

    void deadBreakLoop() {
        wait();
        while (true) {
            while (true) {
                if (false) break;
            }
            wait();
        }
    }
};

int sc_main(int argc, char* argv[]) {
    sc_clock clk("clk", 1, SC_NS);
    A a_mod("a_mod");
    a_mod.clk(clk);
    sc_start();
    return 0;
}