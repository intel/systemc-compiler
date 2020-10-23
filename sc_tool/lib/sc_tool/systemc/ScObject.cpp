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
uint64_t ScDynamicName::id_gen = 0;

uint64_t ScDynamicName::getId() {
    return id_gen++;
}

bool ScDynamicName::operator == (const ScDynamicName& rhs) const {
    return (loc == rhs.loc);
}

std::string DynamicNameFabric::getName(const clang::SourceLocation& loc) {
    ScDynamicName name(loc);
    auto res = names.insert(name);
    if (!res.second) name = *(res.first);
    return ("CH_"+std::to_string(name.getId()));
}

// ============================================================================

/// Unique ID generator
uint64_t ScObject::id_gen = 0;

bool ScObject::operator == (const ScObject& rhs) {
    return (id == rhs.id);
}
}