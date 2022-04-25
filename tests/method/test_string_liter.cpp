/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <sct_assert.h>
#include <sct_sel_type.h>
#include <systemc.h>
#include <cstdint>
#include <iostream>

// String literals and variables converted to SC integers
class A : public sc_module {
public:
    sc_in_clk clk;
    sc_signal<bool> rstn;
    
    SC_HAS_PROCESS(A);

    sc_signal<sc_uint<3>> s;
    
    A(const sc_module_name& name) : sc_module(name) 
    {
        SC_METHOD(literFromString); sensitive << s;
        
        SC_METHOD(stringVarToInt); sensitive << s;
        nstr = "-42";
        ustr = "36893225441449414929";
        ncstr = "-0xAFFFF1111FFFF1111";
    }

    #define CHECK(ARG) sct_assert(ARG); sct_assert_const(ARG);
    
    void literFromString() 
    {
        sc_uint<6> ux = "0b110011";
        ux = "42";
        cout << "42" << ux << endl;
        ux = sc_uint<5>("42");
        ux = "0";
        ux = "0x0";
        ux = "00";
        
        //ux += "42";  -- not supported in SC
        //bool b = ux == "42"; -- not supported in SC
        //ux = ux + "42"; // -- lead to string result

        sc_int<10> ix = "-11";
        ix = "-0x26";
        ix = "-0";
        ix = "-01";
        ix = "-0b1";

        sc_biguint<65> bu;// = "0x1F";
        bu = "0x1FFFF1111FFFF1111";
        bu = "36893225441449414929";

        sc_bigint<100> bi = "-0x1F";
        bi = "-0xAFFFF1111FFFF1111";
         
    }
    
    std::string str = "42";
    std::string ustr;
    std::string nstr;
    
    const char* cstr = "43";
    const char* ucstr = "0x1FFFF1111FFFF1111";
    const char* ncstr;
    
    void stringVarToInt() {
        sc_uint<12> ux = str.c_str();
        ux = str.c_str();
        sc_int<16> ix = cstr;
        ix = nstr.c_str();

        sc_biguint<65> bu = ucstr;
        bu = ustr.c_str();
        sc_bigint<100> bi = ncstr;
        bi = ncstr;
        
        //cout << ux << endl;
        //ux = str;
        
    }
};

int sc_main(int argc, char *argv[]) 
{
    sc_clock clk{"clk", 1, SC_NS};
    A a_mod{"a_mod"};
    a_mod.clk(clk);
    sc_start();
    
    return 0;
}

