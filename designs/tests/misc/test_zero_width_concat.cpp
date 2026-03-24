/******************************************************************************
* Copyright (c) 2025, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_sel_type.h"
#include "systemc.h"

using namespace sct;

// SC type zero width and zero width signals in concatenation and type cast
struct A : public sc_module 
{
    static const unsigned DATA_WIDTH  = 0u;
    typedef sct_uint<DATA_WIDTH> Data_t;
    sc_signal<Data_t>       SC_NAMED(zs);
    sc_signal<sc_uint<8>>   SC_NAMED(r);
    
    A(const sc_module_name& name) : sc_module(name) {
        SC_METHOD(concatProc); sensitive << r << zs;
        SC_METHOD(typeCastProc); sensitive << r << zs;
        SC_METHOD(funcCallProc); sensitive << r << zs;
        SC_METHOD(assignZeroProc); sensitive << r << zs;
    }   

    sc_signal<sc_uint<64>> t0{"t0"};
    void concatProc() {
    	sc_uint<64> l;
        sc_uint<8> u = r.read();
        sct_uint<0> z;
        l = (u, zs.read());
        l = (u, zs);
        l = (u, zs, u);
        l = (zs, u, zs);
        l = (zs, zs);
        l = ((zs, zs), zs);

        l = ((zs, zs), u);
        l = (u, z);
        l = (u, zs.read(), r);
        l = (u, zs, r);
        l = (u, z, r);

        l = (u, sc_uint<4>(z));             // 4'd0 that is OK
        l = (u, sc_uint<4>(zs.read()));     // 4'd0 that is OK
        l = (u, int(z));                    // 1'd0 that is OK
        t0 = l;
    }
    
    sc_signal<sc_uint<64>> t1{"t1"};
    void typeCastProc() {
    	sc_uint<64> l;
        sc_uint<8> u = r.read();
        sct_uint<0> z;

        l = z;
        l = zs.read();
        l = sc_uint<32>(z);
        l = sc_uint<64>(z);
        l = sc_uint<32>(zs.read());
        l = sc_uint<64>(int(z));
        l = sc_uint<64>(sc_uint<4>(z));
        l = sc_uint<64>(sc_uint<4>(zs.read()));
        l = sc_uint<64>(u + z);
        l = sc_uint<64>(u + zs.read());
        l = sc_uint<64>(u + sc_uint<4>(z));
        l = sc_uint<64>(zs.read() + z);
        l = sc_uint<64>(zs.read() + sc_uint<4>(z));
        l = sc_uint<64>(zs.read() + int(z));

        t1 = l;
    }

    template<class T>
    sct_uint<32> f(const T& par) {
        sct_uint<32> result;
        result = par;
        return result;
    }

    sc_signal<sc_uint<64>> t2{"t2"};
    void funcCallProc() {
    	sc_uint<64> l;
        sc_uint<8> u = r.read();
        sct_uint<0> z;

        l = f(u);
        l = f(z);
        l = f(zs.read());
        //auto& i = (z, u); l = i;    Error reported for concat reference
        //l = f((z, u));
        //l = f((u, zs));
        //l = f((zs, u, zs.read()));

        t2 = l;
    }

    sc_signal<sc_uint<64>> t3{"t3"};
    void assignZeroProc() {
    	sct_uint<0> l;
        sc_uint<8> u;
        sct_uint<0> z;
       
        l = z;
        l = zs.read();
        l = zs;
        l = (u, zs);
        (l, u) = r.read();
        (u, l) = (r, zs, u, l);

        l += u;
        u -= zs.read();
        u *= (zs, r);

        u = l ? r.read() : sc_uint<8>(0);
        u = l == 0 ? r.read() : sc_uint<8>(0);

        t3 = l.to_uint();
        t3 = (u, l, zs);
    }
};

int sc_main(int argc, char **argv) 
{
    A a_mod{"a_mod"};
    sc_start();

    return 0;
}


