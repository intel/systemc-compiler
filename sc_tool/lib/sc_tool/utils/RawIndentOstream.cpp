/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Author: Roman Popov
 */

#include "RawIndentOstream.h"

namespace sc {

void RawIndentOstream::write_impl(const char *ptr, size_t size)
{
    for (size_t i = 0; i < size; ++i) {
        if(indentNext) {
            for (auto &fillStr : fillVec)
                backend << fillStr;
            indentNext = false;
        }

        backend << *ptr;

        if (*ptr == '\n')
            indentNext = true;

        ptr++;
    }

    backend.flush();
}

}

