/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Check class inheritance, find virtual function and so on
 * Author: Roman Popov
 */


#ifndef CHECKCPPINHERITANCE_H
#define CHECKCPPINHERITANCE_H

#include <sc_tool/cfg/ScState.h> 

namespace sc {

/// Compare two function declaration, 
/// return true if they have the same name and the same parameter types
bool compareFuncDecl(const clang::FunctionDecl* fst, const clang::FunctionDecl* sec);

/// Check if @tval is base class of @dyntval or these values are the same,  
/// return true if yes
bool checkBaseClass(const SValue& tval, const SValue& dyntval);

/// Get best virtual function for given function declaration and dynamic class
std::pair<SValue, clang::FunctionDecl*> getVirtualFunc(const SValue& tval,
                                                       clang::FunctionDecl* fdecl);

/// Find given class type in class hierarchy of @tval 
/// \return base class value or NO_VALUE
SValue getBaseClass(const SValue& tval, clang::QualType baseType);

/// Get parent class from variable @val declaration
/// \param val -- variable value
llvm::Optional<clang::QualType> getParentClass(const SValue& val);

/// Correct parent to real base class parent for MIF array element member variable
/// \param val -- variable value
void correctParentBaseClass(SValue& val);

/// Get direct base classes for module type
// Not used
//std::vector<clang::Type *> getModuleBases(const clang::CXXRecordDecl *decl);

}

#endif /* CHECKCPPINHERITANCE_H */

