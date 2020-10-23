#include <systemc.h>

// SC_METHOD function declared in base class
template <unsigned U>
struct base : sc_module {
    sc_signal<bool> base_sig;

    SC_HAS_PROCESS(base);
    base(const sc_module_name &name) : sc_module(name)
    {}

    virtual void test_method() {
        bool b = base_sig.read();
    }
};

struct top : base<10> {
    sc_signal<bool> top_sig{"top_sig"};

    SC_HAS_PROCESS(top);
    top(const sc_module_name &name) : base<10>(name) 
    {
        SC_METHOD(test_method);
        this->sensitive << this->base_sig;

        SC_METHOD(test_method2);
        this->sensitive << this->base_sig;
    }

    void test_method2() {
        bool b = base_sig.read();
    }
};

int sc_main (int argc, char **argv) {
    cout << "test_base_scmethod\n";
    top t0{"t0"};
    sc_start();
    return 0;
}
