//
// Created by ripopov on 2/16/18.
//

#include <systemc.h>

SC_MODULE(top) {

    sc_signal <int> int_sig{"int_sig"};
    sc_signal <int> &sig_ref = int_sig;
    const int & int_ref = int_sig.read();

    SC_CTOR(top) {
    }

};

int sc_main(int argc, char **argv) {
    top t{"t"};
    sc_start();
    return 0;
}
