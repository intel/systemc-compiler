/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/* 
 * SC module instance.
 *
 * Author: Mikhail Moiseev
 */

#ifndef SCMODULE_H
#define SCMODULE_H

#include "sc_tool/systemc/ScChannel.h"
#include "sc_tool/systemc/ScObject.h"
#include "sc_tool/cfg/ScState.h"

#include <clang/AST/DeclCXX.h>

namespace sc {

class ScState;

enum ScProcessKind {pkMethod, pkThread};

struct ProcessDescr
{
    typedef std::pair<ScInPort*,bool> Reset_t;
    ScProcessKind           kind;
    clang::CXXMethodDecl*   func;
    ScInPort*               clock;
    ScClockEdge             clockEdge;
    Reset_t                 asyncReset;
    std::vector<Reset_t>    syncResets;
    
    ProcessDescr(clang::CXXMethodDecl* func_, ScProcessKind kind_) : 
                kind(kind_), func(func_), clock(nullptr), 
                asyncReset(Reset_t(nullptr, true)) 
    {}
    void addClock(ScInPort* clock_, ScClockEdge edge_) {
        clock = clock_; clockEdge = edge_;
    }
    void addAsyncReset(ScInPort* rst_, bool level) {
        asyncReset = Reset_t(rst_, level);
    }
    void addSyncReset(ScInPort* rst_, bool level) {
        syncResets.push_back(Reset_t(rst_, level));
    }
};

// ============================================================================

/** 
 * CXX class type
 */ 
class CxxClassType 
{
protected:
    std::string     name;
    // Clang type for module class
    clang::QualType   type;
    // Direct base classes
    std::vector<std::shared_ptr<CxxClassType> >  bases;
    
public:
    explicit CxxClassType(const clang::QualType& type_) : type(type_) {}
    
    // Add base module type
    void addBase(std::shared_ptr<CxxClassType> parent);
    
    std::vector<std::shared_ptr<CxxClassType> > getBases();
    
    clang::QualType getType();
    
    std::string getName();

    bool operator == (const CxxClassType& rhs) const;

    std::string asString();
};

// ============================================================================

// Forward declaration
class ScModule;

/** 
 * General CXX class/structure
 */ 
class CxxClass 
{
protected:
    typedef std::pair<std::shared_ptr<ProcessDescr>, SValue> ProcDescr_t;
    
    /// Class instance variable name, used for debug only
    std::string     varName;
    /// Module type 
    std::shared_ptr<CxxClassType>           type;    
    /// Module instance where this instance is declared, 
    /// @shared_ptr not used here to avoid cross references
    CxxClass*                               parent;
    /// Module instances declared in this module
    std::vector<std::shared_ptr<CxxClass> > childs;
    
    /// Empty process vector, used in getProcesses
    std::vector<ProcDescr_t>  empty;
    
public:
    CxxClass(std::string name, std::shared_ptr<CxxClassType> modType_) : 
        varName(name), type(modType_), parent(nullptr)
    {}

    CxxClass(std::string name, std::shared_ptr<CxxClassType> modType_, 
             CxxClass* parent_) : 
        varName(name), type(modType_), parent(parent_)
    {}

    // Copy constructor with state clone, for other members pointers are copied
    CxxClass(const CxxClass& rsh) : 
        varName(rsh.varName), type(rsh.type), parent(rsh.parent), childs(rsh.childs)
    {}

    virtual ~CxxClass() {}
    
    virtual bool isModule() {
        return false;
    }
    
    std::shared_ptr<CxxClassType> getClassType() {
        return type;
    }
    
    CxxClass* getParent() {
        return parent;
    }
    
    /// Add child module instance
    void addChildClass(std::shared_ptr<CxxClass> child) {
        childs.push_back(child);
    }
    
    std::vector<std::shared_ptr<CxxClass> > getChilds() {
        return childs;
    }
    
    /// Add port instance
    virtual void addPort(ScChannel* port) {
        SCT_TOOL_ASSERT (false, "Add port to non-module class");
    }

    /// Add port binding
    virtual void addPortBind(ScModule* lmodule, ScChannel* lport, 
                             ScModule* rmodule, ScChannel* rport) {
        SCT_TOOL_ASSERT (false, "Bind port in non-module class");
    }

    /// Add process
    virtual void addProcess(std::shared_ptr<ProcessDescr> proc, 
                            const SValue& modval) 
    {
        SCT_TOOL_ASSERT (false, "Add process to non-module class");
    }

    /// Get module processes
    virtual std::vector<ProcDescr_t>& getProcesses() {
        return empty;
    }    

    virtual std::string getName() const;

    virtual std::string asString(bool debug = true) const;

    virtual void print() const;    
};


/** 
 * SC module instance, many instances of the module type are possible.
 * State is created in module constructor and released in destructor
 */ 
class ScModule : public CxxClass, public ScObject 
{
protected:
    // Port, signals and other fields
    std::vector<ScChannel*>     ports;
    
    // Pair with context module and its port 
    typedef std::pair<ScModule*, ScChannel*>  ModPort_t;
    // Port bindings for this module and its children
    // (@left, @right), @left is bound to @right
    std::vector<std::pair<ModPort_t, ModPort_t> >   binds;
   
    // Process list <process description, current module value>
    std::vector<ProcDescr_t>  processes;
    
public:
    ScModule(std::string name, std::shared_ptr<CxxClassType> modType) : 
        CxxClass(name, modType), ScObject(name)
    {}

    ScModule(std::string name, std::shared_ptr<CxxClassType> modType, 
             CxxClass* parent) : 
        CxxClass(name, modType, parent), ScObject(name)
    {}

    // Copy constructor with state clone, for other members pointers are copied
    ScModule(const ScModule& rsh) : 
        CxxClass::CxxClass(rsh),
        ScObject::ScObject(rsh),
        ports(rsh.ports), binds(rsh.binds), processes(rsh.processes)
    {}
    
    virtual ~ScModule() {}

    virtual bool isModule() { 
        return true;
    }
    
    ScObject::Kind getObjectKind() const {
        return ScObject::Kind::scModule;
    }
    
    /// Add port instance
    virtual void addPort(ScChannel* port) {
        ports.push_back(port);
    }

    /// Add port binding
    virtual void addPortBind(ScModule* lmodule, ScChannel* lport, 
                     ScModule* rmodule, ScChannel* rport) {
        binds.push_back(std::pair<ModPort_t, ModPort_t> (
                        ModPort_t(lmodule, lport), ModPort_t(rmodule, rport)));
    }

    /// Add process
    virtual void addProcess(std::shared_ptr<ProcessDescr> proc, 
                            const SValue& modval) 
    {
        processes.push_back(ProcDescr_t(proc, modval));
    }

    /// Get module processes
    virtual std::vector<ProcDescr_t>& getProcesses() {
        return processes;
    }
    
    virtual std::string getName() const;
    
    virtual std::string asString(bool debug = true) const;

    virtual void print() const;
};



}
#endif /* SCMODULE_H */