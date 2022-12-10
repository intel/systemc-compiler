/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>
#include "sct_assert.h"

// Set NO_VALUE for record array and array of records at unknown index access
SC_MODULE(Top) 
{
    sc_signal<int> sig;
    
    SC_CTOR(Top) 
    {
        SC_METHOD(setUnknownTest1); sensitive << sig;
        SC_METHOD(setUnknownTest2); sensitive << sig;
        //SC_METHOD(setUnknownTest3); sensitive << sig;   // #141
    }
    
    struct Inner {
        int c;
    };
    struct UDSimple {
        Inner  rec;
        Inner  rec_arr[2];
    };

    UDSimple su[2];
    UDSimple ssu[2][2];
    void setUnknownTest1() 
    {
        su[1].rec.c = 34;
        su[sig].rec.c = 35;
        sct_assert_unknown(su[1].rec.c);

        ssu[1][1].rec.c = 34;
        ssu[sig][1].rec.c = 35;
        sct_assert_unknown(ssu[1][1].rec.c);
        
        ssu[1][0].rec.c = 34;
        ssu[1][sig].rec.c = 35;
        sct_assert_unknown(ssu[1][0].rec.c);

        ssu[0][1].rec.c = 34;
        ssu[sig][sig].rec.c = 35;
        sct_assert_unknown(ssu[0][1].rec.c);
    }
    
    UDSimple ru[2];
    UDSimple rru[2][2];
    void setUnknownTest2() 
    {
        ru[1].rec_arr[1].c = 34;
        ru[sig].rec_arr[1].c = 35;
        sct_assert_unknown(ru[1].rec_arr[1].c);
        
        rru[0][1].rec_arr[1].c = 34;
        rru[sig][1].rec_arr[1].c = 35;
        sct_assert_unknown(rru[0][1].rec_arr[1].c);

        rru[1][0].rec_arr[0].c = 34;
        rru[1][sig].rec_arr[0].c = 35;
        sct_assert_unknown(rru[1][0].rec_arr[0].c);

        rru[0][1].rec_arr[1].c = 34;
        rru[0][1].rec_arr[sig].c = 35;
        sct_assert_unknown(rru[0][1].rec_arr[1].c);

        rru[0][1].rec_arr[1].c = 34;
        rru[sig][1].rec_arr[sig].c = 35;
        sct_assert_unknown(rru[0][1].rec_arr[1].c);

        rru[0][0].rec_arr[1].c = 34;
        rru[sig][sig].rec_arr[sig].c = 35;
        sct_assert_unknown(rru[0][0].rec_arr[1].c);
    }
    
    // Incorrect code generated here, #141
    UDSimple tu;
    UDSimple ttu[2];
    void setUnknownTest3() 
    {
        ttu[1].rec.c = 34;
        ttu[0].rec_arr[1].c = 35;
        sct_assert_const(ttu[1].rec.c == 34);
        sct_assert_const(ttu[0].rec_arr[1].c == 35);
        
        ttu[sig] = tu;
        sct_assert_unknown(ttu[1].rec.c);
        sct_assert_unknown(ttu[0].rec_arr[1].c);
    }
};

int sc_main(int argc, char **argv) 
{
    Top top{"top"};
    sc_start();

    return 0;
}
