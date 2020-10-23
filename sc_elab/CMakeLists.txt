#******************************************************************************
# Copyright (c) 2020, Intel Corporation. All rights reserved.
# 
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
# 
# *****************************************************************************

# Generate C++ API for protobuf DB
find_package(Protobuf REQUIRED)
message("Using Protobuf from ${Protobuf_INCLUDE_DIRS}")

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS sc_elab.proto)

add_library(sc_elab_proto ${PROTO_SRCS} ${PROTO_HDRS} )
target_link_libraries(sc_elab_proto ${Protobuf_LIBRARIES})
target_include_directories(sc_elab_proto PUBLIC ${Protobuf_INCLUDE_DIRS}
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        )

target_compile_definitions(sc_elab_proto PUBLIC GOOGLE_PROTOBUF_NO_RTTI)

install(TARGETS sc_elab_proto EXPORT SVCTargets DESTINATION lib)
