/******************************************************************************
 * Copyright (c) 2025, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

Promt to create synthesizable SystemC design to demostrate SS channels

Create a project `copilot_chan_demo` to demonstrate how SingleSource channels work.
In this project create a design module in #file:demo.h and testbench in #file:sc_main.cpp. 
Use target, initiator and fifo in this design. Add a method process and a thread process which interconnect through a fifo. The method process gets some data of `int` type from the target, and puts it to the fifo. The thread process gets data from the fifo and puts it into the initiator.
Testbench module instantiate the design and has correspondent initiator and target to bind to the desing onces.
