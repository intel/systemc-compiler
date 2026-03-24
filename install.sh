#!/bin/bash -e

#********************************************************************************
# Copyright (c) 2020-2024, Intel Corporation. All rights reserved.              #
#                                                                               #
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.                      #
#                                                                               #
#********************************************************************************

#################################################################################
# This bash script downloads and builds Protobuf, Clang/LLVM, GDB (optional),   #
# builds and installs ICSC, runs ICSCS examples.                                #
# To build GDB with Python3 compatible with SystemC pretty printers use:        #
# ./install.sh gdb                                                              #
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
echo "Using ICSC_HOME = $ICSC_HOME"

export CMAKE_PREFIX_PATH=$ICSC_HOME:$CMAKE_PREFIX_PATH
export GCC_INSTALL_PREFIX="$(realpath "$(dirname $(which g++))"/..)"
echo "GCC folder = $GCC_INSTALL_PREFIX"
export CWD_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
echo "Source folder = $CWD_DIR"
echo "Build mode = $BUILD_TYPE"

WGET="wget -N --timeout=60 --tries=10 --connect-timeout=30 --no-check-certificate"

echo "Downloading and building Protobuf/LLVM at $ICSC_HOME/build_deps..."
cd $ICSC_HOME
mkdir build_deps -p

# ################################################################################
# Download, unpack, build, install Protobuf
cd $ICSC_HOME/build_deps
${WGET} https://github.com/protocolbuffers/protobuf/archive/v3.19.4.tar.gz
tar -xf v3.19.4.tar.gz --skip-old-files
(
    cd protobuf-3.19.4
    mkdir build -p && cd build
    cmake ../cmake/ -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$ICSC_HOME \
                    -DBUILD_SHARED_LIBS=ON -Dprotobuf_BUILD_TESTS=OFF -DCMAKE_CXX_STANDARD=20
    make -j12
    make install
)

# ################################################################################
# Download, unpack, build, install Clang and LLVM
cd $ICSC_HOME/build_deps
${WGET} https://github.com/llvm/llvm-project/releases/download/llvmorg-18.1.8/cmake-18.1.8.src.tar.xz
${WGET} https://github.com/llvm/llvm-project/releases/download/llvmorg-18.1.8/clang-18.1.8.src.tar.xz
${WGET} https://github.com/llvm/llvm-project/releases/download/llvmorg-18.1.8/llvm-18.1.8.src.tar.xz
tar -xf cmake-18.1.8.src.tar.xz --skip-old-files
tar -xf clang-18.1.8.src.tar.xz --skip-old-files
tar -xf llvm-18.1.8.src.tar.xz --skip-old-files
ln -sfn `realpath cmake-18.1.8.src` cmake
ln -sfn `realpath clang-18.1.8.src` clang
ln -sfn `realpath llvm-18.1.8.src` llvm
(
    cd llvm-18.1.8.src
    mkdir build -p && cd build
    cmake ../ -DLLVM_ENABLE_ASSERTIONS=ON -DLLVM_TARGETS_TO_BUILD=X86 \
              -DLLVM_ENABLE_PROJECTS=clang -DCMAKE_BUILD_TYPE=Release -G "Unix Makefiles" \
              -DCMAKE_INSTALL_PREFIX=$ICSC_HOME -DGCC_INSTALL_PREFIX=$GCC_INSTALL_PREFIX \
              -DCMAKE_CXX_STANDARD=20 -DLLVM_INCLUDE_BENCHMARKS=OFF -DLLVM_INCLUDE_TESTS=OFF
    make -j12
    make install
)

# ################################################################################
# Download, unpack, build, install GDB with Python3
if [[ $1 == gdb ]]
then
    cd $ICSC_HOME/build_deps
    ${WGET} https://sourceware.org/pub/gdb/releases/gdb-13.2.tar.gz
    tar -xf gdb-13.2.tar.gz --skip-old-files
    (
        cd gdb-13.2
        ./configure --prefix="$ICSC_HOME" --with-python=/usr/bin/python3
        make -j12
        make install
    )
fi

# ################################################################################
# Build and install ISCC
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
    ln -s $ICSC_HOME/icsc/components/common $ICSC_HOME/designs/single_source/common
    source setenv.sh
    mkdir build -p && cd build
    cmake ../ -DCMAKE_BUILD_TYPE=$BUILD_TYPE  # prepare Makefiles
    cd designs/examples                # run examples only
    ctest -j12                         # compile and run Verilog generation
                                       # use "-jN" key to run in "N" processes
)
