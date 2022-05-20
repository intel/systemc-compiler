/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Constexpr variables and constexpr functions

// Logarithm based on constexr
constexpr unsigned myLogFunc(uint64_t n, unsigned p) noexcept {
    return (n < 2 ? p : myLogFunc(n/2, p+1));
}
template<uint64_t n> constexpr unsigned myLog = myLogFunc(n-1, 0)+1;

constexpr unsigned myLogFunc2(uint64_t n) noexcept {
    unsigned p = 0;
    while (n > 1) {
        n = n / 2;
        p++;
    }
    return p;
}
template<uint64_t n> constexpr unsigned myLog2 = myLogFunc2(n-1)+1;

// Get array size
template<class T, unsigned N>
constexpr std::size_t getArrSize(const T (&) [N]) noexcept {
    return N;
}


template <unsigned N>
struct A : public sc_module
{
    constexpr static unsigned M1 = myLog<N>;
    constexpr static unsigned M2 = 42;

    sc_in_clk       clk;
    sc_signal<bool>         rst;
    sc_signal<unsigned>     s;
    
    SC_CTOR(A) 
    {
        SC_METHOD(constProc); sensitive << s;
        SC_METHOD(arrProc); sensitive << s;
        SC_METHOD(logProc); sensitive << s;
    }
    
    void constProc() {
        cout << "M1 " << M1 << " M2 " << M2 << endl;
        unsigned i1 = M1;
        unsigned i2 = M2;
    }

    void arrProc() 
    {
        int arr1[12];
        constexpr auto i = getArrSize(arr1);
        cout << i << endl;
        int arr2[i];
        sc_uint<i> a = arr2[0];
    }
    
    void logProc() 
    {
        constexpr unsigned i = 42;
        constexpr uint64_t A = 1ULL << 63;
        
        auto j = myLog<i>;
        cout << "Log of " << i << " is " << j << endl;
        j = myLog<121>;
        cout << "Log of " << 121 << " is " << j << endl;
        j = myLog<7148>;
        cout << "Log of " << 7148 << " is " << j << endl;
        j = myLog<3122741901>;
        cout << "Log of " << 3122741901 << " is " << j << endl;
        j = myLog<A>;
        cout << "Log of " << A << " is " << j << endl;
        
        j = myLog2<i>;
        cout << "Log of " << i << " is " << j << endl;
        j = myLog2<121>;
        cout << "Log of " << 121 << " is " << j << endl;
        j = myLog2<7148>;
        cout << "Log of " << 7148 << " is " << j << endl;
        j = myLog2<3122741901>;
        cout << "Log of " << 3122741901 << " is " << j << endl;
        j = myLog2<A>;
        cout << "Log of " << A << " is " << j << endl;
    }
};

int sc_main(int argc, char **argv) {

    sc_clock clk{"clk", 1, SC_NS};
    A<11> a_mod{"a_mod"};
    a_mod.clk(clk);
    sc_start();

    return 0;
}


