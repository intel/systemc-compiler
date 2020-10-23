/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Author: Roman Popov
 */

#ifndef SCTOOL_RAW_INDENT_OSTREAM_H
#define SCTOOL_RAW_INDENT_OSTREAM_H

#include <llvm/Support/raw_ostream.h>
#include <llvm/ADT/StringRef.h>
#include <vector>
#include <string>

namespace sc {

/// Auto-indenting wrapper over raw_ostream that automatically
/// adds filler string after newline
class RawIndentOstream : public llvm::raw_ostream {
    llvm::raw_ostream &backend;
    bool indentNext = false;

    void write_impl(const char *ptr, size_t size) override;
    uint64_t current_pos() const override { assert(false); return 0; }

    /// String that will be appended after each newline
    std::vector<std::string> fillVec;

public:
    void pushF(llvm::StringRef fillStr) { fillVec.emplace_back(fillStr); }
    void popF() { fillVec.pop_back(); }

    RawIndentOstream(llvm::raw_ostream &backend_os)
        : raw_ostream(true/*unbuffered*/), backend (backend_os) { }
};

/// RAII object that automatically does push and pop for filler string
class IndentScope {
    RawIndentOstream &o;
public:
    IndentScope(RawIndentOstream &os, llvm::StringRef filler) : o (os) {
        o.pushF(filler);
    }

    ~IndentScope() { o.popF(); }
};


} // end namespace llvm

#endif //SCTOOL_RAW_INDENT_OSTREAM_H
