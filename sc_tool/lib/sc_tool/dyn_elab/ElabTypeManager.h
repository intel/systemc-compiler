/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Author: Roman Popov
 */

#ifndef SCTOOL_ELABTYPEMANAGER_H
#define SCTOOL_ELABTYPEMANAGER_H

#include <sc_elab.pb.h>
#include <clang/AST/Type.h>
#include <unordered_map>

namespace std
{

template <>
struct hash<clang::QualType>
{
    std::size_t operator()(const clang::QualType &qualType) const
    {
        return ((uintptr_t)qualType.getAsOpaquePtr()) ^
            (((uintptr_t)qualType.getAsOpaquePtr() >> 9));
    }
};

}

namespace sc_elab {

/**
 * Maps Protobuf ElabDB types to Clang Types
 */
class ElabTypeManager {
public:
    using ID = uint32_t ;

    ElabTypeManager(SCDesign &designDB) : designDB(designDB) {}

    ID getOrCreateTypeID(clang::QualType qualType);
    clang::QualType getTypeByID(ID typeID);
private:

    SCDesign &designDB;
    std::unordered_map<clang::QualType, ID> type2idMap;
    std::unordered_map<ID, clang::QualType> id2typeMap;
};

}

#endif //SCTOOL_ELABTYPEMANAGER_H
