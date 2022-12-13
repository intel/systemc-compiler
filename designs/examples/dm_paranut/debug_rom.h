/*************************************************************************

  This file is part of the ParaNut project.

  Copyright (C) 2019-2020 Alexander Bahle <alexander.bahle@hs-augsburg.de>
      Hochschule Augsburg, University of Applied Sciences

  Description:
    This file contains the instructions contained in the Debug Modules 
    Debug Rom
  
  Redistribution and use in source and binary forms, with or without modification,
  are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice, this 
     list of conditions and the following disclaimer.

  2. Redistributions in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation and/or
     other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 *************************************************************************/

const unsigned char debug_rom_bin[] = {
    0x6f, 0x00, 0xc0, 0x00, // 0x00 jal zero, _entry
    0x6f, 0x00, 0x80, 0x03, // 0x04 jal zero, _resume
    0x6f, 0x00, 0x40, 0x04, // 0x08 jal zero, _exception
    // _entry:
    0x0f, 0x00, 0xf0, 0x0f, // 0x0c fence
    0x73, 0x10, 0x24, 0x7b, // 0x10 csrw CSR_DSCRATCH, s0
    // entry_loop:
    0x73, 0x24, 0x40, 0xf1, // 0x14 csrr s0, CSR_MHARTID
    0x23, 0x20, 0x80, 0x30, // 0x18 sw s0, HALTED(zero)
    0x03, 0x44, 0x04, 0x10, // 0x1c lbu s0, FLAGS(s0)
    0x13, 0x74, 0x34, 0x00, // 0x20 andi s0, s0, (1 << FLAG_GO) | (1 << FLAG_RESUME)
    0xe3, 0x08, 0x04, 0xfe, // 0x24 beqz s0, entry_loop

    0x13, 0x74, 0x14, 0x00, // 0x28 andi s0, s0, (1 << FLAG_GO)
    0x63, 0x08, 0x04, 0x00, // 0x2c beqz s0, _resume

    0x73, 0x24, 0x20, 0x7b, // 0x30 csrr s0, CSR_DSCRATCH
    0x23, 0x22, 0x00, 0x30, // 0x34 sw zero, GOING(zero)
    0x67, 0x00, 0x00, 0x20, // 0x38 jalr zero, zero, 0x200
    // _resume:
    0x73, 0x24, 0x40, 0xf1, // 0x3c csrr s0, CSR_MHARTID
    0x23, 0x24, 0x80, 0x30, // 0x40 sw s0, RESUMING(zero)
    0x73, 0x24, 0x20, 0x7b, // 0x44 csrr s0, CSR_DSCRATCH
    0x73, 0x00, 0x20, 0x7b, // 0x48 dret
    // _exception:
    0x23, 0x26, 0x00, 0x30, // 0x4c sw zero, EXCEPTION(zero)
    0x73, 0x00, 0x10, 0x00 // 0x50 ebreak
};
unsigned int debug_rom_bin_len = 84;

const unsigned int debug_rom[] = {
    0x00c0006f, // 0x00 jal zero, _entry
    0x0380006f, // 0x04 jal zero, _resume
    0x0440006f, // 0x08 jal zero, _exception
    // _entry:
    0x0ff0000f, // 0x0c fence
    0x7b241073, // 0x10 csrw CSR_DSCRATCH, s0
    // entry_loop:
    0xf1402473, // 0x14 csrr s0, CSR_MHARTID
    0x30802023, // 0x18 sw s0, HALTED(zero)
    0x10044403, // 0x1c lbu s0, FLAGS(s0)
    0x00347413, // 0x20 andi s0, s0, (1 << FLAG_GO) | (1 << FLAG_RESUME)
    0xfe0408e3, // 0x24 beqz s0, entry_loop

    0x00147413, // 0x28 andi s0, s0, (1 << FLAG_GO)
    0x00040863, // 0x2c beqz s0, _resume

    0x7b202473, // 0x30 csrr s0, CSR_DSCRATCH
    0x30002223, // 0x34 sw zero, GOING(zero)
    0x20000067, // 0x38 jalr zero, zero, 0x200
    // _resume:
    0xf1402473, // 0x3c csrr s0, CSR_MHARTID
    0x30802423, // 0x40 sw s0, RESUMING(zero)
    0x7b202473, // 0x44 csrr s0, CSR_DSCRATCH
    0x7b200073, // 0x48 dret
    // _exception:
    0x30002623, // 0x4c sw zero, EXCEPTION(zero)
    0x00100073, // 0x50 ebreak
};
