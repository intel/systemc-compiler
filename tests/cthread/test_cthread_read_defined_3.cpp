/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

//
// Created by ripopov on 12/4/18.
//

#include "systemc.h"
#include <sct_assert.h>

// Channels and channel arrays defined and read tests
class A : public sc_module
{
public:
    sc_signal<bool>         clk{"clk"};
    sc_signal<bool>         nrst{"nrst"};
    
    sc_signal<unsigned> s;

    SC_CTOR(A)
    {
        SC_CTHREAD(write_sig_thread, clk);
        async_reset_signal_is(nrst, false);
        
        SC_CTHREAD(read_sig_thread, clk);
        async_reset_signal_is(nrst, false);

        bsignal_ptr_arr[0] = new sc_signal<bool>("bsig_0");
        bsignal_ptr_arr[1] = new sc_signal<bool>("bsig_1");
        
        csignal_ptr_arr[0] = new sc_signal<bool>("csig_0");
        csignal_ptr_arr[1] = new sc_signal<bool>("csig_1");
    }

    sc_signal<bool>  bsignal{"bsignal"};
    sc_signal<bool>  bsignal_array[2];
    sc_signal<bool>  bsignal_array2d[2][2];
    sc_signal<bool> *bsignal_ptr = new sc_signal<bool>("bsignal_ptr");
    sc_signal<bool> *bsignal_array_ptr = new sc_signal<bool>[2];
    sc_signal<bool> *bsignal_ptr_arr[2];
    
    sc_signal<bool>  csignal{"csignal"};
    sc_signal<bool>  csignal_array[2];
    sc_signal<bool>  csignal_array2d[2][2];
    sc_signal<bool> *csignal_ptr = new sc_signal<bool>("csignal_ptr");
    sc_signal<bool> *csignal_array_ptr = new sc_signal<bool>[2];
    sc_signal<bool> *csignal_ptr_arr[2];

    void write_sig_thread() 
    {
        bsignal = false;
        bsignal_array[s.read()] = false;
        bsignal_array2d[s.read()][1] = false;
        bsignal_ptr->write(false);
        bsignal_array_ptr[1] = false;
        bsignal_ptr_arr[0]->write(false);
        
        sct_assert_defined(bsignal);
        sct_assert_defined(*bsignal_ptr);
        sct_assert_array_defined(bsignal_array);
        sct_assert_array_defined(bsignal_array2d);
        sct_assert_array_defined(bsignal_array_ptr);
        sct_assert_array_defined(*bsignal_ptr_arr);
        wait();

        while (1) 
        {
            bsignal = true;
            bsignal_array[0] = bsignal;
            bsignal_array2d[1][s.read()] = true;
            bsignal_ptr->write(bsignal_array[s.read()]);
            bsignal_array_ptr[s.read()] = bsignal_ptr->read();
            bsignal_ptr_arr[s.read()+1]->write(bsignal_array_ptr[0]);

            sct_assert_defined(bsignal);
            sct_assert_read(bsignal);
            sct_assert_array_defined(bsignal_array);
            sct_assert_read(bsignal_array);
            sct_assert_array_defined(bsignal_array2d);
            sct_assert_read(bsignal_array2d, false);
            sct_assert_array_defined(bsignal_array_ptr);
            sct_assert_read(bsignal_array_ptr);
            sct_assert_array_defined(bsignal_ptr_arr);
            sct_assert_read(bsignal_ptr_arr, false);
            wait();
        }
    }
    
    void read_sig_thread() 
    {
        csignal = false;
        if (s) {
            csignal_array[0] = false;
        }
        csignal_array2d[1][0] = false;
        csignal_ptr->write(false);
        csignal_array_ptr[1] = false;
        csignal_ptr_arr[0]->write(false);
        
        sct_assert_defined(csignal);
        sct_assert_defined(*csignal_ptr);
        sct_assert_array_defined(csignal_array);
        sct_assert_array_defined(csignal_array_ptr);
        sct_assert_read(csignal_array_ptr, false);
        wait();
        
        while (1) {
            
            bool b = csignal;
            csignal = !csignal;
            sct_assert_defined(csignal);
            sct_assert_read(csignal);
            
            b = csignal_array[0];
            csignal_array[0] = csignal_array[1];
            csignal_array[1] = csignal_array[0];
            sct_assert_array_defined(csignal_array);
            sct_assert_read(csignal_array);
            
            b = csignal_array2d[1][0];
            csignal_array2d[1][0] = csignal;
            sct_assert_array_defined(csignal_array2d);
            sct_assert_read(csignal_array2d);
            
            wait();
            b = csignal;
            sct_assert_register(csignal);
            sct_assert_defined(csignal, false);
            sct_assert_read(csignal);

            csignal_ptr->write(b);
            b = csignal_ptr->read();
            sct_assert_register(*csignal_ptr, false);
            sct_assert_defined(*csignal_ptr);
            sct_assert_read(*csignal_ptr);

            if (s.read()) {
                b = csignal_array_ptr[s.read()];
                csignal_array_ptr[s.read()] = !b;
            }
            sct_assert_array_defined(csignal_array_ptr);
            sct_assert_read(csignal_array_ptr);
            sct_assert_register(csignal_array_ptr);
            
            while (s.read()) {
                b = *csignal_ptr_arr[csignal_array_ptr[0]];
                csignal_ptr_arr[csignal_array2d[0][s.read()]]->write(true);
            
                sct_assert_array_defined(csignal_ptr_arr);
                sct_assert_read(csignal_ptr_arr);
                wait();
            }
        }
    }
};


int sc_main(int argc, char* argv[])
{
    A a_mod{"b_mod"};
    sc_start();
    return 0;
}

