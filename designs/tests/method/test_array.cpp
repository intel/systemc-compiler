/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"

using namespace sc_core;

// Local an member array of data types, signals and ports
class A : public sc_module {
public:
    int                 m[3];
    int                 n[3][3];
    sc_in<int>          in_ports1[3];
    sc_in<int>          in_ports1b[3];
    sc_in<int>          in_ports2[3][3];
    sc_in<int>          in_ports2b[3][3];

    sc_out<bool>        out_ports1b[3];
    sc_out<int>         out_ports1[3];
    sc_out<int>         out_ports2[3][3];
    sc_out<bool>        out_ports2b[3][3];

    sc_signal<sc_uint<5> > sig1[3];
    sc_signal<bool>     sig1b[3];
    sc_signal<int>      sig2[3][3];
    sc_signal<sc_uint<5> > sig2a[3][3];
    sc_signal<bool>     sig2b[3][3];
    
    sc_in<sc_uint<5> >* in_ports_p1[3];
    sc_out<sc_uint<5> >* out_ports_p1[3];
    sc_signal<int>*     sig_p1[3];
    sc_in<int>*         in_ports_p2[3][3];
    sc_out<int>*        out_ports_p2[3][3];
    sc_signal<int>*     sig_p2[3][3];
    sc_signal<sc_int<4> >*  sig_p2a[3][3];

    sc_signal<bool> dummy;

    SC_CTOR(A) {
        for (int i = 0; i < 3; i++) {
            in_ports_p1[i] = new sc_in<sc_uint<5> >("in_ports_p1");
            out_ports_p1[i] = new sc_out<sc_uint<5> >("out_ports_p1");
            sig_p1[i] = new sc_signal<int>("sig_p1");
            
            for (int j = 0; j < 3; j++) {
                in_ports_p2[i][j] = new sc_in<int>("in_ports_p2");
                out_ports_p2[i][j] = new sc_out<int>("out_ports_p2");
                sig_p2[i][j] = new sc_signal<int>("sig_p2");
                sig_p2a[i][j] = new sc_signal<sc_int<4> >("sig_p2a");
            }    
        }    

        SC_METHOD(chan_array_range); sensitive<< dummy << sig1[0];
        SC_METHOD(chan_array_double_used); sensitive << dummy << sig1b[0];

        SC_METHOD(mem_array); sensitive<< dummy;
        SC_METHOD(var_array); sensitive<< dummy;
        SC_METHOD(in_port_array); sensitive<< dummy << in_ports1[0] << in_ports2[0][0] << in_ports1b[0] << in_ports2b[0][0];
        SC_METHOD(out_port_array); sensitive<< dummy << in_ports1[0] << in_ports2[0][0] << in_ports2b[0][0];
        SC_METHOD(signal_array_part_sel); sensitive<< dummy << sig2a[0][0] << sig1[0] << *sig_p2a[0][0] << *in_ports_p1[0];
        SC_METHOD(signal_array); sensitive<< dummy << sig1[0] <<sig2[0][0] << sig1b[0];
        SC_METHOD(port_pointer_array); sensitive<< dummy << *in_ports_p1[0] << *in_ports_p2[0][0];
        SC_METHOD(sig_pointer_array); sensitive<< dummy << *sig_p1[1] << *sig_p2[1][0] << *out_ports_p1[0] << *in_ports_p1[0] << *in_ports_p2[0][0];

        SC_METHOD(array_init); sensitive<< dummy;
        SC_METHOD(array_record); sensitive << sig;
    }
    

    // Channel array range -- BUG in real design in cache module fixed
    void chan_array_range()
    {
        int i; bool b;
        sc_uint<3> offset = i;
        b = sig1[i].read().range(offset+1, offset); 
    }
    
    // The same channel array double used in the expression --
    // BUG in real design in Flow control module
    void chan_array_double_used() 
    {
        bool b = (sig1b[0].read()) ? false : sig1b[0].read();
    }
    
    // Module member array
    void mem_array() {
        int i = m[0];
        m[0] = i;
        m[1] = m[0]+m[2];
        m[2] = 0;
        n[1][m[1]] = n[m[0]][i];
        n[n[1][2]][2] = n[1][n[2][0]];
        
        for (int j = 0; j < 3; j++) {
            m[j] = j;
            for (int k = 0; k < 3; k++) {
                n[j][k] = j*k;
            }
        }
    }

    // Local variable array
    void var_array() {
        int  ln[2][3];
        int  lm[3] = {3,2,1};
        int i; int j = 0;
        i = lm[0];
        i = ln[1][2];
        lm[i] = i + 1;
        ln[i][j] = i;
        ln[i][j] = lm[0];
        lm[1] = ln[i][j];
    }
    
    // In port array
    void in_port_array() {
        int i; int j = 1;
        i = in_ports1[1];
        i = in_ports1[2].read();
        i = in_ports1b[1];
        bool f = in_ports2b[1][j].read();
        
        i = in_ports2[j][2];
        i = in_ports2[1][i].read();

        // Complex index
        i = in_ports1[j-1].read();
        i = in_ports2[i+j][2+i].read();
    }
    
    // Out port array
    void out_port_array() {
        int i; int j = 0;

        out_ports1[j] = i;
        out_ports1[j].write(i);
        out_ports1b[j] = true;
        out_ports1b[j].write(i);
        
        out_ports2[1][i] = 1;
        out_ports2[j][2].write(i+j);
        out_ports2b[j][i] = false;
        out_ports2b[1][2].write(j-i);

        // Complex index
        out_ports1[i+j] = 1;
        //
        out_ports2[i+1][j-i] = i;
        out_ports2[i+1][j-i].write(1);
        
        // Out to in
        out_ports1[1] = in_ports1[2];
        out_ports2[i+1][j-i] = in_ports1[i+3].read();
        out_ports1b[i] = in_ports2b[1][2];
        
        // Complex index with in port
        out_ports1[in_ports1[2].read()] = 3;
        out_ports1[in_ports1[i]] = in_ports2[1][2];
    }    
    
    void signal_array_part_sel() {
        int i; int j;
        sc_uint<5> x;

        x = sig1[1].read().range(3,2);
        i = sig1[j].read().bit(0);
        i = sig2a[1][2].read().bit(7);
        i = sig2a[1][2].read().range(9,5);
        
        bool b = in_ports_p1[2]->read().bit(3);
        i = in_ports_p1[2]->read().range(2,1);
        
        x = sig_p2a[i][j]->read().range(3,0);
        x = (*sig_p2a[i+j][i-j]).read().range(j+1,j);
        
        sig1[1] = sig1[2].read().range(1,0);
        out_ports1[0] = in_ports_p1[1]->read().bit(2);
        
        x = sig1[0].read().range(j+i+1,j+i);  
    }

    // Signal array
    void signal_array() {
        int i; int j = 0;

        sig1[j] = i;
        sig1[j].write(i+1);
        sig1b[j] = true;
        sig1b[j].write(i);
        
        // In/signal to signal
        sig2[1][i] = sig1[2].read();
        sig2[j][2].write(sig1[1].read()+sig1[2].read());
        sig2b[j][i] = sig1b[0];
        sig2b[1][2].write(!sig1b[0]);
        
        // Signal to out and complex index
        //i = sig1[sig1[2]];    -- not supported!!!
        i = sig2[1][sig1[2].read()];
        i = sig2[1][2];
        out_ports1[i] = sig2[1][sig1[2].read()];
        out_ports1[i] = sig2[sig1[2].read()][i];
        out_ports2[1][2].write(sig1[0].read() + sig2[1][2]);

        // Signal to signal
        sig1[1] = sig1[2];
        sig2[1][2] = sig2[0][1];
        sig2[0][1] = sig2[1][2]+sig2[2][1]+sig1[0].read();
    }    
   
    // Port pointer array
    void port_pointer_array() {
        int i;
        i = *in_ports_p2[1][2];
        i = (*in_ports_p1[1]).read();
        i = in_ports_p1[1]->read();

        *out_ports_p1[2] = i;
        (*out_ports_p1[2]).write(i);
        out_ports_p1[2]->write(i);

        *out_ports_p1[2] = *in_ports_p1[1];
        (*out_ports_p1[2]).write((*in_ports_p1[1]).read());
        out_ports_p1[2]->write(in_ports_p1[1]->read());
        
        *out_ports_p2[1][2] = *in_ports_p2[0][1];
        *out_ports_p2[1][2] = *in_ports_p2[0][1] + *in_ports_p2[2][2];
    }
    
    // Signal pointer array
    void sig_pointer_array() {
        int i; int j;
        i = *sig_p1[1];
        i = (*sig_p1[i+1]).read();
        i = sig_p1[1]->read();

        *sig_p1[2] = i;
        (*sig_p2[i+1][j+2]).write(i);
        sig_p1[2]->write(i);

        // Sig to out
        *out_ports_p1[2] = (*sig_p2[1][2]).read();
        *sig_p2[1][2]    = (*out_ports_p1[2]).read();
        (*out_ports_p1[2]).write((*sig_p1[1]).read());
        out_ports_p2[i][i]->write(sig_p1[1]->read());
        
        // In to sig 
        (*sig_p2[1][2]).write((*in_ports_p2[1][2]).read());
        *sig_p2[1][2] = (*in_ports_p1[0]).read() + *in_ports_p2[1][2];
        sig_p2[0][0]->write(in_ports_p1[0]->read());
        
        // Sig to sig
        *sig_p2[1][2] = *sig_p2[1][2];
        sig_p2[1][2]->write((*sig_p2[1][2]).read());
        *sig_p1[i*i] = sig_p2[i][i]->read() + (*sig_p2[1][2]).read();
    }    
    
    
    // Local array initialization
    void array_init() {
        int  y1[3];
        const int  y2[3] = {1,2,3};

        sc_uint<3>  x1[3];
        const sc_uint<3>  x2[3] = {1,2,3};
        
        int k1 = y2[1];
        int k2 = x2[1];
    }     
    
    // Array of records
    struct Pair {int x; int y;};
    Pair pa[2][2];
    sc_signal<int> sig;

    void array_record() 
    {
        pa[sig][sig].x = 0;
    }
    
};

class B_top : public sc_module {
public:
    sc_signal<int>          in_ports1[3];
    sc_signal<int>          in_ports1b[3];
    sc_signal<int>          in_ports2[3][3];
    sc_signal<int>          in_ports2b[3][3];
    
    sc_signal<bool>         out_ports1b[3];
    sc_signal<int>          out_ports1[3];
    sc_signal<int>          out_ports2[3][3];
    sc_signal<bool>         out_ports2b[3][3];

    sc_signal<sc_uint<5> >  in_ports_p1[3];
    sc_signal<int>          in_ports_p2[3][3];
    sc_signal<int>          out_ports_p2[3][3];
    
    A a_mod{"a_mod"};

    SC_CTOR(B_top) {
        for (int i = 0; i < 3; i++) {
            a_mod.in_ports1[i](in_ports1[i]);
            a_mod.in_ports1b[i](in_ports1b[i]);
            a_mod.out_ports1[i](out_ports1[i]);
            a_mod.out_ports1b[i](out_ports1b[i]);

            a_mod.in_ports_p1[i]->bind(in_ports_p1[i]);
            a_mod.out_ports_p1[i]->bind(in_ports_p1[i]);
            
            for (int j = 0; j < 3; j++) {
                a_mod.in_ports2[i][j](in_ports2[i][j]);
                a_mod.in_ports2b[i][j](in_ports2b[i][j]);
                a_mod.out_ports2[i][j](out_ports2[i][j]);
                a_mod.out_ports2b[i][j](out_ports2b[i][j]);
                
                a_mod.in_ports_p2[i][j]->bind(in_ports_p2[i][j]);
                a_mod.out_ports_p2[i][j]->bind(out_ports_p2[i][j]);
            }
        }
    }
};

int sc_main(int argc, char *argv[]) {
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}

