# Intel&reg; Compiler for SystemC* 

*\*Other names and brands may be claimed as the property of others.*

## Introduction

The SystemC Compiler (ICSC) translates synthesizable SystemC design into equivalent SystemVerilog code.

The SystemC Compiler checks a SystemC design for common coding mistakes and generates human-readable SystemVerilog code. The Compiler supports SystemC synthesizable subset in method and thread processes, and arbitrary C++ code in module constructors. ICSC is based on Clang/LLVM 18.1.8, supports C++ 11/14/17/20 standards and includes SystemC 3.0.1.

See more information at [Intel Compiler for SystemC wiki](https://github.com/intel/systemc-compiler/wiki).

## Common SystemC Library

Common SystemC Library consists of types, modules and functions which could be used in SystemC designs and testbench code. All the components are synthesizable with the SystemC Compiler. 
Common SystemC Library has three parts:
* [Single Source library](https://github.com/intel/systemc-compiler/wiki/Single-Source-library) -- communication channels with functional interfaces similar to TLM 1.0
* [Types and utility functions](https://github.com/intel/systemc-compiler/wiki/Types-and-utility-functions) -- common types and reusables functions
* [Immediate and temporal assertions](https://github.com/intel/systemc-compiler/wiki/Immediate-and-temporal-assertions-in-SystemC) -- SystemC assertions synthesizable to SVA


## AI support

The SystemC Compiler and Common SystemC Library semantics, rules, and restrictions are supported with [Agent Skills](https://github.com/intel/systemc-compiler/blob/main/.github/skills/systemc-tools/SKILL.md). 

The skills are based on several [Basic documents](https://github.com/intel/systemc-compiler/tree/main/doc/skills), ICSC unit tests, designs and examples. We are actively using the the skills for our SystemC designs. 

## Getting started

The SystemC Compiler can be installed at most Linux and Windows Subsystem for Linux (WSL). There is ```install.sh``` script that downloads and builds the compiler and the required dependencies at **Ubuntu 24.04**, **Ubuntu 22.04**, **Ubuntu 20.04**, **SLES15**, **SLES12** and others.

An instruction how to install and run ISCS is given at [Getting started](https://github.com/intel/systemc-compiler/wiki/Getting-started). 

## Documentation 

[User guide](https://github.com/intel/systemc-compiler/blob/main/doc/ug.pdf) document describes installation procedure, run tool options, preparation of SystemC design for synthesis, tool extensions and advanced verification features.

The SystemC Compiler supports [SystemC Synthesizable Subset](https://www.accellera.org/images/downloads/drafts-review/SystemC_Synthesis_Subset_Draft_1_4.pdf). Details of SystemC/C++ subset supported are described at [SystemC/C++ supported](https://github.com/intel/systemc-compiler/wiki/SystemC--supported).

## Publications

* [Single Source library for high-level modelling and hardware synthesis](https://github.com/intel/systemc-compiler/blob/main/doc/papers/preprint_single_source_2024.pdf), at DvCon'2024
* [Intel Compiler for SystemC and SystemC common library](https://github.com/intel/systemc-compiler/blob/main/doc/papers/common_library_2022.pdf) at CHIPS tech summit 2022
* [Temporal assertions in SystemC](https://github.com/intel/systemc-compiler/blob/main/doc/papers/sct_assert.pdf) at DvCon'2020 and SystemC evolution day'2020
* [SystemC-to-Verilog Compiler: a productivity-focused tool for hardware design in cycle-accurate SystemC](https://github.com/intel/systemc-compiler/blob/main/doc/papers/icsc.pdf) at DvCon'2019

## License

ICSC is distributed under the [Apache License v2.0 with LLVM Exceptions](https://github.com/intel/systemc-compiler/blob/main/LICENSE.txt).

## Help

To get help please [submit your question or issue](https://github.com/intel/systemc-compiler/issues)
