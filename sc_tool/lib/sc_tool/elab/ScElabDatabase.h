/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Wrapper over Protobuf SCDesign
 * Additionally it stores
 * - Generated Verilog modules
 * - Map between Protobuf type ID and clang::QualType
 * 
 * Author: Roman Popov
 */

#ifndef SCTOOL_SCELABDATABASE_H
#define SCTOOL_SCELABDATABASE_H

#include <sc_tool/elab/ScObjectView.h>
#include <sc_tool/elab/ScVerilogModule.h>
#include <sc_tool/dyn_elab/ElabTypeManager.h>
#include "sc_tool/expr/ScParseExprValue.h"
#include <sc_tool/cfg/ScState.h>
#include <clang/AST/DeclCXX.h>
#include <sc_elab.pb.h>
#include <vector>
#include <deque>
#include <unordered_map>

namespace sc_elab {

/// Wrapper over Protobuf SCDesign
///
///   Additionally it stores
/// - Generated Verilog modules
/// - Map between Protobuf type ID and clang::QualType
class ElabDatabase {
public:
    ElabDatabase(sc_elab::SCDesign &scDesign,
        sc_elab::ElabTypeManager &typeManager,
        clang::ASTContext &astCtx);

    ObjectView getObj(uint32_t objID) const;
    clang::QualType getType(uint32_t typeID) const;

    clang::ASTContext* getASTContext() const { return &astCtx; }

    /// Get all modules and MIFs in design in top to bottom order
    const std::vector<ModuleMIFView> &getModules() const;

    ModuleMIFView getTopModule() const;

    const std::list<VerilogModule>& getVerilogModules() const;
    std::list<VerilogModule>& getVerilogModules();

    VerilogModule *getVerilogModule(ModuleMIFView scModView);
    const VerilogModule *getVerilogModule(ModuleMIFView scModView) const;
    /// Check if module/MIF exists
    bool hasVerilogModule(ModuleMIFView scModView) const;

    VerilogModule *addVerilogModule(ModuleMIFView scModView);

    /// Remove duplicate verilog modules
    void uniquifyVerilogModules();

    sc_elab::ObjectView createStaticVariable(RecordView parent,
                                             const clang::VarDecl *varDecl);
 
    // Add/check ports already bound, used for cross module bound via dynamic signal
    void addBoundPort(ObjectView port) {
        boundPorts.insert(port);
    }
    bool isPortBound(ObjectView port) {
        return boundPorts.count(port);
    }

    void dump() const;

private:

    void initStaticArray(sc_elab::Object* arrayObj, clang::QualType elmType,
                         clang::APValue initVals);
    
    void initStaticArray(sc_elab::Object* arrayObj, clang::QualType elementType,
                         std::vector<llvm::APSInt>& initVals);


    uint32_t getOrCreateTypeID(clang::QualType type);

    sc_elab::Object * createStaticObject(clang::QualType type,
                                         uint32_t parentID);

    sc_elab::Object * createStaticPrimitive(clang::QualType type,
                                            uint32_t parentID,
                                            llvm::APSInt &initVal);

    sc_elab::Object * createStaticArrray(clang::QualType type,
                                         uint32_t parentID,
                                         uint32_t arraySize);

private:

    sc_elab::SCDesign &designDB;
    sc_elab::ElabTypeManager &typeManager;

    clang::ASTContext &astCtx;
    // All modules and modular IFs
    mutable std::vector<ModuleMIFView> modules;
    
    // @parseValue used to evaluate complicated initializers 
    // Use common state provides constant evaluated from other constants
    sc::ScParseExprValue parseValue;

    // Generated Verilog modules in specific representation
    std::list<VerilogModule> verilogMods;
    std::unordered_map<ModuleMIFView, VerilogModule *> verModMap;
    // Ports already bound, used for cross module bound via dynamic signal
    std::unordered_set<ObjectView> boundPorts;
};

}  // namespace sc_elab


#endif //SCTOOL_SCELABDATABASE_H
