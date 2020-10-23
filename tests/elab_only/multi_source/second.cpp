//
// Created by ripopov on 10/2/18.
//

#include "second.h"

second::second(sc_module_name)
{
    SC_CTHREAD(test_thread, clk.pos());
    reset_signal_is(rstn, false);
}

void second::test_thread()
{
    cout << "second::test_thread()\n";
    while (1) { wait(); }
}
