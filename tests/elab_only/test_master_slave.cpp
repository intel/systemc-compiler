//
// Created by ripopov on 12/10/17.
//

#include <systemc.h>

SC_MODULE(master) {

    sc_in_clk    clk{"clk"};
    sc_in<bool>  rstn{"rstn"};

    sc_out<int>  din{"din"};
    sc_out<bool> din_valid{"din_valid"};

    sc_in<int>   dout{"dout"};
    sc_in<bool>  dout_valid{"dout_valid"};

    SC_CTOR(master)
    {
        SC_CTHREAD(source_thread, clk.pos());
        async_reset_signal_is(rstn, false);

        SC_CTHREAD(sink_thread, clk.pos());
        async_reset_signal_is(rstn, false);
    }

private:

    static const int TEST_SIZE = 10;

    void source_thread() {
        wait();

        while (1) {
            for (int i = 0; i < TEST_SIZE; ++i) {
                din.write(i);
                din_valid.write(1);
                wait();
                din_valid.write(0);
                wait();
            }
            wait();
        }
    }

    void sink_thread() {
        wait();
        while (1) {
            for (int i = 0; i < TEST_SIZE; i += dout_valid) {
                if (dout_valid)
                    cout << dout.read() << endl;
                wait();
            }
            sc_stop();
        }
    }
};

struct int_wrap {
    int val = 100;
};

SC_MODULE(dummy) {
    char val = 42;
    SC_CTOR(dummy) {}
};

SC_MODULE(slave) {

    sc_in_clk    clk{"clk"};
    sc_in<bool>  rstn{"rstn"};

    sc_in<int>   din{"din"};
    sc_in<bool>  din_valid{"din_valid"};

    sc_out<int>  dout{"dout"};
    sc_out<bool> dout_valid{"dout_valid"};

    SC_CTOR(slave)
    {
        SC_CTHREAD(slave_thread, clk.pos());
        async_reset_signal_is(rstn, false);
    }

private:
    dummy d{"d"};
    int_wrap w;

    void slave_thread() {
        wait();
        while (1) {
            if (din_valid) {
                dout_valid.write(1);
                dout.write(din + w.val);
            } else {
                dout_valid.write(0);
            }
            wait();
        }
    }

};

SC_MODULE(top) {

    SC_CTOR(top)
    {
        master0.clk(clk);
        master0.rstn(rstn);
        master0.din(din);
        master0.din_valid(din_valid);
        master0.dout(dout);
        master0.dout_valid(dout_valid);

        slave0.clk(clk);
        slave0.rstn(rstn);
        slave0.din(din);
        slave0.din_valid(din_valid);
        slave0.dout(dout);
        slave0.dout_valid(dout_valid);

        SC_THREAD(reset_thread);
    }

    master master0{"master0"};
    slave  slave0{"slave0"};

    sc_clock clk{"clk", 10, SC_NS};
    sc_signal<bool> rstn{"rstn"};

    sc_signal<int>  din{"din"};
    sc_signal<bool> din_valid{"din_valid"};
    sc_signal<int>  dout{"dout"};
    sc_signal<bool> dout_valid{"dout_valid"};

    void reset_thread () {
        rstn = 0;
        wait(clk.posedge_event());
        wait(clk.posedge_event());
        rstn = 1;
    }

};

int sc_main(int argc, char** argv)
{
    cout << "started\n";
    auto top0 = std::make_unique<top>("top0");
    sc_start();
    return 0;
}
