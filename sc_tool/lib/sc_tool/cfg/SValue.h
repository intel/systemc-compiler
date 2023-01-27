/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/** 
 * Universal value used in analysis
 */

#ifndef SVALUE_H
#define SVALUE_H

#include "sc_tool/diag/ScToolDiagnostic.h"
#include "sc_tool/systemc/ScChannel.h"
#include "llvm/ADT/APSInt.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/Decl.h"
#include <iostream>
#include <vector>

namespace sc {
    
class IMemory;
class SObject;
class SRecord;
class SArray;
class SVariable;
class STmpVariable;

enum class ObjectOwner{
    ooFalse,
    ooTrue
};

class SValue;

/// No value used for @RValue if there is no value in state
extern const SValue NO_VALUE;
/// Zero width integer value, generated as unsigned 32bit 0, radix 100
extern const SValue ZW_VALUE;

/// Typed value in state.
/// SValue owns @otInteger, @otVariable and @otTemporary, 
/// but does not own @otObject, i.e. NOT COPY @otScObject but only COPY POINTER
class SValue 
{
protected:
    /// State operates with object types:
    ///  @otMemory      -- object in memory, all object can be LValue
    ///  @otIntegerXXX  -- integral constant 
    ///  @otChannel     -- any channel like ScPort and ScSignal
    enum SObjectType {otMemory = 0, otChannel = 1, otUnknown = 2, 
                      otIntegerDec = 3, otIntegerHex = 4, otIntegerBin = 5, 
                      otIntegerOct = 6, otZeroWidth = 7};
    
    /// Object type
    SObjectType type;
    
    /// @SValue exclusively owns the value memory, so no pointer/reference 
    /// should be returned, only copy of value 
    union {
        IMemory*        memory;     // LValue/RValue memory object 
        llvm::APSInt*   integer;    // RValue integer in state pair
        ScChannel*      channel;    // RValue channel in state pair
    };
    
    void free_members();
    
public: 
    /// Create unknown value, pointer is @nullptr
    explicit SValue();
    
    /// Create integer value
    explicit SValue(const llvm::APSInt& val, char radix);
    
    /// Create channel value
    explicit SValue(ScChannel* val);

    /// Create variable value
    explicit SValue(const clang::ValueDecl* decl, const SValue& parent);

    /// Create temporary variable value
    explicit SValue(const clang::QualType& type_, const SValue& parent);

    /// Create simple object value
    /// \param owner -- is the pointer owner of its pointee, true for pointers
    ///                 initialized at elaboration phase   
    explicit SValue(const clang::QualType& type_, 
                    ObjectOwner owner = ObjectOwner::ooFalse);

    /// Create array object value, not array variable
    explicit SValue(const clang::QualType& type_, size_t size, size_t offset);

    /// Create global record/module object value
    explicit SValue(const clang::QualType& type_, std::vector<SValue> bases, 
                    const SValue& parent);
    
    /// Create local record/module object value
    /// \param var -- variable/member which has this record value
    /// \param index -- element number in single/multi dimensional array
    explicit SValue(const clang::QualType& type_, std::vector<SValue> bases, 
                    const SValue& parent, const SValue& var, size_t index = 0);
    
    /// Copy constructor
    SValue(const SValue &rhs);
    
    /// Copy assign
    SValue& operator = (const SValue &rhs);
    
    /// Destructor
    ~SValue() {
        free_members();
    }
    
    /// Clone this value in another parent and owner variable if required
    /// No clone for integer and channel, return this value instead
    SValue clone(const SValue& parent = NO_VALUE, const SValue& locvar = NO_VALUE, 
                 size_t index = 0) const;
    
    /// Create zero value of the width/signess of @expr type 
    static SValue zeroValue(const clang::Expr* expr);
    static SValue zeroValue(clang::QualType type, 
                            const clang::Expr* expr = nullptr);
    
    bool isUnknown() const;

    /// True if value is valid
    operator bool() const { return !isUnknown();}
    
    bool isInteger() const;
    
    bool isMemory() const;
    
    bool isVariable() const;

    bool isTmpVariable() const;
    
    // Is the object any kind of SObject, i.e SObject, SArray or SRecord
    bool isObject() const;
    
    // Is the object exact SObject, not SArray and not SRecord
    bool isSimpleObject() const;

    bool isArray() const;

    bool isRecord() const;
    
    bool isScChannel() const;
    
    bool isScInPort() const;
    
    bool isScOutPort() const;
    
    bool isScSignal() const;
    
    bool isReference() const;
    
    bool isConstReference() const;

    bool isPointer() const;

    clang::QualType getType() const;
    const clang::Type* getTypePtr() const;
    
    /// Use this reference before @this SValue end of life
    llvm::APSInt& getInteger() const;
    
    /// Return integer radix or 0
    char getRadix() const;
    
    /// Check is it sct_zero_width literal, radix == 100
    char isScZeroWidth() const;

    /// Set given radix for integer, ignored if @this is not integer
    void setRadix(char radix);
    
    /// Pointer to memory object interface
    IMemory* getMemory() const;

    /// Use this reference before @this SValue end of life
    SVariable& getVariable() const;
    
    STmpVariable& getTmpVariable() const;
    
    /// Any SObject pointer, use this pointer before end of life
    SObject* getObjectPtr() const;

    /// Use this reference before @this SValue end of life
    SObject& getSimpleObject() const;

    /// Use this reference before @this SValue end of life
    SArray& getArray() const;

    /// Use this reference before @this SValue end of life
    SRecord& getRecord() const;
    
    /// Return pointer, it is never deleted
    ScChannel* getScChannel() const;
    
    /// Return pointer, it is never deleted
    ScInPort* getScInPort() const;
    
    /// Return pointer, it is never deleted
    ScOutPort* getScOutPort() const;
   
    /// Strict comparison used to check SValues are the same
    /// Compare pointers for ScObject, as pointer is unique
    /// Compare with checking bit width and signedness of integers
    /// Two unknown values considered as equal
    bool operator == (const SValue& rhs) const;
    
    bool operator != (const SValue& rhs) const;

    /// Get value as string
    /// \param debug -- add debug information like parent and type
    std::string asString(bool debug = true) const;
    
    /// Return boolean representation of this value, used to calculate condition
    bool getBoolValue() const;
    
    /// Create APSInt from bool
    static llvm::APSInt boolToAPSInt(bool val) {
        // @true as boolean is unsigned 
        return llvm::APSInt(llvm::APInt(1, (val) ? 1 : 0), true);   
    }
    
    /// Compare two APSInt including bit width and signedness 
    static bool exactEqual(const llvm::APSInt& int1, const llvm::APSInt& int2) 
    {
        if (int1.getBitWidth() != int2.getBitWidth()) return false;
        if (int1.isUnsigned() != int2.isUnsigned()) return false;
        return (int1 == int2);
    }
    
    /// Check if this is variable value and it is instance of 
    /// @mval module/class or its base classes
    bool isInClassHierarhy(const SValue& mval);

    template <typename OsT>
    friend OsT &operator << (OsT &os, const SValue &val) {
        os << val.asString();
        return os;
    }

};

// ============================================================================

/// Interface for any object in state
class IMemory
{
public:
    enum Kind {otVariable, otObject, otArray, otRecord, otTmpVariable};
    
    virtual ~IMemory() {}
    
    /// Get object kind
    virtual Kind getObjectKind() const = 0;

    /// Get Clang type
    virtual clang::QualType getType() const = 0;

    /// Get string for array object
    virtual std::string asString(bool debug = true) const = 0;
};

// ----------------------------------------------------------------------------

/// Base class for any object in state
class SObject : public IMemory
{
protected:      
    friend class SValue;

    // Unique ID generator
    static uint64_t id_gen;
    // Object unique ID
    uint64_t  id;     
    // Type of Clang object
    clang::QualType type;
    // Is this object owner of its pointee (right part object in tuple)
    ObjectOwner owner;
    
public:
    
    /// Constructor
    explicit SObject(const clang::QualType& type_, 
                     ObjectOwner owner_ = ObjectOwner::ooFalse) : 
        id(id_gen++), type(type_), owner(owner_)
    {}
        
    /// Copy constructor    
    SObject(const SObject& rhs) 
    {
        id = rhs.id;
        type = rhs.type;
        owner = rhs.owner;
    }

    virtual ~SObject() {}
    
    bool operator == (const SObject& rhs) const {
        return (id == rhs.id);
    }

    /// Get object kind
    virtual Kind getObjectKind() const;
    
    /// Get Clang type
    virtual clang::QualType getType() const;
    
    void setType(clang::QualType type_);
    
    uint64_t getId() const {return id;}
    
    bool isOwner() const {return (owner == ObjectOwner::ooTrue);}
    
    /// Get string for array object
    virtual std::string asString(bool debug = true) const;
};

// ----------------------------------------------------------------------------

/// Array object in state
class SArray : public SObject 
{
protected:    
    friend class SValue;

    std::size_t     size;
    std::size_t     offset;
    // Unknown offset
    bool            unknown;
    
public:    
    /// Constructor, array is always object owner    
    explicit SArray(const clang::QualType& type, std::size_t size_, 
                    std::size_t offset_, bool unknown_ = false) : 
        SObject(type, ObjectOwner::ooTrue), size(size_), offset(offset_), 
        unknown(unknown_)
    {}

    /// Copy constructor
    SArray(const SArray& rhs) : 
        SObject(rhs), size(rhs.size), offset(rhs.offset), unknown(rhs.unknown)
    {}
    
    
    /// Set offset for array element and clear @unknown flag
    void setOffset(size_t offset_) {
        if (offset_ < size) {
            offset = offset_;
            unknown = false;
        } else {
            ScDiag::reportScDiag(ScDiag::CPP_ARRAY_OUT_OF_BOUND);
            std::cout << "setOffset offset " << offset_ << ", size " 
                      << size << ", unknown" << unknown << std::endl;
            SCT_TOOL_ASSERT (false, "Incorrect offset");
        }
    }
    
    std::size_t getOffset() {
        return offset;
    }
    
    std::size_t getSize() {
        return size;
    }

    void setUnknownOffset() {
        offset = 0;
        unknown = true;
    }
    
    /// Clear @unknown flag
    void clearUnknown() {
        unknown = false;
    }

    bool isUnknown() { 
        return unknown;
    }

    /// Compare object without consider offsets, 
    /// return @true if both are the same object
    bool compareObject(const SArray& rhs) const {
        return (SObject::operator ==(rhs));
    }

    bool operator == (const SArray& rhs) const {
        return (SObject::operator ==(rhs) && size == rhs.size && 
                offset == rhs.offset && unknown == rhs.unknown);
    }
    
    /// Get object kind
    virtual Kind getObjectKind() const;

    /// Get string for array object
    virtual std::string asString(bool debug = true) const;
};

// ----------------------------------------------------------------------------

/// Module/class object in state
class SRecord : public SObject 
{
public:
    friend class SValue;
    
    /// Direct base classes
    std::vector<SValue>   bases;
    /// Parent class instance, where this class declared/created, for debug
    SValue    parent;
    /// Local variable/member owns this record, for local record only
    /// @NO_VALUE for global records
    SValue  var;
    /// Array element number for local variable/member, for local record only
    size_t  index;
    
    /// Constructor
    explicit SRecord(const clang::QualType& type, std::vector<SValue> bases_, 
                     const SValue& parent_, const SValue& var_, size_t index_) : 
        SObject(type), bases(bases_), parent(parent_), var(var_), index(index_)
    {
        SCT_TOOL_ASSERT(var.isUnknown()||var.isVariable()||var.isTmpVariable(), 
                        "Record owner is not variable");
    }

    /// Copy constructor, create object with the same @id    
    SRecord(const SRecord& rhs) : 
        SObject(rhs), bases(rhs.bases), parent(rhs.parent), var(rhs.var),
        index(rhs.index)
    {}
    
    bool operator == (const SRecord& rhs) const {
        if (var) {
            // Compare type to distinguish base classes of record
            return (var == rhs.var && type == rhs.type && index == rhs.index);
        } else {
            return (SObject::operator ==(rhs));
        }
    }

    /// Get object kind
    virtual Kind getObjectKind() const;

    /// Is local record
    bool isLocal() const {
        return var;
    };
    
    bool isTemp() const {
        return var.isTmpVariable();
    }
    
    /// Get string with all base class names including this class name
    static std::string getBaseString(const SRecord& obj) 
    {
        std::string thisStr = obj.getType()->getAsCXXRecordDecl()->
                              getNameAsString();
        if (obj.bases.size()) {
            return (SRecord::getBaseString(obj.bases.front().getRecord()) +
                    "::" + thisStr);
        }
        return thisStr;
    }

    /// Get string for class/module object
    virtual std::string asString(bool debug = true) const;
};

// ----------------------------------------------------------------------------

/// Variable on stack
class SVariable : public IMemory
{
protected:
    friend class SValue;

    /// Reference to clang object declaration
    const clang::ValueDecl*  decl;  
    /// Parent class instance, where this variable declared
    SValue    parent;
    
public:
    /// Create stack variable 
    explicit SVariable(const clang::ValueDecl* decl_, const SValue& parent_) :
        decl(decl_), parent(parent_)
    {
        if (decl) {
            if (auto *varDecl = llvm::dyn_cast<clang::VarDecl>(decl_)) {
                decl = varDecl->getCanonicalDecl();
                if (varDecl->hasDefinition())
                    decl = varDecl->getDefinition();
            }
            SCT_TOOL_ASSERT(decl, "No declaration");
        }
    }

    /// Copy constructor
    SVariable(const SVariable& rhs) : 
        decl(rhs.decl), parent(rhs.parent)
    {}
    
    bool operator == (const SVariable& rhs) const {
        return (decl == rhs.decl && parent == rhs.parent);
    }
    
    const clang::ValueDecl* getDecl() const {return decl;}
    
    const SValue& getParent() const {return parent;}

    /// Get object kind
    virtual IMemory::Kind getObjectKind() const;

    /// Get Clang type
    virtual clang::QualType getType() const;
    
    virtual std::string asString(bool debug = true) const;
};

// ----------------------------------------------------------------------------

/// Temporary variable on stack used for return value from function 
/// This is artificial variable, so it does not have declaration
class STmpVariable : public SVariable
{
protected:
    friend class SValue;

    /// Reference to clang type
    clang::QualType  type;  
    // Unique ID generator
    static uint64_t id_gen;
    // Object unique ID
    uint64_t  id;

public:
    /// Create stack variable 
    explicit STmpVariable(const clang::QualType& type_, const SValue& parent) : 
        SVariable(nullptr, parent), // No ValueDecl here
        type(type_), id(id_gen++)
    {}

    /// Copy constructor
    STmpVariable(const STmpVariable& rhs) : 
        SVariable(rhs), type(rhs.type), id(rhs.id)
    {}
    
    bool operator == (const STmpVariable& rhs) const {
        return (id == rhs.id && parent == rhs.parent);
    }
    
    uint64_t getId() const {return id;}
    
    /// Get object kind
    virtual IMemory::Kind getObjectKind() const;

    /// Get Clang type
    virtual clang::QualType getType() const;
    
    virtual std::string asString(bool debug = true) const;
    
    static void clearId() {
        id_gen = 0;
    }
};
}

// ----------------------------------------------------------------------------
// Hash functions

namespace std {

template<> 
struct hash<sc::SVariable>  
{
    std::size_t operator () (const sc::SVariable& obj) const;
};
    
template<> 
struct hash<sc::STmpVariable>  
{
    std::size_t operator () (const sc::STmpVariable& obj) const;
};

template<> 
struct hash<sc::SValue>  
{
    std::size_t operator () (const sc::SValue& obj) const;
};

}


#endif /* SVALUE_H */

