/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Author: Mikhail Moiseev
 */

#ifndef CFGFABRIC_H
#define CFGFABRIC_H

#include "clang/Analysis/CFG.h"
#include "clang/AST/Decl.h"
#include <unordered_map>
#include <memory>

namespace sc {
    
class CfgFabric final
{
public:
    /// Get fabric singleton
    static CfgFabric* getFabric(const clang::ASTContext&  context_);
    
    /// Get CFG for function declaration, build and store CFG if it not exist
    static clang::CFG* get(const clang::FunctionDecl* funcDecl);

private:
    clang::CFG* get_impl(const clang::FunctionDecl* funcDecl);

    CfgFabric(const clang::ASTContext&  context_) : context(context_)
    {}
    
    static std::unique_ptr<CfgFabric> fabric;

    /// AST context
    const clang::ASTContext&  context;
    /// CFG of all called functions
    std::unordered_map<const clang::FunctionDecl*, 
                       std::unique_ptr<clang::CFG> >    entries;
};

}

#endif /* CFGFABRIC_H */

