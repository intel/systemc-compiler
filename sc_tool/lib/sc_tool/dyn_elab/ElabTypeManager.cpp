/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Author: Roman Popov
 */

#include <sc_tool/dyn_elab/ElabTypeManager.h>

namespace sc_elab
{

ElabTypeManager::ID
ElabTypeManager::getOrCreateTypeID(clang::QualType qualType)
{

    auto iter = type2idMap.find(qualType);

    if (iter != type2idMap.end())
        return iter->second;

    ID newID = type2idMap.size();

    *designDB.add_types() = qualType.getAsString();
    type2idMap[qualType] = newID;
    id2typeMap[newID] = qualType;

    return newID;
}

clang::QualType
ElabTypeManager::getTypeByID(ElabTypeManager::ID typeID)
{
    assert(typeID < id2typeMap.size());
    return id2typeMap.at(typeID);
}


}