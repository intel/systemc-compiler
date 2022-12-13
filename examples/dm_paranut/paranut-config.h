/*************************************************************************

  This file is part of the ParaNut project.

  Copyright (C) 2019-2022 Alexander Bahle <alexander.bahle@hs-augsburg.de>
                          Christian H. Meyer <christian.meyer@hs-augsburg.de>
      Hochschule Augsburg, University of Applied Sciences

  Description:
    This file contains the configuration options for the ParaNut.

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

#ifndef _CONFIG_
#define _CONFIG_

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// Warning: 	Don't change this file manually! 
// 				Use the "../../config" file located
//				at the root of the project
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

/// @file
/// @brief Configuration Makros used in most *ParaNut* files.
/// @defgroup config Configuration
/// @brief Configuration Makros used in most *ParaNut* files.
/// @{

////////////////////////////////////////////////////////////////////////////////////////////////
/// @defgroup config_simulation Simulation
/// @brief SystemC Simulation options. @{
/// @name SystemC Simulation options...

/// @brief Simulation clock speed in Hz. @brief
/// Defines the simulation clock speed in Hz. The configured value can be read from the
/// pnclockinfo CSR.
#define CFG_NUT_SIM_CLK_SPEED 25000000

/// @brief mtimer timebase. @brief
/// Defines the mtime timer timebase in us
#define CFG_NUT_MTIMER_TIMEBASE_US 1000 

/// @brief mtimer base address. @brief
/// Defines the address at which the mtimer will be added to the system interconnect
#define CFG_NUT_MTIMER_ADDR 0x80000000

/// @brief Simulation memory address. @brief
/// Defines the start address (reset address) of the *ParaNut* during simulation and
/// the address at which the main memory will be added to the system interconnect
/// (MParaNutSystem::MParaNutSystem()).
/// Also used to determine the cacheable memory addresses.
#define CFG_NUT_RESET_ADDR 0x10000000

/// @brief Simulation maximum peripherals number. @brief
/// Defines the maximum number of peripherals that can be connected to the systems Wishbone
/// interconnect (MInterconnect).
#define CFG_NUT_SIM_MAX_PERIPHERY 5

/// @} // @name SystemC Simulation options...
/// @} // @defgroup config_simulation Simulation
////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////
/// @defgroup config_general General
/// @brief General options. @{
/// @name General options...

/// @brief Hardware version derived from git @brief
/// This Machine Architecture ID is storde in csr marchid.
#define CFG_NUT_MIMPID 16777330

/// @brief Number of cores overall as log2. @brief
/// Defines the log2 of the overall number of *ParaNut* cores (ExUs).
#define CFG_NUT_CPU_CORES_LD 2
/// @brief Number of cores overall (derived). @brief
#define CFG_NUT_CPU_CORES (1 << CFG_NUT_CPU_CORES_LD)
/// @brief Number of cpu groups (derived). @brief
#define CFG_NUT_CPU_GROUPS ((CFG_NUT_CPU_CORES-1)/32)

/// @brief Number of cores (ExUs) with mode capability = 1 (linked). @brief
/// Defines the number of *ParaNut* cores (ExUs) that have a mode capability of 1 and thus can only
/// operate in mode 1 (linked mode).
#define CFG_NUT_CPU_CAP1_CORES 0

/// @brief Number of cores (ExUs) with mode capability >= 2 (thread) (derived). @brief
/// Defines the number of *ParaNut* cores (ExUs) that have a mode capability of >= 2 and thus can
/// operate in mode 1 (linked mode) and mode 2 (thread mode).
#define CFG_NUT_CPU_CAP2_CORES (CFG_NUT_CPU_CORES - CFG_NUT_CPU_CAP1_CORES)

/// @brief ParaNut mode 2 capability value (derived). @brief
/// Contains a bitmask of all mode 2 capable cores. The configured value can be read from the
/// pnm2cp CSR.
#define CFG_EXU_PNM2CAP ~(0xffffffff << CFG_NUT_CPU_CAP2_CORES)

/// @brief System memory size. @brief
/// Defines the system memory size in Bytes. 8MB is recommended for simulation, otherwise
/// needs to match the memory size of the used board.
/// Also used to determine the cacheable memory addresses.
#define CFG_NUT_MEM_SIZE (256 * MB)

/// @brief Number of external interrupt lines. @brief
/// Defines the number of external interrupt lines.  Needs to be at least equal to 1.
#define CFG_NUT_EX_INT 2

/// @} // @name General options...
/// @} // @defgroup config_general General
////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////
/// @defgroup config_exu Execution Unit
/// @brief Execution Unit options. @{
/// @name Execution Unit options...

/// @brief RISC-V M-Extension. @brief
/// Defines if the RISC-V M-Extension hardware is enabled/disabled.
/// @param 0 - extension disabled
/// @param 1 - extension enabled
#define CFG_EXU_M_EXTENSION 1
/// @brief RISC-V A-Extension. @brief
/// Defines if the RISC-V A-Extension hardw
/// @param 0 - extension disabled
/// @param 1 - extension enabled
#define CFG_EXU_A_EXTENSION 1

/// @brief Privilege Mode options. @brief
/// Defines the RISC-V privilege mode capability of the ParaNut. Do NOT use any other value!
/// @param 1 - only M-Mode is available
/// @param 2 - M- and U-Mode are available 
/// @param 3 - M-, S- and U-Mode available, enable the Memory Management Unit (MMU)
#define CFG_PRIV_LEVELS 1

/// @brief Performance counter enable. @brief
/// Defines if the hardware performance counters are enabled/disabled.
/// @param 0 - no performance counters
/// @param 1 - performance counter and 64bit cycle counter is added
#define CFG_EXU_PERFCOUNT_ENABLE 1
/// @brief Performance counter register width. @brief
/// Defines the number of bits for the hardware performance counters.
/// @warning Has to be between 33 and 64.
#define CFG_EXU_PERFCOUNTER_BITS 40
/// @brief Performance counter number of registers as log2. @brief
/// Defines the log2 of the number of hardware performance counters.
/// @warning 3 is the only supported value for now (see exu.cpp for implemented counters)
#define CFG_EXU_PERFCOUNTERS_LD 3
/// @brief Performance counter number of registers (derived). @brief
#define CFG_EXU_PERFCOUNTERS (1 << CFG_EXU_PERFCOUNTERS_LD)

/// @} // @name Execution Unit options...
/// @} // @defgroup config_exu Execution Unit
////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////
/// @defgroup config_memu Memory Unit
/// @brief Memory Unit options. @{
/// @name Memory Unit options...

/// @brief Number of cache banks as log2. @brief
/// Defines the log2 of the number of cache banks.
/// A cache line has a size of @ref CFG_MEMU_CACHE_BANKS words. A good starting point is 2 (4 banks).
#define CFG_MEMU_CACHE_BANKS_LD 2
/// @brief Number of cache banks (derived). @brief
#define CFG_MEMU_CACHE_BANKS (1 << CFG_MEMU_CACHE_BANKS_LD)

/// @brief Number of cache sets as log2. @brief
/// Defines the log2 of the number of cache sets.
/// A bank has @ref CFG_MEMU_CACHE_SETS * @ref CFG_MEMU_CACHE_WAYS words.
#define CFG_MEMU_CACHE_SETS_LD 9
/// @brief Number of cache sets (derived). @brief
#define CFG_MEMU_CACHE_SETS (1 << CFG_MEMU_CACHE_SETS_LD)

/// @brief Number of cache ways as log2. @brief
/// Defines the log2 of the number of cache ways (cache associativity).
/// A bank has @ref CFG_MEMU_CACHE_SETS * @ref CFG_MEMU_CACHE_WAYS words.
/// @param 0 - 1-way set-associativity
/// @param 1 - 2-way set-associativity
/// @param 2 - 4-way set-associativity
#define CFG_MEMU_CACHE_WAYS_LD 2
/// @brief Number of cache ways (derived). @brief
#define CFG_MEMU_CACHE_WAYS (1 << CFG_MEMU_CACHE_WAYS_LD)

/// @brief Number of ports per bank. @brief
/// Since every bank is implemented as one Block
/// RAM, for maximum performance this should be set to the maximum number of
/// Block RAM ports supported by the target device (i.e. 2) but can be
/// reduced in order to save some area.
#define CFG_MEMU_BANK_RAM_PORTS 2

/// @brief Cache replacement method. @brief
/// Defines the cache replacement method/strategy, either pseudo-random or least recently used (LRU).
/// @param 0 - random replacement
/// @param 1 - LRU replacement
#define CFG_MEMU_CACHE_REPLACE_LRU 1

/// @brief Arbiter Method. @brief
/// Defines the MemU arbitration method/strategy, either a round-robin or pseudo-random arbitration.
/// @param >0 - round-robin arbitration, switches every (1 << CFG_MEMU_ARBITER_METHOD) clocks
/// @param <0 - pseudo-random arbitration (LFSR-based)
#define CFG_MEMU_ARBITER_METHOD 7

/// @brief Busif Data Width. @brief
/// Defines the width of the master Wishbone data bus.
/// @param 32 - 32 Bit data width
/// @param 64 - 64 Bit data width
#define CFG_MEMU_BUSIF_WIDTH 32

/// @brief Overall cache size in Bytes (derived). @brief
#define CFG_MEMU_CACHE_SIZE (CFG_MEMU_CACHE_SETS * CFG_MEMU_CACHE_WAYS * CFG_MEMU_CACHE_BANKS * 4)

/// @brief Number of write ports (WPORTS) in the MemU (derived). @brief
#define CFG_MEMU_WPORTS CFG_NUT_CPU_CORES
/// @brief Number of read ports (RPORTS) in the MemU (derived). @brief
#define CFG_MEMU_RPORTS (2 * CFG_NUT_CPU_CORES)

/// @} // @name Memory Unit options...
/// @} // @defgroup config_memu Memory Unit
////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////
/// @defgroup config_mmu Memory Management Unit (MMU)
/// @brief Memory Management Unit (MMU) options. @{
/// @name Memory Management Unit (MMU) options...

/// @brief TLB enable @brief
/// Enable or disable the Translation Lookaside Buffer (TLB)
/// @param 0 - TLB disabled
/// @param 1 - TLB enabled
#define CFG_MMU_TLB_ENABLE 0

/// @brief Number of TLB entries as log2. @brief
/// Defines the log2 of the number of TLB entries
#define CFG_MMU_TLB_ENTRIES_LD 2
/// @brief Number of TLB entries (derived). @brief
#define CFG_MMU_TLB_ENTRIES (1 << CFG_MMU_TLB_ENTRIES_LD)

/// @name Memory Management Unit (MMU) options...
/// @defgroup config_mmu Memory Management Unit (MMU)
////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////
/// @defgroup config_ifu Instruction Fetch Unit
/// @brief Instruction Fetch Unit options. @{
/// @name Instruction Fetch Unit options...

/// @brief Instruction buffer size as log2. @brief
/// Defines the log2 of the number of instruction buffer elements (max. number of prefetched
/// instructions).
#define CFG_IFU_IBUF_SIZE_LD 2
/// @brief Instruction buffer size (derived). @brief
#define CFG_IFU_IBUF_SIZE (1 << CFG_IFU_IBUF_SIZE_LD)

/// @} // @name Instruction Fetch Unit options...
/// @} // @defgroup config_ifu Instruction Fetch Unit
////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////
/// @defgroup config_lsu Load Store Unit
/// @brief Load Store Unit options. @{
/// @name Load Store Unit options...

/// @brief LSU write buffer size as log2. @brief
/// Defines the log2 of the number of write buffer elements in the LSU.
#define CFG_LSU_WBUF_SIZE_LD 2
/// @brief LSU write buffer size (derived). @brief
#define CFG_LSU_WBUF_SIZE (1 << CFG_LSU_WBUF_SIZE_LD)

/// @} // @name Load Store Unit options...
/// @} // @defgroup config_lsu Load Store Unit
////////////////////////////////////////////////////////////////////////////////////////////////

/// @} // @defgroup config Configuration
#endif

