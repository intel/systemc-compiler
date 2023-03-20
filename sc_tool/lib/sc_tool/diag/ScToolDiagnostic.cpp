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

#include "ScToolDiagnostic.h"
#include <iostream>

// Hash functions
namespace std {
    
std::size_t hash<std::pair<unsigned, unsigned>>::
operator () (const std::pair<unsigned, unsigned>& obj) const 
{
    using std::hash;
    return (std::hash<unsigned>()(obj.first) ^ std::hash<unsigned>()(obj.second));
}
}

namespace sc {

/// All reported issues to filter duplicates
std::unordered_set<std::pair<unsigned, unsigned>> ScDiag::diagIssues;

    
class ScDiagBuilder {
public:
    static void build(clang::DiagnosticsEngine *diagEngine) {
        auto &scDiag = ScDiag::instance();
        scDiag.engine = diagEngine;
        scDiag.initialize();

        for (const auto &scid : scDiag.idFormatMap) {
            scDiag.sc2clangMap[scid.first] =
                scDiag.engine->getDiagnosticIDs()->getCustomDiagID(
                                    scid.second.first, scid.second.second);
        }
    }
    
    static void reportErrorException() {
        auto &scDiag = ScDiag::instance();
        scDiag.hasException = true;
    }
    
    static int getDiagnosticStatus() {
        auto &scDiag = ScDiag::instance();
        if (scDiag.hasException) {
            return 103;
        } else 
        if (scDiag.hasFatal()) {
            return 102;
        } else 
        if (scDiag.hasError()) {
            return 101;
        }
        return 0;
    }
};

void initDiagnosticEngine(clang::DiagnosticsEngine *diagEngine) {
    ScDiagBuilder::build(diagEngine);
}

void reportErrorException() {
    ScDiagBuilder::reportErrorException();
}

int getDiagnosticStatus() {
    return ScDiagBuilder::getDiagnosticStatus();
}

ScDiag &sc::ScDiag::instance() {
    static ScDiag s;
    return s;
}

void ScDiag::reportErrAndDie(clang::SourceLocation loc,
                             llvm::StringRef message) 
{
    auto &engine = *(instance().engine);
    engine.setSuppressAllDiagnostics(false);
    auto id = engine.getDiagnosticIDs()->getCustomDiagID(
                                clang::DiagnosticIDs::Fatal, message);
    engine.Report(loc, id);
    exit(1);
}

void ScDiag::reportErrAndDie(llvm::StringRef message) {
    ScDiag::reportErrAndDie(clang::SourceLocation(), message);
}

clang::DiagnosticBuilder ScDiag::reportCustom(clang::SourceLocation loc,
                                              clang::DiagnosticIDs::Level level,
                                              llvm::StringRef formatString) 
{
    auto &engine = *instance().engine;
    engine.setSuppressAllDiagnostics(false);
    auto id = engine.getDiagnosticIDs()->getCustomDiagID(level, formatString);
    return engine.Report(loc, id);
}

clang::DiagnosticBuilder ScDiag::reportCustom(clang::DiagnosticIDs::Level level,
                                              llvm::StringRef formatString) {
    return ScDiag::reportCustom(clang::SourceLocation(), level, formatString);
}

clang::DiagnosticBuilder ScDiag::reportScDiag(clang::SourceLocation loc,
                                              ScDiag::ScDiagID id, 
                                              bool checkDuplicate) 
{
    auto &engine = *instance().engine;
    engine.setSuppressAllDiagnostics(false);
    auto issue = std::make_pair(loc.getRawEncoding(), unsigned(id));

    // Avoid duplicates
    if (checkDuplicate) {
        if (diagIssues.count(issue) != 0) engine.setSuppressAllDiagnostics(true);
        diagIssues.insert(issue);
    }
    
    auto clangId = instance().sc2clangMap.at(id);
    return engine.Report(loc, clangId);
}

clang::DiagnosticBuilder ScDiag::reportScDiag(ScDiag::ScDiagID id,
                                              bool checkDuplicate) {
    return ScDiag::reportScDiag(clang::SourceLocation(), id, checkDuplicate);
}

}
