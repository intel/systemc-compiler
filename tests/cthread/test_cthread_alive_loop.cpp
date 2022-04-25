/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include <systemc.h>

// Loop with wait() with unknown number of iterations, use @SCT_ALIVE_LOOP
class Top : sc_module
{
public:
    sc_in<bool>     clk;
    sc_signal<bool> rstn;
    
    sc_signal<int>  s;
    sc_signal<int>  t;

    SC_HAS_PROCESS(Top);
    
    Top(sc_module_name)
    {
        SC_METHOD(bugMethProc);
        sensitive << s;
        
        SC_CTHREAD(stSmemPullFsmProc1, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(stSmemPullFsmProc2, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(stSmemPullFsmProc3, clk.pos());
        async_reset_signal_is(rstn, false);
        

        SC_CTHREAD(for_workaround, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(dowhile_workaround, clk.pos());
        async_reset_signal_is(rstn, false);
        

        SC_CTHREAD(for_alive1, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(for_alive2, clk.pos());
        async_reset_signal_is(rstn, false);
        
        SC_CTHREAD(for_alive3, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(for_alive4, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(for_alive5, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(for_alive6, clk.pos());
        async_reset_signal_is(rstn, false);
        
        
        SC_CTHREAD(while_alive1, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(while_alive2, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(while_alive3, clk.pos());
        async_reset_signal_is(rstn, false);
                
    }
    
    // Bug in Decoma DME related to function call in evaluateConstInt()
    // that leads to unintended initialization of @lastContext
    int f(int j) {
        return (j+1);
    }
    
    typedef sc_uint<4> TileDim_t;
    void bugMethProc()
    {
        int k = 0;                  // B8
        
        for (int i = 0; i < 1; ++i) {           // B7
            const TileDim_t N0 = f(s.read());   // B6
            TileDim_t x0 = 0;
    
            do {                    // B5   
                k++;                // B4
            } while (x0 <= N0);     // B3
            k++;                    // B2
        }                           // B1
    }    
    
    void stSmemPullFsmProc1()
    {
        int k = 0;
        wait();

        while (true) {
            const TileDim_t N0 = f(s.read());
 
            TileDim_t x0 = 0;
            do {
                k = 1;
                wait();

            } while (x0 <= N0);
            
            wait();
        }
    }
    
    void stSmemPullFsmProc2()
    {
        int k = 0;
        wait();

        while (true) {
            k = 1;
            wait();
            k = 2;
           
            const TileDim_t N3 = s.read();
            const TileDim_t N0 = f(s.read());
 
            TileDim_t x3 = 0;
            do {
                TileDim_t x0 = 0;
                do {
                    k = 3;
                    wait();             // 2
                    
                } while (x0 <= N0);
                x3 += 1;
            } while (x3 <= N3);
            
            wait();
            k = 4;
        }
    }

    void stSmemPullFsmProc3()
    {
        int k = 0;
        wait();

        while (true) {
            // wait until slot becomes available for processing
            while (s.read() || t.read()) {
                wait();                         // 1
            }
            k = 1;
            wait();                             // 2
            k = 2;
           
            // cout << "stSmemPullFsmProc(): x3=" << x3 << ", N3=" << N3 << endl;
            TileDim_t x2 = 0;
            const TileDim_t N2 = s.read();
            do {
                // cout << "\tstSmemPullFsmProc(): x2=" << x2 << ", N2=" << N2 << endl;
                TileDim_t x1 = 0;
                const TileDim_t N1 = s.read();
                do {
                    // cout << "\t\tstSmemPullFsmProc(): x1=" << x1 << ", N1=" << N1 << endl;
                    TileDim_t x0 = 0;
                    const TileDim_t N0 = s.read();
                    do {
                        // cout << "\t\t\tstSmemPullFsmProc(): x0=" << x0 << ", N0=" << N0 << endl;
                        k = s.read();
                        wait();                         // 3

                        k = s.read()+1;

                        // wait till memory request is accepted
                        while(!s.read()) wait();        // 4

                        k = 3;

                    } while (x0 <= N0);
                    x1 += 1;
                } while (x1 <= N1);
                x2 += 1;
            } while (x2 <= N2);
            
            wait();
            k = 4;
        }
    }
    
// ============================================================================    

    void for_workaround()
    {
        wait();
        
        while (1) {
            
            const int N = s.read();
            for (int j = 0; j < N; j++) {
                wait();
                for (int i = 1; i < N; i++) {
                    wait();   // 1
                }
            }
            
            wait();  // 2
        }
    }
    
    void dowhile_workaround()
    {
        wait();
        
        while (1) {
            const int M = s.read();
            const int N = s.read();
            for (int j = 0; j < M; j++) {
                int i = 0;
                do {
                    i++; wait();   // 1
                } while (i < N);
            }
            
            wait();  // 2
        }
    }
    
    // 1x alive loops
    void for_alive1()
    {
        wait();
        
        while (1) {
            
            const int M = s.read();
            const int N = s.read()+1;
            for (int j = 0; j < M; j++) {
                SCT_ALIVE_LOOP(
                for (int i = 0; i < N; i++) {
                    wait();   // 1
                });
            }
            
            wait();  // 2
        }
    }
    
    // 2x alive loops
    void for_alive2()
    {
        wait();
        
        while (1) {
            
            const int M = s.read();
            const int N = s.read()+1;
            for (int k = 0; k < M; k++) {
                SCT_ALIVE_LOOP(
                for (int j = 0; j < M; j++) {
                    SCT_ALIVE_LOOP(
                    for (int i = 0; i < N; i++) {
                        wait();   // 1
                    });
                });
            }
            
            wait();  // 2
        }
    }
    
    // With conditional wait()
    void for_alive3()
    {
        wait();
        
        while (1) {
            
            const int M = s.read();
            const int N = s.read()+1;
            for (int j = 0; j < M; j++) {
                if (s.read()) {
                    wait();     // 1
                }
                
                SCT_ALIVE_LOOP(
                for (int i = 0; i < N; i++) {
                    wait();     // 2
                });
                
            }
            
            wait();  // 3
        }
    }
    
    // In main loop w/o wait()
    void for_alive4()
    {
        const int N = s.read()+1;
        wait();
        
        while (1) {
            
            SCT_ALIVE_LOOP(
            for (int i = 0; i < N; i++) {
                wait();   // 1
            });
        }
    }
    
    
    // With other loops
    void for_alive5()
    {
        wait();
        
        while (1) {
            
            const int M = s.read();
            const int N = s.read()+1;
            for (int j = 0; j < M; j++) {
                
                SCT_ALIVE_LOOP(
                for (int i = 0; i < N; i++) {
                    wait();     // 1
                });
                
                for (int i = 0; i < N; i++) {
                    wait();     // 2
                }
            }
            
            wait();  // 3
        }
    }
    
    // Several alive loops
    void for_alive6()
    {
        const int M = s.read();
        const int N = s.read()+1;
        wait();
        
        while (1) {
            SCT_ALIVE_LOOP();
            for (int j = 0; j < M; j++) {
                
                SCT_ALIVE_LOOP(
                for (int i = 0; i < N; i++) {
                    wait();     // 1
                });
                
                SCT_ALIVE_LOOP(
                for (int i = 0; i < N; i++) {
                    wait();     // 2
                });
                
                if (s.read()) {
                   for (int i = 0; i < N; i++) {
                       wait();  // 3
                    } 
                }
            }
        }
    }
    
// ----------------------------------------------------------------------------
    
    void while_alive1()
    {
        const int M = s.read();
        wait();
        
        while (1) {
            
            for (int j = 0; j < M; j++) {
                int i = 0;
                SCT_ALIVE_LOOP(
                while (i < s.read()+1) {
                    wait();     // 1
                    i++;
                });
            }
            
            wait();  // 2
        }
    }    

    void while_alive2()
    {
        wait();
        
        while (1) {
            int j  = 0;
            SCT_ALIVE_LOOP(
            while (j < s.read()+2) {
                SCT_ALIVE_LOOP(
                int i = 0;
                while (i < s.read()+1) {
                    wait();     // 1
                    i++;
                });
                j++;
            });
        }
    }    
    
    void while_alive3()
    {
        const int N = s.read()+1;
        wait();
        
        while (1) {
            int j  = 0;
            while (j < s.read()+2) {
                if (s.read() == 42) {
                    SCT_ALIVE_LOOP(
                    for (int i = 0; i < N; i++) {
                        wait();     // 1
                    });
                } else {
                    SCT_ALIVE_LOOP(
                    int i = 0;
                    while (i < s.read()+1) {
                        wait();     // 2
                        i++;
                    });
                }
                j++;
            }
            
            wait();  // 3
        }
    } 
    

// ============================================================================
    // Loop with undefined number of iteration works itself but cannot be 
    // in another loop without wait();
    void for_noiter_error1()
    {
        wait();
        
        while (1) {
            
            const int N = s.read();
            for (int j = 0; j < 1; j++) {
                for (int i = 0; i < N; i++) {
                    wait();   
                }
            }
            
            wait();
        }
    }
    
//  The problem is in code generation for such a loop, as there is no stop
//  for loop unrolling and analysis hangs up
//    if (j < 1) {
//       a = 0;
//       if (i < N) SS = 1; return;
//       a = 1;
//       j++;
//       ???    // Need enter the loop again and again
//    }
//    
// Potentially it is possibly to generate SV with two cases : 
// 1) internal loop is dead 2) internal loop is alive
// But that is quite complicated.    
//    if (0 < N) {
//       if (j < 1) {
//          a = 0;
//          SS = 1; return;
//       } 
//    } else {
//       for (int j = 0; j < 1; j++) {...}
//    }
           
    // The same problem with conditional wait()
    void for_noiter_error2()
    {
        wait();
        
        while (1) {
            
            const int N = s.read();
            for (int j = 0; j < 1; j++) {
                if (N) wait();
            }
            
            wait();
        }
    }
};

int sc_main(int argc, char *argv[])
{
    sc_clock clk{"clk", 10, SC_NS};
    Top top_inst{"top_inst"};
    top_inst.clk(clk);
    sc_start(100, SC_NS);
    return 0;
}

