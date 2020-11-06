/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by ripopov on 11/26/18.
//

#include <systemc.h>

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////


struct sync_module : sc_module
{
    sc_in<bool> clk{"clk"};
    sc_in<bool> rstn{"rstn"};

protected:
    void before_end_of_elaboration() override
    {
        for(auto *child: get_child_objects()) {
            if (sync_module *sync_child = dynamic_cast<sync_module *>(child)) {
                sync_child->clk(clk);
                sync_child->rstn(rstn);
            }
        }
    }
};



/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////




#define SYNC_THREAD(func) \
    SC_CTHREAD(func, clk.pos()); \
    async_reset_signal_is(rstn, false);

#define SYNC_METHOD(func) \
    SC_METHOD(func); \
    sensitive << clk.pos() << rstn.neg();




struct submod : sync_module {
    SC_CTOR(submod) {}
};






struct demo : sync_module {

    sc_vector<submod>  submods{"submods", 2};

    SC_CTOR(demo) {
        // @SC_METHOD w/o all used signals in sensitivity not supported
        //SYNC_METHOD(example0);
        //SYNC_METHOD(example1);
        //SYNC_METHOD(example2);
        //SYNC_METHOD(example3);
        SYNC_THREAD(thread0);
        SYNC_THREAD(thread1);
        SYNC_THREAD(thread2);
    }

    /////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////

    sc_signal<int> counter{"counter"};
    void example0 () {
        if (!rstn) {
            counter = 0;
        } else {
            counter = counter + 1;
        }
    }

    /////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////

    sc_int<16> regfile[16];
    sc_signal<bool>       read_en{"read_en"};
    sc_signal<bool>       write_en{"write_en"};
    sc_signal<sc_uint<4>> address{"address"};
    sc_signal<sc_int<16>> write_data{"write_data"};
    sc_signal<sc_int<16>> read_data{"read_data"};

    void example1 () {
        if (~rstn) {
            for (int i = 0; i < 16; ++i) {
                regfile[i] = 0;
            }
        } else {
            if (read_en) {
                read_data = regfile[address.read()];
            } else if (write_en) {
                regfile[address.read()] = write_data;
            }
        }
    }

    /////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////


    int max (int a, int b) {
        if (a > b)
            return a;
        else
            return b;
    }

    template <typename T>
    void increment(T & acc) {
        acc = acc + 1;
    }

    sc_signal<int> a{"a"};
    sc_signal<int> b{"b"};
    sc_signal<int> m{"s"};
    sc_signal<int> acc{"acc"};

    void example2() {
        m = max(a,b);

        increment(acc);
    }

    /////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////

    const int x = rand() - 10;
    sc_signal<int> outi{"outi"};
    void example3() {
        outi = 0;
        int y = x;
        if (y + 1 > 2) {
            outi = 1;
        } else {
            outi = 2;
        }

    }

    /////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////

    sc_signal<int> tout{"tout"};
    void thread0() {
        tout = 0;
        wait();

        while (1) {
            tout = 1;
            wait();
            tout = 2;
            wait();
            tout = 3;
        }
    }

    /////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////

    sc_signal<bool> ready{"ready"};
    sc_signal<bool> valid{"valid"};

    void thread1() {
        valid = 0;
        wait();

        valid = 1;
        do { wait(); } while (!ready);
    }

    /////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////

    sc_signal<bool> scond{"scond"};
    sc_signal<bool> scond2{"scond"};
    sc_signal<int > t2out{"t2out"};
    void thread2() {
        t2out = 0;
        while (1) {
            wait();

            for (int i =0 ; i < t2out ; i++) {
                wait();

            }
        }
    }

    /////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////

};


struct top : sc_module {

    sc_clock        clk{"clk", 10, SC_NS};
    sc_signal<bool> rstn{"rstn"};

    demo demo0{"demo0"};

    SC_CTOR(top) {
        demo0.clk(clk);
        demo0.rstn(rstn);
    }

};

int sc_main(int argc, char **argv) {
    auto t = std::make_unique<top>("top");
    sc_start();
    return 0;
}
