#!/bin/bash -e

#********************************************************************************
# Copyright (c) 2020, Intel Corporation. All rights reserved.                   #
#                                                                               #
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.                      #
#                                                                               #
#********************************************************************************

#################################################################################
# This bash script downloads and builds Protobuf, Clang/LLVM, GDB (optional),   #
# builds and installs ICSC, runs ICSCS examples.                                #
#################################################################################


# NOCHECKSERT=--no-check-certificate


function usage() {
    echo "Usage: $0 <install prefix> [--debug|--release|--rel-debug] [proto] [llvm] [gdb] [icsc]"
    echo ""
    echo "Optionally download, compile and install the components."
    echo "<install prefix> is the installation target folder"
    echo ""
    echo "       * Protobuf"
    echo "       * LLVM and Clang"
    echo "       * SystemC simulation libraries"
    echo "       * GDB with Python3"
    echo "       * Verlog code generation tool and SystemC libraries"
    echo ""
    echo "Building icsc depends on having proto and llvm compiled in the .\build_deps\ folder."
    echo "Installing all in one command takes care of the build order."
    echo ""
    echo "Add --debug, --no-debug, or --rel-debug before components to switch debug mode on or off"
    echo "Example:"
    echo "  $0 /tmp/icsc --release proto --debug llvm icsc"
    echo ""
    echo "icsc will always be compiled in both release and debug mode"
    exit 1;
}

function maybe_download() {
    if [ ! -z "${download[$1]}" ] || [ ! -f "$(basename "$2")" ]; then (
        wget "$2" $NOCHECKSERT
        tar -xf "$(basename "$2")" --skip-old-files
    );
    fi;
}

function dump() {
    if [ "${build_type[$1]}" == "" ]; then (
        printf "* %-7s Skip\n" $1
    );
    else (
        printf "* %-7s %-17s %s\n" $1 "${build_type[$1]}"  "${download[$1]}"
    );
    fi;
}

test -z "$1" && usage
[[ "$1" =~ ^(proto|llvm|gdb|icsc)$ ]] && usage
[[ "$1" =~ ^-- ]] && usage

LLVM_VER=12.0.1

CMAKE_INSTALL_PREFIX=$(realpath $1)
CMAKE_PREFIX_PATH=$CMAKE_INSTALL_PREFIX
CWD_DIR=$(realpath $(dirname "${BASH_SOURCE[0]}"))

cd $CWD_DIR
mkdir build_deps -p
cd build_deps

echo "***************************************************************"
echo "* Building from: $CWD_DIR"
echo "* Building in:   $CWD_DIR/build_deps"
echo "* Installing to: $CMAKE_INSTALL_PREFIX"
echo "*"

CMAKE_PREFIX_PATH=$ICSC_HOME:$CMAKE_PREFIX_PATH
GCC_INSTALL_PREFIX="$(realpath "$(dirname $(which g++))"/..)"

CMAKE_BUILD_TYPE=Release
DOWNLOAD=

shift

declare -A build_type
declare -A download

while [ "$1" != "" ]; do
    case "$1" in
    "--debug")
        CMAKE_BUILD_TYPE=Debug
        ;;
    "--release")
        CMAKE_BUILD_TYPE=Release
        ;;
    "--rel-debug")
        CMAKE_BUILD_TYPE=RelWithDebInfo
        ;;
    "--download")
        DOWNLOAD=--download
        ;;
    "proto")
        build_type['proto']=$CMAKE_BUILD_TYPE
        download['proto']=$DOWNLOAD
        ;;
    "llvm")
        build_type['llvm']=$CMAKE_BUILD_TYPE
        download['llvm']=$DOWNLOAD
        echo "$build_type"
        ;;
    "gdb")
        build_type['gdb']=$CMAKE_BUILD_TYPE
        download['gdb']=$DOWNLOAD
        ;;
    "icsc")
        build_type['icsc']='Debug + Release'
        ;;
    *)
        echo "'$1'"
        usage
        ;;
    esac
    shift
done;

echo "*"
dump "proto"
dump "llvm"
dump "gdb"
dump "icsc"
echo "*"
echo "Press ENTER to continue...."
read

# ################################################################################
# Download, unpack, build, install Protobuf 3.13
if [ "${build_type['proto']}" != "" ]; then (
    maybe_download proto https://github.com/protocolbuffers/protobuf/archive/v3.13.0.tar.gz
    CMAKE_BUILD_TYPE="${build_type['proto']}"
    (
        cd protobuf-3.13.0
        cmake cmake/ -Bbuild -DBUILD_SHARED_LIBS=ON -Dprotobuf_BUILD_TESTS=OFF -DCMAKE_CXX_STANDARD=17 -DCMAKE_INSTALL_PREFIX=$CMAKE_INSTALL_PREFIX -DCMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE
        cd build
        make -j12 install
    )
);
fi;

# ################################################################################
# Download, unpack, build, install Clang and LLVM
if [ "${build_type['llvm']}" != "" ]; then (
    maybe_download llvm https://github.com/llvm/llvm-project/releases/download/llvmorg-$LLVM_VER/clang-$LLVM_VER.src.tar.xz
    maybe_download llvm https://github.com/llvm/llvm-project/releases/download/llvmorg-$LLVM_VER/llvm-$LLVM_VER.src.tar.xz
    CMAKE_BUILD_TYPE="${build_type['llvm']}"
    (
        cd llvm-$LLVM_VER.src
        ln -sf ../../clang-$LLVM_VER.src tools/clang
        cmake ./ -Bbuild -DLLVM_ENABLE_ASSERTIONS=ON -DLLVM_TARGETS_TO_BUILD="X86" -DCMAKE_BUILD_TYPE=Release -DGCC_INSTALL_PREFIX=$GCC_INSTALL_PREFIX -DCMAKE_CXX_STANDARD=17 -DCMAKE_INSTALL_PREFIX=$CMAKE_INSTALL_PREFIX -DCMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE
        cd build
        make -j12 install
    )
);
fi;

# ################################################################################
# Download, unpack, build, install GDB with Python3
if [ "${build_type['gdb']}" != "" ]; then (
    maybe_download gdb https://ftp.gnu.org/gnu/gdb/gdb-11.2.tar.gz
    CMAKE_BUILD_TYPE="${build_type['gdb']}"
    (
        cd gdb-11.2
        ./configure --prefix="$ICSC_HOME" --with-python="$(which python3)"
        make -j12 install
    )
);
fi;

# ################################################################################
# Build and install ISCC
if [ "${build_type['icsc']}" != "" ]; then (
    cd $CWD_DIR
    cmake . -Bbuild_icsc/release -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_STANDARD=17 -DCMAKE_INSTALL_PREFIX=$CMAKE_INSTALL_PREFIX
    cd build_icsc/release && make -j12 install

    cmake . -Bbuild_icsc/debug -DCMAKE_BUILD_TYPE=Debug -DCMAKE_DEBUG_POSTFIX=d -DCMAKE_CXX_STANDARD=17 -DCMAKE_INSTALL_PREFIX=$CMAKE_INSTALL_PREFIX
    cd build_icsc/debug && make -j12 install
);
fi;

# # ################################################################################
# # Build and run examples
# echo "*** Building Examples ***"
# cd $ICSC_HOME
# (
#     source setenv.sh
#     mkdir build -p && cd build
#     cmake ../                          # prepare Makefiles
#     cd designs/examples                # run examples only
#     ctest -j12                         # compile and run Verilog generation
#                                        # use "-jN" key to run in "N" processes
# )

