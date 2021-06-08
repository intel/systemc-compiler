/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by ripopov on 7/30/18.
//

#include <systemc.h>
#include <sct_assert.h>


SC_MODULE(top) {

    SC_CTOR(top) {
        SC_METHOD(test_ctors_sc_int);
        sensitive << din;
        SC_METHOD(test_ctors_sc_uint);
        sensitive << din;
        SC_METHOD(test_ctors_sc_bigint);
        sensitive << din;
        SC_METHOD(test_ctors_sc_biguint);
        sensitive << din;
        SC_METHOD(test_bigint_assign);
        sensitive << din;
    }

    sc_signal<int> din;

    void test_ctors_sc_int () {

        // sc_int()
        sc_int<42> x0{};

        // sc_int( int_type v )
        sc_int<42> x1{ sc_dt::int_type(1) };

        // sc_int( const sc_int<W>& a )
        sc_int<42> x2{ x1 };

        sc_int<5> tmp3 {3};
        // sc_int( const sc_int_base& a )
        sc_int<42> x3{ tmp3 };

        // TODO:
        // sc_int( const sc_int_subref_r& a )

        // TODO:
        // sc_int( const sc_generic_base<T>& a )

        // sc_int( const sc_signed& a )
        sc_bigint<42> tmp4{-4};
        sc_int<42> x4{ tmp4 };

        sc_biguint<42> tmp5{5};
        sc_int<42> x5{ tmp5 };

        cout << x0 << "\n";
        cout << x1 << "\n";
        cout << x2 << "\n";
        cout << x3 << "\n";
        cout << x4 << "\n";
        cout << x5 << "\n";

        sct_assert_const(x0 == 0);
        sct_assert_const(x1 == 1);
        sct_assert_const(x2 == 1);
        sct_assert_const(x3 == 3);
        sct_assert_const(x4 == -4);
        sct_assert_const(x5 == 5);

        cout << "test_ctors_sc_int\n";
    }

    void test_ctors_sc_uint () {

        // sc_int()
        sc_uint<42> x0{};

        // sc_uint( int_type v )
        sc_uint<42> x1{ sc_dt::int_type(-1) };

        // sc_uint( const sc_uint<W>& a )
        sc_uint<42> x2{ x1 };

        sc_uint<5> tmp3 {3};
        // sc_uint( const sc_uint_base& a )
        sc_uint<42> x3{ tmp3 };

        // TODO:
        // sc_uint( const sc_uint_subref_r& a )

        // TODO:
        // sc_uint( const sc_generic_base<T>& a )

        // sc_uint( const sc_signed& a )
        sc_bigint<42> tmp4{-4};
        sc_uint<42> x4{ tmp4 };

        sc_biguint<42> tmp5{5};
        sc_uint<42> x5{ tmp5 };

        cout << x0 << "\n";
        cout << x1 << "\n";
        cout << x2 << "\n";
        cout << x3 << "\n";
        cout << x4 << "\n";
        cout << x5 << "\n";

        sct_assert_const(x0 == 0);
        sct_assert_const(x1 == 4398046511103);
        sct_assert_const(x2 == 4398046511103);
        sct_assert_const(x3 == 3);
        sct_assert_const(x4 == 4398046511100);
        sct_assert_const(x5 == 5);
//
        cout << "test_ctors_sc_uint\n";

    }

    void test_ctors_sc_bigint() {
        sc_bigint<512> x0{};

        // sc_bigint( int_type v )
        sc_bigint<512> x1{ sc_dt::int_type(1) };

        // sc_bigint( const sc_bigint<W>& a )
        sc_bigint<512> x2{ x1 };

        sc_bigint<512> tmp3 {INT64_MAX};
        // sc_bigint( const sc_bigint_base& a )
        sc_bigint<512> x3{ tmp3 };

        // TODO:
        // sc_bigint( const sc_bigint_subref_r& a )

        // TODO:
        // sc_bigint( const sc_generic_base<T>& a )

        // sc_bigint( const sc_signed& a )
        sc_bigint<512> tmp4{-4};
        sc_bigint<512> x4{ tmp4 };

        sc_biguint<512> tmp5{5};
        sc_bigint<512> x5{ tmp5 };

        cout << x0 << "\n";
        cout << x1 << "\n";
        cout << x2 << "\n";
        cout << x3 << "\n";
        cout << x4 << "\n";
        cout << x5 << "\n";
//
//        sct_assert_const(x0 == 0);
//        sct_assert_const(x1 == 1);
//        sct_assert_const(x2 == 1);
//        sct_assert_const(x3 == INT64_MAX);
//        sct_assert_const(x4 == -4);
//        sct_assert_const(x5 == 5);

        cout << "test_ctors_sc_bigint\n";

    }

    void test_ctors_sc_biguint() {

        sc_biguint<512> x0{};

        // sc_biguint( int_type v )
        sc_biguint<512> x1{ sc_dt::int_type(1) };

        // sc_biguint( const sc_biguint<W>& a )
        sc_biguint<512> x2{ x1 };

        sc_bigint<512> tmp3 {-INT64_MAX};
        // sc_bigint( const sc_bigint_base& a )
        sc_biguint<512> x3{ tmp3 };

        // TODO:
        // sc_bigint( const sc_bigint_subref_r& a )

        // TODO:
        // sc_bigint( const sc_generic_base<T>& a )

        // sc_bigint( const sc_signed& a )
        sc_bigint<512> tmp4{-4};
        sc_biguint<512> x4{ tmp4 };

        sc_biguint<512> tmp5{5};
        sc_biguint<512> x5{ tmp5 };

        cout << x0 << "\n";
        cout << x1 << "\n";
        cout << x2 << "\n";
        cout << x3 << "\n";
        cout << x4 << "\n";
        cout << x5 << "\n";
//
//        sct_assert_const(x0 == 0);
//        sct_assert_const(x1 == 1);
//        sct_assert_const(x2 == 1);
////        sct_assert_const(x3 == INT64_MAX);
////        sct_assert_const(x4 == -4);
//        sct_assert_const(x5 == 5);

        cout << "test_ctors_sc_biguint\n";
    }


    void test_bigint_assign() {

        sc_int<3> x = -1;
        sc_biguint<512> bigux = -1;
        sc_bigint<512> bigx = 0;
        sc_uint<3> y;

        y = x;
        x = y;

        bigx = bigux;

        sct_assert_const(x == -1);
        sct_assert_const(y == 7);


        cout << "x " << x << endl;
        cout << "y " << y << endl;
        cout << "bigx " << bigx << endl;

    }

    };

int sc_main (int argc, char ** argv ) {

    top t_inst{"t_inst"};
    sc_start();

    return 0;
}
