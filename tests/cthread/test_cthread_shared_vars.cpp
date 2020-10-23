#include <systemc>

using namespace sc_core;
using namespace sc_dt;

SC_MODULE(test_split_shared) {
    sc_signal<bool> clk;
    sc_signal<bool> rstn;

    SC_CTOR(test_split_shared) {
        SC_CTHREAD(thread_0, clk);
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(thread_1, clk);
        async_reset_signal_is(rstn, false);
    }

    sc_signal<int> a_sig;
    sc_signal<int> b_sig;
    sc_signal<int> c_sig;

    const int x = 12;
    int y;
    int z;
    int comb;

    void thread_0() {
        a_sig = 0;
        wait();
        while (1) {
            z = 0;
            comb = z;
            a_sig = 1 + comb;
            wait();
            a_sig = 2 + z;
            wait();
            a_sig = x;
        }

    }

    void thread_1() {
        b_sig = 0;
        c_sig = 0;
        wait();
        while (1) {
            c_sig = a_sig + c_sig;
            wait();
            b_sig = x;
            wait();
        }
    }

};


int sc_main(int argc, char* argv[])
{
    test_split_shared test{"test"};
    sc_start();
    return 0;
}
