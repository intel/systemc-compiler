//
// Created by ripopov on 10/2/18.
//

#include "top.h"

top::top(sc_module_name) {
    finst.clk(clk);
    finst.rstn(rstn);

    sinst.clk(clk);
    sinst.rstn(rstn);

    SC_THREAD(reset_thread);
    sensitive << clk.posedge_event();
}

void top::reset_thread() {
    rstn = 0;
    wait();
    rstn = 1;
    wait();
    sc_stop();
}