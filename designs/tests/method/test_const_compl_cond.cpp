/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <iostream>
#include <sct_assert.h>
#include "systemc.h"


// IF with false and true constant condition lead to unreachable code which 
// is non-dead code assertion -- removed now
class A : public sc_module 
{
public:
    static const bool USE_TRANS_INTERLEAVE_F = false;
    static const bool USE_TRANS_INTERLEAVE_T = true;
    static const unsigned EXTRA_TAG_WIDTH = 4;
    
    static const unsigned TAG_WIDTH = 4;
    static const unsigned PORT_TAG_WIDTH = 4;
    static const unsigned REORDER_TAG_WIDTH = 1;
    
    static const unsigned ACTIVE_TRANS_NUM = 2;
    static const unsigned ACTIVE_BEAT_NUM = 4;
    static const unsigned ACTIVE_TRANS_NUM_WIDTH = 1;
    static const unsigned ACTIVE_BEAT_NUM_WIDTH = 2;

    sc_signal<bool> s;

    SC_CTOR(A) {
        SC_METHOD(parseResponseProc0); 
        sensitive << s;

        SC_METHOD(parseResponseProc1); 
        sensitive << s;

        SC_METHOD(parseResponseProc2); 
        sensitive << resp_fifo_out;
        for (int i = 0; i < ACTIVE_TRANS_NUM; ++i) {
            sensitive << active_trans[i];
            if (EXTRA_TAG_WIDTH) {
                for (int j = 0; j < ACTIVE_BEAT_NUM; ++j) {
                    sensitive << beat_extra_tags[i][j];
                }
            }
        }
    }

    typedef sc_uint<1> ExtraTag_t;
    typedef sc_uint<REORDER_TAG_WIDTH> ReordTag_t;
    typedef sc_uint<2> BeatId_t;
    typedef sc_uint<ACTIVE_TRANS_NUM_WIDTH> TransIndex_t;
    
    sc_signal<bool> last_beat_resp;
    
    // False condition
    void parseResponseProc0()
    {
        if (USE_TRANS_INTERLEAVE_F || !EXTRA_TAG_WIDTH) {
            last_beat_resp = 1;  
        } else {
            last_beat_resp = 0; 
        }
    }
    
    // True condition
    void parseResponseProc1()
    {
        if (USE_TRANS_INTERLEAVE_T || !EXTRA_TAG_WIDTH) {
            last_beat_resp = 1;  // B2
        } else {
            last_beat_resp = 0; 
        }
    }
    
    // Example from real design FlowControl module
    sc_signal<sc_uint<REORDER_TAG_WIDTH + 1> > active_trans[ACTIVE_TRANS_NUM];
    sc_signal<ExtraTag_t> beat_extra_tags[ACTIVE_TRANS_NUM][ACTIVE_BEAT_NUM];
    sc_signal<ExtraTag_t>  master_resp_tag;
    sc_signal<sc_uint<42>>  resp_fifo_out;
    
    void parseResponseProc2()
    {
        sc_uint<TAG_WIDTH> respTag = (sc_uint<TAG_WIDTH>)resp_fifo_out.read().
                                     range(TAG_WIDTH + 1, 2);
        TransIndex_t transIndex = respTag.range(TAG_WIDTH - 1, 
                                  ACTIVE_BEAT_NUM_WIDTH);
        ReordTag_t transId = active_trans[transIndex].read().range(
                             REORDER_TAG_WIDTH - 1, 0);

        ExtraTag_t extraTag = 0;
        if (EXTRA_TAG_WIDTH > 0) {
            BeatId_t beatId = respTag.range(ACTIVE_BEAT_NUM_WIDTH - 1, 0);
            extraTag        = beat_extra_tags[transIndex][beatId];
            master_resp_tag = (extraTag, transId);
        } else {
            master_resp_tag = transId;
        }

        if (USE_TRANS_INTERLEAVE_T || !EXTRA_TAG_WIDTH) {
            last_beat_resp = 1;  // not used
        } else {
            assert(EXTRA_TAG_WIDTH > 0);
            // Bit 0 contain last beat flag if it is required
            last_beat_resp = extraTag.bit(0);
        }
    }
};

int sc_main(int argc, char *argv[])
{
    A top_inst{"top_inst"};
    sc_start();
    return 0;
}

