#******************************************************************************
# Copyright (c) 2020-2024, Intel Corporation. All rights reserved.
# 
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
# 
# *****************************************************************************
#
# Intel(r) Compiler for SystemC*, version 1.6.1 
#
# *****************************************************************************

# Setup environment to run examples, tests and user designs

#!/bin/bash
export SHELL=/bin/sh

export LLVM_VER=15.0.7
export ICSC_HOME="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

export CMAKE_PREFIX_PATH=$ICSC_HOME
export PATH=$ICSC_HOME/bin:$ICSC_HOME/include:$PATH
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$ICSC_HOME/lib64:$ICSC_HOME/lib
export SYSTEMC_HOME=$ICSC_HOME

