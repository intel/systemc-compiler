/******************************************************************************
 * Copyright (c) 2025, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

Promt to create synthesizable SystemC design of APB subordinate port.

In folder `copilot_apb_port` ceate a APB subordinate port module in #file:apb_port.h and testbench in #file:sc_main.cpp. 
The APB port has the following external ports:
- PCLK - rising edge of PCLK times all transfers on the APB.
- PRESETn -B reset signal is active LOW. 
- PADDR - input address, PADDR width is template parameter.
- PSEL - indicates that this device is selected for a data transfer in the next cycle.
- PENABLE - indicates the second and subsequent cycles of an APB transfer.
- PWRITE - indicates an APB write access when HIGH and an APB read access when LOW.
- PWDATA - write data input, PWDATA width is template parameter.
- PSTRB - indicates which byte lanes to update during a write transfer. There is one write strobe for each eight bits of the write data bus. Therefore, PSTRB[n] corresponds to PWDATA[(8n + 7):(8n)].
- PREADY - output signal to confirm the trasfer is accepted if the signal is HIGH. If this signal level is LOW, then APB transfer is extended to the next cycle.
- PRDATA - read data output, width is the same as PWDATA. The selected device drives this bus during read cycles when PWRITE is LOW. 
- PSLVERR - indicates a transfer failure. APB peripherals are not required to support the PSLVERR pin. Where a peripheral does not include this pin then the appropriate input to the APB bridge is tied LOW.

Testbench module instantiate the design and has correspondent initiator and target to bind to the desing onces and signals to bind to APB extrnal ports. Testbench process drives APB signals and checks the correspondent request at the target. Testbench process puts response for the request to the initiator and check it at APB signals.
