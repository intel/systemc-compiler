/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Non-owning lightweight view that bundles together pointers
 * to Protobuf Object and ElabDatabase.
 * - Provides easy to use Object tree navigation
 * - Can be used as a key in std::unordered_map
 * 
 * Author: Roman Popov
 */

#ifndef SCTOOL_SCOBJECTVIEW_H
#define SCTOOL_SCOBJECTVIEW_H

#include <llvm/ADT/Optional.h>
#include <llvm/Support/raw_ostream.h>
#include <clang/AST/Type.h>
#include <clang/AST/DeclCXX.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/APSInt.h>
#include <sc_elab.pb.h>
#include <functional>
#include <deque>

namespace sc_elab
{

class ObjectView;
class ElabDatabase;
class VerilogModule;
class VerilogVar;
class VerilogVarRef;
class ModuleMIFView;
class PrimitiveView;
class RecordView;
class ArrayView;
class SignalView;
class ProcessView;
class ValueView;
class PtrOrRefView;
class PortView;
struct ArrayElemObjWithIndices;

using IndexVec = llvm::SmallVector<uint32_t, 4>;
using APSIntVec = llvm::SmallVector<llvm::APSInt, 4>;
using ElabObjVec = llvm::SmallVector<ObjectView, 4>;
using ArrayElemVec = llvm::SmallVector<ArrayElemObjWithIndices, 4>;

class ObjectView
{
public:

    friend std::hash<ObjectView>;

    ObjectView(const sc_elab::Object *obj, const sc_elab::ElabDatabase *db);
    ObjectView() = default;

    friend bool operator==(const ObjectView &one, const ObjectView &other);
    friend bool operator!=(const ObjectView &one, const ObjectView &other);

    clang::QualType getType() const;

    uint32_t getID() const { return getProtobufObj()->id(); }

    /// Elaboration time constant
    bool isConstant() const { return getProtobufObj()->is_constant(); }

    /// Object is outside of module hierarchy
    bool hasNoParent() const
    {
        return getProtobufObj()->rel_type() == Object::NO_PARENT;
    }

    /// non-aggregate type : value, pointer, reference
    bool isPrimitive() const
    {
        return getProtobufObj()->kind() == Object::PRIMITIVE;
    }

    bool isPointerOrRef() const;

    /// Array or Record
    bool isAggragete() const { return isRecord() || isArrayLike(); }

    /// C-style array, std::array, std::vector, sc_vector
    bool isArrayLike() const
    {
        return getProtobufObj()->kind() == Object::ARRAY;
    }

    /// C++ class or struct (including modules and modular interfaces)
    bool isRecord() const { return getProtobufObj()->kind() == Object::RECORD; }

    /// Module (Not modular interface!)
    bool isModule() const
    {
        return getProtobufObj()->sckind() == Object::SC_MODULE;
    }

    bool isModularInterface() const
    {
        return getProtobufObj()->sckind() == Object::SC_MODULAR_INTERFACE;
    }

    bool isSignal() const
    {
        return getProtobufObj()->sckind() == Object::SC_SIGNAL;
    }

    bool isBaseClass() const
    {
        return getProtobufObj()->rel_type() == Object::BASE_CLASS;
    }

    bool isDataMember() const
    {
        return getProtobufObj()->rel_type() == Object::DATA_MEMBER;
    }

    bool isStatic() const
    {
        return getProtobufObj()->rel_type() == Object::STATIC;
    }

    bool isArrayElement() const
    {
        return getProtobufObj()->rel_type() == Object::ARRAY_ELEMENT;
    }

    bool isDynamic() const
    {
        return getProtobufObj()->rel_type() == Object::DYNAMIC;
    }
    
    bool isProcess() const
    {
        if (isPrimitive()) {
            return getProtobufObj()->primitive().kind() == Primitive::PROCESS;
        }
        return false;
    }

    /// Signal or Signal Port
    bool isChannel() const;

    /// Root of object hierarchy, top level module
    bool isTopMod() const;

    /// Get list of pointer/reference objects pointing to this object
    ElabObjVec getPointers() const;
    /// Get list of ports (not-pointers) bound to this object
    ElabObjVec getPorts() const;

    /// If Object is not a top level module, returns parent Object.
    /// otherwise behavior undefined
    ObjectView getParent() const;

    /// If Object is not a top level module, returns parent Module Object.
    /// otherwise behavior undefined
    /// ( Excludes modular interfaces )
    ModuleMIFView getParentModule() const;

    ModuleMIFView getParentModuleOrMIF() const;

    /// If Object is array element, or pointee of array of pointers
    /// returns array
    llvm::Optional<ObjectView> getTopmostParentArray() const;

    /// Return topmost array or pointer variable 
    llvm::Optional<ObjectView> getTopmostParentArrayOrPointer() const;
    
    /// Find common parent module for two objects
    /// If one is top, behavior undefined
    ModuleMIFView nearestCommonParentModule(ObjectView other) const;

    /// Get list of parent modules starting with top level
    std::deque<ModuleMIFView> getParentModulesList() const;

    /// Get list of parent modules starting with given module
    std::deque<ModuleMIFView> getParentModulesList(const ModuleMIFView &rootMod) const;

    /// If Object is array element, returns index of element in array
    llvm::Optional<uint32_t> getElementIndex() const;

    /// If Object is inside array, or hierarchy of arrays, selects all
    /// similar objects in hierarchy of arrays.
    /// Example: Path to object is  MOD.x.y[2].z[3].bob
    /// Result: All objects: MOD.x.y[*].z[*].bob
    ArrayElemVec getAllSimilarArrayElements() const;

    /// If Object is inside array, or hierarchy of arrays
    /// returns first array element (representing array for verilog gen)
    /// and indicies of this object
    /// Example: Path to this object is  MOD.x.y[2].z[3].bob
    /// Result: ( MOD.x.y[0].z[0].bob , (2,3) )
    ArrayElemObjWithIndices getAsArrayElementWithIndicies() const;
    ArrayElemObjWithIndices getAsArrayElementWithIndicies(PortView port) const;
    /// The same as previous but return PortView
    PortView getPortBound(PortView port) const;

    /// If Object is pointer or reference, recursively deferefence it
    /// at return pointee object
    llvm::Optional<ObjectView> derefRecursively() const;

    /// Get related ObjectView that has name in Verilog
    /// 1. Dereferences pointers and references
    /// 2. For non-const arrays name is stored inside first array element
    llvm::Optional<ObjectView> getVerilogNameOwner() const;

    /// If Object is data member or static member, returns declaration
    clang::ValueDecl *getValueDecl() const;

    /// If Object is data member, returns field name
    const std::string *getFieldName() const;
    /// If Object is data member, returns field decl
    const clang::FieldDecl *getFieldDecl() const;

    /// Get Verilog variables representing SystemC object
    /// returns empty vector if object is not represented directly in Verilog
    llvm::SmallVector<VerilogVarRef, 1> getVerilogVars() const;
    /// Do not change offset to zero
    llvm::SmallVector<VerilogVarRef, 1> getVerilogVarsOffset() const;
    
    llvm::Optional<PrimitiveView> primitive() const;
    llvm::Optional<RecordView> record() const;
    llvm::Optional<ModuleMIFView> moduleMIF() const;
    llvm::Optional<ArrayView> array() const;
    llvm::Optional<SignalView> signal() const;
    llvm::Optional<std::string> string() const;

    std::string getDebugString() const;

    friend llvm::raw_ostream &operator<<(llvm::raw_ostream &os,
                                         const ObjectView &view)
    {
        os << view.getDebugString();
        return os;
    }

    void dumpHierarchy(bool traverseSubmods = false) const;

    const sc_elab::Object *getProtobufObj() const { return obj; }
    const sc_elab::ElabDatabase *getDatabase() const { return db; }

private:
    const sc_elab::Object *obj = nullptr;
    const sc_elab::ElabDatabase *db = nullptr;
};

struct ArrayElemObjWithIndices {
    ObjectView obj;
    IndexVec indices;
};

class ArrayView : public ObjectView
{
public:
    friend std::hash<ArrayView>;

    ArrayView(const ObjectView &objView);

    uint32_t size() const;
    /// For optimization purposes elements of non-const arrays of primitives
    /// are not saved by elaborator
    bool hasElements() const;
    std::vector<size_t> getOptimizedArrayDims() const;
    std::size_t getOptimizedArrayBitwidth() const;

    llvm::Optional<ObjectView> getFirstNonArrayEl() const;
    bool isChannelArray() const;

    bool isConstPrimitiveArray() const;

    ObjectView at(std::size_t idx) const;
};

class RecordView : public ObjectView
{
public:
    RecordView(const ObjectView &objView);

    llvm::Optional<ObjectView> getField(const clang::FieldDecl *fieldDecl) const;
    llvm::Optional<ObjectView> getField(const std::string fieldName) const;

    llvm::Optional<ObjectView> getBase(clang::QualType baseType) const;

    /// get base classes in declaration order
    std::vector<RecordView> getBases() const;
    /// get fields in declaration order
    ElabObjVec getFields() const;

    /// get static variables
    ElabObjVec getStaticFields() const;

    /// return both fields , bases and static variables
    ElabObjVec getMembers() const;
};

/// Module OR Modular Interface
class ModuleMIFView : public RecordView
{
    friend std::hash<ModuleMIFView>;
public:

    ModuleMIFView(const ObjectView &objView);

    bool isModularInterface() const { return RecordView::isModularInterface(); }

    /// sc_object name
    const std::string &getName() const { return getProtobufObj()->sc_name(); }

    std::vector<ProcessView> getProcesses() const;

    /// dynamically allocated objects including processes
    ElabObjVec getDynamics() const;

    /// bases, fields, dynamics including processes
    ElabObjVec getAllMembers() const;

    const VerilogModule *getVerilogModule() const;
};

/// sc_signal<T>
class SignalView : public ObjectView
{
public:

    SignalView(const ObjectView &objView);

    /// sc_object name
    const std::string &getName() { return getProtobufObj()->sc_name(); }

    /// underlying value object, can be used-defined type!
    ObjectView getSignalValue();

};

class PrimitiveView : public ObjectView
{
public:

    PrimitiveView(const ObjectView &objView);

    /// Value: integral, fixed or float
    bool isValue() const
    {
        return getProtobufObj()->primitive().kind() == Primitive::VALUE;
    }

    bool isPointer() const
    {
        return getProtobufObj()->primitive().kind() == Primitive::POINTER;
    }

    bool isPort() const
    {
        return getProtobufObj()->primitive().kind() == Primitive::PORT;
    }

    bool isReference() const
    {
        return getProtobufObj()->primitive().kind() == Primitive::REFERENCE;
    }

    bool isExport() const
    {
        return getProtobufObj()->primitive().kind() == Primitive::EXPORT;
    }

    bool isEvent() const
    {
        return getProtobufObj()->primitive().kind() == Primitive::EVENT;
    }

    bool isProcess() const
    {
        return getProtobufObj()->primitive().kind() == Primitive::PROCESS;
    }

    bool isString() const
    {
        return getProtobufObj()->primitive().kind() == Primitive::STRING;
    }
    
    bool isUnsupported() const
    {
        return getProtobufObj()->primitive().kind() == Primitive::UNSUPPORTED;
    }

    llvm::Optional<ValueView> value() const;
    llvm::Optional<PtrOrRefView> ptrOrRef() const;
    llvm::Optional<PortView> port() const;
    llvm::Optional<ProcessView> process() const;
};

class ValueView : public PrimitiveView
{
public:
    ValueView(const ObjectView &objView);

    std::size_t bitwidth() const;

    /// Bitwidth defined during elaboration, not compile-time
    bool isDynamicBitwidth() const;

    llvm::Optional<int64_t> int64Val() const;
    llvm::Optional<uint64_t> uint64Val() const;
    llvm::Optional<double> doubleVal() const;
};

class PtrOrRefView : public PrimitiveView
{
public:
    PtrOrRefView(const ObjectView &objView);

    bool isNull() const { return getProtobufObj()->primitive().ptr_val().is_null(); }
    bool isNotNullDangling() const;

    /// pointers into optimized arrays are stored as base and offset
    bool isBaseOffsetPtr() const;

    /// If not dangling, returns ID of pointee
    llvm::Optional<uint32_t> getPointeeID() const;

    /// if pointer is BaseOffsetPtr, returns offset
    llvm::Optional<uint32_t> getOffset() const;

    llvm::Optional<ObjectView> pointee() const;

    /// if pointer is pointer to first element of array, returns array
    /// othersise works as a regular pointee()
    llvm::Optional<ObjectView> pointeeOrArray() const;

    llvm::Optional<ObjectView> getFirstNonPointerPointee() const;

};

enum class PortDirection { IN, OUT, INOUT, NONE };

class PortView : public PrimitiveView
{
public:

    PortView(const ObjectView &objView);

    bool isSignalPort() const;

    SignalView getBindedSignal() const;

    /// port or signal directly binded to this port
    ObjectView getDirectBind() const;

    PortDirection getDirection() const;
    bool isInput() const { return getDirection() == PortDirection::IN; }
    bool isOutput() const { return getDirection() == PortDirection::OUT; }
    bool isInout() const { return getDirection() == PortDirection::INOUT; }

    /// Get pointee module/MIF for sc_port<IF>
    llvm::Optional<ObjectView> pointee() const;
    
    /// sc_object name
    const std::string &getSCName() const { return getProtobufObj()->sc_name(); }

private:
    mutable bool dirInitialized = false;
    mutable PortDirection dir;
};

class ProcessView : public PrimitiveView
{
    friend std::hash<ProcessView>;
public:
    typedef Process::ProcessKind ProcessKind;
    typedef Sensitive::EventKind EventKind;

    struct SensEvent
    {
        SensEvent(ObjectView srcObj, EventKind kind)
            : sourceObj(srcObj), kind(kind) {}
        ObjectView sourceObj;
        EventKind kind;
        
        bool isPosedge() const
        {
            return kind == EventKind::Sensitive_EventKind_POSEDGE;
        }
        bool isNegedge() const
        {
            return kind == EventKind::Sensitive_EventKind_NEGEDGE;
        }
        bool isDefault() const
        {
            return kind == EventKind::Sensitive_EventKind_DEFAULT;
        }

    };

    struct Reset
    {
        Reset(ObjectView srcObj, bool level, bool isAsync)
            : sourceObj(srcObj), level(level), isAsync(isAsync) {}
        const ObjectView sourceObj;
        bool level;
        bool isAsync;
    };

    ProcessView(const ObjectView &objView);

    bool isScMethod() const;
    bool isScThread() const;
    bool isScCThread() const;

    // Check if method is combinational or sequential
    bool isCombinational() const;
    
    void setHasLatch() { hasLatch = true; }
    bool getHasLatch() const { return hasLatch; }

    std::vector<SensEvent> staticSensitivity() const;
    std::vector<Reset> resets() const;

    /// Get host class and function declarations for given process name and 
    /// class where process run by SC_METHOD/SC_CTHREAD
    std::pair<clang::CXXRecordDecl*, clang::CXXMethodDecl*>
    getProcDeclFromBase(clang::CXXRecordDecl* moduleDecl,
                        const std::string& procName) const;
    /// Get object where process was spawned and method declaration
    std::pair<RecordView, clang::CXXMethodDecl*> getLocation() const;
    /// There is latch(es) in the process, generate full sensitivity events
    bool hasLatch = false;
    /// Process name to us ein Verilog, unique in the module
    std::string procName;
};

} // end namespace sc_elab


#define OBJ_SPECIALIZE_STD_HASH(TYPE) \
namespace std \
{ \
    template<> struct hash<TYPE> \
    { \
        typedef TYPE argument_type; \
        typedef std::size_t result_type; \
        result_type operator()(argument_type const &obj) const noexcept \
        { \
            result_type const h1 \
                = std::hash<const sc_elab::Object *>{}(obj.getProtobufObj()); \
            result_type const \
                h2 = std::hash<const sc_elab::ElabDatabase *>{}(obj.getDatabase()); \
            return h1 ^ (h2 << 1); \
        } \
    }; \
} // namespace std

OBJ_SPECIALIZE_STD_HASH(sc_elab::ObjectView);
OBJ_SPECIALIZE_STD_HASH(sc_elab::ModuleMIFView);
OBJ_SPECIALIZE_STD_HASH(sc_elab::ProcessView);
OBJ_SPECIALIZE_STD_HASH(sc_elab::ArrayView);
OBJ_SPECIALIZE_STD_HASH(sc_elab::PortView);

#undef SPECIALIZE_STD_HASH_

#endif //SCTOOL_SCOBJECTVIEW_H
