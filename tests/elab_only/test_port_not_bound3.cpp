//
// Created by ripopov on 02/08/18.
//

#include <systemc.h>

// Not connected port -- test will FAIL
struct top : sc_module {
    sc_signal<int> sig{"sig"};
    sc_in<int>* in;

    SC_CTOR(top) 
    {
        in = new sc_in<int>("in");
    }
};

int sc_main(int argc, char** argv)
{
    cout << "test virtual ports\n";
    top t0("t0");
    
    sc_start();
    return 0;
}
