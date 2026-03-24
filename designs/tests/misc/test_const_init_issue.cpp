/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include <systemc.h>

// SC data type initialization from string issue
// Value @0xF2 is extended with @1 in all higher bits
struct A : public sc_module 
{
    sc_signal<sc_uint<32>> s;
    
    SC_HAS_PROCESS(A);
    
    explicit A(const sc_module_name& name) : sc_module(name) 
    {
        SC_METHOD(useLargeConst);
        sensitive << s;
    }
    
    static const sc_biguint<77> BS1;
    static const sc_biguint<77> BS2;
    const sc_biguint<77> BC1 = "0x0F2";
    const sc_biguint<77> BC2 = "0xF2";  
    const sc_uint<27> IC1 = "0x0F2";
    const sc_uint<27> IC2 = "0xF2";
    sc_biguint<77> BV1 = "0x0F2";
    sc_biguint<77> BV2 = "0xF2";
    sc_uint<27> IV1 = "0x0F2";
    sc_uint<27> IV2 = "0xF2";
    sc_uint<27> IV3;

    sc_signal<sc_biguint<128>> t0;
    void useLargeConst() 
    {
        cout << hex << "BS1 " << BS1 << " BS2 " << BS2 << endl;
        cout << "BC1 " << BC1 << " BC2 " << BC2 << endl;
        cout << "BV1 " << BV1 << " BV2 " << BV2 << endl;
        
        IV3 = "0xF2";
        cout << "IV1 " << IV1 << " IV2 " << IV2 << " IV3 " << IV3 << endl;

        t0 = BS1 + BS2 + BC1 + BC2 + BV1 + BV2;
        t0 = IV1 + IV2 + IV3;
    }
    
};

const sc_biguint<77> A::BS1 = "0x0F2";
const sc_biguint<77> A::BS2 = "0xF2";

int sc_main(int argc, char **argv) {

    A modA{"modA"};
    sc_start();

    return 0;
}

