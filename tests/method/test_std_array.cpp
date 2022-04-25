/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>
#include <array>

// C++ std::array

struct base_test
{
    static constexpr int data[3] = {1,2,3};
    typedef std::array<int, 3> RInfo_t;
    static constexpr RInfo_t fillArray() 
    {
        RInfo_t tmp{1, 2, 3};
        return tmp;
    }
};

template<class T>
struct test : public sc_module, public base_test {

    sc_signal<unsigned> s;
    
    SC_CTOR(test) 
    {
        SC_METHOD(const_array_test); sensitive << s;
        SC_METHOD(array_test); sensitive << s;
        SC_METHOD(static_const_array_test); sensitive << s;
        //SC_METHOD(local_array_test); sensitive << s;   // see #
    }

// ----------------------------------------------------------------------------    
// Constant member std::array and std::vector
    const sc_uint<3> c[3] = {1, 2, 3};
    const std::array<int, 3> a = {2, 3, 4};
    const std::vector<int> b = {5, 6, 7};
    const std::vector<sc_int<12>> bx = {8, -9, 10};

    void const_array_test() 
    {
        int l;
        unsigned i = s.read();

        l = a[1];
        l = b[2];
        l = c[0];
        
        l = a[i] + a[i+1] + a[s.read()] + a[c[i]];
        l = b[i] + b[i+1] + b[s.read()] + b[c[i]];
        
        l = bx[0];
        l = 1 - bx[i-1];
    }
    
    
// ----------------------------------------------------------------------------    
// Non-constant member std::array
    std::array<int, 3> d;
    std::array<std::array<int, 2>, 3> e;
    int f[3];
    
    void array_test() 
    {
        int l;
        unsigned i = s.read();

        f[0] = 41;
        d[0] = 42;
        d[1] = c[i];

        l = f[0];
        l = d[0];
        l = d[1] ? d[f[1]] : e[s.read()+1][1];
        
        if (e[2][l]) {
            e[0][1] = d[i];
        }
    }
    
// ----------------------------------------------------------------------------
// Static constant std::array and std::vector 

    typedef std::array<int, 3> RInfo_t;
    static constexpr RInfo_t fillArrayLoc() 
    {
        RInfo_t tmp{1, 2, 3};
//        for (unsigned i = 0; i < 3; ++i) {
//            tmp[i] = T::data[i];
//        }
        return tmp;
    }
    
    static constexpr RInfo_t r = fillArray();
    static constexpr RInfo_t p = fillArrayLoc();
    static constexpr RInfo_t q = T::fillArray();

    static constexpr std::array<int, 3> g = {1, -2, 3};
    static constexpr std::array<unsigned long, 3> g1 = {4, 5, 6};
    static constexpr int h[3] = {1, 2, 3};

    void static_const_array_test() {
        int l;
        unsigned i = s.read();
        
        l = r[0] + p[0] + q[0];
        l = g[0] + g1[1];
        l = h[0];
        l = r[0] + q[1];
        
        l = r[i] / q[i-1];
        l = g[r[1]] + g1[q[i]];
    }
    
// ----------------------------------------------------------------------------
// Local std::array  
    
//    void local_array_test() 
//    {
//        std::array<int, 3> m;
//        int l;
//        unsigned i = s.read();
//
//        m[0] = 41;
//
//        l = m[0];
//    }
//    
    
};


//const std::vector<int> test::b = {1, 2, 3};

int sc_main(int argc, char **argv) {
    test<base_test> t{"t"};
    sc_start();

    return 0;
}
