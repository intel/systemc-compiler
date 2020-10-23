#include "systemc.h"

using namespace sc_core;

// Variables name resolving
class A : public sc_module {
public:

    sc_in<bool>     clk{"clk"};
    sc_in<bool>     rst{"rst"};
    
    sc_signal<bool> a{"a"};
    sc_signal<bool> b{"b"};
    sc_signal<bool> c{"c"};
    int m;
    int k;
    
    
    SC_CTOR(A) {
        SC_HAS_PROCESS(A);
        
        SC_METHOD(local_varname); sensitive << a;

        SC_CTHREAD(doble_varname_func, clk.pos());
        async_reset_signal_is(rst, true);

        SC_CTHREAD(doble_varname_scope, clk.pos());
        async_reset_signal_is(rst, true);

        SC_CTHREAD(doble_varname_scope_reg, clk.pos());
        async_reset_signal_is(rst, true);

        SC_CTHREAD(doble_varname_scope_comb, clk.pos());
        async_reset_signal_is(rst, true);
        

        SC_CTHREAD(double_reg1, clk.pos());
        async_reset_signal_is(rst, true);

        SC_CTHREAD(double_reg2, clk.pos());
        async_reset_signal_is(rst, true);
    }
    
    // Method local variable name
    void local_varname()
    {
        int m = 1;  // Register
        int k = 2;  // Combinational
        int a;
    }
    
    // Two variables with the same name one in called function, 
    // Bug in HS SMEM in EMC
    void syncWaiting() {
        // Register
        sc_uint<3> check_hiwait_delay = a ? b.read() : c.read();
        wait();
        m = check_hiwait_delay;
    }
    void doble_varname_func() 
    {
        wait();
        
        while (true) {
            sc_uint<2> check_hiwait_delay = 1;
            m = check_hiwait_delay;
            
            syncWaiting();
            
            wait();
            k = m;
        }
    }
    
    // Register and combinational
    void doble_varname_scope() 
    {
        wait();
        
        while (true) {
            sc_uint<2> acheck_hiwait_delay = 1;
            m = acheck_hiwait_delay;
            
            if (a) {
                // Register
                sc_uint<3> acheck_hiwait_delay = 1;
                wait(); 
                m = acheck_hiwait_delay; 
            }
            
            wait();
        }
    }
    
    // Two registers
    void doble_varname_scope_reg() 
    {
        wait();
        
        while (true) {
            sc_uint<2> bcheck_hiwait_delay = 1;
            wait();
            m = bcheck_hiwait_delay;
            
            if (a) {
                // Register
                sc_uint<3> bcheck_hiwait_delay = 1;
                wait();
                m = bcheck_hiwait_delay; 
            }
            
            wait();
        }
    }
    
    // Two combinational
    void doble_varname_scope_comb() 
    {
        wait();
        
        while (true) {
            sc_uint<2> ccheck_hiwait_delay = 1;
            m = ccheck_hiwait_delay;
            
            if (a) {
                // Register
                sc_uint<3> ccheck_hiwait_delay = 1;
                m = ccheck_hiwait_delay; 
            }
            
            wait();
        }
    }

    //------------------------------------------------------------------------
    
    // Two processes with local variables with same name becomes registers
    sc_signal<bool> s1;
    void double_reg1() 
    {
        s1 = 1;
        sc_uint<3> varA;
        wait();
        
        while (true) {
            sc_bigint<4> varB = 1;
            
            if (a) {
                bool varC = b;
                wait();                     // 1
                s1 = varC ? varA : varB;
                varB = 3;
            }
            wait();                         // 2
            
            varA = varB;
            wait();                         // 3
        }
    }
    
    sc_signal<bool> s2;
    void double_reg2() 
    {
        s2 = 0;
        sc_uint<3> varA = 1;
        wait();
        
        while (true) {
            if (b) {
                sc_int<4> varB = a.read() ? 1 : 2;
                wait();                     // 1
                s2 = varB;
            }
            
            int varC = varA + 1;
            wait();                         // 2
            
            varA++;
            s2 = varC;
        }
    }
    
};

class B_top : public sc_module {
public:
    sc_in<bool>         clk{"clk"};
    sc_signal<bool>     rst{"rst"};
    sc_clock clk_gen{"clk", sc_time(1, SC_NS)};

    A a_mod{"a_mod"};

    SC_CTOR(B_top) {
        clk (clk_gen);
        a_mod.clk(clk);
        a_mod.rst(rst);
        
        //SC_HAS_PROCESS(B_top);
        //SC_CTHREAD(reset_proc);
    }
    
    /*void reset_proc() {
        rst = 1;
        wait();
        
        rst = 0;
    }*/
};

int sc_main(int argc, char *argv[]) {

    B_top b_mod{"b_mod"};
//    b_mod.clk(clk);
    
    sc_start();
    return 0;
}
