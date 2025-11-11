/*
 * Simple METHOD test with SYNC option in initiator
 */

#ifndef DUT_TEST_H
#define DUT_TEST_H

#include "sct_common.h"
#include "sct_assert.h"
#include <systemc.h>

// Simple module with target/initiator and in/out ports to generate portmap
template<class T, class TI, class TO>
struct Dut : public sc_module 
{
    sc_in<bool>         clk{"clk"};
    sc_in<bool>         nrst{"nrst"};

    sct_target<T>       run{"run"};
    sct_initiator<T>    resp{"resp"};
    
    sct_in<TI>              in{"in"};
    sct_out<TO>             out{"out"};
    sc_vector<sct_out<T>>   vo{"vo", 2};
    
    //SC_HAS_PROCESS(Dut);
  
    SC_CTOR(Dut)
    {
        run.clk_nrst(clk, nrst);
        resp.clk_nrst(clk, nrst);

        SC_METHOD(methProc);
        sensitive << run << resp << in;
    }
    
    void methProc() {
        out = 0; 
        run.reset_get();
        resp.reset_put();
        vo[0] = 0; vo[1] = 1;
        
        if (run.request() && resp.ready()) {
            resp.put(run.get());
            out = TO(run.peek().to_uint());
        } else {
            out = TO(in.read());
        }
    }
};


#endif /* DUT_TEST_H */

