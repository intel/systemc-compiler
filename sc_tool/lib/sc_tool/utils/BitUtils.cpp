/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Author: Roman Popov
 */

#include "BitUtils.h"
#include <iostream>

// Number of bits required to represent @n 
size_t bitsInNumber(size_t n)
{
    if (n == 0) {
        return 1;
    }

    size_t res = 0;
    do {
        res += 1;
        n >>= 1;
    } while (n != 0);

    return res;
}

// Number of bits required to represent index for @n objects, (0...@n-1)
size_t bitsForIndex(size_t n) 
{
    if (n == 0) {
        return 1;
    }
    return bitsInNumber(n-1);
}

