/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Author: Mikhail Moiseev
 */

#include "sc_tool/utils/CfgFabric.h"
#include "CfgFabric.h"

using namespace sc;
using namespace clang;

// Get fabric singleton
CfgFabric* CfgFabric::getFabric(const clang::ASTContext&  context_) {
    if (fabric == nullptr) {
        fabric = std::unique_ptr<CfgFabric>(new CfgFabric(context_));
    }
    return fabric.get();
}

// Get CFG for function declaration, build and store CFG if it not exist
clang::CFG* CfgFabric::get(const clang::FunctionDecl* funcDecl)
{
    return getFabric(funcDecl->getASTContext())->get_impl(funcDecl);
}

std::unique_ptr<CfgFabric> CfgFabric::fabric = nullptr;
clang::CFG *CfgFabric::get_impl(const clang::FunctionDecl *funcDecl)
{
    auto i = entries.find(funcDecl);

    if (i != entries.end()) {
        return i->second.get();

    } else {
        std::unique_ptr<CFG> cfg( CFG::buildCFG(funcDecl, funcDecl->getBody(),
                                                const_cast<ASTContext*>(&context),
                                                CFG::BuildOptions()) );

        auto result = cfg.get();
        entries.emplace(funcDecl, std::move(cfg));
        return result;
    }
}
