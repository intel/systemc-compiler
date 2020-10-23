//
// Created by ripopov on 1/29/18.
//

#include <systemc.h>
#include <array>

struct top : sc_module {

    std::array<int, 10> int_array {{1,2,3,4,5,6,7,8,9,10}};

    int *int_ptr = &int_array.at(0);
    std::array<int, 10> *std_array_ptr = &int_array;

    SC_CTOR(top) {
        for (size_t i = 0; i < 10; ++i) {
            cout << int_array[i] << endl;
        }
    }
};

int sc_main(int argc, char** argv)
{
    auto t0 = std::make_unique<top>("top_inst");
    sc_start();
    return 0;
}
