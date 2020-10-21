# Intel&reg; Compiler for SystemC* 

*\*Other names and brands may be claimed as the property of others.*

Intel&reg; Compiler for SystemC* (ICSC) translates synthesizable cycle-accurate SystemC code to synthesizable SystemVerilog.

ICSC supports SystemC synthesizable subset in method and thread processes and arbitrary C++ at elaboration phase. The tool produces human-readable SystemVerilog for complex multi-module designs in tens of seconds. The tool performs design checks to detect non-synthesizable code and common coding mistakes. 
ICSC is focused on improving productivity of design and verification engineers and leaves optimization work for an underlying logic synthesis tool. ICSC is based on LLVM/Clang frontend, that allows to support modern C++.

ICSC includes library of common modules like FIFO, clock gate cells, zero-delay channels and others. ICSC supports SystemC temporal assertions translation into SystemVerilog Assertions (SVA).

See detailed description, build and run instruction and other information at [Intel Compiler for SystemC wiki](https://github.com/intel/systemc-compiler/wiki/Intel-Compiler-for-SystemC).

