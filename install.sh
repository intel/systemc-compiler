#!/bin/bash -e

test -z $ICSC_HOME && { echo "ICSC_HOME is not configured"; exit 1; }
echo "Using ICSC_HOME = $ICSC_HOME"

export CWD_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
export CMAKE_PREFIX_PATH=$ICSC_HOME:$CMAKE_PREFIX_PATH

echo "Downloading and building Protobuf/LLVM at $CWD_DIR/build_deps..."
mkdir build_deps -p && cd build_deps

# Download, unpack, build, install Protobuf 3.13
wget -N https://github.com/protocolbuffers/protobuf/archive/v3.13.0.tar.gz --no-check-certificate
tar -xf v3.13.0.tar.gz --skip-old-files
(
    cd protobuf-3.13.0
    mkdir build -p && cd build
    cmake ../cmake/ -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$ICSC_HOME -DBUILD_SHARED_LIBS=ON -Dprotobuf_BUILD_TESTS=OFF
    make -j12
    make install
)

# Download, unpack, build, install Clang and LLVM
wget -N https://github.com/llvm/llvm-project/releases/download/llvmorg-12.0.1/clang-12.0.1.src.tar.xz --no-check-certificate
wget -N https://github.com/llvm/llvm-project/releases/download/llvmorg-12.0.1/llvm-12.0.1.src.tar.xz --no-check-certificate
tar -xf clang-12.0.1.src.tar.xz --skip-old-files
tar -xf llvm-12.0.1.src.tar.xz --skip-old-files
ln -sf ../../clang-12.0.1.src llvm-12.0.1.src/tools/clang
(
    cd llvm-12.0.1.src
    mkdir build -p && cd build
    cmake ../ -DLLVM_ENABLE_ASSERTIONS=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$ICSC_HOME
    make -j12
    make install
)

# Download, unpack, build, install GDB with Python3
#wget -N https://ftp.gnu.org/gnu/gdb/gdb-11.2.tar.gz --no-check-certificate
#tar -xf gdb-11.2.tar.gz --skip-old-files
#(
#    cd gdb-11.2
#    ./configure --prefix="$ICSC_HOME" --with-python=/usr/bin/python3
#    make -j12
#    make install
#)


# Build and install ISCC
cd $CWD_DIR
(
    mkdir build_icsc_rel -p && cd build_icsc_rel
    cmake ../ -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$ICSC_HOME
    make -j12
    make install
    
    cd ..
    
    mkdir build_icsc_dbg -p && cd build_icsc_dbg
    cmake ../ -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=$ICSC_HOME -DCMAKE_DEBUG_POSTFIX=d
    make -j12
    make install
)
echo "*** ISCC Build and Installation Complete! ***"


# ################################################################################
# Build Tests using ISCC
echo "*** Build Examples ***"
cd $CWD_DIR
(
    source $ICSC_HOME/setenv.sh
    cd designs/examples
    mkdir build -p && cd build
    cmake ../                          # prepare Makefiles

    ctest -j12                         # compile and run Verilog generation
                                       # use "-jN" key to run in "N" processes
)
