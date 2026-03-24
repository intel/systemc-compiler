#!/bin/bash -e

#********************************************************************************
# Copyright (c) 2020-2024, Intel Corporation. All rights reserved.              #
#                                                                               #
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.                      #
#                                                                               #
#********************************************************************************

#################################################################################
# This bash builds and installs ICSC in Release and Debug configuration         #
# It indendent to be used after pull new version of ICSC                        #
#################################################################################

BUILD_TYPE="Release"
BUILD_DIR="build_icsc_rel"
DEBUG_POSTFIX_ARG=""

if [ "$#" -gt 1 ]; then
    echo "Usage: $0 [--debug|-d]"
    exit 1
fi

if [ "$#" -eq 1 ]; then
    case "$1" in
        --debug|-d)
            BUILD_TYPE="Debug"
            BUILD_DIR="build_icsc_dbg"
            DEBUG_POSTFIX_ARG="-DCMAKE_DEBUG_POSTFIX=d"
            ;;
        *)
            echo "Unknown option: $1"
            echo "Usage: $0 [--debug|-d]"
            exit 1
            ;;
    esac
fi

test -z $ICSC_HOME && { echo "ICSC_HOME is not configured"; exit 1; }
echo "Installation folder (ICSC_HOME) = $ICSC_HOME"

export CMAKE_PREFIX_PATH=$ICSC_HOME:$CMAKE_PREFIX_PATH
export GCC_INSTALL_PREFIX="$(realpath "$(dirname $(which g++))"/..)"
echo "GCC folder = $GCC_INSTALL_PREFIX"
export CWD_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
echo "Source folder = $CWD_DIR"
echo "Build mode = $BUILD_TYPE"

# ################################################################################
# Build and install ISCC
echo "*** ISCC Build and Installation ... ***"

cd $CWD_DIR
(
    if test -f "systemc/PostInstall.cmake"
    then
    cp systemc/PostInstall.cmake PostInstall.cmake
    fi

    mkdir "$BUILD_DIR" -p && cd "$BUILD_DIR"
    cmake ../ -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_INSTALL_PREFIX=$ICSC_HOME -DCMAKE_CXX_STANDARD=20 -DENABLE_PTHREADS=$PTHREADS $DEBUG_POSTFIX_ARG
    make -j12
    make install
)

echo "*** ISCC Build and Installation Complete! ***"


# ################################################################################
# Build and run examples
echo "*** Building Examples ***"
cd $ICSC_HOME
(    
    source setenv.sh
    mkdir build -p && cd build
    cmake ../ -DCMAKE_BUILD_TYPE=$BUILD_TYPE # prepare Makefiles
    cd designs/examples                # run examples only
    ctest -j12                         # compile and run Verilog generation
                                       # use "-jN" key to run in "N" processes
)
