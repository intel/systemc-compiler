/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by ripopov on 11/16/18.
//

#include <systemc.h>

using DT = sc_uint<2>;

struct IntPair {
    int x;
    int y;
    bool operator == (const IntPair &other) const { return x == other.x && y == other.y; }
    friend std::ostream& operator << (std::ostream &os, const IntPair &p) { return os; }
    friend void sc_trace(sc_trace_file *tf, const IntPair & v, const std::string & NAME ) {
        sc_trace(tf,v.x, NAME + ".x");
        sc_trace(tf,v.y, NAME + ".y");
    }
};

SC_MODULE(bottom) {
    sc_in<bool>    clk{"clk"};
    sc_in<bool>    rstn{"rstn"};

    sc_in<DT>      in[2];
    sc_out<DT>     out[2];

    sc_in<DT>      in2d[2][2];
    sc_out<DT>     out2d[2][2];

    SC_CTOR(bottom) {
        SC_METHOD(test_method);
        sensitive << clk.pos() << in2d[1][0] << in[1];
    }

    void test_method() {
        out[0] = in[1];
        out2d[0][1] = in2d[1][0].read() + 1;
    }

};

SC_MODULE(middle) {
    sc_in<bool>    clk{"clk"};
    sc_in<bool>    rstn{"rstn"};

    sc_in<DT>      in[2];
    sc_out<DT>     out[2];
    sc_out<DT>     out2d[2][2];

    sc_signal<DT>  in2d[2][2];

    bottom         b_inst{"b_inst"};

    SC_CTOR(middle) {

        b_inst.clk(clk);
        b_inst.rstn(rstn);

        b_inst.in[0]( in[0] );
        b_inst.in[1]( in[1] );

        b_inst.out[0]( out[0] );
        b_inst.out[1]( out[1] );

        b_inst.in2d[0][0]( in2d[0][0] );
        b_inst.in2d[0][1]( in2d[0][1] );
        b_inst.in2d[1][0]( in2d[1][0] );
        b_inst.in2d[1][1]( in2d[1][1] );

        b_inst.out2d[0][0]( out2d[0][0] );
        b_inst.out2d[0][1]( out2d[0][1] );
        b_inst.out2d[1][0]( out2d[1][0] );
        b_inst.out2d[1][1]( out2d[1][1] );

        SC_METHOD(drive_in2d_method);
        sensitive << clk.pos() << in2d[0][0];
    }

    void drive_in2d_method() {
        in2d[0][0] = 0;
        in2d[0][1] = in2d[0][0].read() + 1;
        auto & slice = in2d[0];
        slice[1] = 1;
    }
};

struct SignalPair {
    sc_signal<int> x_sig;
    sc_signal<int> y_sig;
};

struct ModIf : sc_modular_interface {
    ModIf(sc_module_name = sc_gen_unique_name("ModIf")) {}
    sc_signal<int> x_sig;
    sc_signal<int> y_sig;

};

SC_MODULE(top) {
    sc_signal<bool>   clk{"clk"};
    sc_signal<bool>   rstn{"rstn"};

    sc_in<IntPair>     inPair[2];
    sc_signal<IntPair> sigPair[2];

    SignalPair sPairArray[2];

    ModIf modIfArray[2];

    const int carray[2][2] = {{1,2},{3,4}};
    int array[2];

    sc_signal<DT>     in[2];
    sc_signal<DT>     out[2];
    sc_signal<DT>     out2d[2][2];

    sc_signal<DT>     *sp = new sc_signal<DT>[2];

    middle m_inst{"m_inst"};

    SC_CTOR(top) {

        for (size_t i = 0; i < 2; ++i) {
            inPair[i](sigPair[i]);
        }

        m_inst.clk(clk);
        m_inst.rstn(rstn);

        m_inst.in[0]( in[0] );
        m_inst.in[1]( in[1] );

        m_inst.out[0]( out[0] );
        m_inst.out[1]( out[1] );

        m_inst.out2d[0][0]( out2d[0][0] );
        m_inst.out2d[0][1]( out2d[0][1] );
        m_inst.out2d[1][0]( out2d[1][0] );
        m_inst.out2d[1][1]( out2d[1][1] );
    }

};



int sc_main(int argc, char **argv) {

    top t_inst{"t_inst"};
    sc_start();

    return 0;
}

