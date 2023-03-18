#!/bin/bash -e

#********************************************************************************
# Copyright (c) 2020, Intel Corporation. All rights reserved.                   #
#                                                                               #
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.                      #
#                                                                               #
#********************************************************************************

#################################################################################
# This bash builds and installs ICSC in Release and Debug configuration         #
# It indendent to be used after pull new version of ICSC                        #
#################################################################################

test -z $ICSC_HOME && {
    echo "ICSC_HOME is not configured. Please set it to the installation target folder.";
    exit 1; 
}

export INSTALL_PREFIX=$ICSC_HOME
export ICSC_HOME=$(realpath $(dirname "${BASH_SOURCE[0]}"))
export CWD_DIR=$ICSC_HOME

echo "Building from: $ICSC_HOME"
echo "Installing to: $INSTALL_PREFIX"
echo ""

export CMAKE_PREFIX_PATH=$ICSC_HOME:$CMAKE_PREFIX_PATH
export GCC_INSTALL_PREFIX="$(realpath "$(dirname $(which g++))"/..)"

# ################################################################################
# Build and install ISCC

echo "*** ISCC Build and Installation ... ***"

cd $CWD_DIR
(
    mkdir build_icsc_rel -p && cd build_icsc_rel
    cmake ../ -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$INSTALL_PREFIX -DCMAKE_CXX_STANDARD=17
    make -j12
    make install
    
    cd ..
    
    mkdir build_icsc_dbg -p && cd build_icsc_dbg
    cmake ../ -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=$INSTALL_PREFIX -DCMAKE_CXX_STANDARD=17 -DCMAKE_DEBUG_POSTFIX=d
    make -j12
    make install
<<<<<<< HEAD
=======

    cp $CWD_DIR/cmake/CMakeLists.top $INSTALL_PREFIX/CMakeLists.txt
>>>>>>> a1f1bca (Implement out-of-repo installation)
)

echo "*** ISCC Build and Installation Complete! ***"


# ################################################################################
# Build and run examples
echo "*** Building Examples ***"
cd $ICSC_HOME
(
    source setenv.sh
    mkdir build -p && cd build
    cmake ../                          # prepare Makefiles
    cd designs/examples                # run examples only
    ctest -j12                         # compile and run Verilog generation
                                       # use "-jN" key to run in "N" processes
)
