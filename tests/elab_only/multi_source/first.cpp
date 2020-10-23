//
// Created by ripopov on 10/2/18.
//

#include "first.h"

first::first(sc_module_name)
{

    SC_THREAD(test_thread);

    sensitive << clk.pos();
    async_reset_signal_is(rstn, false);
}

void first::test_thread()
{
    cout << "first::test_thread()\n";
    while (1) {wait();}
}

