/******************************************************************************
* Copyright (c) 2026, Intel Corporation. All rights reserved.
*
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
*
*****************************************************************************/

#include "sct_common.h"
#include <systemc.h>


/// Record and record array in MIF array compared with default operator ==()

using namespace sc_core;

struct Simple {
    bool a;
    int b;

    bool operator == (const Simple& other) const = default;
};

namespace std {
inline ::std::ostream& operator << (::std::ostream& os, const Simple&)
{
    return os;
}
}

struct mod_if : public sc_module, sc_interface {
    sc_in_clk clk{"clk"};
    sc_in<bool> nrst{"nrst"};

    sc_signal<unsigned> index{"index"};
    sc_signal<Simple> sr1{"sr1"};
    sc_vector<sc_signal<Simple>> sr2{"sr2", 2};
    sc_signal<bool> meth_result{"meth_result"};
    sc_signal<bool> thrd_result{"thrd_result"};

    Simple mr1;
    Simple mr2[2];
    Simple mr3;
    Simple mr4[2];
    Simple mr5;
    Simple mr6[2];
    Simple mr7;
    Simple mr8[2];

    SC_CTOR(mod_if)
    {
        SC_METHOD(equal_meth);
        sensitive << index << sr1 << sr2[0] << sr2[1];

        SC_CTHREAD(equal_thrd, clk.pos());
        async_reset_signal_is(nrst, false);
    }

    int func(unsigned i) {
        int res = 0;
        Simple lr[2];
        if (sr2[i].read() == mr8[i+1]) {
            mr8[i].b = 0;
            if (lr[i] == mr7) {
                mr8[i].b++;
            }
            res = 1;
        }
        return res;
    }

    void equal_meth()
    {
        bool result;
        unsigned i = index.read();
        Simple r1;
        Simple r2[2];

        result = r1 == mr1;
        result = r1 == sr1.read();
        result = sr1.read() == r1;
        result = r2[i] == mr2[i+1];
        result = sr2[i].read() == r2[i+1];

        if (mr2[i] == sr2[i].read() || r1 == mr1) {
            result = true;
        }
        meth_result = result;
    }

    void equal_thrd()
    {
        bool result;
        Simple r1;
        Simple r2[2];
        result = r1 == mr5;
        thrd_result = result;
        wait();

        while (true) {
            unsigned i = index.read();
            Simple r3 = sr2[i].read();

            result = r3 == r2[i];
            result = r2[i] == sr2[0].read();
            result = mr6[i] == r1;
            thrd_result = result;
            wait();
        }
    }
};

SC_MODULE(top) {
    sc_in_clk clk{"clk"};
    sc_in<bool> nrst{"nrst"};

    sc_vector<mod_if> minst{"minst", 2};

    SC_CTOR(top)
    {
        for (unsigned i = 0; i < 2; ++i) {
            minst[i].clk(clk);
            minst[i].nrst(nrst);
        }

        SC_CTHREAD(top_thread, clk.pos());
        async_reset_signal_is(nrst, false);

        SC_CTHREAD(func_thread, clk.pos());
        async_reset_signal_is(nrst, false);
    }

    sc_signal<bool> t1;
    void top_thread() {
        int i = 0;
        bool b = minst[i].mr4[0] == minst[i].mr4[1];
        wait();
        while (true) {
            i = minst[0].index.read();
            b = minst[0].sr1.read() == minst[1].sr2[i].read();  
            b = minst[0].sr2[i].read() == minst[1].sr2[i+1].read();
            b = minst[i].mr3 == minst[i].mr4[i+1];  
            minst[i].mr3 = minst[i].mr4[i+1];
            minst[i].sr2[i+1].write(minst[i+2].mr3);    
            minst[i].sr2[i+1].write(minst[i+2].mr4[i+3]);
            t1 = b;

            if (minst[i].mr3 == minst[i+1].mr4[i+2]) {
                t1 = !b;
            }
            wait();
        }
    }

    sc_signal<bool> t2;
    void func_thread() {
        wait();
        while (true) {
            int j = minst[0].index.read();
            int i = minst[j].func(0);
            t2 = i == 0;
            wait();
        }
    }
};

SC_MODULE(tb) {
    sc_clock clk{"clk", sc_time(1, SC_NS)};
    sc_signal<bool> nrst{"nrst"};
    top top_inst{"top_inst"};

    SC_CTOR(tb)
    {
        top_inst.clk(clk);
        top_inst.nrst(nrst);
    }
};

int sc_main(int argc, char** argv)
{
    tb tb_inst{"tb_inst"};
    sc_start();
    return 0;
}