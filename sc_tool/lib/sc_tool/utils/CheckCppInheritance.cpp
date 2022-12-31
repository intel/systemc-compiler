/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Author: Roman Popov
 */

#include "sc_tool/utils/CheckCppInheritance.h"
#include "sc_tool/utils/ScTypeTraits.h"
#include "sc_tool/utils/CppTypeTraits.h"
#include "clang/AST/DeclTemplate.h"

namespace sc {

using namespace std;
using namespace clang;
using namespace llvm;

// Compare two function declaration, 
// return true if they have the same name and the same parameter types
bool compareFuncDecl(const FunctionDecl* fst, const FunctionDecl* sec) 
{
    if (fst->getNameAsString() != sec->getNameAsString()) {
        return false;
    }
    
    if (fst->getNumParams() != sec->getNumParams()) {
        return false;
    }
    
    for (unsigned i = 0; i < fst->getNumParams(); i++) {
        // Use string comparison as the SC types are not equal
        if (fst->getParamDecl(i)->getType().getAsString() != 
            sec->getParamDecl(i)->getType().getAsString()) {
            return false;
        }
    }
    
    return true;
}

// Check if @tval is base class of @dyntval or these values are the same,  
// return true if yes
bool checkBaseClass(const SValue& tval, const SValue& dyntval) 
{
    SCT_TOOL_ASSERT (tval.isRecord() && dyntval.isRecord(), 
                     "Not record in checkBaseClass()");

    if (tval == dyntval) {
        return true;
    }
    
    for (auto base : dyntval.getRecord().bases) {
        if (checkBaseClass(tval, base)) {
            return true;
        }
    }
    return false;    
}

// Get best virtual function for given function declaration and dynamic class
pair<SValue, FunctionDecl*> getVirtualFunc(const SValue& tval, 
                                           FunctionDecl* fdecl) 
{
    //cout << "----- getVirtualFunc tval " << tval << " fdecl " << fdecl->getNameAsString() << endl;
    SCT_TOOL_ASSERT (tval.isRecord(), "No record in getVirtualFunc()");

    CXXRecordDecl* recordDecl = tval.getRecord().getType()->getAsCXXRecordDecl();
    CXXMethodDecl* methodDecl = dyn_cast<CXXMethodDecl>(fdecl);

    auto dynMetDecl = methodDecl->getCorrespondingMethodInClass(recordDecl);
    
//    cout << "  methodDecl " << (methodDecl ? methodDecl->getAsFunction()->getNameAsString() : "--")<< ", tval " << tval << endl;
//    cout << "  dynMetDecl recordDecl " << (dynMetDecl ? dynMetDecl->getParent()->getNameAsString() : "--") << endl;
//    cout << "  Curr recordDecl " << (recordDecl ? recordDecl->getNameAsString() : "--") << endl;
    
    // @getCorrespondingMethodInClass returns @dynMetDecl even if it defined in 
    // base class, so it needs to check @dynMetDecl record is this class 
    // Return exact class with method definition required to access channel in it. 
    if (dynMetDecl && dynMetDecl->getParent() == recordDecl) {

        if (dynMetDecl->isPure()) {
            cout << recordDecl->getQualifiedNameAsString() << endl;
            recordDecl->dumpColor();
            SCT_TOOL_ASSERT(false, "Pure virtual method");
        }
        return pair<SValue, FunctionDecl*>(tval, dynMetDecl);
    }

    // Recursively check in base classes
    if (tval.getRecord().bases.size() > 0) {
        return getVirtualFunc(tval.getRecord().bases.at(0), fdecl);
    }
    
    ScDiag::reportScDiag(fdecl->getSourceRange().getBegin(),
                         ScDiag::CPP_NO_VIRT_FUNC) << fdecl->getNameAsString();
    return pair<SValue, FunctionDecl*>(tval, fdecl);
}

// Find given class type in class hierarchy of @tval 
// \return base class value or NO_VALUE
SValue getBaseClass(const SValue& tval, QualType baseType)
{
    SCT_TOOL_ASSERT (tval.isRecord(), "No record in getBaseClass()");
    
    // Check if @tval has required type itself
    auto thisType = getPureType(tval.getRecord().getType());
    baseType = getPureType(baseType);
    if (thisType == baseType) {
        return tval;
    }
    
    for (const auto& bval : tval.getRecord().bases) {
        thisType = getPureType(bval.getRecord().getType());
        if (thisType == baseType) {
            return bval;
        }
        
        SValue res = getBaseClass(bval, baseType);
        if (res != NO_VALUE) {
            return res;
        }
    }
    
    return NO_VALUE;
}


// Get parent class where variable @val is declared
// \param val -- variable value
llvm::Optional<QualType> getParentClass(const SValue& val)
{
    SCT_TOOL_ASSERT (val.isVariable(), "Value is not variable");
    
    auto decl = val.getVariable().getDecl();
    if (decl && !decl->isInvalidDecl()) {
        auto declCtx = decl->getDeclContext();

        if (auto recDecl = dyn_cast<RecordDecl>(declCtx)) {
            return QualType(recDecl->getTypeForDecl(), 0);
        }
    }
    
    return llvm::None;
}


// Correct parent to real base class parent for MIF array element member variable
// \param val -- variable value
void correctParentBaseClass(SValue& val)
{
    if (val.isVariable()) {
        SValue parent = val.getVariable().getParent();
        if (parent.isRecord()) {
            if (auto baseParentType = getParentClass(val)) {
                SValue baseParent = getBaseClass(parent, *baseParentType);
                SCT_TOOL_ASSERT (baseParent.isRecord(), "Base parent is not record");

                // Create variable with updated parent
                auto varDecl = val.getVariable().getDecl();
                val = SValue(varDecl, baseParent);
                //cout << "Corrected parent, val " << val << endl;
            }
        }
    }
}

// Get direct base classes for module type
// Not used
/*std::vector<clang::Type*> getModuleBases(const clang::CXXRecordDecl* decl) 
{
    std::vector<clang::Type*> bases;
    for (auto&& b: decl->bases()) {
        const auto* baseClass = b.getType()->getAsCXXRecordDecl();
        if (baseClass == nullptr) {
            if(llvm::dyn_cast<clang::CXXRecordDecl>(
                    b.getType()->getAs<clang::TemplateSpecializationType>()->
                    getTemplateName().getAsTemplateDecl()->getTemplatedDecl())) 
            {
                clang::Type* type = const_cast<clang::Type*>(baseClass->getTypeForDecl());
                bases.push_back(type);
            }
        } else {
            if (baseClass->getQualifiedNameAsString() != "sc_core::sc_module") {
                clang::Type* type = const_cast<clang::Type*>(baseClass->getTypeForDecl());
                bases.push_back(type);
            }    
        }
    }
    return bases;
}*/

}