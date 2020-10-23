/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Author: Roman Popov
 */

#ifndef SCTOOL_SCTOOL_H
#define SCTOOL_SCTOOL_H

#include <string>
#include <iostream>

/*
 * This header should be included before other headers in SVC unity source
 * (single translation unit, that merges all DUT sources)
 */

/// Parameters for SVC Clang tool should be supplied using this string
extern const char* __sctool_args_str;


namespace sc {

/// Entry point to SVC tool
[[ noreturn ]] void runScElab(const char * commandLine);

/// This function should replace regular sc_start, to run code generation instead
/// of simulation
template <class ...Ts>
void sctool_start(Ts... args) {
    runScElab(__sctool_args_str);
}

}

/*
 * SC_ELAB2_BUILD is defined during SCTool library build. It should not be defined
 * during application build
 */
#ifndef SC_ELAB2_BUILD
    // override sc_start
    #include <sysc/kernel/sc_simcontext.h>

    // Replace sc_start with SVC sctool_start
    #define sc_start(...) \
            sc::sctool_start(__VA_ARGS__)
#endif

#endif //SCTOOL_SCTOOL_H
