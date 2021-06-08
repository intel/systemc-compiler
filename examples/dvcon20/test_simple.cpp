#include <systemc.h>
#include "sct_assert.h"

template<unsigned M>
struct Dut : public sc_module 
{
    typedef sc_uint<M> TT;
    
    sc_in_clk       clk{"clk"};
    sc_in<bool>     nrst{"nrst"};
    
    sc_in<TT>        a{"a"};
    sc_in<TT>        b{"b"};
    sc_out<TT>       sum{"b"};
    sc_out<bool>    crr{"crr"};

    SC_CTOR(Dut) {
        SC_CTHREAD(proc, clk.pos());
        async_reset_signal_is(nrst, 0);
        
        for (int i = 0; i < N; ++i) {
            actv[i].init(M);
        }
        
        SC_CTHREAD(thread_proc, clk.pos());
        async_reset_signal_is(nrst, 0);
    }
    
    //SCT_ASSERT(!nrst || b > a, clk.pos());
    //SCT_ASSERT(a.read(), SCT_TIME(1), b.read(), clk.pos());
    //SCT_ASSERT(a.read(), SCT_TIME(1, 3), b.read(), clk.pos());
    SCT_ASSERT(a.read(), SCT_TIME(10, 30), b.read(), clk.pos());
  
    void proc() {
        sum = 0;
        wait();
        
        while(true) {
            sc_uint<N+1> res = a.read() + b.read();
            wait();
            
            sum = res;
            crr = res.bit(N);
        }
    }
    
// DvCon example, listing 2
static const unsigned T = 3;
static const uint8_t N = 4;
/*sc_signal<bool> req{"req"};
sc_signal<bool> resp{"resp"};
sc_signal<sc_uint<8>> val{"val"};
sc_vector<sc_signal<bool>> enbl {"enbl", N};

SCT_ASSERT(req || !resp, clk.pos()); 
SCT_ASSERT(req, SCT_TIME(1), resp, clk.pos());
SCT_ASSERT(req, (2), val.read() == N, clk.neg());      
SCT_ASSERT(val.read() == 0, SCT_TIME(3,1), val.read() == 1, clk);  
SCT_ASSERT(enbl[0], (3,1), enbl[1], clk);  
SCT_ASSERT(!resp, SCT_TIME(T+1,T), resp, clk);  
*/

// DvCon example, listing 6
sc_vector<sc_signal<bool>> enbl {"enbl", N};
sc_vector<sc_vector<sc_signal<bool>>> actv{"actv", N};

void thread_proc() {
   // Reset section
   for (int i = 0; i < N; ++i) {
      SCT_ASSERT_LOOP(enbl[i], SCT_TIME(1), !enbl[i], i);
      for (int j = 0; j < M; ++j) {
         SCT_ASSERT_LOOP(actv[i][j], SCT_TIME(2), actv[i][M-j-1], i, j);
   }}
   wait();                        
   while (true) { 
      wait();
}}

};


SC_MODULE(Tb) {
    sc_in_clk clk{"clk"};
    sc_signal<bool> nrst{"nrst"};

    typedef sc_uint<32> T;
    sc_signal<T>    a{"a"};
    sc_signal<T>    b{"b"};
    sc_signal<T>    sum{"sum"};
    sc_signal<bool> crr{"crr"};
    
    Dut<32>  dut{"dut"};
    
    SC_CTOR(Tb) {
        dut.clk(clk);
        dut.nrst(nrst);
        dut.a(a);
        dut.b(b);
        dut.sum(sum);
        dut.crr(crr);
        
        SC_CTHREAD(tests, clk.pos());
    }
    
    const unsigned N = 4000000;
    
    void tests() {
        nrst = 0;
        wait(10);
        nrst = 1;
        
        for (int i = 0; i < N; i++) {
            a = i;
            b = i + 1;
            wait();
            
            if (sum.read() && sum.read() != 2*i-3) {
                cout << "sum " << sum.read() << " i " << i << endl;
                assert (false && "error");
            }
            if (crr) {
                cout << "is overflow " << endl;
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