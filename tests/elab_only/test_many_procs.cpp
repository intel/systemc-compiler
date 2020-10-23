//
// Created by ripopov on 2/1/18.
//

#include <systemc.h>

struct mod_if : sc_modular_interface {
    SC_CTOR(mod_if) {
        SC_THREAD(test_thread);
        SC_METHOD(test_method);
    }

    void test_thread() {}
    void test_method() {}

};

SC_MODULE(sub) {
    SC_CTOR(sub) {
        SC_METHOD(test_method);
    }

    void test_method() {}
};

SC_MODULE(top) {

    sub s{"s"};

    sc_vector<mod_if> mod_ifs{"mod_ifs", 3};

    SC_CTOR(top) {
        SC_METHOD(test_method);
        SC_THREAD(test_thread);
    }

    void test_thread() {}
    void test_method() {}
    void test_cthread() {}

};

int sc_main(int argc, char **argv) {
    top t{"t"};
    sc_start();
    return 0;
}