/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Async and sync reset combinations
SC_MODULE(test_reset) {

	// Inputs: Clock, resets, and 3 bit data inputs

	sc_in<bool> clk{"clk"};
	sc_in<bool> a0reset{"a0reset"};
	sc_in<bool> a1reset{"a1reset"};
	sc_in<bool> a2reset{"a2reset"};
	sc_in<bool> s0reset{"s0reset"};
	sc_in<bool> s1reset{"s1reset"};
	sc_in<bool> s2reset{"s2reset"};

    sc_in<sc_uint<3>> a_reg_in{"a_reg_in"};
    sc_in<sc_uint<3>> s_reg_in{"s_reg_in"};

    // Outputs: Data Outputs
    sc_out<sc_uint<3>> data0_reg_o{"data0_reg_o"};
    sc_out<sc_uint<3>> data1_reg_o{"data1_reg_o"};
    sc_out<sc_uint<3>> data2_reg_o{"data2_reg_o"};
    sc_out<sc_uint<3>> data3_reg_o{"data3_reg_o"};
    sc_out<sc_uint<3>> data4_reg_o{"data4_reg_o"};
    sc_out<sc_uint<3>> data5_reg_o{"data5_reg_o"};
    sc_out<sc_uint<3>> data6_reg_o{"data6_reg_o"};
    sc_out<sc_uint<3>> data7_reg_o{"data7_r6eg_o"};
    sc_out<sc_uint<3>> data8_reg_o{"data8_reg_o"};
    sc_out<sc_uint<3>> data9_reg_o{"data9_reg_o"};
    sc_out<sc_uint<3>> data10_reg_o{"data10_reg_o"};
    sc_out<sc_uint<3>> data11_reg_o{"data11_reg_o"};
    sc_out<sc_uint<3>> data12_reg_o{"data12_reg_o"};
    sc_out<sc_uint<3>> data13_reg_o{"data13_reg_o"};


    SC_CTOR(test_reset) {
    	// Combination 1
        SC_CTHREAD(async_rst, clk.pos());
        async_reset_signal_is(a0reset,false);

    	// Combination 2
        SC_CTHREAD(async_rst_active_high, clk.pos());
        async_reset_signal_is(a0reset,true);

    	// Combination 3
        SC_CTHREAD(sync_rst, clk.pos());
        reset_signal_is(s0reset,true);

    	// Combination 4
        SC_CTHREAD(sync_rst_active_low, clk.pos());
        reset_signal_is(s0reset,false);

    	// Combination 5
        SC_CTHREAD(sync_rst_multi, clk.pos());
        reset_signal_is(s0reset,true);
        reset_signal_is(s1reset,true);

    	// Combination 6
        SC_CTHREAD(async_rst_multi, clk.pos());
        async_reset_signal_is(a0reset,true);
        async_reset_signal_is(a1reset,true);

    	// Combination 7
        SC_CTHREAD(sync_rst_multi_active_low, clk.pos());
        reset_signal_is(s0reset,false);
        reset_signal_is(s1reset,false);

    	// Combination 8
        SC_CTHREAD(async_rst_multi_active_low, clk.pos());
        async_reset_signal_is(a0reset,false);
        async_reset_signal_is(a1reset,false);

    	// Combination 9
        SC_CTHREAD(sync_rst_multi_active_low_high, clk.pos());
        reset_signal_is(s0reset,false);
        reset_signal_is(s1reset,true);

    	// Combination 10
        SC_CTHREAD(sync_rst_multi_active_high_low, clk.pos());
        reset_signal_is(s0reset,true);
        reset_signal_is(s1reset,false);

    	// Combination 11
        SC_CTHREAD(sync_rst_multi_async_single_sync, clk.pos());
        async_reset_signal_is(a0reset,true);
        async_reset_signal_is(a1reset,true);
        reset_signal_is(s0reset,false);

    	// Combination 12
        SC_CTHREAD(sync_rst_multi_sync_single_async, clk.pos());
        reset_signal_is(s0reset,true);
        reset_signal_is(s1reset,false);
        async_reset_signal_is(a0reset,false);

    	// Combination 13
        SC_CTHREAD(sync_rst_multi_sync_multi_async, clk.pos());
        reset_signal_is(s0reset,true);
        reset_signal_is(s1reset,false);
        async_reset_signal_is(a0reset,true);
        async_reset_signal_is(a1reset,false);
        async_reset_signal_is(a2reset,false);

    }

	// Combination 1
    void async_rst() {
    	data0_reg_o.write(0);
    	wait();

        while(1) {
        	data0_reg_o.write(a_reg_in.read());
        	wait();
        }
    }

	// Combination 2
    void async_rst_active_high() {
    	data1_reg_o.write(0);
    	wait();

        while(1) {
        	data1_reg_o.write(a_reg_in.read());
        	wait();
        }
    }

	// Combination 3
    void sync_rst() {
    	data2_reg_o.write(0);
    	wait();

        while(1) {
        	data2_reg_o.write(a_reg_in.read());
        	wait();
        }
    }

	// Combination 4
    void sync_rst_active_low() {
    	data3_reg_o.write(0);
    	wait();

        while(1) {
        	data3_reg_o.write(a_reg_in.read());
        	wait();
        }
    }

	// Combination 5
    void sync_rst_multi() {
    	data4_reg_o.write(0);
    	wait();

        while(1) {
        	data4_reg_o.write(a_reg_in.read());
        	wait();
        }
    }

    // Combination 6
    void async_rst_multi() {
    	data5_reg_o.write(0);
    	wait();

        while(1) {
        	data5_reg_o.write(a_reg_in.read());
        	wait();
        }
    }

    // Combination 7
    void sync_rst_multi_active_low() {
    	data6_reg_o.write(0);
    	wait();

        while(1) {
        	data6_reg_o.write(a_reg_in.read());
        	wait();
        }
    }

    // Combination 8
    void async_rst_multi_active_low() {
    	data7_reg_o.write(0);
    	wait();

        while(1) {
        	data7_reg_o.write(a_reg_in.read());
        	wait();
        }
    }

    // Combination 9
    void sync_rst_multi_active_low_high() {
    	data8_reg_o.write(0);
    	wait();

        while(1) {
        	data8_reg_o.write(a_reg_in.read());
        	wait();
        }
    }

    // Combination 10
    void sync_rst_multi_active_high_low() {
    	data9_reg_o.write(0);
    	wait();

        while(1) {
        	data9_reg_o.write(a_reg_in.read());
        	wait();
        }
    }

    // Combination 11
    void sync_rst_multi_async_single_sync() {
    	data10_reg_o.write(0);
    	wait();

        while(1) {
        	data10_reg_o.write(a_reg_in.read());
        	wait();
        }
    }

    // Combination 12
    void sync_rst_multi_sync_single_async() {
    	data11_reg_o.write(0);
    	wait();

        while(1) {
        	data11_reg_o.write(a_reg_in.read());
        	wait();
        }
    }

    // Combination 13
   void sync_rst_multi_sync_multi_async() {
    	data12_reg_o.write(0);
    	wait();

        while(1) {
        	data12_reg_o.write(a_reg_in.read());
        	wait();
        }
    }


};

int sc_main(int argc, char **argv) {
	sc_clock sclk("sclk", sc_time(10,SC_PS));
	sc_signal<bool> arst;
	sc_signal<bool> srst;

	sc_signal<sc_uint<3>> ain_reg;
	sc_signal<sc_uint<3>> sin_reg;
	sc_signal<sc_uint<3>> data0_out_reg;
	sc_signal<sc_uint<3>> data1_out_reg;
	sc_signal<sc_uint<3>> data2_out_reg;
	sc_signal<sc_uint<3>> data3_out_reg;
	sc_signal<sc_uint<3>> data4_out_reg;
	sc_signal<sc_uint<3>> data5_out_reg;
	sc_signal<sc_uint<3>> data6_out_reg;
	sc_signal<sc_uint<3>> data7_out_reg;
	sc_signal<sc_uint<3>> data8_out_reg;
	sc_signal<sc_uint<3>> data9_out_reg;
	sc_signal<sc_uint<3>> data10_out_reg;
	sc_signal<sc_uint<3>> data11_out_reg;
	sc_signal<sc_uint<3>> data12_out_reg;
	sc_signal<sc_uint<3>> data13_out_reg;

    test_reset t_inst{"t_inst"};
    t_inst.clk(sclk);
    t_inst.a0reset(arst);
    t_inst.a1reset(arst);
    t_inst.a2reset(arst);

    t_inst.s0reset(srst);
    t_inst.s1reset(srst);
    t_inst.s2reset(srst);

    t_inst.a_reg_in(ain_reg);
    t_inst.s_reg_in(sin_reg);

    t_inst.data0_reg_o(data0_out_reg);
    t_inst.data1_reg_o(data1_out_reg);
    t_inst.data2_reg_o(data2_out_reg);
    t_inst.data3_reg_o(data3_out_reg);
    t_inst.data4_reg_o(data4_out_reg);
    t_inst.data5_reg_o(data5_out_reg);
    t_inst.data6_reg_o(data6_out_reg);
    t_inst.data7_reg_o(data7_out_reg);
    t_inst.data8_reg_o(data8_out_reg);
    t_inst.data9_reg_o(data9_out_reg);
    t_inst.data10_reg_o(data10_out_reg);
    t_inst.data11_reg_o(data11_out_reg);
    t_inst.data12_reg_o(data12_out_reg);
    t_inst.data13_reg_o(data13_out_reg);

    sc_start();
    return 0;
}

