/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Author: Roman Popov
 */

#ifndef SCTOOL_DEMANGLE_H
#define SCTOOL_DEMANGLE_H

#include <string>

namespace sc_elab
{

/// C++ ABI demangle
std::string demangle(const char* mangledName);

} // namespace sc_elab

#endif //SCTOOL_DEMANGLE_H
