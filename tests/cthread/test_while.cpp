//
// Created by ripopov on 3/13/18.
//

#include <systemc.h>

class top : sc_module
{
public:
    sc_clock clk{"clk", 10, SC_NS};
    sc_signal<bool> arstn{"arstn", 1};
    sc_signal<int> in{"in"};
    sc_signal<int> out{"out"};

    SC_HAS_PROCESS(top);
    top(sc_module_name)
    {
        SC_THREAD(while_with_wait0);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(while_with_wait0a);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
          
        SC_THREAD(while_with_wait1);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(while_with_wait2);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(while_with_for);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(while_with_signal_cond);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
        
        SC_THREAD(while_with_binary_oper);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(while_with_binary_oper1);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(while_with_binary_oper2);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_THREAD(while_with_binary_oper3);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
        // TODO: Fix me, #133
        //SC_THREAD(no_wait_main_loop);
        //sensitive << clk.posedge_event();
        //async_reset_signal_is(arstn, false);
    }

    // @while with wait
    void while_with_wait0()
    {
        wait();
        
        while (1) { // B6

            int i = 0;  // B5
            while (i < 3) { // B4
                wait();     // B3
                i++;
            }           // B2
        }   // B1
    }
    
    // @while with wait and global iteration counter
    void while_with_wait0a()
    {
        int i = 0;  
        wait();
        
        while (1) { 
            while (i < 3) {
                wait();     
                i++;
            }
            wait();
        }   
    }
    
    // @while with wait
    void while_with_wait1()
    {
        out = 0;
        wait();
        
        while (1) {

            int i = 0;
            while (i < 3) {
                i++;
                out = 1;
                wait();     // 2
            }
            out = 2;
            wait();     // 3
        }
    }
    
    // @while with conditional wait
    void while_with_wait2()
    {
        out = 0;
        wait();
        
        while (1) {

            int i = 0;
            while (i < 3) {
                i++;
                out = 1;
                wait();     // 2
                
                if (in.read() > 1) {
                    out = 2;
                    wait();  // 3
                }
            }
            out = 3;
            wait();     // 4
        }
    }
    
    // @while with inner @for 
    void while_with_for()
    {
        out = 0;
        wait();
        
        while (1) {

            int i = 0;
            while (i < 3) {
                i++;
                out = 1;
                
                for (int j = 0; j < 2; j++) {
                    if (in.read() > 1) {
                        out = j;
                    }
                    wait();  // 2
                }
            }
            out = 3;
            wait();     // 3
        }
    }

    // @while with signal condition
    void while_with_signal_cond()
    {
        out = 0;
        wait();
        
        while (1) {

            while (in.read()) {
                out = 1;
                wait();     // 1
            }

            out = 2;
            wait();     // 2
        }
    }

    // While with binary ||/&& operator -- BUG in HS SMEM EMC
    void while_with_binary_oper()
    {
        bool b1, b2;
        int k = 0;
        wait();
        
        while (1) {             // B7
            while (b1 || b2) {  // B6, B5
                k = 1;        // B4
                wait();
                k = 2;
            }                   // B3
            wait();             // B2, B1
        }
    }
    
    void while_with_binary_oper1()
    {
        bool b1, b2;
        int k = 0;
        wait();
        
        while (1) {             
            while (b1 && b2) {  
                k = 1;          
                wait();
                k = 2;
            }                   
            wait();             
        }
    }
    
    // While with binary ||/&& operator -- BUG in HS SMEM EMC fixed
    void while_with_binary_oper2()
    { 
        bool b1, b2, b3;
        int k = 0;
        wait();     // B9
        
        while (1) {         // B8
            while ((b1 || b2) && b3) {  // B7, B6, B5
                k = 1;
                wait();     // B4
                k = 2;
            }               // B3
            wait();         // B2
        }
    }
    
    void while_with_binary_oper3()
    { 
        bool b1, b2, b3;
        int k = 0;
        wait();     
        
        while (1) { 
            while ((b1 && b2) || b3) { 
                k = 1;
                wait();     
                k = 2;
            }               
            wait();         
        }
    }
            
    
    // Main loop w/o wait(), see #133
    sc_signal<int> s;
    void no_wait_main_loop()
    {
        s = 0;
        wait();
        
        while (true) { 
            s = 1;
        } 
    }
        
};

int sc_main(int argc, char *argv[])
{
    top top_inst{"top_inst"};
    sc_start(100, SC_NS);
    return 0;
}
