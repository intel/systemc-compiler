#include <systemc.h>
#include "AdvFifo.h"

template<unsigned N>
struct Dut : public sc_module 
{
    typedef sc_uint<N> T;
    
    sc_in_clk       clk{"clk"};
    sc_in<bool>     nrst{"nrst"};
    
    sc_in<T>        in{"in"};
    sc_out<T>       out{"out"};
    
    adv_fifo_mif<T, 0, 0, 0, 6, 3, 1> fifo{"fifo"};

    SC_CTOR(Dut) {
        fifo.clk_nrst(clk, nrst);
        
        SC_METHOD(reqProc);
        fifo.addTo(sensitive);
        sensitive << in;

        SC_METHOD(respProc);
        fifo.addTo(sensitive);

    }
    
    void reqProc() {
        if (fifo.ready()) {
            fifo.push(in.read());
            cout << sc_time_stamp() << " push " << in.read() << endl;
        } else {
            fifo.push(0, 0);
        }
    }
    
    void respProc() {
        if (fifo.valid()) {
            T data = fifo.pop();
            out = data;
            cout << sc_time_stamp() << " pop " << data << endl;
        } else {
            fifo.pop(0);
            out = 0;
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
    
    const unsigned N = 10;
    
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