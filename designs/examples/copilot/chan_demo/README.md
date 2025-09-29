/******************************************************************************
 * Copyright (c) 2025, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

# copilot_chan_demo

Minimal example showing SingleSource channels (sct_target, sct_fifo, sct_initiator)
interaction between an SC_METHOD and an SC_THREAD.

## Structure
- `chan_demo` module (`demo.h`)
  - Method `push_method`: non-blocking pull from `in_targ` -> push to `data_fifo`.
  - Thread `pop_thread`: non-blocking transfer from `data_fifo` -> `out_init`.
- Testbench (`sc_main.cpp`)
  - `in_init` drives integers 0..19 into DUT.
  - `out_targ` receives forwarded integers and prints them.

## Key Points
- Separate clock/reset wiring with `clk_nrst` on channels.
- Internal FIFO decouples producer and consumer processes.
- Demonstrates handshake: `request()/get()` on target, `ready()/put()` on initiator.

## Build
Integrated via example `CMakeLists.txt`:
```
add_executable(copilot_chan_demo demo.h sc_main.cpp)
svc_target(copilot_chan_demo copilot_chan_demo)
```
Run after configuring the top-level build:
```
ctest -R copilot_chan_demo  # if integrated as test
./copilot_chan_demo         # or direct run from build dir
```
