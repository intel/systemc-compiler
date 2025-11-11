#include "sct_ipc_if.h"

#ifdef RTL_SIM
  #include "sct_common.h"
  #include "dut.h"
#else 
  #include "dut_test.h"
#endif
#include <systemc.h>

// Use @bool instead of @sc_uint<1>
// Use @sct_uint instead of @sc_uint/@sc_biguint
class Test_top : public sc_module
{
public:
    using T = sct_uint<65>;
    using TI = bool;
    using TO = sct_uint<16>;
    
    sc_in_clk           clk{"clk"};
    sc_signal<bool>     nrst{"nrst"};

    sct_initiator<T>    run{"run"};
    sct_target<T>       resp{"resp"};

    sct_signal<TI>              in_s{"in_s"};
    sct_signal<TO>              out_s{"out_s"};
    sc_vector<sct_signal<T>>    vo_s{"vo_s", 2};
    
#ifdef RTL_SIM
    dut                 dut_inst{"dut_inst"};
#else
    Dut<T, TI, TO>      dut_inst{"dut_inst"};
#endif

    SC_CTOR(Test_top) {
        dut_inst.clk(clk);
        dut_inst.nrst(nrst);        
        run.clk_nrst(clk, nrst);
        resp.clk_nrst(clk, nrst);

        dut_inst.in(in_s);
        dut_inst.out(out_s);
        dut_inst.vo(vo_s);
        
    #ifdef RTL_SIM
        SCT_BIND_CHANNEL(dut_inst, run, run);
        SCT_BIND_CHANNEL(dut_inst, resp, resp);
    #else
        run.bind(dut_inst.run);
        resp.bind(dut_inst.resp);
    #endif
        
        SC_THREAD(testProc);
        sensitive << run << resp;
        async_reset_signal_is(nrst, 0);

        SCT_CTHREAD(resetProc, clk, 0);
    }

    void testProc()
    {
        T data; 
        run.reset_put();
        resp.reset_get();
        in_s = 0;
        wait();
        
        resp.get(data);

        in_s = 42;
        while (!run.put(42)) wait();
        wait();
        while (!resp.get(data)) wait();
        assert (data == 42); wait();
        
        cout << sc_time_stamp() << " " << sc_delta_count() << " : all tests done " << endl;
        wait();

        while (true) {
            wait();
        }
    }
    
    void resetProc() {
        nrst = 1;
        wait();
        nrst = 0;
        wait(100);

        cout << endl;
        cout << "--------------------------------" << endl;
        cout << "|       Test passed OK         |" << endl;
        cout << "--------------------------------" << endl;
        sc_stop();
    }
};

int sc_main(int argc, char* argv[])
{
    sct_clock<> clk{"clk", 1, SC_NS};
    Test_top test_top{"test_top"};
    test_top.clk(clk);
    sc_start();
    return 0;
}
