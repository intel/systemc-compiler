#ifdef SIGNAL
    #include "signal_thrd_test.h"
#elif PORT
    #include "signal_port_test.h"
#endif   
#include <systemc.h>

class Test_top : public sc_module
{
public:
    sc_in_clk           clk{"clk"};
    sc_signal<bool>     nrst{"nrst"};

    simple_test dut{"dut"};

    SC_CTOR(Test_top) {
        dut.clk(clk);
        dut.nrst(nrst);

        SC_CTHREAD(testProc, clk.pos());
    }

    void testProc() {
        cout << sc_time_stamp() << " " << sc_delta_count() << " 1st reset" << endl;
        nrst = 0;
        wait(10);
        nrst = 1;
        wait(100);
        
        cout << sc_time_stamp() << " " << sc_delta_count() << " 2nd reset" << endl;
        nrst = 0;
        wait(10);
        nrst = 1;
        wait(100);

        cout << endl;
        cout << "--------------------------------" << endl;
        cout << "|       Test passed OK         |" << endl;
        cout << "--------------------------------" << endl;
        sc_stop();
    }
};

int sc_main(int argc, char* argv[])
{
    sc_clock clk{"clk", 1, SC_NS};
    Test_top test_top{"test_top"};
    test_top.clk(clk);
    sc_start();
    return 0;
}
