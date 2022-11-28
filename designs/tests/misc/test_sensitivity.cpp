/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by ripopov on 10/20/18.
//

#include <systemc.h>

struct foobar {
    int foo;
    int bar;
};

SC_MODULE(dut) {

    sc_in<int> in0{"in0"};
    sc_in<int> in1{"in1"};
    sc_in<int> in2{"in2"};
    sc_in<int> in3{"in3"};
    sc_in<int> in4{"in4"};
    sc_in<int> in5{"in5"};

    sc_in<int> inarray[4];

    sc_signal<int> sig0{"sig0"};
    sc_signal<int> sig1{"sig1"};
    sc_signal<int> sig2{"sig2"};
    sc_signal<int> sig3{"sig3"};
    sc_signal<int> sig4{"sig4"};
    sc_signal<int> sig5{"sig5"};

    sc_out<int> out0{"out0"};
    sc_out<int> out1{"out1"};
    sc_out<int> out2{"out2"};
    sc_out<int> out3{"out3"};


    SC_CTOR(dut) {
        SC_METHOD(method0); sensitive << sig5;
        SC_METHOD(method1); sensitive << sig5;
        SC_METHOD(method2); sensitive << sig5;
        SC_METHOD(method3); sensitive << sig5;
    }

    void method0() {
        sig0 = in0 + in1;
    }

    void method1() {
        if (in1) {
            sig1 = in2;
        } else {
            sig1 = in0 - in3;
        }
    }

    void method2() {
        for (size_t i = 0; i < 4; ++i) {
            int x = in3 - inarray[i];
            if (x) {
                out3 = x + 1;
            }
        }
    }

    void method3() {
        switch(out3.read()) {
            case 0: sig3 =  in1;
                break;
            case 1: sig4 =  in2;
        }
    }

    int m4 = 0;
    foobar fb;

    void fcall() {
    }

    void method4() {
        int x4;
        m4 ++;
        x4 = 0;
        fb.bar = 12;
        x4 = fb.foo;
    }

};

SC_MODULE(test) {

    dut dut0{"dut0"};

    sc_signal<int> in0{"in0"};
    sc_signal<int> in1{"in1"};
    sc_signal<int> in2{"in2"};
    sc_signal<int> in3{"in3"};
    sc_signal<int> in4{"in4"};
    sc_signal<int> in5{"in5"};
    sc_signal<int> out0{"out0"};
    sc_signal<int> out1{"out1"};
    sc_signal<int> out2{"out2"};
    sc_signal<int> out3{"out3"};

    sc_signal<int> inarray[4];

    SC_CTOR(test) {
        for (size_t i = 0; i < 4; ++i) {
            dut0.inarray[i](inarray[i]);
        }

        dut0.in0(in0);
        dut0.in1(in1);
        dut0.in2(in2);
        dut0.in3(in3);
        dut0.in4(in4);
        dut0.in5(in5);
        dut0.out0(out0);
        dut0.out1(out1);
        dut0.out2(out2);
        dut0.out3(out3);
    }

};

int sc_main(int argc, char **argv) {
    test t0{"t0"};
    sc_start();
    return 0;
}
