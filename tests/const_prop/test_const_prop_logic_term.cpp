//
// Created by ripopov on 7/7/18.
//

#include <systemc.h>
#include <sct_assert.h>


SC_MODULE(top) {

    SC_CTOR(top) {
        SC_METHOD(test_method);
    }

    sc_signal<int> din;

    int dec(int &x1) {
        return x1--;
    }


    void test_method () {
        int x = 1;

//        x = dec(x) + 1;
//
        int y = dec(x) && dec(x) && dec(x) && dec(x);

        sct_assert_const(x == -1);
        sct_assert_const(y == 0);

        x = (y++ == 0) && y;

        sct_assert_const(x == 1);
        sct_assert_const(y == 1);

        int z = dec(y) || dec(x) || dec(x);

        sct_assert_const(x == 1);
        sct_assert_const(y == 0);
        sct_assert_const(z == 1);

    }

};

int sc_main (int argc, char ** argv ) {

    top t_inst{"t_inst"};
    sc_start();

    return 0;
}