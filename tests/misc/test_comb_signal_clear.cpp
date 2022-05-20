/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_comb_signal.h"
#include <systemc.h>
#include <string>

// Combinational signal with clear true and clear signal tests, 
// check `{default:0} generated for array only
struct comb_signal_module : public sc_module
{
    sc_in<bool>             clk{"clk"};
    sc_in<bool>             nrst{"nrst"};

    using T= sc_uint<3>;
    
    static const unsigned N = 2;
    typedef sct_comb_signal<bool, 1> CombSig_t;
    CombSig_t    css;
    CombSig_t*   csp;
    CombSig_t    csa[N];
    CombSig_t*   cspa[N];

    typedef sct_clear_signal<sc_uint<3>> ClearSig_t;
    ClearSig_t    ccs;
    ClearSig_t*   ccp;
    ClearSig_t    cca[N];
    ClearSig_t*   ccpa[N];
    sc_vector<ClearSig_t> ccv{"ccv", 3};
    
    
    SC_CTOR(comb_signal_module) 
    {
        csp = new CombSig_t("csp");
        ccp = new ClearSig_t("ccp");
        for (int i = 0; i < N; i++) {
            cspa[i] = new CombSig_t("cspa");
            ccpa[i] = new ClearSig_t("ccpa");
        }
        
        SC_CTHREAD(thrdProc, clk.pos());
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(clearThrdProc, clk.pos());
        async_reset_signal_is(nrst, 0);
        
        SC_CTHREAD(allCombThread, clk.pos());
        async_reset_signal_is(nrst, 0);

        SC_METHOD(allCombMethod);
        sensitive << css;
    }
    
    void thrdProc() 
    {
        css = 1;
        csa[0] = 1;
        *csp = 1;
        *cspa[0] = 1;
        
        wait();             // 0
        
        while (true) {
            css = 5;
            csa[0] = 7;
            
            *csp = 6;
            for (int i = 0; i < N; i++) {
                csa[i] = 7;
                *cspa[i] = 8;
            }
            wait();         // 1
        }
    }

    void clearThrdProc() 
    {
        ccs = 1;
        cca[0] = 1;
        *ccpa[0] = 1;
        *ccp = 1;
        ccv[0] = 1;
        
        wait();             // 0
        
        while (true) {
            ccs = 5;
            cca[0] = 7;
            
            *ccp = 6;
            for (int i = 0; i < N; i++) {
                cca[i] = 7;
                *ccpa[i] = 8;
                ccv[i+1] = i;
            }
            wait();         // 1
        }
    }
    
    // Compare all kind of comb/clear and normal signals
    sc_signal<T> s{"s"};
    sct_comb_signal<T, 0> cms{"cms"};
    sct_comb_signal<T, 1> cmc{"cmc"};
    sct_clear_signal<T> cc{"cc"};
    
    void allCombThread() {
        s = 1;
        cms = 1;
        cmc = 1;
        cc = 1;
        wait();
        
        while (1) {
            s = 2;
            cms = 2;
            cmc = 2;
            cc = 2;
            wait();
        }
    }   
    
    sc_signal<T> s_{"s"};
    sct_comb_signal<T, 0> cms_{"cms_"};
    sct_comb_signal<T, 1> cmc_{"cmc_"};
    sct_clear_signal<T> cc_{"cc"};
    
    void allCombMethod() {
        s_ = 1;
        cms_ = 1;
        cmc_ = 1;
        cc_ = 1;
        
        if (css.read()) {
            s_ = 2;
            cms_ = 2;
            cmc_ = 2;
            cc_ = 2;
        }
    }
};

struct testbench : public sc_module
{
    sc_in<bool>         clk{"clk"};
    sc_signal<bool>     nrst{"nrst"};
    comb_signal_module  mod{"mod"};
    
    SC_CTOR(testbench) 
    {
        mod.clk(clk);
        mod.nrst(nrst);
        SC_CTHREAD(test, clk.pos());
    }
    
    void test() {
        nrst = 0;
        wait(5); 
        
        nrst = 1;
        wait(10);   
        
        sc_stop();
    }
};


int sc_main(int argc, char **argv) {

    sc_clock clock_gen{"clock_gen", 1, SC_NS};
    testbench tb{"tb"};
    tb.clk(clock_gen);
    sc_start();

    return 0;
}

