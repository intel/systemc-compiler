#include <systemc.h>
#include "AdvFifo.h"
#include "sct_assert.h"

template<unsigned N>
struct Dut : public sc_module 
{
    typedef sc_uint<N> T;
    
    sc_in_clk       clk{"clk"};
    sc_in<bool>     nrst{"nrst"};
    
    sc_in<T>        in{"in"};
    sc_out<T>       out{"out"};
    
    adv_fifo_mif<T, 1, 1, 1, 6, 3, 1> fifo{"fifo"};

    SC_CTOR(Dut) {
        fifo.clk_nrst(clk, nrst);
        
        SC_CTHREAD(reqProc, clk.pos());
        async_reset_signal_is(nrst, 0);

        SC_CTHREAD(respProc, clk.pos());
        async_reset_signal_is(nrst, 0);

    }
    
    void reqProc() {
        wait();
        
        while(true) {
            if (fifo.ready()) {
                fifo.push(in.read());
            }
            wait();
        }
    }
    
    void respProc() {
        wait();
        
        while(true) {
            if (fifo.valid()) {
                out = fifo.pop();
            }
            wait();

        }
    }
};


SC_MODULE(Tb) {
    sc_in_clk clk{"clk"};
    sc_signal<bool> nrst{"nrst"};

    typedef sc_uint<32> T;
    sc_signal<T>    in{"in"};
    sc_signal<T>    out{"out"};
    
    Dut<32>  dut{"dut"};
    
    SC_CTOR(Tb) {
        dut.clk(clk);
        dut.nrst(nrst);
        dut.in(in);
        dut.out(out);
        
        SC_CTHREAD(tests, clk.pos());
    }
    
    const unsigned N = 4000000;
    
    void tests() {
        nrst = 0;
        wait(10);
        nrst = 1;
        
        for (int i = 0; i < N; i++) {
            in = i;
            wait();
            
            if (out.read() > in.read()) {
                cout << "in " << in.read() << " out " << out.read() << endl;
                assert (false && "error");
            }
        }
        
        sc_stop();
    }
};

int sc_main(int argc, char **argv) 
{
    sc_clock clk("clk", sc_time(1, SC_NS));
    Tb tb{"tb"};
    tb.clk(clk);
    
    sc_start();

    return 0;
}