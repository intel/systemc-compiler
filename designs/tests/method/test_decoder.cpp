/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"

using namespace sc_core;

//Encoders
//    Encoder - Using if-else Statement
//    Encoder - Using case Statement
// Priority Encoders
//    Pri-Encoder - Using if-else Statement
//    Encoder - Using assign Statement
// Decoders
//    Decoder - Using case Statement
//    Decoder - Using assign Statement

class A : public sc_module 
{
public:
    sc_in<bool>         enable{"enable"}; // Enable of the encoder
    sc_in<sc_uint<16>>  data_in{"data_in"};
    sc_in<sc_uint<4>>   bdata_in{"bdata_in"};
    sc_out<sc_uint<4>>  binary_dout{"binary_dout"};
    sc_out<sc_uint<4>>  binary_case_dout{"binary_case_dout"};
    sc_out<sc_uint<4>>  binary_prior_dout{"binary_prior_dout"};
    sc_out<sc_uint<4>>  binary_prior_assign_dout{"binary_prior_assign_dout"};
    sc_out<sc_uint<16>>  binary_decoder_case_dout{"binary_decoder_case_dout"};
    sc_out<sc_uint<16>>  binary_decoder_assign_dout{"binary_decoder_assign_dout"};

    SC_CTOR(A) {
        SC_METHOD(encoder_ifelse); sensitive << enable << data_in;
        SC_METHOD(encoder_case); sensitive << enable << data_in;
        SC_METHOD(priority_ifelse); sensitive << enable << data_in;
        SC_METHOD(priority_assign); sensitive << enable << data_in;
        SC_METHOD(decoder_case); sensitive << enable << bdata_in;
        SC_METHOD(decoder_assign); sensitive << enable << bdata_in;
    }
    
    void encoder_ifelse() {
        binary_dout = 0;
        if (enable.read()) {
           if (data_in.read() == 0x0002) {
            binary_dout = 1;
           }  if (data_in.read() == 0x0004) {
            binary_dout = 2;
           }  if (data_in.read() == 0x0008) {
            binary_dout = 3;
           }  if (data_in.read() == 0x0010) {
            binary_dout = 4;
           }  if (data_in.read() == 0x0020) {
            binary_dout = 5;
           }  if (data_in.read() == 0x0040) {
            binary_dout = 6;
           }  if (data_in.read() == 0x0080) {
            binary_dout = 7;
           }  if (data_in.read() == 0x0100) {
            binary_dout = 8;
           }  if (data_in.read() == 0x0200) {
            binary_dout = 9;
           } if (data_in.read() == 0x0400) {
            binary_dout = 10;
           }  if (data_in.read() == 0x0800) {
            binary_dout = 11;
           }  if (data_in.read() == 0x1000) {
            binary_dout = 12;
           }  if (data_in.read() == 0x2000) {
            binary_dout = 13;
           }  if (data_in.read() == 0x4000) {
            binary_dout = 14;
           } if (data_in.read() == 0x8000) {
            binary_dout = 15;
           }
        }
    }
 
    void encoder_case() {
        binary_case_dout = 0;
        if (enable.read()) {
          switch (data_in.read()) {
              case 0x0002 : binary_case_dout = 1; break;
              case 0x0004 : binary_case_dout = 2; break;
              case 0x0008 : binary_case_dout = 3; break;
              case 0x0010 : binary_case_dout = 4; break;
              case 0x0020 : binary_case_dout = 5; break;
              case 0x0040 : binary_case_dout = 6; break;
              case 0x0080 : binary_case_dout = 7; break;
              case 0x0100 : binary_case_dout = 8; break;
              case 0x0200 : binary_case_dout = 9; break;
              case 0x0400 : binary_case_dout = 10; break;
              case 0x0800 : binary_case_dout = 11; break;
              case 0x1000 : binary_case_dout = 12; break;
              case 0x2000 : binary_case_dout = 13; break;
              case 0x4000 : binary_case_dout = 14; break;
              case 0x8000 : binary_case_dout = 15; break;
          }
        }
    }

    void priority_ifelse() {
        binary_prior_dout = 0;
        if (enable.read()) {
         if (data_in.read()[0] == 1) {
          binary_prior_dout = 0;
         } else if (data_in.read()[1] == 1) {
          binary_prior_dout = 1;
         } else if (data_in.read()[2] == 1) {
          binary_prior_dout = 2;
         } else if (data_in.read()[3] == 1) {
          binary_prior_dout = 3;
         } else if (data_in.read()[4] == 1) {
          binary_prior_dout = 4;
         } else if (data_in.read()[5] == 1) {
          binary_prior_dout = 5;
         } else if (data_in.read()[6] == 1) {
          binary_prior_dout = 6;
         } else if (data_in.read()[7] == 1) {
          binary_prior_dout = 7;
         } else if (data_in.read()[8] == 1) {
          binary_prior_dout = 8;
         } else if (data_in.read()[9] == 1) {
          binary_prior_dout = 9;
         } else if (data_in.read()[10] == 1) {
          binary_prior_dout = 10;
         } else if (data_in.read()[11] == 1) {
          binary_prior_dout = 11;
         } else if (data_in.read()[12] == 1) {
          binary_prior_dout = 12;
         } else if (data_in.read()[13] == 1) {
          binary_prior_dout = 13;
         } else if (data_in.read()[14] == 1) {
          binary_prior_dout = 14;
         } else if (data_in.read()[15] == 1) {
             binary_prior_dout = 15;
            }
        }
    }

    void priority_assign() {
        binary_prior_assign_dout  = ( ! enable.read()) ? 0 : (
           (data_in.read()[0]) ? 0 :
           (data_in.read()[1]) ? 1 :
           (data_in.read()[2]) ? 2 :
           (data_in.read()[3]) ? 3 :
           (data_in.read()[4]) ? 4 :
           (data_in.read()[5]) ? 5 :
           (data_in.read()[6]) ? 6 :
           (data_in.read()[7]) ? 7 :
           (data_in.read()[8]) ? 8 :
           (data_in.read()[9]) ? 9 :
           (data_in.read()[10]) ? 10 :
           (data_in.read()[11]) ? 11 :
           (data_in.read()[12]) ? 12 :
           (data_in.read()[13]) ? 13 :
           (data_in.read()[14]) ? 14 : 15);
    }

    void decoder_case() {
        binary_decoder_case_dout = 0;
        if (enable.read()) {
          switch (bdata_in.read()) {
            case 0x0 : binary_decoder_case_dout = 0x0001; break;
            case 0x1 : binary_decoder_case_dout = 0x0002; break;
            case 0x2 : binary_decoder_case_dout = 0x0004; break;
            case 0x3 : binary_decoder_case_dout = 0x0008; break;
            case 0x4 : binary_decoder_case_dout = 0x0010; break;
            case 0x5 : binary_decoder_case_dout = 0x0020; break;
            case 0x6 : binary_decoder_case_dout = 0x0040; break;
            case 0x7 : binary_decoder_case_dout = 0x0080; break;
            case 0x8 : binary_decoder_case_dout = 0x0100; break;
            case 0x9 : binary_decoder_case_dout = 0x0200; break;
            case 0xA : binary_decoder_case_dout = 0x0400; break;
            case 0xB : binary_decoder_case_dout = 0x0800; break;
            case 0xC : binary_decoder_case_dout = 0x1000; break;
            case 0xD : binary_decoder_case_dout = 0x2000; break;
            case 0xE : binary_decoder_case_dout = 0x4000; break;
            case 0xF : binary_decoder_case_dout = 0x8000; break;
          }
        }
    }
    void decoder_assign() {
        binary_decoder_assign_dout = (enable.read()) ? (1 << bdata_in.read()) : 0b0 ;

    }

};

struct dut : sc_core::sc_module {
    typedef dut SC_CURRENT_USER_MODULE;
    dut(::sc_core::sc_module_name) {}
};

class B_top : public sc_module
{
public:
    sc_signal<bool> enable{"enable"};
    sc_signal<sc_uint<16>> data_in{"data_in"};
    sc_signal<sc_uint<4>> bdata_in{"bdata_in"};

    sc_signal<sc_uint<4>> binary_dout{"binary_dout"};
    sc_signal<sc_uint<4>> binary_case_dout{"binary_case_dout"};
    sc_signal<sc_uint<4>> binary_prior_dout{"binary_prior_dout"};
    sc_signal<sc_uint<4>> binary_prior_assign_dout{"binary_prior_assign_dout"};
    sc_signal<sc_uint<16>> binary_decoder_case_dout{"binary_decoder_case_dout"};
    sc_signal<sc_uint<16>> binary_decoder_assign_dout{"binary_decoder_assign_dout"};


    A a_mod{"a_mod"};

    SC_CTOR(B_top) {
        a_mod.enable(enable);
        a_mod.data_in(data_in);
        a_mod.bdata_in(bdata_in);
        a_mod.binary_dout(binary_dout);
        a_mod.binary_case_dout(binary_case_dout);
        a_mod.binary_prior_dout(binary_prior_dout);
        a_mod.binary_prior_assign_dout(binary_prior_assign_dout);
        a_mod.binary_decoder_case_dout(binary_decoder_case_dout);
        a_mod.binary_decoder_assign_dout(binary_decoder_assign_dout);
    }
};

int sc_main(int argc, char* argv[])
{
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}

