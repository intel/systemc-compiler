/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Author: Roman Popov
 */

#ifndef SCTOOL_SCWAITSTATE_H
#define SCTOOL_SCWAITSTATE_H

#include <clang/AST/DeclCXX.h>
#include <clang/AST/Decl.h>
#include <clang/Analysis/CFG.h>

#include <llvm/ADT/Hashing.h>

#include <limits>

namespace sc {

/// Cursor to CFG element
class CfgCursor {
    const clang::FunctionDecl *funcDecl = nullptr; /// host Function
    const clang::CFGBlock *block = nullptr;        /// block in host function
    size_t   elementID = 0;                        /// elementID in block

public:

    CfgCursor() {}

    CfgCursor ( const clang::FunctionDecl *funcDecl,
                const clang::CFGBlock *block,
                size_t elementID )
    : funcDecl(funcDecl), block(block), elementID(elementID)
    {
            //assert(elementID < block->size());
    }

    bool operator==(const CfgCursor &rhs) const
    {
        return funcDecl == rhs.funcDecl &&
            block == rhs.block && elementID == rhs.elementID;
    }

    size_t getElementID() const
    {
        return elementID;
    }

    void setElementID(size_t v)
    {
        elementID = v;
    }

    clang::FunctionDecl *getFuncDecl() const
    {
        return const_cast<clang::FunctionDecl *>(funcDecl);
    }

    const clang::CFGBlock *getBlock() const
    {
        return block;
    }

    bool isValid() const
    {
        return (funcDecl != nullptr) && (block != nullptr);
    }

};

typedef llvm::SmallVector<CfgCursor, 6> CfgCursorStack;

const clang::CFGBlock * curBlock(const CfgCursorStack &stack);
size_t curElem(const CfgCursorStack &stack);

template <typename StreamT>
StreamT& operator<<(StreamT&  os,
                         const sc::CfgCursorStack &waitStack) {

    for (size_t i = 0; i < waitStack.size(); ++i) {
        bool last = i == waitStack.size() - 1;
        const auto &cursor = waitStack[i];

        os << "(" << cursor.getFuncDecl()->getNameAsString()
           << " B:" << cursor.getBlock()->getBlockID()
           << " E:" << cursor.getElementID() << ")";

        if (!last)
            os << " => ";
    }

    return os;
}

inline llvm::hash_code hash_value(const sc::CfgCursor& obj)
{
    return llvm::hash_combine(obj.getBlock(), obj.getElementID(),
                              obj.getFuncDecl());
}

inline llvm::hash_code hash_value(const sc::CfgCursorStack& obj)
{
    return llvm::hash_combine_range(obj.begin(), obj.end());
}


} // namespace sc

//
//  Hashing for CfgCursor
//
namespace std
{

template<>
struct hash<sc::CfgCursor>
{
    std::size_t
    operator()(const sc::CfgCursor &obj) const
    {
        return hash_value(obj);
    }
};

template<>
struct hash<sc::CfgCursorStack>
{
    std::size_t operator () (const sc::CfgCursorStack& obj) const {
        return hash_value(obj);
    }
};

} // namespace std

#endif //SCTOOL_SCWAITSTATE_H
