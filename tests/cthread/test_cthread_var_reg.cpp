#include <systemc.h>

// Member and local channels and variables declaration as registers

// Variable which should become registers:
//  1) used/defined in one thread,
//  2) defined in one thread and used in another thread/method
struct A : public sc_module
{
    sc_in_clk               clk;
    sc_in<bool>             rst;

    sc_in<int>               in;     
    sc_out<int>              out;
    sc_signal<bool>          a;
    sc_signal<sc_uint<4>>    b;
    sc_signal<int>           c;
    sc_signal<sc_bigint<33>> d;
    sc_signal<unsigned>      e;    // Register, besides it is defined before use
    sc_signal<sc_uint<4>>    f;
    sc_signal<sc_uint<4>>    g;
    sc_signal<sc_uint<4>>    h; 
    sc_signal<sc_uint<4>>    arr[3];

    SC_CTOR(A) 
    {
        SC_METHOD(channels0);
        sensitive << a << b << c;

        SC_CTHREAD(channels1, clk.pos());
        async_reset_signal_is(rst, true);

        SC_CTHREAD(channels2, clk.pos());
        async_reset_signal_is(rst, true);

        SC_CTHREAD(variables1, clk.pos());
        async_reset_signal_is(rst, true);

        SC_CTHREAD(variables2, clk.pos());
        async_reset_signal_is(rst, true);
    }

    void channels0() 
    {
        bool x = a;
        sc_uint<4> y = b.read() + c;
        h = y;
    }
    
    // Channels and ports
    void channels1() 
    {
        a = 1;
        b = 0;
        out = 0;
        wait();
        
        while (true) {
            b = 42;
            c = 43;
            d = 44;
            out = in;
            e = 0;
            wait();
            
            e = 1;
            c = d.read().to_int() + e.read();
        }
    }
    
    // Channels and ports accessed at some paths
    void channels2() 
    {
        f = 0;      // RnD
        g = 0;      // WO
        arr[0] = 1;
        for (int i = 1; i < 3; ++i) {
            arr[i] = i;
        }
        wait();
        
        while (true) {
            if (a) {
                g = f;
            }
            f = arr[1];
            arr[b.read()] = in.read();
            wait();
        }
    }

    // Local and member variables 
    bool k;                     // assigned in reset only
    unsigned m;                 // RnD at one path only
    sc_uint<4> n;               // not assigned in reset
    sc_uint<4> p;               // WO, not reg
    void variables1() {
        sc_uint<4> x = 1;
        k = false;              
        m = 0; 
        wait();
        
        while (true) {
            sc_uint<4> z = b.read() + n; // RnD at some paths    
            int t = m;                   // RnD 
            p = n;
            
            wait();
            
            if (a) z = 0;   
            n = z + (k ? t : 0);
            m++;
        }
    }
    
    // Variables accessed at some paths
    sc_uint<1> arrv[3];
    sc_signal<int> s1;
    sc_uint<4> w;
    void variables2() {
        arrv[0] = 1;
        sc_uint<2> r = a.read() ? arrv[0] : (sc_uint<1>)0;
        w = 0;
        s1 = 0;
        wait();
        
        while (true) 
        {
            if (b.read()) {
                auto v = r >> arrv[b.read()];
                while (c.read()) {
                    s1 = w + v;
                    wait();
                }

                s1 = v; 
            }
            wait();
        }
    }
    
};

SC_MODULE(Top) {

    sc_in_clk       clk{"clk"};
    sc_signal<bool> rst;
    
    A               a_mod{"a_mod"};
    sc_signal<int>  t;      

    SC_CTOR(Top) 
    {
        a_mod.clk(clk);
        a_mod.rst(rst);
        a_mod.in(t);
        a_mod.out(t);
    }
};

int sc_main(int argc, char **argv) 
{
    sc_clock clk {"clk", sc_time(1, SC_NS)};
    Top top{"top"};
    top.clk(clk);
    
    sc_start();

    return 0;
}