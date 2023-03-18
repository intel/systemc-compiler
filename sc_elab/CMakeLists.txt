#******************************************************************************
# Copyright (c) 2020, Intel Corporation. All rights reserved.
# 
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
# 
# *****************************************************************************

# Generate C++ API for protobuf DB
find_package(protobuf REQUIRED)
message(STATUS "Protobuf executable is ${Protobuf_PROTOC_EXECUTABLE}")

# Add a target
add_library(sc_elab_proto)

# Generate the protobuf into the target
protobuf_generate(LANGUAGE cpp PROTOS sc_elab.proto TARGET sc_elab_proto)

# Must add the include directory for others to find
target_include_directories(sc_elab_proto PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)

# Install declaration(s)
install(TARGETS sc_elab_proto EXPORT SVCTargets DESTINATION lib)
