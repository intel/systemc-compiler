//
// Created by ripopov on 10/2/18.
//

#ifndef SCTOOL_FIRST_H
#define SCTOOL_FIRST_H

#include <systemc.h>

struct first : sc_module
{
    sc_in<bool> clk{"clk"};
    sc_in<bool> rstn{"rstn"};

    SC_CTOR(first);

    void test_thread();
};

#endif //SCTOOL_FIRST_H
