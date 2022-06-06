/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * SystemCRTTI.h - RTTI-free wrapper over SystemC library functions, required
 * for dynamic elaboration.
 * 
 * SVC Elaboration can't include SystemC headers directly, because it is
 * compiled without RTTI.
 * 
 * Author: Roman Popov
 */

#ifndef SCTOOL_SYSTEMC_RTTI_H
#define SCTOOL_SYSTEMC_RTTI_H

#include <vector>
#include <string>
#include <sc_elab/sensitivity.h>
#include <sc_elab/process_type_info.h>
#include <llvm/ADT/APSInt.h>

namespace sc_core {
class sc_module;
class sc_object;
class sc_port_base;
class sc_process_b;
class sc_simcontext;
}

namespace sc_elab
{

sc_core::sc_simcontext* getScSimContext();

// TODO: check on windows
/// Get linkage name, works only for types with typeinfo
std::string getDynamicMangledTypeName(const void *objPtr);

/// Get module and function name for process
/// sc_spawn is not supported
process_type_info getProcessTypeInfo(const sc_core::sc_object *procPtr);

/// Find module by hierarchical name
/// If name is empty returns ptr to top-level module
void *getTopModulePtr(const std::string &name="");

/// Get pointer to parent port (for hierarchical binds)
sc_core::sc_port_base **getFirstParentPortPtr(sc_core::sc_port_base *port);

// convert sc_int/sc_bigint to APSInt
llvm::APSInt getScIntValue(const void *objPtr);
llvm::APSInt getScUIntValue(const void *objPtr);
llvm::APSInt getScBigIntValue(const void *objPtr);
llvm::APSInt getScBigUIntValue(const void *objPtr);
llvm::APSInt getScBitVectorValue(const void *objPtr);

/// Get list of processes sensitive to given port
std::vector<sc_elab::port_sens_proc>
getSensitiveProcs (const sc_core::sc_port_base *portPtr);

} // end namespace sc_elab


#endif //SCTOOL_SYSTEMC_RTTI_H
