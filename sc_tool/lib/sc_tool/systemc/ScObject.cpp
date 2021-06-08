/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Author: Mikhail Moiseev
 */

#include "sc_tool/systemc/ScObject.h"

namespace sc {

/// Unique ID generator
uint64_t ScObject::id_gen = 0;

bool ScObject::operator == (const ScObject& rhs) {
    return (id == rhs.id);
}
}