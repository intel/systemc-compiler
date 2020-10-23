//
// Created by ripopov on 3/13/18.
//

#include <systemc.h>

class top : sc_module
{
public:
    sc_clock clk{"clk", 10, SC_NS};
    sc_signal<bool> arstn{"arstn", 1};
    sc_signal<int> in{"in"};
    sc_signal<int> out{"out"};

    SC_HAS_PROCESS(top);
    top(sc_module_name)
    {
        //SC_THREAD(test_thread);
        //SC_THREAD(test_simple_thread);
        SC_THREAD(test_simple_thread2);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
    }

    void test_simple_thread()
    {
        int i = 0;
        out = 0;
        wait();
        
        while (1) {
            out = 1;
            wait();
        }
    }

    void test_simple_thread2()
    {
        int i = 0;
        out = 0;
        wait();
        
        while (1) {
            out = 1+i;
            wait();
            out = 2;
            i++;
            wait();
        }
    }
    
    int func (int x) {
        return x + in.read();
    }

    void wait_wrapper() {
        wait();
    }

    void test_thread()
    {
        out = 0;
        wait();
        while (1) {

            out = 1;

            wait();

            out.write(2 + in.read());

            wait_wrapper();

            out.write(2 + in.read() + out.read());

            wait();

            int x = in.read();
            out = 3 + x;

            wait();

            if (in.read())
                out = 1;
            else
                out = 2;

            wait();

            for (size_t i = 0; i < 2; ++i) {
                out = in.read() + i;
            }

            wait();

            out = func(in.read());

            wait();

            if (in.read() == out.read())
                wait();

            wait();
        }
    }
};

int sc_main(int argc, char *argv[])
{
    top top_inst{"top_inst"};
    sc_start(100, SC_NS);
    return 0;
}
