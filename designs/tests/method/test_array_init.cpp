/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"

using namespace sc_core;

// Local/member array initialization including "{}"
class A : public sc_module {
public:
    sc_in_clk           clk;
    sc_signal<bool>     rst;
    sc_signal<int>      s;
    
    SC_CTOR(A) 
    {
        SC_METHOD(init_3D_array_local); sensitive<< s;
        
        SC_METHOD(no_init_2D_array_local); sensitive<< s;
        SC_METHOD(part_init_2D_array_local); sensitive<< s;
        SC_METHOD(full_init_2D_array_local); sensitive<< s;
        SC_METHOD(sc_no_init); sensitive<< s;
        SC_METHOD(no_init_local1); sensitive<< s;
        SC_METHOD(part_init_local1); sensitive<< s;
        SC_METHOD(no_init_member1); sensitive<< s;
        SC_METHOD(part_init_member1); sensitive<< s;
    }
    
    void init_3D_array_local() 
    {
        sc_uint<4>  a[2][3][2];
        sct_assert_const(a[0][0][0] == 0);
        sct_assert_const(a[1][1][1] == 0);
        sct_assert_const(a[1][2][1] == 0);

        int b[2][3][2] = {};
        sct_assert_const(b[0][0][0] == 0);
        sct_assert_const(b[1][0][1] == 0);
        sct_assert_const(b[1][2][1] == 0);

        bool c[2][3][2] = {true, false, true, false, true, true};
        sct_assert_const(c[0][0][0] == 1);
        sct_assert_const(c[0][0][1] == 0);
        sct_assert_const(c[0][1][0] == 1);
        sct_assert_const(c[0][1][1] == 0);
        sct_assert_const(c[0][2][0] == 1);
        sct_assert_const(c[0][2][1] == 1);

        long d[2][3][2] = {{{1, 2}, {-3, 4}, {5, 6}}};
        sct_assert_const(d[0][0][0] == 1);
        sct_assert_const(d[0][0][1] == 2);
        sct_assert_const(d[0][1][0] == -3);
        sct_assert_const(d[0][1][1] == 4);
        sct_assert_const(d[0][2][0] == 5);
        sct_assert_const(d[0][2][1] == 6);
    }
 
    void no_init_2D_array_local() 
    {
        sc_uint<4>  a[3][2];
        sct_assert_const(a[0][0] == 0);
        sct_assert_const(a[0][1] == 0);
        sct_assert_const(a[1][0] == 0);
        sct_assert_const(a[1][1] == 0);
        sct_assert_const(a[2][0] == 0);
        sct_assert_const(a[2][1] == 0);
        
        sc_int<4>  b[3][2] = {};
        sct_assert_const(b[0][0] == 0);
        sct_assert_const(b[0][1] == 0);
        sct_assert_const(b[1][0] == 0);
        sct_assert_const(b[1][1] == 0);
        sct_assert_const(b[2][0] == 0);
        sct_assert_const(b[2][1] == 0);

        bool  c[3][2] = {};
        sct_assert_const(c[0][0] == 0);
        sct_assert_const(c[0][1] == 0);
        sct_assert_const(c[1][0] == 0);
        sct_assert_const(c[1][1] == 0);
        sct_assert_const(c[2][0] == 0);
        sct_assert_const(c[2][1] == 0);

        int d[3][2] = {};
        sct_assert_const(d[0][0] == 0);
        sct_assert_const(d[0][1] == 0);
        sct_assert_const(d[1][0] == 0);
        sct_assert_const(d[1][1] == 0);
        sct_assert_const(d[2][0] == 0);
        sct_assert_const(d[2][1] == 0);
    }
    
    void part_init_2D_array_local() 
    {
        // No partial initialization of sub-array
        //const sc_uint<4>  b[3][2] = {{1}, {2, 3}}; -- error reported
        //const sc_uint<4>  b[3][2] = {1, 2, 3}; -- error reported
        
        sc_uint<4>  b[3][2] = {1, 2, 3, 4};
        sct_assert_const(b[0][0] == 1);
        sct_assert_const(b[0][1] == 2);
        sct_assert_const(b[1][0] == 3);
        sct_assert_const(b[1][1] == 4);
        sct_assert_const(b[2][0] == 0);
        sct_assert_const(b[2][1] == 0);

        bool  c[3][2] = {true, 0};
        sct_assert_const(c[0][0] == 1);
        sct_assert_const(c[0][1] == 0);
        sct_assert_const(c[1][0] == 0);
        sct_assert_const(c[1][1] == 0);
        sct_assert_const(c[2][0] == 0);
        sct_assert_const(c[2][1] == 0);

        int d[3][2] = {{1, 2}, {-3, 4}};
        sct_assert_const(d[0][0] == 1);
        sct_assert_const(d[0][1] == 2);
        sct_assert_const(d[1][0] == -3);
        sct_assert_const(d[1][1] == 4);
        sct_assert_const(d[2][0] == 0);
        sct_assert_const(d[2][1] == 0);
    }
    
     void full_init_2D_array_local() 
     {
        sc_int<4>  a[3][2] = {1,2, 3,4, -5,6};
        sct_assert_const(a[0][0] == 1);
        sct_assert_const(a[0][1] == 2);
        sct_assert_const(a[1][0] == 3);
        sct_assert_const(a[1][1] == 4);
        sct_assert_const(a[2][0] == -5);
        sct_assert_const(a[2][1] == 6);

        sc_uint<4>  b[2][3] = {{1,2,3}, {4,5,6}};
        sct_assert_const(b[0][0] == 1);
        sct_assert_const(b[0][1] == 2);
        sct_assert_const(b[0][2] == 3);
        sct_assert_const(b[1][0] == 4);
        sct_assert_const(b[1][1] == 5);
        sct_assert_const(b[1][2] == 6);

        bool  c[3][2] = {1,0, 1,0, 0,1};
        sct_assert_const(c[0][0] == 1);
        sct_assert_const(c[0][1] == 0);
        sct_assert_const(c[1][0] == 1);
        sct_assert_const(c[1][1] == 0);
        sct_assert_const(c[2][0] == 0);
        sct_assert_const(c[2][1] == 1);

        int  d[3][2] = {{1,2}, {3,4}, {-5,6}};
        sct_assert_const(d[0][0] == 1);
        sct_assert_const(d[0][1] == 2);
        sct_assert_const(d[1][0] == 3);
        sct_assert_const(d[1][1] == 4);
        sct_assert_const(d[2][0] == -5);
        sct_assert_const(d[2][1] == 6);
    }
    
    void sc_no_init() {
        sc_uint<4>  c[3];
        sct_assert_const(c[0] == 0);
        sct_assert_const(c[1] == 0);
        sct_assert_const(c[2] == 0);
    }
    
    void no_init_local1() 
    {
        bool            a[3] = {};
        sct_assert_const(a[0] == false);
        sct_assert_const(a[1] == false);
        sct_assert_const(a[2] == false);

        int             b[3] = {};
        sct_assert_const(b[0] == 0);
        sct_assert_const(b[1] == 0);
        sct_assert_const(b[2] == 0);

        sc_uint<4>      c[3] = {};
        sct_assert_const(c[0] == 0);
        sct_assert_const(c[1] == 0);
        sct_assert_const(c[2] == 0);
        
        sc_bigint<4>    d[3] = {};
        sct_assert_const(d[0] == 0);
        sct_assert_const(d[1] == 0);
        sct_assert_const(d[2] == 0);

        //sc_bigint<4>    e[2][3] = {}; -- not supported yet
    }

    void part_init_local1() {
        bool            a[3] = {true};
        sct_assert_const(a[0] == true);
        sct_assert_const(a[1] == false);
        sct_assert_const(a[2] == false);

        int             b[3] = {1,2};
        sct_assert_const(b[0] == 1);
        sct_assert_const(b[1] == 2);
        sct_assert_const(b[2] == 0);

        sc_uint<4>      c[3] = {3};
        sct_assert_const(c[0] == 3);
        sct_assert_const(c[1] == 0);
        sct_assert_const(c[2] == 0);
        
        sc_bigint<4>    d[3] = {4,-5};
        sct_assert_const(d[0] == 4);
        sct_assert_const(d[1] == -5);
        sct_assert_const(d[2] == 0);
    }

    const bool      ma[3] = {};
    const int             mb[3] = {};
    const sc_uint<4>      mc[3] = {};
    const sc_bigint<4>    md[3] = {};
    
    void no_init_member1() {
        sct_assert_const(ma[0] == false);
        sct_assert_const(ma[1] == false);
        sct_assert_const(ma[2] == false);

        sct_assert_const(mb[0] == 0);
        sct_assert_const(mb[1] == 0);
        sct_assert_const(mb[2] == 0);

        sct_assert_const(mc[0] == 0);
        sct_assert_const(mc[1] == 0);
        sct_assert_const(mc[2] == 0);
        
        sct_assert_const(md[0] == 0);
        sct_assert_const(md[1] == 0);
        sct_assert_const(md[2] == 0);
    }
    
    const bool            mma[3] = {true};
    const int             mmb[3] = {1,2};
    const sc_uint<4>      mmc[3] = {3};
    const sc_bigint<4>    mmd[3] = {4,-5};
    
    void part_init_member1() {
        sct_assert_const(mma[0] == true);
        sct_assert_const(mma[1] == false);
        sct_assert_const(mma[2] == false);

        sct_assert_const(mmb[0] == 1);
        sct_assert_const(mmb[1] == 2);
        sct_assert_const(mmb[2] == 0);

        sct_assert_const(mmc[0] == 3);
        sct_assert_const(mmc[1] == 0);
        sct_assert_const(mmc[2] == 0);
        
        sct_assert_const(mmd[0] == 4);
        sct_assert_const(mmd[1] == -5);
        sct_assert_const(mmd[2] == 0);
    }

    
};

int sc_main(int argc, char *argv[]) 
{
    sc_clock clk {"clk", sc_time(1, SC_NS)};
    A top{"top"};
    top.clk(clk);

    sc_start();
    return 0;
}

