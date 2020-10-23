/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Author: Roman Popov
 */

#include <sc_tool/dyn_elab/Demangle.h>

#include <cstdlib>
#include <memory>

#ifndef _MSC_VER

#include <cxxabi.h>

namespace sc_elab {

std::string demangle(const char* mangledName) {

    int status = -4; // some arbitrary value to eliminate the compiler warning

    // enable c++11 by passing the flag -std=c++11 to g++
    std::unique_ptr<char, void(*)(void*)> res {
        abi::__cxa_demangle(mangledName, NULL, NULL, &status),
        std::free
    };

    return (status==0) ? res.get() : mangledName ;
}

} // namespace sc_elab

#else

std::string demangle(const char* mangledName) {
    // TODO
    return mangledName;
}

#endif