/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <iostream>
#include <sct_assert.h>
#include "systemc.h"

// Bitwise complement operation (~) for various types
class A : public sc_module {
public:
    sc_in_clk           clk;
    sc_signal<bool>     arstn;
    sc_signal<bool>     dummy;

    sc_signal<sc_uint<8>>       u8s;
    sc_signal<sc_uint<8>>       u8vs;
    sc_signal<sc_uint<32>>      u32s;
    sc_signal<sc_uint<32>>      u32vs;
    sc_signal<sc_biguint<8>>    b8s;
    sc_signal<sc_biguint<61>>   b61s;
    sc_signal<char>             crs; 
    sc_signal<unsigned>         uns;

    sc_signal<sc_int<10>>       i10s;
    sc_signal<sc_bigint<64>>    bi64s;
    sc_signal<sc_biguint<40>>   bu40s;
    
    sc_signal<bool>             cs;
    
    SC_HAS_PROCESS(A);
    A(sc_module_name) 
    {
        SC_METHOD(bitwise_in_concat); sensitive << dummy;
        
        SC_METHOD(bool_bitwise); 
        sensitive << dummy;
        
        SC_METHOD(sign_bitwise); 
        sensitive << dummy;
        
        SC_METHOD(uint_bitwise); 
        sensitive << dummy;
        
        SC_METHOD(biguint_bitwise); 
        sensitive << dummy;

        SC_METHOD(unsigned_bitwise); 
        sensitive << dummy;
    }
    
    // Verilog simulation
    SCT_ASSERT(u8s.read() == 0x7F, clk.pos());
    SCT_ASSERT(u8vs.read() == 0x7F, clk.pos());
    SCT_ASSERT(u32s.read() == 0xFFFFFF7F, clk.pos());
    SCT_ASSERT(u32vs.read() == 0xFFFFFF7F, clk.pos());
    
    SCT_ASSERT(b8s.read() == 0x7F, clk.pos());
    SCT_ASSERT(b61s.read() == 0x1FFFFFFFFFFFFF7F, clk.pos());

    SCT_ASSERT(crs.read() == 0x7F, clk.pos());
    SCT_ASSERT(uns.read() == 0xFFFFFF7F, clk.pos());
    
    SCT_ASSERT(i10s.read() == -7, clk.pos());
    SCT_ASSERT(bi64s.read() == 6, clk.pos());
    SCT_ASSERT(bu40s.read() == 5, clk.pos());
    
    #define CHECK(ARG) sct_assert(ARG); sct_assert_const(ARG);
    
    
    // Example from DCC
    void bitwise_in_concat() 
    {
        sc_uint<16> u;
        sc_uint<16> u1 = 1;
        sc_uint<16> u2 = 0x2;
        sc_uint<16> u3 = 0x1;
        sc_biguint<16> b2 = 0x2;
        sc_biguint<16> b3 = 0x1;
        
        // Error reported -- that is OK
        //u = (u1.range(2,0), u2.range(2,0) & ~u3.range(2,0));    // 3
        //cout << "uint " << u << endl;
        
        u = (u1.range(2,0), sc_uint<3>(u2.range(2,0) & ~u3.range(2,0))); // 10
        cout << "uint cast " << u << endl;
        CHECK(u == 10);
        
        u = (u1.range(2,0), b2.range(2,0) & ~b3.range(2,0));    // 34
        cout << "biguint " << u << endl;
        
        // TODO: check me
        u = (u1.range(2,0), sc_biguint<3>(b2.range(2,0) & ~b3.range(2,0))); // 10
        cout << "biguint cast " << u << endl;
        //CHECK(u == 10);
    }
    
// ---------------------------------------------------------------------------    
    
    void bool_bitwise() 
    {
        sc_uint<1> u1 = 1;
        u1 = ~u1;               // u1 = 0
        
        sc_uint<7> u7 = 11;  
        bool d0 = ~u7.bit(0);   // d = 0
        bool d1 = ~u7.bit(1);   // d = 0
        bool d2 = ~u7.bit(2);   // d = 1
        bool d3 = ~u7.bit(3);   // d = 0
        sc_uint<4> d = (sc_uint<1>(d3), sc_uint<1>(d2), sc_uint<1>(d1), sc_uint<1>(d0));
        sc_uint<5> u5 = ~u7.range(4,0);
        
        cout << hex << " u1 " << u1 << " d " << d << " u5 " << u5 << dec << endl;   
        
        // SVC CPA assert 
        sct_assert_const(u1 == 0);
        // No value for @d
        // No value for @u5

        // SystemC simulation
        sct_assert(u1 == 0);
        sct_assert(d == 4);
        sct_assert(u5 == 0x14);
    }

    void sign_bitwise() 
    {
        sc_int<10> i10 = -7;
        sc_bigint<64> bi64 = ~i10;       
        sc_biguint<40> bu40 = ~(i10+1);
        cout << hex << "i10: " << i10 << " bi64 " << bi64 << " bu40 " << bu40 << dec << endl;   

        CHECK(i10 == -7);
        CHECK(bi64 == 6);
        CHECK(bu40 == 5);
        
        i10s = i10;
        bi64s = bi64;
        bu40s = bu40;
     }
    
    // Module member array
    void uint_bitwise() 
    {
        sc_uint<8>  u8 = ~((sc_uint<8>) 128);   // SV: 127
        sc_uint<32> u32 = ~((sc_uint<8>) 128);
        sc_uint<32> u32a = u8;
        
        u8s  = ~((sc_uint<8>) 128);
        u8vs = u8;
        u32s = ~((sc_uint<8>) 128);
        u32vs = u32;
        
        cout << hex << "u8: " << u8 << " u32 " << u32 << " u32a " << u32a << dec << endl;
        
        // SVC CPA assert 
        CHECK(u8 == 0x7F);
        CHECK(u32 == 0xFFFFFF7F);
        CHECK(u32a == 0x7F);  
    }
    
    void biguint_bitwise() 
    {
        const sc_biguint<8> b8a = 128;
        const sc_uint<8> b8b = 128;
        sc_biguint<8> b8;
        sc_biguint<9> b9;
        // Lost value as width is not determinable for biguint
        b8 = ~b8a;
        b9 = ~b8a;  
        sct_assert_unknown(b8);
        sct_assert_unknown(b9);
        
        b8  = ~b8b;  
        b9  = ~b8b;  
        
        sc_biguint<61> b61 = ~b8b;
        b8s  = ~b8b;
        b61s = ~b8b;
        
        cout << hex << "b8: " << b8 << "b9: " << b9 << " b61 " << b61 << dec << endl;
        
        CHECK(b8 == 0x7F);
        CHECK(b9 == 0x17F);
        CHECK(b61 == 0x1FFFFFFFFFFFFF7F);
     }
    
     void unsigned_bitwise() 
     {
        char cr  = ~128;  
        unsigned un = ~128;
        crs  = ~128;
        uns = ~128;
        
        cout << hex << "cr: " << unsigned(cr) << " un " << un << dec << endl;

        CHECK(cr == 0x7F);
        CHECK(un == 0xFFFFFF7F);
     }
    
};

int sc_main(int argc, char *argv[])
{
    sc_clock clk{"clk", 10, SC_NS};
    A top_inst{"top_inst"};
    top_inst.clk(clk);

    sc_start(100, SC_NS);
    return 0;
}


