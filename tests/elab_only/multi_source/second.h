//
// Created by ripopov on 10/2/18.
//

#ifndef SCTOOL_SECOND_H
#define SCTOOL_SECOND_H

#include <systemc.h>

struct second : sc_module
{
    sc_in<bool> clk{"clk"};
    sc_in<bool> rstn{"rstn"};

    SC_CTOR(second);

    void test_thread();
};

#endif //SCTOOL_SECOND_H
