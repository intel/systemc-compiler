#include "sct_assert.h"
#include "systemc.h"

// Assertions and SVA generation test
class A : public sc_module 
{
public:

    sc_in<bool>         clk{"clk"};
    sc_in<bool>         rstn{"rstn"};
    
    sc_in<sc_uint<4>>   a{"a"};
    sc_out<sc_uint<4>>  b{"b"};

    sc_signal<int>      s;
    sc_signal<int>      s_d;
    sc_signal<int>      s_d2;
    sc_signal<int>      s_d3;
    
    sc_signal<int>*     ps;

    bool                arr[3];
    sc_signal<bool>     sarr[3];
    sc_signal<bool>     sarr_d[3];
    sc_signal<bool>     sarr2[3][2];
    
    static const size_t N = 4;
    const unsigned long M = 1;
    
    SC_HAS_PROCESS(A);
    
    A(sc_module_name, unsigned m)
    {
        ps = new sc_signal<int>("ps");
        
        auto& val = const_cast<unsigned long&>(M);
        val = m;
        cout << "M " << M << endl;
        
        SC_CTHREAD(test_thread, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(sct_assert_thread, clk.pos());
        async_reset_signal_is(rstn, false);
        
        SC_CTHREAD(sct_assert_thread1, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(sct_assert_thread2, clk.pos());
        async_reset_signal_is(rstn, false);
        
        SC_CTHREAD(sct_assert_pointer, clk.pos());
        async_reset_signal_is(rstn, false);
        
        SC_CTHREAD(sct_assert_loop, clk.pos());
        async_reset_signal_is(rstn, false);
    }
    
    // Equivalent 4 and 2-arguments assertions
    SCT_ASSERT(!rstn, (0), !s, clk.pos());
    SCT_ASSERT(rstn || !s, clk.pos());
    
    SCT_ASSERT(s_d2, (0), s, clk.pos());
    SCT_ASSERT(!s_d2 || s, clk.pos());

    // 4-arguments assertions
    SCT_ASSERT(s, (1,N), s_d, clk.pos());
    
    SCT_ASSERT(s, SCT_TIME(M+1), s_d, clk.pos());
    SCT_ASSERT(s, SCT_TIME(M+1,M+2), s_d, clk.pos());
    
    SCT_ASSERT(s, (N+3), s_d, clk.pos());
    SCT_ASSERT(s, (1,N+1), s_d, clk.pos());
    
    SCT_ASSERT(s, (1), s_d, clk.pos());
    SCT_ASSERT(s_d, (1), s_d2, clk.pos());
    SCT_ASSERT(a.read() == 1, (1), b.read() == 3, clk.pos());
    SCT_ASSERT(*ps, (1), s_d2, clk.pos());
    SCT_ASSERT((*this ).s, SCT_TIME(0), this->s, clk.pos());

    // Provide test signals
    void test_thread() 
    {
        s = 0; s_d = 0; s_d2 = 0; s_d3 = 0; *ps = 0;
        wait();

        while (true) {
            s_d = s; s_d2 = s_d; s_d3 = s_d2;
            *ps = s;
            s = !s;
            cout << "." << flush;
            wait();
        }
    }

    void sct_assert_thread() 
    {
        SCT_ASSERT_THREAD(s, SCT_TIME(1), s_d, clk.pos());
        wait();

        while (true) {
            //std::cout << "thread " << sc_time_stamp() << " dc " << sc_delta_count() << std::endl;
            wait();
        }
    }
    
    // Simple immediate assertion test
    void sct_assert_thread1() 
    {
        SCT_ASSERT_THREAD(s, (0), s, clk.pos());
        SCT_ASSERT_THREAD(s, SCT_TIME(1), s_d, clk.pos());
        SCT_ASSERT_THREAD(s, SCT_TIME(2,3), s_d3, clk.pos());
        wait();

        while (true) {
            wait();
        }
    }
    
    // Multiple-wait thread
    void sct_assert_thread2() {
        wait();
        SCT_ASSERT_THREAD(s, SCT_TIME(1), s_d, clk.pos());
        SCT_ASSERT_THREAD(s || s_d, (1,2), s_d2, clk.pos());

        while (true) {
            wait();
        }
    }    
    
    // Use pointer in assertion
    void sct_assert_pointer()
    {
        wait();
        SCT_ASSERT_THREAD(ps->read(), (0), s_d, clk.pos());

        while (true) {
            wait();
        }
    }
    
    void sct_assert_loop() 
    {
        sc_uint<3> k = 0;
        for (int i = 0; i < 3; i++) {
            sarr[i] = 0; sarr_d[i] = 0;
        }
        wait();

        for (int i = 0; i < 3; i++) {
            // Check @k is captured by reference
            SCT_ASSERT_LOOP(sarr[i], (1), sarr_d[i] || k == 1, clk.pos(), i);
            for (int j = 0; j < 2; j++) {
                SCT_ASSERT_LOOP(sarr2[i][j], SCT_TIME(0), sarr2[i][j], clk.pos(), i, j);
            }
        }
        
        while (true) {
            for (int i = 0; i < 3; i++) {
                sarr[i] = !sarr[i];
                //if (i == 1) continue;
                if (k) sarr_d[i] = sarr[i];
            }
            k = (k != 2) ? k+1 : 0;
            //cout << k << " ";
            wait();
        }
    }
};

class Test_top : public sc_module
{
public:
    sc_signal<sc_uint<4>>  a{"a"};
    sc_signal<sc_uint<4>>  b{"b"};
    sc_signal<bool>        rstn{"rstn"};
    sc_clock clk{"clock", 10, SC_NS};

    A a_mod{"a_mod", 4};

    SC_CTOR(Test_top) {
        a_mod.a(a);
        a_mod.b(b);
        a_mod.clk(clk);
        SC_CTHREAD(testProc, clk);
        a_mod.rstn(rstn);

    }

    void testProc() {
    	rstn = 0;
        a = 0;
        wait(2);
    	rstn = 1;
        a = 0;
    	wait(12);
        
        cout << endl;
        cout << "--------------------------------" << endl;
        cout << "|       Test passed OK         |" << endl;
        cout << "--------------------------------" << endl;
        sc_stop();
    }
};

int sc_main(int argc, char* argv[])
{
    Test_top test_top{"test_top"};
    sc_start();
    return 0;
}
