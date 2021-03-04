/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>
#include <sct_assert.h>

// Member static constant and template parameter arrays
template <unsigned DOMAIN_NUM>
struct DOMAIN_PARAMETERS {
    static const unsigned INT_VALS[DOMAIN_NUM];
    static const int64_t DOMAIN_VALS[DOMAIN_NUM];
};

template <>
const unsigned DOMAIN_PARAMETERS<4>::INT_VALS[4] = {1, 2, 3, 4};
template <>
const int64_t DOMAIN_PARAMETERS<4>::DOMAIN_VALS[4] = {- 1, -2, -3, -4};


template <typename PARAMS>
SC_MODULE(top) {

    // localparam [31:0] INT_2D[2][2] = {{0, 1}, {2, 6}};
    static constexpr int INT_2D[2][2] = {{0, 1}, {2, 6}};

    static constexpr int INT_VALS[4] = {42, 43, 44, 45};

    sc_signal<bool> dummy{"dummy"};

    SC_CTOR(top) {
        SC_METHOD(test_method); sensitive << dummy;
    }

    void test_method () {
        int i;
        int b = PARAMS::DOMAIN_VALS[i];

        sct_assert_const(INT_VALS[1] == 43);
        sct_assert_const(PARAMS::INT_VALS[1] == 2);

        int x = PARAMS::INT_VALS[0] - 1;
        int y = 0;
        int z = 0;

        for (int i = 0; i < 4; ++i) {
            x += INT_VALS[i];
            y += PARAMS::INT_VALS[i];
            z += INT_2D[0x1 & (i>>1)][0x1 & (i>>1)];
        }

        cout << x << endl;
        cout << y << endl;
        cout << z << endl;

        sct_assert_const(x == 174);
        sct_assert_const(y == 10);
        sct_assert_const(z == 12);


        cout << "done\n";
    }

};

template<typename PARAMS>
constexpr int top<PARAMS>::INT_VALS[4];

template<typename PARAMS>
constexpr int top<PARAMS>::INT_2D[2][2];

int sc_main(int argc, char **argv) {

    top<DOMAIN_PARAMETERS<4>> top_inst{"top_inst"};

    sc_start();

    return 0;
}
