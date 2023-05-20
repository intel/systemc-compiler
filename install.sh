#!/bin/bash -e

#********************************************************************************
# Copyright (c) 2020-2023, Intel Corporation. All rights reserved.              #
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

test -z $ICSC_HOME && { echo "ICSC_HOME is not configured"; exit 1; }
echo "Using ICSC_HOME = $ICSC_HOME"

export CWD_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
echo $CWD_DIR
export CMAKE_PREFIX_PATH=$ICSC_HOME:$CMAKE_PREFIX_PATH
export GCC_INSTALL_PREFIX="$(realpath "$(dirname $(which g++))"/..)"

echo "Downloading and building Protobuf/LLVM at $CWD_DIR/build_deps..."
mkdir build_deps -p && cd build_deps

# ################################################################################
# Download, unpack, build, install Protobuf 3.13
wget -N https://github.com/protocolbuffers/protobuf/archive/v3.13.0.tar.gz --no-check-certificate
tar -xf v3.13.0.tar.gz --skip-old-files
(
    cd protobuf-3.13.0
    mkdir build -p && cd build
    cmake ../cmake/ -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$ICSC_HOME -DBUILD_SHARED_LIBS=ON -Dprotobuf_BUILD_TESTS=OFF -DCMAKE_CXX_STANDARD=17
    make -j12
    make install
)

# ################################################################################
# Download, unpack, build, install Clang and LLVM
wget -N https://github.com/llvm/llvm-project/releases/download/llvmorg-12.0.1/clang-12.0.1.src.tar.xz --no-check-certificate
wget -N https://github.com/llvm/llvm-project/releases/download/llvmorg-12.0.1/llvm-12.0.1.src.tar.xz --no-check-certificate
tar -xf clang-12.0.1.src.tar.xz --skip-old-files
tar -xf llvm-12.0.1.src.tar.xz --skip-old-files
ln -sf ../../clang-12.0.1.src llvm-12.0.1.src/tools/clang
(
    cd llvm-12.0.1.src
    mkdir build -p && cd build
    cmake ../ -DLLVM_ENABLE_ASSERTIONS=ON -DLLVM_TARGETS_TO_BUILD="X86" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$ICSC_HOME -DGCC_INSTALL_PREFIX=$GCC_INSTALL_PREFIX -DCMAKE_CXX_STANDARD=17
    make -j12
    make install
)

# ################################################################################
# Download, unpack, build, install GDB with Python3
if [[ $1 == gdb ]]
then
   wget -N https://ftp.gnu.org/gnu/gdb/gdb-11.2.tar.gz --no-check-certificate
   tar -xf gdb-11.2.tar.gz --skip-old-files
   (
       cd gdb-11.2
       ./configure --prefix="$ICSC_HOME" --with-python="$(which python3)"
       make -j12
       make install
   )
fi

# ################################################################################
# Build and install ISCC
cd $CWD_DIR
(
    mkdir build_icsc_rel -p && cd build_icsc_rel
    cmake ../ -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$ICSC_HOME -DCMAKE_CXX_STANDARD=17
    make -j12
    make install
    
    cd ..
    
    mkdir build_icsc_dbg -p && cd build_icsc_dbg
    cmake ../ -DCMAKE_BUILD_TYPE=Debug   -DCMAKE_INSTALL_PREFIX=$ICSC_HOME -DCMAKE_CXX_STANDARD=17 -DCMAKE_DEBUG_POSTFIX=d
    make -j12
    make install

    cp $CWD_DIR/cmake/CMakeLists.top $ICSC_HOME/CMakeLists.txt
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
    cmake ../                          # prepare Makefiles
    cd designs/examples                # run examples only
    ctest -j12                         # compile and run Verilog generation
                                       # use "-jN" key to run in "N" processes
)
