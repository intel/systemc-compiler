//
// Created by ripopov on 10/10/18.
//

#include <systemc.h>

struct top : sc_module {

    sc_in <bool>     in{"in"};
    sc_signal <bool> sig{"sig"};


    SC_CTOR(top) {
        SC_THREAD(test_thread);
    }

protected:

    void before_end_of_elaboration() override
    {
        in(sig);
        cout << "before end of elaboration\n";
    }

    void test_thread() {
        cout << "Test thread\n";
        while (1) {wait();}
    }

};

int sc_main(int argc, char **argv) {

    top tinst{"tinst"};
    sc_start();

    return 0;
}