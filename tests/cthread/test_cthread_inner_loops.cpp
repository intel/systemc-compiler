//
// Created by ripopov on 3/15/18.
//

#include <systemc.h>

class top : sc_module
{
public:
    sc_clock clk{"clk", 10, SC_NS};
    sc_signal<bool> arstn{"arstn", 1};
    sc_signal<int> out{"out"};
    sc_signal<int> in{"in"};
    sc_signal<bool> in_bool{"in_bool"};

    SC_HAS_PROCESS(top);
    top(sc_module_name)
    {
        SC_THREAD(test_thread1);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(test_thread2);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
    }

    
    void test_thread1()
    {
        wait();
        
        while (1) {
            for (size_t i = 0; i < 3; ++i) { // B8
                while (in.read() > 10) {     // B7 
                    wait();   // 2  // B6        
                }                   // B5
                wait();   // 3      // B4
            }                       // B3
        }
    }

    void test_thread2()
    {
        out = 0;
        wait();
        
        while (1) {
            for (size_t i = 0; i < 3; ++i) {
                while (in.read() > 10) {
                    out = 1;
                    if (in_bool.read()) {
                        wait();    // 1
                        out = 2;
                    }
                    wait();   // 2
                    out = 3;
                }
                wait();   // 3
                out = 4;
            }
            cout << "tick\n";
        }
    }

};

int sc_main(int argc, char *argv[])
{
    top top_inst{"top_inst"};
    sc_start(100, SC_NS);
    return 0;
}
