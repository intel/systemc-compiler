#include "single_test.h"
#include <systemc.h>

class Test_top : public sc_module
{
public:
    sc_clock clk{"clk", 1, SC_NS};
    sc_signal<bool>     nrst{"nrst"};

    simple_test dut{"dut"};

    SC_CTOR(Test_top) {
        dut.clk(clk);
        dut.nrst(nrst);

        SC_CTHREAD(testProc, clk);
    }

    void testProc() {
        nrst = 0;
        wait(2);
        nrst = 1;
        wait(1000);

        cout << endl;
        cout << "--------------------------------" << endl;
        cout << "|       Test passed OK         |" << endl;
        cout << "--------------------------------" << endl;
        sc_stop();
    }
};

int sc_main(int argc, char* argv[])
{
    Test_top test_top{"test_top"};
    sc_start();
    return 0;
}
