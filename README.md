# Intel&reg; Compiler for SystemC* 

*\*Other names and brands may be claimed as the property of others.*

## Introduction

Intel&reg; Compiler for SystemC* (ICSC) translates synthesizable SystemC design to synthesizable SystemVerilog design.

ICSC supports SystemC synthesizable subset in method and thread processes and arbitrary C++ code in module constructors. The tool produces human-readable SystemVerilog for complex multi-module designs in tens of seconds. ICSC performs design checks to detect non-synthesizable code and common coding mistakes. 

See more information at [Intel Compiler for SystemC wiki](https://github.com/intel/systemc-compiler/wiki).

## License

ICSC is distributed under the [Apache License v2.0 with LLVM Exceptions](https://github.com/intel/systemc-compiler/blob/main/LICENSE.txt).

## Getting started

ICSC is based on Clang/LLVM frontend and can be installed at most Linux OS. 

An instruction how to build and run ISCS at **Ubuntu 20.04** is given at [Getting started](https://github.com/intel/systemc-compiler/wiki/Getting-started). 

## Documentation 

[User guide](https://github.com/intel/systemc-compiler/blob/main/doc/ug.pdf) document describes installation procedure, run tool options, preparation of SystemC design for ICSC, and also tool extensions and advanced verification features.

SystemC/C++ various features supported by ICSC described at [SystemC/C++ supported](https://github.com/intel/systemc-compiler/wiki/SystemC--supported).

## Publications

* [SystemC-to-Verilog Compiler: a productivity-focused tool for hardware design in cycle-accurate SystemC](https://github.com/intel/systemc-compiler/blob/main/doc/papers/icsc.pdf) at DvCon'2019
* [Temporal assertions in SystemC](https://github.com/intel/systemc-compiler/blob/main/doc/papers/sct_assert.pdf) at DvCon'2020 and SystemC evolution day'2020

## Help

To get help please [submit your question or issue](https://github.com/intel/systemc-compiler/issues).
