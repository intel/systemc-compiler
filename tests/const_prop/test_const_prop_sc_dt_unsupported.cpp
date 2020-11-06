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
//        SC_METHOD(test_ctors_sc_int);
//        SC_METHOD(test_ctors_sc_uint);
//        SC_METHOD(test_ctors_sc_bigint);
        SC_METHOD(test_ctors_sc_biguint);
    }

    void test_ctors_sc_int () {

        // sc_int( const sc_bv_base& a )
        sc_int<42> x0{sc_bv<42>{"100"}};

        // sc_int( const sc_lv_base& a )
        sc_int<42> x1{sc_lv<42>{"101"}};

        // sc_int( const char* a )
        sc_int<42> x2{"102"};

        // sc_int( double a )
        sc_int<42> x3{103.001};

        cout << x0 << "\n";
        cout << x1 << "\n";
        cout << x2 << "\n";
        cout << x3 << "\n";
    }

    void test_ctors_sc_uint () {

        // sc_int( const sc_bv_base& a )
        sc_uint<42> x0{sc_bv<42>{"100"}};

        // sc_int( const sc_lv_base& a )
        sc_uint<42> x1{sc_lv<42>{"101"}};

        // sc_int( const char* a )
        sc_uint<42> x2{"102"};

        // sc_int( double a )
        sc_uint<42> x3{103.001};

        cout << x0 << "\n";
        cout << x1 << "\n";
        cout << x2 << "\n";
        cout << x3 << "\n";
    }

    void test_ctors_sc_bigint () {
        // sc_bigint( const char* v )
        sc_bigint<512> big0{"101"};

        // sc_bigint( double v )
        sc_bigint<512> big1{102.00003};

        // sc_bigint( const sc_bv_base& v )
        sc_bigint<512> big2{sc_bv<10>()};

        // sc_bigint( const sc_lv_base& v )
        sc_bigint<512> big3{sc_lv<3>("000")};

        cout << big0 << "\n";
        cout << big1 << "\n";
        cout << big2 << "\n";
        cout << big3 << "\n";
    }

    void test_ctors_sc_biguint () {

        // sc_bigint( const char* v )
        sc_biguint<512> big0{"101"};

        // sc_biguint( double v )
        sc_biguint<512> big1{102.00003};

        // sc_biguint( const sc_bv_base& v )
        sc_biguint<512> big2{sc_bv<10>()};

        // sc_biguint( const sc_lv_base& v )
        sc_biguint<512> big3{sc_lv<3>("000")};

        cout << big0 << "\n";
        cout << big1 << "\n";
        cout << big2 << "\n";
        cout << big3 << "\n";
    }


    };

int sc_main (int argc, char ** argv ) {

    top t_inst{"t_inst"};
    sc_start();

    return 0;
}
