/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Author: Roman Popov
 */
#ifndef SCTOOL_BITUTILS_H
#define SCTOOL_BITUTILS_H

#include <cstddef>

/// Number of bits required to represent @n 
size_t bitsInNumber(size_t n);

/// Number of bits required to represent index for @n objects, (0...@n-1)
size_t bitsForIndex(size_t n);

#endif //SCTOOL_BITUTILS_H
