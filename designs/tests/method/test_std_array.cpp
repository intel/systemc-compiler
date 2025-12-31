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
        SC_METHOD(local_array_test); sensitive << s;   
        SC_METHOD(local_array_oper_test); sensitive << s;   
        //SC_METHOD(local_vector_error_test); sensitive << s;   
        SC_METHOD(local_array_sig_test); sensitive << s;   
        for (int i = 0; i < 3; ++i) sensitive << sa[i];
        
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
    
    sc_signal<int> t3;
    void local_array_test() 
    {
        int n[3] = {1,2,3};
        std::array<int, 3> m;
        std::array<sc_uint<16>, 3> k;
        std::array<int, 3> p = {2,3,4};
        
        m[0] = 41;
        k[s.read()+1] = m[s.read()];
        m[s.read()] = 42 + p[1];
        t3 = m[0] + n[s.read()] + k[1] + p[s.read()-1];

        std::array<std::array<int, 2>, 3> mm;
        std::array<std::array<int, 2>, 3> pp = {{{1,2}, {3,4}, {5,6}}};

        mm[0][1] = s.read();
        pp[2][0] = s.read();
        
        t3 = mm[0][s.read()];
        t3 = mm[s.read()-1][1] + pp[s.read()][0];
    }
    
    sc_signal<int> t3a;
    void local_array_oper_test() 
    {
        std::array<int, 2> m;
        std::array<int, 2> k;
        std::array<std::array<int, 2>, 3> mm;
        std::array<std::array<int, 2>, 3> kk;
        
        m = k;
        mm = kk;
        mm[0] = m;
        t3a = m[0]+mm[0][0];
        
        bool lb = m[0] == k[1];
        //b = m == k;                       // Error reported
        t3a = lb ? m[0] : mm[0][0];
    }
    
    // std::vector is not supported in functions
    sc_signal<int> t4;
    void local_vector_error_test() 
    {
        std::vector<int> v = {1,2,3};       // Fatal error reported
        t4 = v[s.read()];
    }
    
    std::array<sc_signal<int>, 3> sa;
    std::array<sc_signal<int>, 3> sb;
    sc_signal<int> t5;
    void local_array_sig_test() 
    {
        sb[s.read()] = sa[1].read() + 1;
        t5 = sa[0];
    }
    
    
};


//const std::vector<int> test::b = {1, 2, 3};

int sc_main(int argc, char **argv) {
    test<base_test> t{"t"};
    sc_start();

    return 0;
}
