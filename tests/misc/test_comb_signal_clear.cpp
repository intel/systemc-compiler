#include "sct_comb_signal.h"
#include <systemc.h>
#include <string>

// Combinational signal clear test, check `{default:0} generated for array only
struct comb_signal_module : public sc_module
{
    sc_in<bool>             clk{"clk"};
    sc_in<bool>             nrst{"nrst"};

    static const unsigned N = 2;
    typedef sct_comb_signal<bool, 1> CombSig_t;
    CombSig_t    cs;
    CombSig_t*   csp;
    CombSig_t    csa[N];
    CombSig_t*   cspa[N];

    
    SC_CTOR(comb_signal_module) 
    {
        csp = new CombSig_t("csp");
        for (int i = 0; i < N; i++) {
            cspa[i] = new CombSig_t("cspa");
        }
        
        SC_CTHREAD(thrdProc, clk.pos());
        async_reset_signal_is(nrst, 0);
    }
    
    void thrdProc() 
    {
        wait();             // 0
        
        while (true) {
            cs = 5;
            csa[0] = 7;
            
            *csp = 6;
            for (int i = 0; i < N; i++) {
                csa[i] = 7;
                *cspa[i] = 8;
            }
            wait();         // 1
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
