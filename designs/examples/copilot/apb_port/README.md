APB subordinate port example
============================

Files:
- apb_port.h : Synthesizable APB subordinate translating APB bus into request (sct_target) and response (sct_initiator) channels.
- sc_main.cpp : Simple memory model testbench exercising write and read transfers.

Template parameters:
- ADDR_W address width (PADDR)
- DATA_W data width (PWDATA/PRDATA)

Behavior summary:
- Captures APB setup/enable handshake.
- Issues request at first enable phase when PSEL=1 & PENABLE=1.
- For writes, completes immediately (PREADY next enable cycle).
- For reads, waits response from resp_init channel then asserts PREADY and drives PRDATA.

Limitations:
- Single outstanding transfer (standard APB).
- PSLVERR only set if response record sets slverr.

Build integration: add target in CMake using svc_target() with ELAB_TOP tbt.
