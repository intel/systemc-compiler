//
// Created by ripopov on 3/28/18.
//

#include <systemc.h>
#include <typeinfo>


struct bundle : sc_modular_interface {

    sc_vector<sc_signal<int>>  **vecptr;

    SC_CTOR(bundle) {
        vecptr = sc_new_array< sc_vector<sc_signal<int>> *> (2);
        vecptr[0] = new sc_vector<sc_signal<int>>("vecptr0",2);
        vecptr[1] = new sc_vector<sc_signal<int>>("vecptr1",2);
    }

};

struct top : sc_module {

    bundle **vecarray;

    SC_CTOR(top) {
        vecarray = sc_new_array<bundle*>(2);
        vecarray[0] = new bundle("bundle0");
        vecarray[1] = new bundle("bundle1");
    }

};

int sc_main(int argc, char** argv)
{
    top top_inst{"top_inst"};
    sc_start();
    return 0;
}
