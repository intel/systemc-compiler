#******************************************************************************
# Copyright (c) 2020, Intel Corporation. All rights reserved.
# 
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
# 
# *****************************************************************************

cmake_minimum_required(VERSION 3.5)
project(sctool)

set(CMAKE_CXX_STANDARD 17)

find_package(LLVM $ENV{LLVM_VER} EXACT REQUIRED CONFIG)

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -O3 -fno-tree-vectorize")

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
message(STATUS "LIBDIR ${LLVM_LIBRARY_DIRS}")
message(STATUS "LLVM_TOOLS_BINARY_DIR ${LLVM_TOOLS_BINARY_DIR}")
message(STATUS "LLVM_INCLUDE_DIRS ${LLVM_INCLUDE_DIRS}")
message(STATUS "CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE}")
message(STATUS "CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE}")

include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})
add_definitions(${LLVM_DEFINITIONS})


# Support for using custom linker
if( LLVM_USE_LINKER )
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=${LLVM_USE_LINKER}")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -fuse-ld=${LLVM_USE_LINKER}")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fuse-ld=${LLVM_USE_LINKER}")
    message("USING CUSTOM LINKER: ${CMAKE_EXE_LINKER_FLAGS}")
endif()

set(BUILD_SHARED_LIBS ON)

# Patched SystemC kernel with additional elaboration instrumentation
add_subdirectory(systemc)

# SystemC Precompiled header to speed-up parsing in debug build
SET(CLANG_CXX_EXECUTABLE ${LLVM_TOOLS_BINARY_DIR}/clang++)
SET(CLANG_INT_INC_DIR ${LLVM_LIBRARY_DIR}/clang/${LLVM_PACKAGE_VERSION}/include)

message(STATUS "CLANG_CXX_EXECUTABLE ${CLANG_CXX_EXECUTABLE}")
message(STATUS "CLANG_INT_INC_DIR ${CLANG_INT_INC_DIR}")

SET(SYSTEMC_PCH ${CMAKE_BINARY_DIR}/systemc/systemc.h.pch)

# Create SystemC pre-compiled headers
add_custom_command(OUTPUT ${SYSTEMC_PCH}
        COMMAND
        ${CLANG_CXX_EXECUTABLE} -Xclang -emit-pch -x c++-header
        ${CMAKE_SOURCE_DIR}/systemc/src/systemc.h -o ${SYSTEMC_PCH} ${MSVC_FLAGS}
        -D__SC_TOOL__ -D__SC_TOOL_ANALYZE__ -DNDEBUG -std=c++17
        -I${CMAKE_SOURCE_DIR}/systemc/src
        DEPENDS systemc
        COMMENT "Generating SystemC precompiled header ${SYSTEMC_PCH}"
        )
add_custom_target(systemcPCH DEPENDS ${SYSTEMC_PCH})

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Dynamic Elaboration (Requires GDB 8.0.1 configured with Python3)
add_subdirectory(sc_elab)

# ScTool main library
add_subdirectory(sc_tool)

# Integration tests :  SystemC Design -> Elaboration -> StaticAnalysis
set(SYSTEMC_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/systemc/src)
include(cmake/svc_target.cmake)

enable_testing()

# Compile tests and test designs
#if( ${CMAKE_BUILD_TYPE} STREQUAL "Debug" ) 
#    message (STATUS "Debug mode: add tests and test designs")
#    add_subdirectory(tests)
#    add_subdirectory(examples)
#    add_subdirectory(designs)
#endif()

###############################################################################

# Install targets
install (EXPORT SVCTargets NAMESPACE SVC::
         DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/SVC)

# Create the SVCConfig.cmake, installed @ICSC_HOME/lib/cmake/SVC
include(CMakePackageConfigHelpers)
configure_package_config_file(cmake/SVCConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/SVCConfig.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/SVC)

install(FILES ${PROJECT_BINARY_DIR}/SVCConfig.cmake cmake/svc_target.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/SVC)

# Install sctCommon
install(DIRECTORY components/common/sctcommon DESTINATION include)

# Install setenv scripts
install(FILES cmake/setenv.sh gdb/gdbinit-example.txt
        DESTINATION $ENV{ICSC_HOME})

# Install GDB pretty-printers
install(FILES gdb/sysc23x_printers/singlsrc_printers.py
        gdb/sysc23x_printers/sysc23x_printers.py
        DESTINATION $ENV{ICSC_HOME}/share/gdb/python/sysc23x_printers)

# Install designs
install(DIRECTORY designs DESTINATION $ENV{ICSC_HOME})


