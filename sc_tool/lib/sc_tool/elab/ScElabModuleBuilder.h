/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Author: Roman Popov
 */

#ifndef SCELABMODULEBUILDER_H
#define SCELABMODULEBUILDER_H

#include <unordered_map>

namespace sc_elab {

class ElabDatabase;

/// Creates Verilog modules inside elaboration database
void buildVerilogModules(sc_elab::ElabDatabase *elabDB,
                         const std::unordered_map<std::size_t, std::size_t>& movedObjs);

} // end namespace sc


#endif // SCELABMODULEBUILDER_H
