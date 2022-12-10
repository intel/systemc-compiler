/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>
#include "sct_assert.h"

// Record with record array and array of records with record array inside
// accessed at unknown index
SC_MODULE(Top) {
    sc_signal<int> sig;
    
    SC_CTOR(Top) 
    {
        SC_METHOD(arrayRecord); sensitive << sig;
        SC_METHOD(useDefBug); sensitive << sig;
        SC_METHOD(useDefBug2D); sensitive << sig;
    }
    
    struct Pair {int x; int y;};
    Pair pa[2][2];
    Pair pa_[2][2];
    Pair pa__[2][2];
    Pair pa___[2][2];

    void arrayRecord() 
    {
        pa[1][0].x = 11;
        pa_[sig][1].x = 12;
        pa__[0][sig].x = 13;
        pa___[sig][sig].x = 14;
    }
    
    struct Inner {
        int c;
    };
    struct UDSimple {
        Inner  rec;
        Inner  rec_arr[2];
    };
    UDSimple at[2];
    UDSimple at_[2];
    UDSimple at__[2];
    UDSimple at___[2];
    
    void useDefBug() 
    {
        at[sig].rec.c = 30;
        at[1].rec_arr[0].c = 31;
        at_[sig].rec_arr[1].c = 32;
        at__[0].rec_arr[sig].c = 33;
        at___[sig].rec_arr[sig].c = 34;
    }
    
    struct D2Simple {
        Inner  rec_arr[2][3];
    };
    D2Simple dt[2][3];
    
    void useDefBug2D() 
    {
        dt[sig][sig].rec_arr[sig][sig].c = 34;
    }
};

int sc_main(int argc, char **argv) 
{
    Top top{"top"};
    sc_start();

    return 0;
}
