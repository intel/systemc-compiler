#!/bin/bash -e

#export ICSC_HOME=`realpath .`
echo "Setting ICSC_HOME = $ICSC_HOME"

# Clone ISCC
#git clone https://github.com/intel/systemc-compiler $ICSC_HOME/icsc
cd $ICSC_HOME

# Download, unpack, build, install Protobuf 3.13
##wget https://github.com/protocolbuffers/protobuf/releases/download/v3.13.0/protoc-3.13.0-linux-x86_64.zip --no-check-certificate
wget https://github.com/protocolbuffers/protobuf/archive/v3.13.0.tar.gz --no-check-certificate
tar -xvf v3.13.0.tar.gz
(
    cd protobuf-3.13.0
    mkdir build -p && cd build
    cmake ../cmake/ -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$ICSC_HOME -DBUILD_SHARED_LIBS=ON -Dprotobuf_BUILD_TESTS=OFF
    make -j12
    make install
)
cd $ICSC_HOME

# Download, unpack, build, install Clang and LLVM
wget https://github.com/llvm/llvm-project/releases/download/llvmorg-12.0.1/clang-12.0.1.src.tar.xz --no-check-certificate
wget https://github.com/llvm/llvm-project/releases/download/llvmorg-12.0.1/llvm-12.0.1.src.tar.xz --no-check-certificate
tar -xvf clang-12.0.1.src.tar.xz
tar -xvf llvm-12.0.1.src.tar.xz
mv clang-12.0.1.src llvm-12.0.1.src/tools/clang
(
    cd llvm-12.0.1.src
    mkdir build -p && cd build
    cmake ../ -DLLVM_ENABLE_ASSERTIONS=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$ICSC_HOME
    make -j12
    make install
)
cd $ICSC_HOME

export CMAKE_PREFIX_PATH=$ICSC_HOME:$CMAKE_PREFIX_PATH

# Build and install ISCC
(
    cd icsc
    mkdir build -p && cd build
    cmake ../ -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$ICSC_HOME
    make -j12
    make install
)
cd $ICSC_HOME

# ################################################################################
# Build Tests using ISCC
(
    source $ICSC_HOME/setenv.sh
    mkdir build -p && cd build
    cmake ../                          # prepare Makefiles 
    cd icsc/examples                   # build examples only, 
                                       # comment this line to build all tests
    ctest -j12                         # compile and run Verilog generation
                                       # use "-jN" key to run in "N" processes
)
