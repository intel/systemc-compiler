/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

#include "systemc.h"

using namespace sc_core;

// Simple version
// Encoders
//    Encoder - Using if-else Statement
//    Encoder - Using case Statement
// Priority Encoders
//    Pri-Encoder - Using if-else Statement
//    Encoder - Using assign Statement
// Decoders
//    Decoder - Using case Statement
//    Decoder - Using assign Statement

struct Dut : sc_module 
{
    sc_in<bool>         enable{"enable"}; // Enable of the encoder
    sc_in<sc_uint<4>>  data_in{"data_in"};
    sc_in<sc_uint<2>>   bdata_in{"bdata_in"};
    sc_out<sc_uint<2>>  binary_dout{"binary_dout"};
    sc_out<sc_uint<2>>  binary_case_dout{"binary_case_dout"};
    sc_out<sc_uint<2>>  binary_prior_dout{"binary_prior_dout"};
    sc_out<sc_uint<2>>  binary_prior_assign_dout{"binary_prior_assign_dout"};
    sc_out<sc_uint<4>>  binary_decoder_case_dout{"binary_decoder_case_dout"};
    sc_out<sc_uint<4>>  binary_decoder_assign_dout{"binary_decoder_assign_dout"};

    SC_CTOR(Dut) {
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
           if (data_in.read() == 0x2) {
            binary_dout = 1;
           }  if (data_in.read() == 0x4) {
            binary_dout = 2;
           }  if (data_in.read() == 0x8) {
            binary_dout = 3;
           }
        }
    }
 
    void encoder_case() {
        binary_case_dout = 0;
        if (enable.read()) {
          switch (data_in.read()) {
              case 0x2 : binary_case_dout = 1; break;
              case 0x4 : binary_case_dout = 2; break;
              case 0x8 : binary_case_dout = 3; break;
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
         }
        }
    }

    void priority_assign() {
        binary_prior_assign_dout  = ( ! enable.read()) ? 0 : (
           (data_in.read()[0]) ? 0 :
           (data_in.read()[1]) ? 1 :
           (data_in.read()[2]) ? 2 : 3);
    }

    void decoder_case() {
        binary_decoder_case_dout = 0;
        if (enable.read()) {
          switch (bdata_in.read()) {
            case 0x0 : binary_decoder_case_dout = 0x1;
            case 0x1 : binary_decoder_case_dout = 0x2;
            case 0x2 : binary_decoder_case_dout = 0x4;
            case 0x3 : binary_decoder_case_dout = 0x8;
          }
        }
    }
    void decoder_assign() {
        binary_decoder_assign_dout = (enable.read()) ? (1 << bdata_in.read()) : 0b0 ;
    }

};
