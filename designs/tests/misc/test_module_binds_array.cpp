/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Bind ports to individual elements signal array/vector
SC_MODULE(A) {
    sc_out<sc_uint<4>>  opa;
    sc_out<sc_uint<4>>  opb;
    sc_in<sc_uint<4>>   ipa;
    
    sc_out<sc_uint<4>>  aao[2];
    sc_vector<sc_in<sc_uint<4>>>   vvi{"vvi", 3};
    
    sc_vector<sc_vector<sc_in<sc_uint<4>>>>   v2i{"vvi", 3};
    sc_vector<sc_in<sc_uint<4>>>   v1i{"vvi", 3};
    
    
    sc_signal<int>      s;
    
    SC_CTOR(A) {
        for (int i = 0; i < 3; ++i) {
            v2i[i].init(2);
        }
        
        SC_METHOD(meth);
        sensitive << s << ipa << vvi[1] << vvi[2] << v2i[2][1] << v1i[1] << v1i[2];
    }
    
    void meth()
    {
        opa = s.read() + vvi[1].read();
        opb = s.read()+ipa.read();
        aao[0] = vvi[2].read() + v2i[2][1].read();
        aao[1] = v1i[1].read() + v1i[2].read();
    }
};

SC_MODULE(C) {
    sc_vector<sc_signal<sc_uint<4>>>  ivs{"ivs", 2};
    sc_signal<sc_uint<4>>  ias[2];
    
    sc_signal<sc_uint<4>>  aas[3][2];
    sc_vector<sc_signal<sc_uint<4>>>  vvs{"vvs", 4};

    sc_vector<sc_vector<sc_signal<sc_uint<4>>>>   v2s{"vvs", 3};
    sc_vector<sc_vector<sc_signal<sc_uint<4>>>>   v2s_{"vvs_", 2};
    
    SC_CTOR(C) {
        for (int i = 0; i < 3; ++i) {
            v2s[i].init(2);
        }
        for (int i = 0; i < 2; ++i) {
            v2s_[i].init(3);
        }
        
        SC_METHOD(meth);
        sensitive << ivs[0] << ias[1] << aas[1][1];
    }
    
    void meth()
    {
        int i = ivs[0].read() + ias[1].read();
        ias[0] = aas[1][1];
        for (int i = 0; i < 4; ++i) {
            vvs[i] = i;
        }
        for (int i = 0; i < 2; ++i) {
            v2s[i][i] = i; 
            v2s_[1][i] = i+1;
        }
    }

};

SC_MODULE(Top) 
{
    A a{"a"};
    C c{"c"};
    
    SC_CTOR(Top) {
        a.opa(c.ivs[0]);
        a.opb(c.ias[1]);
        
        a.ipa(c.ias[0]);
        
        a.aao[0](c.aas[0][0]);
        a.aao[1](c.aas[1][1]);
        for (int i = 0; i < 3; ++i) {
            a.vvi[i](c.vvs[i+1]);
        }
        
        a.v2i(c.v2s);
        a.v1i(c.v2s_[1]);
    }
};

int sc_main(int argc, char **argv) {
    Top mod{"mod"};
    sc_start();
    return 0;
}
