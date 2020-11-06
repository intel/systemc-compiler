/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by ripopov on 4/2/18.
//

#include <systemc.h>

struct adder : sc_module {

    sc_in_clk             clk{"clk"};
    sc_in<bool>           arstn{"arstn"};
    sc_vector<sc_in<int>> inputs{"input"};
    sc_out<int>           output{"output"};

    adder (sc_module_name, size_t n_inputs) {
        inputs.init(n_inputs);
        SC_HAS_PROCESS(adder);
        SC_CTHREAD(adder_thread, clk.pos());
        async_reset_signal_is(arstn, false);
    }

    void adder_thread() {
        int res = 0;
        while (1) {
            wait();

            for (auto &in: inputs)
                res += in;
            output = res;
        }
    }
};

struct tb: sc_module {

    const size_t N_INPUTS;

    adder dut{"dut", N_INPUTS};

    sc_clock clk_gen{"clk_gen", 10, SC_NS};
    sc_signal<bool> arstn;
    sc_vector<sc_signal<int>> inputs{"input", N_INPUTS};
    sc_signal<int> output{"output"};


    tb (sc_module_name, size_t  n_inputs) : N_INPUTS(n_inputs) {
        SC_HAS_PROCESS(tb);
        SC_THREAD(tb_thread);
        sensitive << clk_gen.posedge_event();

        dut.clk(clk_gen);
        dut.arstn(arstn);
        dut.inputs(inputs);
        dut.output(output);
    }

    void tb_thread() {
        arstn = 0;
        wait();
        arstn = 1;

        int sum = 0;
        for (size_t i = 0; i < N_INPUTS; ++i) {
            inputs[i] = i;
            sum += i;
        }

        wait(2);

        cout << "output == " << output << endl;
        sc_assert(sum == output);
        sc_stop();
    }

};


int sc_main(int argc, char **argv) {
    tb tb_inst{"tb_inst", 4 };
    sc_start();
    return 0;
}
