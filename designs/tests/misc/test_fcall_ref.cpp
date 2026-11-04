/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <sct_common.h>

using namespace sc_core;
using namespace sct;

// Bug #89 (external) for function call with reference parameters

struct Dut : sc_module {
    sct_in<sct_uint<8>> SC_NAMED(in);
    sct_out<sct_uint<2>> SC_NAMED(out_a);
    sct_out<sct_uint<3>> SC_NAMED(out_b);
    sct_out<sct_uint<9>> SC_NAMED(out_c);

    SC_HAS_PROCESS(Dut);

    explicit Dut(sc_module_name name) : sc_module(name) {
        //SC_METHOD(proc); sensitive << in;
        SC_METHOD(proc2); sensitive << in;
    }

    static void split(const sct_uint<8>& data, bool& sign,
                      sct_uint<2>& cls) {
        sign = data[7];             // Bug #89: this statement removed as unused
        cls = data == 0 ? 1 : 2;
    }

    void proc() {
        const sct_uint<8> data = in.read();

        bool sign_unused = false;
        sct_uint<2> cls_only = 0;
        split(data, sign_unused, cls_only);
        out_a = cls_only;

        bool sign_used = false;
        sct_uint<2> cls_used = 0;
        split(data, sign_used, cls_used);
        out_b = (sct_uint<1>(sign_used), cls_used);
    }

    void proc2() {
        unsigned count = in.read();
        unsigned unused1 = count++;
        unsigned unused2; unused2 = count++;
        out_c = count;
    }
};

int sc_main(int argc, char** argv) {
    sct_signal<sct_uint<8>> SC_NAMED(in);
    sct_signal<sct_uint<2>> SC_NAMED(out_a);
    sct_signal<sct_uint<3>> SC_NAMED(out_b);
    sct_signal<sct_uint<9>> SC_NAMED(out_c);
    Dut SC_NAMED(top);

    top.in(in);
    top.out_a(out_a);
    top.out_b(out_b);
    top.out_c(out_c);

    const unsigned inputs[] = {0x00, 0x01, 0x80, 0xff};

    for (unsigned index = 0; index < 4; ++index) {
        in = inputs[index];
        sc_start(1, SC_NS);
        sc_assert(out_c.read() == inputs[index] + 2u);
    }

    return 0;
}