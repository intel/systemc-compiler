/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Author: Roman Popov
 */

#ifndef SCTOOL_SCELABFRONTENDACTION_H
#define SCTOOL_SCELABFRONTENDACTION_H

#include <clang/AST/ASTConsumer.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Tooling/Tooling.h>
#include <sc_elab.pb.h>

namespace sc_elab { 
    class ElabDatabase; 
}

namespace sc {

class SCElabASTConsumer : public clang::ASTConsumer 
{
public:
    static const std::string TOOL_VERSION;
    static const std::string TOOL_DATE;
    
    explicit SCElabASTConsumer() {}

    /// This is entry to SVC, called after Clang parses source to AST
    /// Currently only a single translation unit is supported
    void HandleTranslationUnit(clang::ASTContext &astCtx) final;
    
    /// Move dynamic objects from module where allocated to module where 
    /// pointer to this object
    std::unordered_map<size_t, size_t> moveDynamicObjects(
                                            sc_elab::SCDesign& designDB);

    /// Create *.sv output file, generate all Verilog modules and intrinsics
    void runVerilogGeneration(sc_elab::ElabDatabase& elabDB,
                        const std::unordered_map<size_t, size_t>& movedObjs);
};

class SCElabFrontendAction : public clang::ASTFrontendAction 
{
public:
    explicit SCElabFrontendAction()
    {}

    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
        clang::CompilerInstance &compiler,
        llvm::StringRef /*inFile*/);

};

std::unique_ptr<clang::tooling::FrontendActionFactory>
getNewSCElabActionFactory();


} // namespace sc_elab


#endif //SCTOOL_SCELABFRONTENDACTION_H
