/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Author: Roman Popov
 */

#include <sc_tool/dyn_elab/MangledTypeDB.h>

#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>

#include <clang/AST/Mangle.h>

using namespace clang;
using namespace clang::ast_matchers;

namespace sc_elab {

MangledTypeDB::MangledTypeDB(clang::ASTContext &astCtx)
{

    auto recordDeclMatches = match(cxxRecordDecl().bind("cxxRecordDecl"), astCtx);

    MangleContext *mangleCtx = astCtx.createMangleContext();

    std::string mangledName;
    llvm::raw_string_ostream osStr{mangledName};

    // Iterate over all types in design and generated mangled names for them
    for (auto type : astCtx.getTypes()) {
        if (!type->isPlaceholderType() &&
            !type->isDependentType() &&
            !type ->isFixedPointType() && (
            type->isRecordType() ||
            type->isPointerType() ||
            type->isBuiltinType() ||
            type->isEnumeralType())) {


            if (auto builtinType =  type->getAs<BuiltinType>()) {
                // Skip types not yet supported
                if (builtinType->getKind() == BuiltinType::Kind::Float128)
                    continue;

                if (builtinType->getKind() == BuiltinType::Kind::BFloat16)
                    continue;
            }

            // Skip variable array types as they break @mangleTypeName()
            // These types could be only in testbench and not supported 
            // for synthesis anyway
            if (type->isVariableArrayType()) continue;
            if (type->isArrayType() || type->isPointerType()) {
                if (type->getPointeeOrArrayElementType()->isVariableArrayType()) 
                    continue;
            }
            
            auto cannonType = type->getCanonicalTypeInternal();

            //llvm::outs() << "Is placeholder? " << cannonType->isPlaceholderType() << " "<< cannonType.getAsString() << "\n";
            mangleCtx->mangleTypeName(cannonType, osStr);
            osStr.str();

#ifndef _MSC_VER 
            // Remove _ZTS
            mangledName = mangledName.substr(4);
#else
//            // Remove ?A
//            mangledName = mangledName.substr(2);
#endif // !_MSC_VER

//            llvm::outs() <<  mangledName << "\n";
            typeMap.emplace(mangledName, cannonType);

            mangledName.clear();
        }
    }
}

clang::QualType MangledTypeDB::getType(llvm::StringRef mangledTypeName)
{
    return typeMap.at(mangledTypeName.str());
}

} // namespace sc_elab