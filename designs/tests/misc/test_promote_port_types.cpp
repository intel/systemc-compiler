/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Port promotion of various types of ports
SC_MODULE(inner) 
{
    sc_in_clk   clk{"clk"};
    sc_in<bool> rstn{"rstn"};

    sc_in<sc_uint<8>>*      din;
    sc_out<sc_biguint<9>>   dout[3];
    
    sc_signal<bool> req;
    sc_signal<int> data1; 
    sc_signal<unsigned long> data2; 
    sc_signal<uint16_t> data3; 

    sc_signal<sc_uint<1>> req_;
    sc_signal<sc_int<16>> data1_; 
    sc_signal<sc_uint<8>> data2_; 
    sc_signal<sc_uint<9>> data3_; 
    sc_signal<sc_bigint<66>> data4_; 
    sc_signal<sc_biguint<66>> data5_; 
    
    SC_CTOR(inner) 
    {
        din = new sc_in<sc_uint<8>>("din");
                
        SC_CTHREAD(proc, clk.neg());
        async_reset_signal_is(rstn, false);
        
        SC_METHOD(meth1);
        sensitive << req << data1 << data2 << data3;

        SC_METHOD(meth2);
        sensitive << data3_;
    }
    
    void proc() 
    {
        wait();
        
        while(1) {
            for (int i = 0; i < 3; i++) dout[i] = din->read() + i;
            wait();
        }
    }
    
    void meth1() {
        int l;
        if (req) l = 1;
        if (data1) l = 2;
        bool a = req;
        int b = data1;
        a = data1;
        unsigned c = data1;
        uint16_t d = data2;
        data1 = a;
        data2 = d;
        req = data1;
        data3 = req;
    }
    
    void meth2() {
        int l;
        sc_uint<1> lreq_;
        if (lreq_) l = 0;  
        // Commented are errors
        //if (req_) l = 1; 
        //if (data1_) l = 2;
        //bool a = req_;
        //int b = data1_;
        //int b = data4_;
        bool a; int i;
        req_ = a;     // OK
        data1_ = a;   // OK
        data2_ = a;   // OK
        data4_ = a;   // OK
        data1_ = i;   // OK
        data2_ = i;   // OK
        data4_ = i;   // OK
        //req = data1_;
        //data1_ = req;
        //req_ = data1_;
        //data1_ = req_;
        //data2_ = data3_; 
        //data3_ = data4_; 
        data4_ = (sc_biguint<66>)data3_.read();
    }
};

SC_MODULE(top) {
    sc_in_clk   clk{"clk"};
    sc_in<bool> rstn{"rstn"};

    inner inner0{"inner0"};

    SC_CTOR(top) {
        inner0.rstn(rstn);
    }
};


int sc_main(int argc, char **argv) {

    cout << "test_promote_ports\n";

    sc_signal<sc_uint<8>>      din;
    sc_signal<sc_biguint<9>>   dout[3];

    sc_signal<bool> rstn;
    sc_clock clk{"clk", 1, SC_NS};
    top top0{"top0"};
    
    top0.clk(clk);
    top0.rstn(rstn);
    top0.inner0.clk(clk);

    top0.inner0.din->bind(din);
    for (int i = 0; i < 3; i++) {
        top0.inner0.dout[i](dout[i]);
    }
    sc_start();

    return 0;
}


