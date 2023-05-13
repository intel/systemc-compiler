/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>
#include <array>

// Standard C++ template library collections: pair, array
struct MyPair {
    int x;
    int y;
};

struct PairPair {
    MyPair a;
    MyPair b;
};

template <class T, unsigned N>
struct A : public sc_module {
    SC_HAS_PROCESS(A);
    
    sc_signal<bool> s;
    const std::array<T, N> arr;
    const std::vector<T> vec;
    
    explicit A(const sc_module_name& name, 
               const std::array<T, N>& arr_, const std::vector<T>& vec_) : 
        sc_module(name), arr(arr_), vec(vec_)  
    {
        SC_METHOD(testMeth); sensitive << s;
    }
    
    void testMeth() {
        T i = arr[0];
        i = vec[i];
    }
};

SC_MODULE(test) {

    sc_signal<bool> s;
    
    A<unsigned, 2> a{"a", {1, 2}, {3, 4}};

    SC_CTOR(test) 
    {
        for (int i = 0; i < 10; i++) {
            intArray[i] = 1+i;
        }
        
        SC_METHOD(pair_test); sensitive << s;

        SC_METHOD(array_test); sensitive << s;

        // Not supported for generation yet
        //SC_METHOD(tuple_method);
        //sensitive << dummy;
    }

// ----------------------------------------------------------------------------
    std::pair<int, sc_int<10>> intPair;
    std::pair<int, sc_int<10>> intPairArray[2];
    const std::pair<int, sc_int<10>> intPair1 = {1, 2};
    //static constexpr std::pair<int, int> intPair2 = {1, 2};

    MyPair mpa[2];
    PairPair ppa[2];

    void pair_test() 
    {
        int i = intPair1.first + intPair1.second;
        intPair.first = 12;
        intPair.second = 13;

        intPairArray[1].second = 1;
        auto l = intPairArray[1].second;
        auto k = intPairArray[l].second;

        mpa[0].y = mpa[0].x;
        mpa[1].y = mpa[1].x;

        ppa[0].a.x = 1;
        ppa[1].b.y = 0;
        int idx = s;
        intPairArray[idx].first = 1;

        int idx1 = s;
        int idx2 = s;
        mpa[idx1].x = idx2;
        
        ppa[1].a.x = 1;
        idx2 = ppa[1].a.x + ppa[1].b.y;
    }
    
// ----------------------------------------------------------------------------
    std::array<int, 10> intArray;
    
    void array_test() 
    {
        int l = intArray[0];
    }
    
// ----------------------------------------------------------------------------
    std::tuple<int, int, int> intTuple;

    void tuple_method() {
        // Not supported for generation yet
        std::get<0>(intTuple) = 12;
    }

};

int sc_main(int argc, char **argv) {
    test t{"t"};
    sc_start();

    return 0;
}
