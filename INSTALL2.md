# Introduction of new install script

## Reason

In some environments, the system admin may require the installed files to be separate from the source code.
The added install2.sh script allows one to set the install prefix, as well as specifying per-package CMAKE_BUILD_TYPE.

## Usage

./install2.sh <install prefix> [--debug|--release|--rel-debug] [--download] [proto] [llvm] [gdb] [icsc]

The debug/release switches can be placed before any of the packages (proto, llvm, gdb, icsc),
and they can be mixed on one command line:

./install2.sh /tmp/icsc --release llvm --debug proto icsc

This example will install to /tmp/icsc.
It will download, compile and install release versions of llvm.
It will download, compile and install debug versions of protobuf.
It will compile and install both debug and release versions of icsc.

The icsc package is always compiled as both Release and Debug, with "d" library postfix for the debug libraries.

The script will not download a package a second time, unless --download is also specified.
Not that all of the switches (--debug, --release, --rel-debug, --download) take effect on all packages following it.

The script will mind the correct build order (llvm + protobuf + gdb first, then icsc).
You can omit packages already compiled.

## Changelist

* Scripts and files updated to use CMAKE_INSTALL_PREFIX set in the install script(s)
* Examples updated to reduce dependency of $ENV{ICSC_HOME}
* Use newer CMake protobuf interface
* cmake/CMakeLists.top.in -- search for SVC using CMAKE_INSTALL_PREFIX path
* cmake/SVCConfig.cmake.in
  * set ICSC_LLVM_VER to the installed version, to help matching the correct LLVM runtime
  * set properties on the SVC::systemc and SVC::SCTool targets ("modern" cmake interface)
* cmake/svc_target.cmake
  * "modern" cmake interface
* design/**/CMakeLists.txt -- removed stuff not needed
