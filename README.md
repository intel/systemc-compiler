# Intel&reg; Compiler for SystemC* 

*\*Other names and brands may be claimed as the property of others.*

Intel&reg; Compiler for SystemC* (ICSC) translates synthesizable SystemC code to synthesizable SystemVerilog.

ICSC supports SystemC synthesizable subset (IEEE1666) in method and thread processes and arbitrary C++ code in module constructors. The tool produces human-readable SystemVerilog for complex multi-module designs in tens of seconds. 
This tool is focused on improving productivity of design and verification engineers and leaves optimization work for an underlying logic synthesis tool. It performs design checks to detect non-synthesizable code and common coding mistakes. ICSC is based on LLVM/Clang frontend, that allows to support modern C++ standards.

ICSC has a common library which includes collections of FIFOs, clock gate cells, zero-delay channels and others. ICSC supports SystemC immediate and temporal assertions translation into SystemVerilog Assertions (SVA).

See more information at [Intel Compiler for SystemC wiki](https://github.com/intel/systemc-compiler/wiki/Main-page).

