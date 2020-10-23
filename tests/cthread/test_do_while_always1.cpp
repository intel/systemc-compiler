//
// Created by Huzaifa Rampurawala on 9/9/19.
//

#include <systemc.h>

// do/whjle additional tests
class top : sc_module
{
public:
    sc_clock clk{"clk", 10, SC_NS};
    sc_signal<bool> arstn{"arstn", 1};
    sc_signal<int> out{"out"};
    sc_signal<int> in{"in"};

    SC_HAS_PROCESS(top);
    top(sc_module_name)
    {
        SC_THREAD(dowhile_forever);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(for_ever);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
        
    }

    // @do..while with wait
    void dowhile_forever()
    {
        out = 0;
        wait();
        
        do {             // B7

            int i = 0;          // B6
            do {
                out = 1;        // B4    
                wait();  // 2
                i++;
            } while (i < 3);    // B3, B5
            
            out = 2;            // B2, B1
        } while(1);
    }
    

    // @while with inner @for 
    void for_ever()
    {
        out = 0;
        wait();
        
        for (;;) {

            int i = 0;
            do {
                i++;
                out = 1;
                
                for (int j = 0; j < 2; j++) {
                    if (in.read() > 1) {
                        out = j;
                    }
                    wait();  // 2
                }
            } while (i < 3);
            out = 3;
            wait();     // 3
        }
    }

};

int sc_main(int argc, char *argv[])
{
    top top_inst{"top_inst"};
    sc_start(100, SC_NS);
    return 0;
}
