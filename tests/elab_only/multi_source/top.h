//
// Created by ripopov on 10/2/18.
//

#ifndef SCTOOL_TOP_H
#define SCTOOL_TOP_H

#include <systemc.h>
#include "first.h"
#include "second.h"

struct top : sc_module
{
    sc_clock clk{"clk", 10 , SC_NS};
    sc_signal<bool> rstn{"rstn"};

    first  finst{"finst"};
    second sinst{"sinst"};

    SC_CTOR(top);

    void reset_thread();
};

#endif //SCTOOL_TOP_H
