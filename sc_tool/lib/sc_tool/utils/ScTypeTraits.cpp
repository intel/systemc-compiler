/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Author: Roman Popov
 * Modified by: Mikhail Moiseev
 */
#include "ScTypeTraits.h"

#include "sc_tool/utils/CppTypeTraits.h"
#include "sc_tool/diag/ScToolDiagnostic.h"
#include <clang/AST/DeclCXX.h>
#include <clang/AST/Decl.h>
#include <clang/AST/Expr.h>
#include <clang/AST/ExprCXX.h>
#include <clang/AST/Type.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <unordered_set>
#include <string>
#include <iostream>


using namespace llvm;
using namespace clang;
using namespace clang::ast_matchers;

namespace sc
{

namespace {
    
using std::cout; using std::endl; using std::hex; using std::dec;

auto getDeclContexts(const clang::NamedDecl *decl)
{
    const DeclContext *declCtx = decl->getDeclContext();
    llvm::SmallVector<const clang::NamedDecl *, 5>  contexts;

    // Collect contexts
    while (declCtx && isa<NamedDecl>(declCtx)) {
        contexts.push_back(dyn_cast<NamedDecl>(declCtx));
        declCtx = declCtx->getParent();
    }

    return contexts;
}

bool isDerivedFrom(clang::QualType type, clang::QualType baseClassType) 
{
    if (type.isNull()) return false;
    
    if (type.getCanonicalType() == baseClassType)
        return true;
    
    if (CXXRecordDecl* recDecl = type->getAsCXXRecordDecl()) {
        recDecl = recDecl->getDefinition();
        
        if (!recDecl) {
            type->getAsCXXRecordDecl()->getTypeForDecl()->dump();
            SCT_TOOL_ASSERT (false, "No definition for declaration");
        }
        
        for (auto base : recDecl->bases()) {
            if (isDerivedFrom(base.getType(), baseClassType))
                return true;
        }
    }

    return false;
}

const clang::ClassTemplateDecl* getAsClassTemplateDecl(clang::QualType type) 
{
    if (type.isNull()) return nullptr;
    
    if (auto recDecl = type->getAsCXXRecordDecl()) {
        if (auto tempSpecDecl = 
            dyn_cast<clang::ClassTemplateSpecializationDecl>(recDecl)) {

            return tempSpecDecl->getSpecializedTemplate()->getCanonicalDecl();
        }
    }
    return nullptr;
}


/// Database of "built-in" SystemC types and functions
class DeclDB {
public:
    static void initDB(const clang::ASTContext &astCtx);
    DeclDB(const clang::ASTContext &ctx);

    clang::ASTContext &astCtx;
    llvm::SmallSet<const clang::FunctionDecl*,5> waitDecls;

    clang::QualType scInterfaceType;
    clang::QualType scModularInterfaceType;

    clang::QualType scIntBaseType;
    clang::QualType scUIntBaseType;
    clang::QualType scUnsignedType;
    clang::QualType scSignedType;
    clang::QualType scBitVector;

    clang::QualType scModuleType;
    clang::QualType scPortBaseType;
    clang::QualType scObjectType;

    clang::QualType scProcessBType;

    clang::QualType scMethodProcessType;
    clang::QualType scThreadProcessType;
    clang::QualType scCthreadProcessType;

    clang::QualType scSimContextType;
    clang::QualType scSignalChannelType;

    const clang::ClassTemplateDecl *scVectorDecl;
    const clang::ClassTemplateDecl *stdVectorDecl;
    const clang::ClassTemplateDecl *stdArrayDecl;
    const clang::ClassTemplateDecl *scInDecl;
    const clang::ClassTemplateDecl *sctInDecl;
    const clang::ClassTemplateDecl *scOutDecl;
    const clang::ClassTemplateDecl *sctOutDecl;
    const clang::ClassTemplateDecl *scInOutDecl;
    const clang::ClassTemplateDecl *scPortDecl;
};

static std::unique_ptr<DeclDB> db;

void DeclDB::initDB(const clang::ASTContext &astCtx)
{
    if (!db)
        db = std::unique_ptr<DeclDB>(new DeclDB(astCtx));
}


DeclDB::DeclDB(const clang::ASTContext &ctx)
    : astCtx(const_cast<ASTContext&>(ctx))
{
    auto matches = match(functionDecl(hasName("wait"))
                             .bind("waitDecl"), astCtx);

    for (auto match: matches) {
        const FunctionDecl *fDecl = match.getNodeAs<FunctionDecl>("waitDecl");
        auto contexts = getDeclContexts(fDecl);
        if (contexts.back()->getNameAsString() == "sc_core")
            waitDecls.insert(fDecl);
    }

    matches = match(cxxRecordDecl(hasName("sc_core::sc_interface"), isDefinition()).bind("sc_core::sc_interface"), astCtx);
    SCT_TOOL_ASSERT(matches.size() == 1, "Error declaration match");
    scInterfaceType = matches[0].getNodeAs<CXXRecordDecl>("sc_core::sc_interface")->getTypeForDecl()->getCanonicalTypeInternal();

    matches = match(cxxRecordDecl(hasName("sc_core::sc_modular_interface"), isDefinition()).bind("sc_core::sc_modular_interface"), astCtx);
    SCT_TOOL_ASSERT(matches.size() == 1, "Error declaration match");
    scModularInterfaceType = matches[0].getNodeAs<CXXRecordDecl>("sc_core::sc_modular_interface")->getTypeForDecl()->getCanonicalTypeInternal();

    matches = match(cxxRecordDecl(hasName("sc_dt::sc_int_base"), isDefinition()).bind("sc_int_base"), astCtx);
    SCT_TOOL_ASSERT(matches.size() == 1, "Error declaration match");
    scIntBaseType = matches[0].getNodeAs<CXXRecordDecl>("sc_int_base")->getTypeForDecl()->getCanonicalTypeInternal();

    matches = match(cxxRecordDecl(hasName("sc_dt::sc_uint_base"), isDefinition()).bind("sc_uint_base"), astCtx);
    SCT_TOOL_ASSERT(matches.size() == 1, "Error declaration match");
    scUIntBaseType = matches[0].getNodeAs<CXXRecordDecl>("sc_uint_base")->getTypeForDecl()->getCanonicalTypeInternal();

    matches = match(cxxRecordDecl(hasName("sc_dt::sc_signed"), isDefinition()).bind("sc_signed"), astCtx);
    SCT_TOOL_ASSERT(matches.size() == 1, "Error declaration match");
    scSignedType = matches[0].getNodeAs<CXXRecordDecl>("sc_signed")->getTypeForDecl()->getCanonicalTypeInternal();

    matches = match(cxxRecordDecl(hasName("sc_dt::sc_unsigned"), isDefinition()).bind("sc_unsigned"), astCtx);
    SCT_TOOL_ASSERT(matches.size() == 1, "Error declaration match");
    scUnsignedType = matches[0].getNodeAs<CXXRecordDecl>("sc_unsigned")->getTypeForDecl()->getCanonicalTypeInternal();

    matches = match(cxxRecordDecl(hasName("sc_dt::sc_bv_base"), isDefinition()).bind("sc_bv_base"), astCtx);
    SCT_TOOL_ASSERT(matches.size() == 1, "Error declaration match");
    scBitVector = matches[0].getNodeAs<CXXRecordDecl>("sc_bv_base")->getTypeForDecl()->getCanonicalTypeInternal();

    matches = match(cxxRecordDecl(hasName("sc_core::sc_module"), isDefinition()).bind("sc_module"), astCtx);
    SCT_TOOL_ASSERT(matches.size() == 1, "Error declaration match");
    scModuleType = matches[0].getNodeAs<CXXRecordDecl>("sc_module")->getTypeForDecl()->getCanonicalTypeInternal();

    matches = match(cxxRecordDecl(hasName("sc_core::sc_port_base"), isDefinition()).bind("sc_port_base"), astCtx);
    SCT_TOOL_ASSERT(matches.size() == 1, "Error declaration match");
    scPortBaseType = matches[0].getNodeAs<CXXRecordDecl>("sc_port_base")->getTypeForDecl()->getCanonicalTypeInternal();

    matches = match(cxxRecordDecl(hasName("sc_core::sc_signal_channel"), isDefinition()).bind("sc_signal_channel"), astCtx);
    SCT_TOOL_ASSERT(matches.size() == 1, "Error declaration match");
    scSignalChannelType = matches[0].getNodeAs<CXXRecordDecl>("sc_signal_channel")->getTypeForDecl()->getCanonicalTypeInternal();

    matches = match(cxxRecordDecl(hasName("sc_core::sc_simcontext"), isDefinition()).bind("sc_simcontext"), astCtx);
    SCT_TOOL_ASSERT(matches.size() == 1, "Error declaration match");
    scSimContextType = matches[0].getNodeAs<CXXRecordDecl>("sc_simcontext")->getTypeForDecl()->getCanonicalTypeInternal();

    matches = match(cxxRecordDecl(hasName("sc_core::sc_method_process"), isDefinition()).bind("sc_method_process"), astCtx);
    SCT_TOOL_ASSERT(matches.size() == 1, "Error declaration match");
    scMethodProcessType = matches[0].getNodeAs<CXXRecordDecl>("sc_method_process")->getTypeForDecl()->getCanonicalTypeInternal();

    matches = match(cxxRecordDecl(hasName("sc_core::sc_thread_process"), isDefinition()).bind("sc_thread_process"), astCtx);
    SCT_TOOL_ASSERT(matches.size() == 1, "Error declaration match");
    scThreadProcessType = matches[0].getNodeAs<CXXRecordDecl>("sc_thread_process")->getTypeForDecl()->getCanonicalTypeInternal();

    matches = match(cxxRecordDecl(hasName("sc_core::sc_cthread_process"), isDefinition()).bind("sc_cthread_process"), astCtx);
    SCT_TOOL_ASSERT(matches.size() == 1, "Error declaration match");
    scCthreadProcessType = matches[0].getNodeAs<CXXRecordDecl>("sc_cthread_process")->getTypeForDecl()->getCanonicalTypeInternal();

    matches = match(cxxRecordDecl(hasName("sc_core::sc_process_b"), isDefinition()).bind("sc_process_b"), astCtx);
    SCT_TOOL_ASSERT(matches.size() == 1, "Error declaration match");
    scProcessBType = matches[0].getNodeAs<CXXRecordDecl>("sc_process_b")->getTypeForDecl()->getCanonicalTypeInternal();

    matches = match(cxxRecordDecl(hasName("sc_core::sc_object"), isDefinition()).bind("sc_core::sc_object"), astCtx);
    SCT_TOOL_ASSERT(matches.size() == 1, "Error declaration match");
    scObjectType = matches[0].getNodeAs<CXXRecordDecl>("sc_core::sc_object")->getTypeForDecl()->getCanonicalTypeInternal();

    matches = match(classTemplateDecl(hasName("sc_core::sc_vector")).bind("sc_vector"), astCtx);
    SCT_TOOL_ASSERT(matches.size() > 0, "Error declaration match");
    scVectorDecl = matches[0].getNodeAs<ClassTemplateDecl>("sc_vector")->getCanonicalDecl();

    matches = match(classTemplateDecl(hasName("std::vector")).bind("std::vector"), astCtx);
    SCT_TOOL_ASSERT(matches.size() > 0, "Error declaration match");
    stdVectorDecl = matches[0].getNodeAs<ClassTemplateDecl>("std::vector")->getCanonicalDecl();

    matches = match(classTemplateDecl(hasName("std::array")).bind("std::array"), astCtx);
    SCT_TOOL_ASSERT(matches.size() > 0, "Error declaration match");
    stdArrayDecl = matches[0].getNodeAs<ClassTemplateDecl>("std::array")->getCanonicalDecl();

    matches = match(classTemplateDecl(hasName("sc_core::sc_in")).bind("sc_core::sc_in"), astCtx);
    SCT_TOOL_ASSERT(matches.size() > 0, "Error declaration match");
    scInDecl = matches[0].getNodeAs<ClassTemplateDecl>("sc_core::sc_in")->getCanonicalDecl();

    matches = match(classTemplateDecl(hasName("sct::sct_in")).bind("sct::sct_in"), astCtx);
    sctInDecl = matches.size() > 0 ? matches[0].getNodeAs<ClassTemplateDecl>(
                                     "sct::sct_in")->getCanonicalDecl() : nullptr;

    matches = match(classTemplateDecl(hasName("sc_core::sc_out")).bind("sc_core::sc_out"), astCtx);
    SCT_TOOL_ASSERT(matches.size() > 0, "Error declaration match");
    scOutDecl = matches[0].getNodeAs<ClassTemplateDecl>("sc_core::sc_out")->getCanonicalDecl();

    matches = match(classTemplateDecl(hasName("sct::sct_out")).bind("sct::sct_out"), astCtx);
    sctOutDecl = matches.size() > 0 ? matches[0].getNodeAs<ClassTemplateDecl>(
                                      "sct::sct_out")->getCanonicalDecl() : nullptr;

    matches = match(classTemplateDecl(hasName("sc_core::sc_inout")).bind("sc_core::sc_inout"), astCtx);
    SCT_TOOL_ASSERT(matches.size() > 0, "Error declaration match");
    scInOutDecl = matches[0].getNodeAs<ClassTemplateDecl>("sc_core::sc_inout")->getCanonicalDecl();

    matches = match(classTemplateDecl(hasName("sc_core::sc_port")).bind("sc_core::sc_port"), astCtx);
    SCT_TOOL_ASSERT(matches.size() > 0, "Error declaration match");
    scPortDecl = matches[0].getNodeAs<ClassTemplateDecl>("sc_core::sc_port")->getCanonicalDecl();
}

} // end anonymous namespace

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

void initTypeTraits(const clang::ASTContext& astCtx)
{
    DeclDB::initDB(astCtx);
}

bool isStdFuncDecl(const clang::FunctionDecl* funcDecl)
{
    auto contexts = getDeclContexts(funcDecl);

    std::unordered_set<std::string>
        builtinNamespaces {"std", "sc_core", "sc_dt"};

    if (contexts.empty())
        return false;
    else if (builtinNamespaces.count(contexts.back()->getNameAsString()))
        return true;
    else
        return false;
}

bool isWaitDecl(const clang::FunctionDecl *funcDecl)
{
    return db->waitDecls.find(funcDecl) != db->waitDecls.end();
}

// Check is type is any signed type
bool isSignedType(clang::QualType type) 
{
    if (type.isNull()) return false;
    type = getPureType(type);
    
    // Boolean is considered as logic unsigned
    if (type->isBooleanType()) return false;
    if (type->isSignedIntegerOrEnumerationType()) return true;
    if (isScInt(type) || isScBigInt(type)) return true;
    return false; 
}

bool isScInt(clang::QualType type)
{
    if (type.isNull()) return false;
    type = getPureType(type);
    return isDerivedFrom(type, db->scIntBaseType);
}

bool isScUInt(clang::QualType type)
{
    if (type.isNull()) return false;
    type = getPureType(type);
    return isDerivedFrom(type, db->scUIntBaseType);
}

bool isScBigInt(clang::QualType type)
{
    if (type.isNull()) return false;
    type = getPureType(type);
    return isDerivedFrom(type, db->scSignedType);
}

bool isScBigUInt(clang::QualType type)
{
    if (type.isNull()) return false;
    type = getPureType(type);
    return isDerivedFrom(type, db->scUnsignedType);
}

bool isScBitVector(clang::QualType type)
{
    if (type.isNull()) return false;
    type = getPureType(type);
    return isDerivedFrom(type, db->scBitVector);
}

bool isZeroWidthType(clang::QualType type) {
    if (type.isNull()) return false;
    type = getPureType(type);
    if (auto chanType = isUserClassChannel(type)) {
        type = *chanType;
    }
    return (!type.getAsString().compare("struct sc_dt::sct_zero_width"));
}

bool isZeroWidthArrayType(clang::QualType type) {
    if (type.isNull()) return false;
    type = getArrayElementType(type);
    type = getPureType(type);
    if (auto chanType = isUserClassChannel(type)) {
        type = *chanType;
    }
    return (!type.getAsString().compare("struct sc_dt::sct_zero_width"));
}

bool isAnyScInteger(clang::QualType type)
{
    if (type.isNull()) return false;
    
    return isScInt(type) || isScUInt(type) ||
           isScBigInt(type) || isScBigUInt(type) || isScBitVector(type);
}

bool isAnyScIntegerRef(QualType type, bool checkPointer) 
{
    if (type.isNull()) return false;

    if (checkPointer) {
        while (type->isPointerType()) {
            type = type->getPointeeType();
        }
    }
    type = getPureType(type);

    if (isAnyScInteger(type)) {
        return true;
    }
    std::string typeStr = type.getAsString();
    return (typeStr.find("sc_uint_base") != std::string::npos || 
            typeStr.find("sc_int_base") != std::string::npos || 
            typeStr.find("sc_bv_base") != std::string::npos ||
            typeStr.find("sc_uint_bitref") != std::string::npos || 
            typeStr.find("sc_int_bitref") != std::string::npos || 
            typeStr.find("sc_uint_subref") != std::string::npos || 
            typeStr.find("sc_int_subref") != std::string::npos || 
            typeStr.find("sc_concatref") != std::string::npos || 
            typeStr.find("sc_value_base") != std::string::npos ||
            typeStr.find("sc_signed") != std::string::npos ||
            typeStr.find("sc_unsigned") != std::string::npos);
}

bool isScNotSupported(QualType type, bool checkPointer) 
{
    if (type.isNull()) return false;

    if (checkPointer) {
        while (type->isPointerType()) {
            type = type->getPointeeType();
        }
    }
    type = getPureType(type);

    std::string typeStr = type.getAsString();
    return (typeStr.find("sc_lv") != std::string::npos || 
            typeStr.find("sc_bit") != std::string::npos || 
            typeStr.find("sc_logic") != std::string::npos ||
            typeStr.find("sc_fix") != std::string::npos ||
            typeStr.find("sc_fxval") != std::string::npos ||
            typeStr.find("sc_fxnum") != std::string::npos ||
            typeStr.find("sc_ufix") != std::string::npos ||
            typeStr.find("sc_fixed") != std::string::npos ||
            typeStr.find("sc_ufixed") != std::string::npos);
}

bool isAnyIntegerRef(clang::QualType type)
{
    if (type.isNull()) return false;
    type = getPureType(type);

    if (auto builtinType = dyn_cast<clang::BuiltinType>(type)) {
        return builtinType->isInteger();
        
    } else if (isa<clang::EnumType>(type)) {
        return true;
        
    } else {
        return isAnyScIntegerRef(type);
    }
}

bool isAnyInteger(clang::QualType type)
{
    if (type.isNull()) return false;
    type = getPureType(type);

    if (auto builtinType = dyn_cast<clang::BuiltinType>(type)) {
        return builtinType->isInteger();
        
    } else if (isa<clang::EnumType>(type)) {
        return true;
        
    } else {
        return isAnyScInteger(type);
    }
}

bool isSignedOrArrayOfSigned(clang::QualType type)
{
    if (type.isNull()) return false;
 
    // Get array element type
    type = getArrayElementType(type);
    type = getPureType(type);

    if (type->isSignedIntegerType())
        return true;

    if (isScInt(type) || isScBigInt(type))
        return true;

    return false;
}

// Check array of @sc_int/@sc_bigint/@sc_uint/@sc_biguint type or 
// pointer to such array
bool isScIntegerArray(QualType type, bool checkPointer) 
{
    if (type.isNull()) return false;
    
    // Get array element type
    type = getArrayElementType(type);
    
    // Remove pointer
    if (checkPointer) {
        while (type->isPointerType()) {
            type = type->getPointeeType();
        }
    }    

    return isAnyScInteger(type);
}

// Try to get integer type for which width can be obtained by @getIntTraits()
clang::QualType getTypeForWidth(const clang::Expr* expr) 
{
    bool transform;
    QualType origType = expr->getType();
    
    do {
        // Avoid @nullptr processing
        if (!expr) break;
        
        QualType type = expr->getType();
        if (getIntTraits(type, true)) {
            return type;
        }
    
        transform = false;
        if (const CastExpr* castExpr = dyn_cast<const CastExpr>(expr)) {
            expr = castExpr->getSubExpr();
            transform = true;
        }
    } while (transform);

    // Cannot obtain integer type, return original expression type
    return origType;
}

// Get width of any integral type including SC data types
// \return < width, isUnsigned >
std::optional<std::pair<size_t, bool>> getIntTraits(clang::QualType type, 
                                                     bool checkPointer)
{
    if (type.isNull()) return std::nullopt;
    
    // Get the most inner array element type
    if (checkPointer) {
        type = type->getPointeeOrArrayElementType()->getCanonicalTypeInternal();
    }
    type = getPureType(type);
    
    if (isZeroWidthType(type)) {
        // ZWI value is 32bit unsigned
        return std::pair<size_t, bool>(32, true);
        
    } else 
    if (auto etype = dyn_cast<EnumType>(type)) {
        auto edecl = etype->getDecl();
        size_t width= edecl->getNumPositiveBits() + edecl->getNumNegativeBits();
        return std::pair<size_t, bool>(width, !edecl->getNumNegativeBits());
        
    } else 
    if (type->isIntegerType()) {
        // Get real platform dependent type width
        size_t width = db->astCtx.getIntWidth(type);
        bool isUnsigned = type->isUnsignedIntegerType();
        
        return std::pair<size_t, bool>(width, isUnsigned);
        
    } else 
    if (auto width = getScUintBiguintBitVec(type)) {
        return std::pair<size_t, bool>(*width, true);
        
    } else 
    if (auto width = getScIntBigint(type)) {
        return std::pair<size_t, bool>(*width, false);
        
    }
    
    return std::optional<std::pair<size_t, bool> >();
}

// Get width of any integer type wrapped into given @type, based on @getIntTraits
// Can work for any integral type, integral pointer, channel, pointer to channel
std::optional<size_t> getAnyTypeWidth(clang::QualType type, bool checkPointer, 
                                       bool checkChannel)
{
    if (type.isNull()) return std::nullopt;
    
    // Get array element type
    type = getArrayElementType(type);
    
    // Remove pointer
    if (checkPointer) {
        while (type->isPointerType()) {
            type = type->getPointeeType();
        }
    }
    
    if (checkChannel) {
        if (isScChannel(type)) {
            if (auto argType = getTemplateArgAsType(type, 0)) {
                type = *argType;
            }
        }
    }
    
    type = type->getCanonicalTypeInternal();

    if (auto typeInfo = getIntTraits(type, false)) {
        return typeInfo->first;
    }
    
    return std::nullopt;
}

std::optional<size_t> getScUintBiguintBitVec(QualType type) 
{
    if (type.isNull()) return std::nullopt;
    type = getPureType(type);

    if (isScUInt(type) || isScBigUInt(type) || isScBitVector(type)) {
        auto rdecl = type->getAsCXXRecordDecl();
        SCT_TOOL_ASSERT (rdecl, "SC integer is not record");
        
        // There is no template for @sc_unsigned
        if (auto sdecl= dyn_cast<clang::ClassTemplateSpecializationDecl>(rdecl)) 
        {
            APSInt arg= sdecl->getTemplateArgs().operator [](0).getAsIntegral();
            return (size_t)arg.getExtValue();
        }
    }
    return std::optional<size_t>();
}

std::optional<size_t> getScIntBigint(QualType type) 
{
    if (type.isNull()) return std::nullopt;
    type = getPureType(type);

    if (isScInt(type) || isScBigInt(type)) {
        auto rdecl = type->getAsCXXRecordDecl();
        SCT_TOOL_ASSERT (rdecl, "SC integer is not record");
        
        // There is no template for @sc_signed
        if (auto sdecl= dyn_cast<clang::ClassTemplateSpecializationDecl>(rdecl)) 
        {
            APSInt arg= sdecl->getTemplateArgs().operator [](0).getAsIntegral();
            return (size_t)arg.getExtValue();
        }
    }
    return std::optional<size_t>();
}

// Check type is @sc_signed which is base class for @sc_bigint but not template 
bool isScSigned(QualType type) 
{
    if (type.isNull()) return false;
    type = getPureType(type);

    if (isScBigInt(type)) {
        auto rdecl = type->getAsCXXRecordDecl();
        SCT_TOOL_ASSERT (rdecl, "SC integer is not record");
        
        // There is no template for @sc_signed
        return (!isa<clang::ClassTemplateSpecializationDecl>(rdecl));
    }
    return false;
}

// Check type is @sc_unsigned which is base class for @sc_biguint but not template 
bool isScUnsigned(QualType type) 
{
    if (type.isNull()) return false;
    type = getPureType(type);

    if (isScBigUInt(type)) {
        auto rdecl = type->getAsCXXRecordDecl();
        SCT_TOOL_ASSERT (rdecl, "SC integer is not record");
        
        // There is no template for @sc_signed
        return (!isa<clang::ClassTemplateSpecializationDecl>(rdecl));
    }
    return false;
}

// Check for module only
bool isScModule(clang::QualType type, bool checkPointer)
{
    if (type.isNull()) return false;

    // Remove pointer
    if (checkPointer) {
        while (type->isPointerType()) {
            type = type->getPointeeType();
        }
        type = getPureType(type);

        // @sc_port cannot contain module, but MIF only
        if (isScPort(type)) {
            return false;
        }
    }
    type = getPureType(type);
    
    if (isDerivedFrom(type, db->scInterfaceType)) {
        return false;
    }
    
    return isDerivedFrom(type, db->scModuleType);
}

// Check for module or modular interface
bool isScModuleOrInterface(clang::QualType type, bool checkPointer)
{
    if (type.isNull()) return false;
    
    // Remove pointer
    if (checkPointer) {
        while (type->isPointerType()) {
            type = type->getPointeeType();
        }
        type = getPureType(type);
        
        // @sc_port<IF> can points only to module
        if (isScPort(type)) {
            return true;
        }
    }
    type = getPureType(type);

    return isDerivedFrom(type, db->scModuleType);
}

// Check for modular interface only
bool isScModularInterface(clang::QualType type)
{
    if (type.isNull()) return false;
    type = getPureType(type);
    
    if (isDerivedFrom(type, db->scModularInterfaceType))
        return true;

    if (isScModuleOrInterface(type) && isDerivedFrom(type, db->scInterfaceType))
        return true;

    return false;
}

bool isScObject(clang::QualType type)
{
    if (type.isNull()) return false;
    type = getPureType(type);
    
    return isDerivedFrom(type, db->scObjectType);
}

bool isScVector(clang::QualType type)
{
    if (type.isNull()) return false;
    type = getPureType(type);
    
    return (!type.isNull() && getAsClassTemplateDecl(type) == db->scVectorDecl);
}

bool isStdVector(clang::QualType type)
{
    if (type.isNull()) return false;
    type = getPureType(type);
    
    return (!type.isNull() && getAsClassTemplateDecl(type) == db->stdVectorDecl);
}

bool isStdArray(clang::QualType type)
{
    if (type.isNull()) return false;
    type = getPureType(type);
    
    return (!type.isNull() && getAsClassTemplateDecl(type) == db->stdArrayDecl);
}

bool isScBasePort(clang::QualType type)
{
    if (type.isNull()) return false;
    type = getPureType(type);
    
    return isDerivedFrom(type, db->scPortBaseType);
}

bool isScPort(clang::QualType type)
{
    if (type.isNull()) return false;
    type = getPureType(type);
    
    if ( isScBasePort(type) ) {
        if (auto argType = getTemplateArgAsType(type, 0)) {
            type = getPureType(*argType);
            return isDerivedFrom(type, db->scInterfaceType);
        }
    }
    
    return false;
}

bool isScIn(clang::QualType type)
{
    if (type.isNull()) return false;
    type = getPureType(type);
    
    auto decl = getAsClassTemplateDecl(type);
    return (decl == db->scInDecl || (db->sctInDecl && decl == db->sctInDecl));
}

bool isScOut(clang::QualType type)
{
    if (type.isNull()) return false;
    type = getPureType(type);

    auto decl = getAsClassTemplateDecl(type);
    return (decl == db->scOutDecl || (db->sctOutDecl && decl == db->sctOutDecl));
}

bool isScInOut(clang::QualType type)
{
    if (type.isNull()) return false;
    type = getPureType(type);

    return (getAsClassTemplateDecl(type) == db->scInOutDecl);
}

// Check if the type is pointer or @sc_port<IF>
bool isPointerOrScPort(QualType type) 
{
    if (type.isNull()) return false;
    type = getPureType(type);

    return (isPointer(type) || isScPort(type));
}

bool isScSignal(clang::QualType type)
{
    if (type.isNull()) return false;
    type = getPureType(type);

    return isDerivedFrom(type, db->scSignalChannelType);
}

bool isScToolCombSignal(QualType type, bool checkPointer)
{
    if (type.isNull()) return false;
    
    // Remove pointer
    if (checkPointer) {
        // sc_port<IF> cannot point to channel
        while (type->isPointerType()) {
            type = type->getPointeeType();
        }
    }
    type = getPureType(type);
    
    if (isScSignal(type)) {
        return (type.getAsString().find("sc_core::sct_comb_signal") != 
                std::string::npos);
    }
    return false;
}

bool isScToolClearSignal(QualType type, bool checkPointer)
{
    if (type.isNull()) return false;
    
    // Remove pointer
    if (checkPointer) {
        // sc_port<IF> cannot point to channel
        while (type->isPointerType()) {
            type = type->getPointeeType();
        }
    }
    type = getPureType(type);
    
    if (isScSignal(type)) {
        return (type.getAsString().find("sc_core::sct_clear_signal") != 
                std::string::npos);
    }
    return false;
}

bool isScChannel(clang::QualType type, bool checkPointer)
{
    if (type.isNull()) return false;
    
    // Remove pointer
    if (checkPointer) {
        // sc_port<IF> cannot point to channel
        while (type->isPointerType()) {
            type = type->getPointeeType();
        }
    }
    type = getPureType(type);
    
    // sc_port<IF> is not a channel
    return (!isScPort(type) && (isScSignal(type) || isScBasePort(type)) );
}

// Check array of any SC channel type
// \param checkPointer -- check array of pointers to channel
bool isScChannelArray(clang::QualType type, bool checkPointer) 
{
    if (type.isNull()) return false;
    
    // Get array element type
    type = getArrayElementType(type);
    // sc_port<IF> cannot point to channel
    if (checkPointer && isPointer(type)) {
        type = type->getPointeeType();
    }
    type = getPureType(type);

    return (isScChannel(type, false));
}

// Get record type if it is SC channel of record type, or none
std::optional<clang::QualType>  
isUserClassChannel(clang::QualType type, bool checkPointer)
{
    if (type.isNull()) return std::nullopt;
    
    // Remove pointer
    if (checkPointer) {
        // sc_port<IF> cannot point to channel
        while (type->isPointerType()) {
            type = type->getPointeeType();
        }
    }
    type = getPureType(type);
    
    if ((!isScPort(type) && (isScSignal(type) || isScBasePort(type)))) {
        auto chanType = getTemplateArgAsType(type, 0);
        if (chanType && isUserClass(*chanType)) {
            return *chanType;
        }
    }
    return std::nullopt;
}


// Any the type in sc_core namespace
bool isAnyScCoreObject(clang::QualType type) 
{
    if (type.isNull()) return false;
    type = getPureType(type);
    
    std::string typeStr = type.getAsString();
    return (typeStr.find("sc_core::") != std::string::npos);
}

bool isScCoreType(clang::QualType type)
{
    if (type.isNull()) return false;
    type = getPureType(type);

    if (CXXRecordDecl* recDecl = type->getAsCXXRecordDecl()) {
        auto declCtx = recDecl->getDeclContext();

        if (!declCtx->isNamespace())
            return false;

        auto *nsDecl = cast<NamespaceDecl>(declCtx);
        auto *id = nsDecl->getIdentifier();

        return id && id->isStr("sc_core");
    }
    return false;
}

bool isSctFifo(clang::QualType type) {
    if (type.isNull()) return false;
    type = getPureType(type);
    
    std::string typeStr = type.getAsString();
    return (typeStr.find("sct_fifo") != std::string::npos);
}

bool isSctTarg(clang::QualType type) {
    if (type.isNull()) return false;
    type = getPureType(type);
    
    std::string typeStr = type.getAsString();
    return (typeStr.find("sct_target") != std::string::npos);
}

bool isSctCombTarg(clang::QualType type) {
    if (type.isNull()) return false;
    type = getPureType(type);
    
    std::string typeStr = type.getAsString();
    return (typeStr.find("sct_comb_target") != std::string::npos);
}

bool isSctInit(clang::QualType type) {
    if (type.isNull()) return false;
    type = getPureType(type);
    
    std::string typeStr = type.getAsString();
    return (typeStr.find("sct_initiator") != std::string::npos);
}

bool isSctChannelSens(clang::QualType type, const FunctionDecl* funcDecl) 
{
    if (type.isNull()) return false;
    type = getPureType(type);
    
    std::string typeStr = type.getAsString();
    std::string funcNameStr = funcDecl ? funcDecl->getNameAsString() : "";
    
    return (typeStr.find("sct_target") != std::string::npos ||
            typeStr.find("sct_comb_target") != std::string::npos ||
            typeStr.find("sct_initiator") != std::string::npos ||
            typeStr.find("sct_fifo") != std::string::npos ||
            typeStr.find("sct_register") != std::string::npos ||
            typeStr.find("sct_multi_target") != std::string::npos ||
            typeStr.find("sct_multi_initiator") != std::string::npos ||
            (typeStr.find("sct_ff_synchronizer") != std::string::npos &&
             (funcNameStr.empty() || 
              funcNameStr.find("read") != std::string::npos ||
              funcNameStr.find("operator bool") != std::string::npos))
            );
}

bool isAssertInThread(clang::Stmt* stmt) 
{
    if (auto callExpr = dyn_cast<CallExpr>(stmt)) {
        FunctionDecl* funcDecl = callExpr->getDirectCallee();
        std::string fname = funcDecl->getNameAsString();
        if (fname == "sct_assert_in_proc_start" || 
            fname == "sct_assert_in_proc_func") { 
            return true;
        }
    }
    return false;    
}

bool isAssignOperatorSupported(clang::QualType type) 
{
    if (type.isNull()) return false;
    type = getPureType(type);
    
    std::string typeStr = type.getAsString();
    return (typeStr.find("sct_register") != std::string::npos ||
            typeStr.find("sct_ff_synchronizer") != std::string::npos);
}


bool isScProcess(clang::QualType type)
{
    if (type.isNull()) return false;
    type = getPureType(type);
    
    return isDerivedFrom(type, db->scProcessBType);
}

bool isScMethod(clang::QualType type)
{
    if (type.isNull()) return false;
    type = getPureType(type);
    
    return (type == db->scMethodProcessType);
}

bool isScThread(clang::QualType type)
{
    if (type.isNull()) return false;
    type = getPureType(type);

    return (type == db->scThreadProcessType);
}

bool isScCThread(clang::QualType type)
{
    if (type.isNull()) return false;
    type = getPureType(type);

    return (type == db->scCthreadProcessType);
}

clang::QualType getScPortBaseType()
{
    return db->scPortBaseType;
}

clang::QualType getScProcessBType()
{
    return db->scProcessBType;
}

clang::QualType getScSimContextType()
{
    return db->scSimContextType;
}


} // namespace sc

