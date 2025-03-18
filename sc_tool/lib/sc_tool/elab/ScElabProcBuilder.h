/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * ProcBuilder converts elaborator state into SValue-based ScState
 * and run process-level analysis to generate Verilog for processes
 *
 * Should be used for all process of one module
 * 
 * Author: Roman Popov
 */

#ifndef SCELABPROCBUILDER_H
#define SCELABPROCBUILDER_H

#include <sc_tool/elab/ScVerilogModule.h>
#include <sc_tool/cfg/ScState.h>
#include <sc_tool/ScProcAnalyzer.h>
#include <sc_tool/elab/ScObjectView.h>
#include <sc_tool/elab/ScElabDatabase.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <sc_elab.pb.h>

namespace sc_elab {

class ProcBuilder {
public:
    /// Maximal number of elements in array, other elements not put into state, 
    /// instead that last element used
    //static const unsigned MAX_ARR_ELEM_NUM = sc::ScState::MAX_ARR_ELEM_NUM;
    
    ProcBuilder(ModuleMIFView moduleView, ElabDatabase &elabDB,
                const std::unordered_set<uint32_t>& extrTargInit_);

    /// Generate module level SVA properties code from declarations
    std::string generateSvaProperties(VerilogModule& verMod);
    
    VerilogProcCode generateVerilogProcess(ProcessView& procView);

    /// Prepare state for process generation, fill derived classes inside
    void prepareState(ModuleMIFView hostModule);

    
    // Get SValue for given RecorView
    static sc::SValue getRecordValue(const RecordView& recView) 
    {
        auto i = recordMap.find(recView.getID());
        assert (i != recordMap.end());
        return i->second;
    }
    // Fill SValue for given RecorView
    static void fillRecordValues(std::unordered_map<uint32_t, sc::SValue>& vals) 
    {
        recordMap.swap(vals);
    }
    
private:

    void reportUnsuported(ObjectView objView);

    sc::SValue traverseRecord(RecordView recView, bool isVerModule = false);

    sc::SValue traverseField(ObjectView fieldView);
    sc::SValue traversePrimitive(PrimitiveView primView);
    sc::SValue traverseArray(ArrayView arrayView);
    sc::SValue traverse(ObjectView  objView);

    sc::SValue createPrimitiveSValue(ValueView valueView);
    sc::SValue createPortSValue(PortView portView);
    void createSignalSValue(RecordView& recView, ElabObjVec& allFields);
    sc::SValue createSignalSValue(SignalView signalView);
    sc::SValue getOrCreatePointeeSValue(PtrOrRefView ptrOrRefView);

private: // data
    clang::ASTContext &ctx;
    // Common for all processes in module
    std::unique_ptr<sc::ScProcAnalyzer> procAnalyzer;
    ModuleMIFView rootModView;
    ElabDatabase &elabDB;

    /// Current context: array of MIF array variables
    sc::ValDeclVector mifArrayCtx;
    /// <array of MIF array variables, array of fields in this context>
    std::unordered_map<sc::ValDeclVector, sc::SValueVector> mifArrayVars;
    
    std::shared_ptr<sc::ScState> moduleState;
    // Parent hierarchy to create SRecord
    std::vector<sc::SValue> classHierStack;

    std::vector<PtrOrRefView> unresolvedPointers;
    
    // Array pointers used to replace its pointed array variable to array object
    std::unordered_set<sc::SValue> arrayPointers;

    /// Maps Elaborator Objects to Process analyzer state values
    /// The same as state::elabs2SValMap
    std::unordered_map<ObjectView, sc::SValue> objSValMap;
    
    /// Mapping record object ID to SValue for all records in the design
    static std::unordered_map<uint32_t, sc::SValue> recordMap;
    
    /// Pointer to constant flag to get value of its pointe
    bool constPointe = false;
    
    /// Targets/Initiators id`s in top module bounded externally, to detect errors
    std::unordered_set<uint32_t> extrTargInit;
};


/// Values for module/MIF, that provides the same value for ObjectView instance
class RecordValues {
public:
    RecordValues() = delete;

    static void setElabDB(ElabDatabase* elabDB_) {
        elabDB = elabDB_;
    }
    /// Add modules/MIFs with NO_VALUE
    static void addRecordView(const RecordView& recView);
    /// Get SValue for module/MIF object, return NO_VALUE for other 
    static const sc::SValue& getRecordView(const RecordView& recView);
    /// Fill all the values 
    static void fillValues();
    static void print();

private:
    /// Get base classes values fro given record
    static std::vector<sc::SValue> getBaseValues(const RecordView& recView);
    /// Get record value from @recordMap or create it there and return
    static sc::SValue getOrCreateRecordValue(const RecordView& recView);
    /// Create record value in @recordMap and return it
    static void createRecordValue(const RecordView& recView, 
                                  const sc::SValue& parent = sc::NO_VALUE);
    /// Add top module SValue into @recordMap
    static void fillTopModValue();
    /// Add value for modules with ready parents into @recordMap
    static void fillValuesWithParent();
    
    /// Mapping record object ID to SValue for all records in the design
    static std::unordered_map<uint32_t, sc::SValue> recordMap;
    static ElabDatabase* elabDB;
};



} // end namespace sc_elab

#endif // SCELABPROCBUILDER_H
