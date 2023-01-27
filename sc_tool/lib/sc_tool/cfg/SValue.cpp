/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Author: Mikhail Moiseev
 */

#include "sc_tool/cfg/SValue.h"
#include "sc_tool/utils/StringFormat.h"
#include "sc_tool/utils/ScTypeTraits.h"
#include "sc_tool/utils/CppTypeTraits.h"

using namespace sc;
using namespace std;
using namespace clang;
using namespace llvm;

const SValue sc::NO_VALUE = SValue();
const SValue sc::ZW_VALUE = SValue(APSInt(32, true), 100);

// ===========================================================================
// SValue implementation 

void SValue::free_members() {
    if (type == otMemory) {
        SCT_TOOL_ASSERT (memory, "Null memory in free");
        delete memory;
        memory = nullptr;
    } else
    if (type == otIntegerDec || type == otIntegerHex || type == otIntegerBin ||
        type == otIntegerOct || type == otZeroWidth) {
        SCT_TOOL_ASSERT (integer, "Null integer in free");
        delete integer;
        integer = nullptr;
    } else 
    if (type == otChannel) {
        // Do not delete channel
        SCT_TOOL_ASSERT (channel, "Null channel in free");
        channel = nullptr;
    } else {
        // Do not delete anything for unknown type
        SCT_TOOL_ASSERT (integer == nullptr, "Not null unknown pointer");
    }
    type = otUnknown;
}

SValue::SValue() : type(otUnknown), integer(nullptr)
{}

SValue::SValue(const APSInt& val, char radix) : 
    type(radix == 10 ? otIntegerDec : (radix == 16 ? otIntegerHex : 
         (radix == 100 ? otZeroWidth :(radix == 8 ? otIntegerOct : otIntegerBin))))
{
    integer = new APSInt(val, val.isUnsigned());
}

SValue::SValue(ScChannel* val) : 
    type(otChannel) 
{
    channel = val;
}

// Create variable value
SValue::SValue(const clang::ValueDecl* decl, const SValue& parent) : 
    type(otMemory)  
{
    memory = new SVariable(decl, parent); 
}

// Create temporary variable value
SValue::SValue(const clang::QualType& type_, const SValue& parent) : 
    type(otMemory)  
{
    memory = new STmpVariable(type_, parent); 
}

// Create simple object value
SValue::SValue(const clang::QualType& type_, ObjectOwner owner) :
    type(otMemory)
{
    memory = new SObject(type_, owner); 
}

// Create array object value, not array variable
SValue::SValue(const clang::QualType& type_, size_t size, size_t offset) :
    type(otMemory)
{
    memory = new SArray(type_, size, offset); 
}

// Create global record/module object value
SValue::SValue(const clang::QualType& type_, std::vector<SValue> bases, 
               const SValue& parent) :
    type(otMemory)
{
    memory = new SRecord(type_, bases, parent, NO_VALUE, 0); 
}

// Create local record/module object value
// \param var -- variable/member which has this record value
// \param index -- element number in single/multi dimensional array
SValue::SValue(const clang::QualType& type_, std::vector<SValue> bases, 
               const SValue& parent, const SValue& var, size_t index) :
    type(otMemory)
{
    memory = new SRecord(type_, bases, parent, var, index); 
}

// Copy constructor
SValue::SValue(const SValue &rhs) 
{
    if (rhs.isVariable()) {
        SCT_TOOL_ASSERT (rhs.type == otMemory, "Incorrect SValue type");
        memory = new SVariable(rhs.getVariable());
    } else 
    if (rhs.isTmpVariable()) {
        SCT_TOOL_ASSERT (rhs.type == otMemory, "Incorrect SValue type");
        memory = new STmpVariable(rhs.getTmpVariable());
    } else 
    if (rhs.isArray()) {
        SCT_TOOL_ASSERT (rhs.type == otMemory, "Incorrect SValue type");
        memory = new SArray(rhs.getArray());
    } else
    if (rhs.isRecord()) {
        SCT_TOOL_ASSERT (rhs.type == otMemory, "Incorrect SValue type");
        memory = new SRecord(rhs.getRecord());
    } else
    if (rhs.isSimpleObject()) {
        SCT_TOOL_ASSERT (rhs.type == otMemory, "Incorrect SValue type");
        memory = new SObject(rhs.getSimpleObject());
    } else
    if (rhs.isInteger()) {
        SCT_TOOL_ASSERT (rhs.type == otIntegerDec || rhs.type == otIntegerHex || 
                         rhs.type == otIntegerBin || rhs.type == otIntegerOct ||
                         rhs.type == otZeroWidth, "Incorrect SValue type");
        integer = new APSInt(rhs.getInteger());
    } else 
    if (rhs.isScChannel()) {
        // Copy pointer for channel
        SCT_TOOL_ASSERT (rhs.type == otChannel, "Incorrect SValue type");
        channel = rhs.channel; 
    } else {
        // Type unknown
        SCT_TOOL_ASSERT (rhs.type == otUnknown, "Incorrect SValue type");
        integer = nullptr;
    }
    type = rhs.type;
}

// Copy assign
SValue& SValue::operator = (const SValue &rhs) 
{
    // Exclude self-assignment, must be here as @free_members() called below 
    // destroys @rhs if it is the same object
    if (operator == (rhs)) {
        return *this;
    }
    
    free_members();
    if (rhs.isVariable()) {
        SCT_TOOL_ASSERT (rhs.type == otMemory, "Incorrect SValue type");
        memory = new SVariable(rhs.getVariable());
    } else 
    if (rhs.isTmpVariable()) {
        SCT_TOOL_ASSERT (rhs.type == otMemory, "Incorrect SValue type");
        memory = new STmpVariable(rhs.getTmpVariable());
    } else 
    if (rhs.isArray()) {
        SCT_TOOL_ASSERT (rhs.type == otMemory, "Incorrect SValue type");
        memory = new SArray(rhs.getArray());
    } else 
    if (rhs.isRecord()) {
        SCT_TOOL_ASSERT (rhs.type == otMemory, "Incorrect SValue type");
        memory = new SRecord(rhs.getRecord());
    } else 
    if (rhs.isSimpleObject()) {
        SCT_TOOL_ASSERT (rhs.type == otMemory, "Incorrect SValue type");
        memory = new SObject(rhs.getSimpleObject());
    } else 
    if (rhs.isInteger()) {
        SCT_TOOL_ASSERT (rhs.type == otIntegerDec || rhs.type == otIntegerHex || 
                         rhs.type == otIntegerBin || rhs.type == otIntegerOct ||
                         rhs.type == otZeroWidth, 
                         "Incorrect SValue type");
        integer = new APSInt(rhs.getInteger());
    } else 
    if (rhs.isScChannel()) {
        // Copy pointer for channel
        SCT_TOOL_ASSERT (rhs.type == otChannel, "Incorrect SValue type");
        channel = rhs.channel; 
    } else {
        // Type unknown
        SCT_TOOL_ASSERT (rhs.type == otUnknown, "Incorrect SValue type");
        integer = nullptr;
    }
    type = rhs.type;
    return *this;
}

// Clone this value in another parent and owner variable if required
// No clone for integer and channel, return this value instead
SValue SValue::clone(const SValue& parent, const SValue& locvar, size_t index) const 
{
    if (isVariable()) {
        SCT_TOOL_ASSERT (parent, "No parent for variable copy");
        return SValue(getVariable().decl, parent);
    } else 
    if (isTmpVariable()) {
        SCT_TOOL_ASSERT (parent, "No parent for temporary variable copy");
        return SValue(getTmpVariable().type, parent); 
    } else 
    if (isArray()) {
        return SValue(getArray().type, getArray().size, getArray().offset); 
    } else 
    if (isRecord()) {
        // Always create local record, copy of global record is local record
        //cout << "parent && locvar " << parent << " " << locvar << endl;
        SCT_TOOL_ASSERT (parent && locvar, "No parent/variable for record copy");
        return SValue(getRecord().type, getRecord().bases, parent, locvar, index); 
    } else
    if (isSimpleObject()) {
        return SValue(getSimpleObject().type, getSimpleObject().owner);
    } else
    if (isInteger()) {
        return *this; 
    } else
    if (isScChannel()) {
        return *this; 
    } else 
    if (isObject()) {
        SCT_TOOL_ASSERT (false, "Unexpected value type in clone()");
    }
    return NO_VALUE;
}

// Create zero value of the width/signess of @expr type
SValue SValue::zeroValue(const Expr* expr)
{
    clang::QualType type = getTypeForWidth(expr);
    return zeroValue(type, expr);
}

SValue SValue::zeroValue(clang::QualType type, const Expr* expr)
{
    size_t width = 64;
    bool isUnsigned = true;

    if (auto typeInfo = getIntTraits(type, true)) {
        width = typeInfo.getValue().first;
        isUnsigned = typeInfo.getValue().second;
        //cout << "Zero width " << width << " isUnsigned " << isUnsigned << endl;
    } else {
        if (expr) {
            ScDiag::reportScDiag(expr->getBeginLoc(),
                                 ScDiag::SYNTH_UNKNOWN_TYPE_WIDTH) << 
                                 type.getAsString();
        } else {
            ScDiag::reportScDiag(ScDiag::SYNTH_UNKNOWN_TYPE_WIDTH) << 
                                 type.getAsString();
        }
    }

    SValue val;
    if (width != 0) {
        val = SValue(APSInt(width, isUnsigned), 10);
    } else {
        if (expr) {
            ScDiag::reportScDiag(expr->getBeginLoc(), 
                                 ScDiag::SYNTH_ZERO_TYPE_WIDTH) << "";
        } else {
            ScDiag::reportScDiag(ScDiag::SYNTH_ZERO_TYPE_WIDTH) << "";
        }
    }
    
    return val;
}

bool SValue::isUnknown() const {
    return type == otUnknown;
}

bool SValue::isInteger() const {
    return (type == otIntegerDec || type == otIntegerHex || 
            type == otIntegerBin || type == otIntegerOct || type == otZeroWidth);
}

bool SValue::isMemory() const {
    return (type == otMemory);
}

bool SValue::isVariable() const {
    return (type == otMemory && 
            memory->getObjectKind() == IMemory::Kind::otVariable);
}

bool SValue::isTmpVariable() const {
    return (type == otMemory && 
            memory->getObjectKind() == IMemory::Kind::otTmpVariable);
}

bool SValue::isArray() const {
    return (type == otMemory && 
            memory->getObjectKind() == IMemory::Kind::otArray);
}

bool SValue::isRecord() const {
    return (type == otMemory && 
            memory->getObjectKind() == IMemory::Kind::otRecord);
}

bool SValue::isSimpleObject() const {
    return (type == otMemory && 
            memory->getObjectKind() == IMemory::Kind::otObject);
}

bool SValue::isObject() const {
    return (type == otMemory && 
            (memory->getObjectKind() == IMemory::Kind::otObject || 
             memory->getObjectKind() == IMemory::Kind::otArray ||
             memory->getObjectKind() == IMemory::Kind::otRecord));
}

bool SValue::isScChannel() const {
    return type == otChannel;
}

bool SValue::isScInPort() const {
    if (type == otChannel) {
       return (channel->getObjectKind() == ScObject::Kind::scInPort);
    }
    return false;
}

bool SValue::isScOutPort() const {
    if (type == otChannel) {
       return (channel->getObjectKind() == ScObject::Kind::scOutPort);
    }
    return false;
}

bool SValue::isScSignal() const {
    if (type == otChannel) {
       return (channel->getObjectKind() == ScObject::Kind::scSignal);
    }
    return false;
}

bool SValue::isReference() const 
{
    auto valType = getType();
    return sc::isReference(valType);
}

bool SValue::isConstReference() const 
{
    auto valType = getType();
    return sc::isConstReference(valType);
}

bool SValue::isPointer() const
{
    if (auto typePtr = getTypePtr())
        return isPointerOrScPort(QualType(typePtr, 0));
    return false;
}

clang::QualType SValue::getType() const
{
    if (isVariable()) {
        return getVariable().getType();
    }
    if (isTmpVariable()) {
        return getTmpVariable().getType();
    }
    if (isObject()) {
        return getObjectPtr()->getType();
    }
    if (isScChannel()) {
        return getScChannel()->getType();
    }

    return QualType{};
}

const clang::Type* SValue::getTypePtr() const
{
    return isVariable() ? getVariable().getType().getTypePtr()
           : isTmpVariable() ? getTmpVariable().getType().getTypePtr()
           : isObject() ? getObjectPtr()->getType().getTypePtr()
           : nullptr;
}

// Use this reference before @this SValue end of life
APSInt& SValue::getInteger() const {
    SCT_TOOL_ASSERT (type == otIntegerDec || type == otIntegerHex || 
                     type == otIntegerBin || type == otIntegerOct ||
                     type == otZeroWidth, "Not integer value");
    return *integer;
}

// Return integer radix or 0
char SValue::getRadix() const 
{
    if (type == otIntegerDec) {
        return 10;
    } else if (type == otIntegerHex) {
        return 16;
    } else if (type == otZeroWidth) {
        return 100; 
    } else if (type == otIntegerBin) {
        return 2;
    } else if (type == otIntegerOct) {
        return 8;
    }
    SCT_TOOL_ASSERT(false, "Incorrect radix");
    return 0;
}

char SValue::isScZeroWidth() const 
{
    return type == otZeroWidth;
}

// Set given radix for integer, ignored if @this is not integer
void SValue::setRadix(char radix) 
{
    if (!isInteger()) return;
    
    switch (radix) {
        case 2  : type = otIntegerBin; break;
        case 8  : type = otIntegerOct; break;
        case 10 : type = otIntegerDec; break;
        case 16 : type = otIntegerHex; break;
        case 100 : type = otZeroWidth; break;
        default : SCT_TOOL_ASSERT(false, "Incorrect radix");
    }
}


// Use this reference before @this SValue end of life
IMemory* SValue::getMemory() const {
    SCT_TOOL_ASSERT (type == otMemory, "SValue::getMemory() incorrect type");
    return memory;
}

// Use this reference before @this SValue end of life
SVariable& SValue::getVariable() const {
    SCT_TOOL_ASSERT (isVariable(), "SValue::getVariable() incorrect type");
    return *(static_cast<SVariable*>(memory));
}

STmpVariable& SValue::getTmpVariable() const {
    SCT_TOOL_ASSERT (isTmpVariable(), "SValue::getTmpVariable() incorrect type");
    return *(static_cast<STmpVariable*>(memory));
}

SObject* SValue::getObjectPtr() const {
    SCT_TOOL_ASSERT (isObject(), "SValue::getObjectPtr() incorrect type");
    return static_cast<SObject*>(memory);
}

// Use this reference before @this SValue end of life
SObject& SValue::getSimpleObject() const {
    SCT_TOOL_ASSERT (isSimpleObject(), "SValue::getSimpleObject() incorrect type");
    return *(static_cast<SObject*>(memory));
}

// Use this reference before @this SValue end of life
SArray& SValue::getArray() const {
    SCT_TOOL_ASSERT (isArray(), "SValue::getArray() incorrect type");
    return *(static_cast<SArray*>(memory));
}

// Use this reference before @this SValue end of life
SRecord& SValue::getRecord() const {
    SCT_TOOL_ASSERT (isRecord(), "SValue::getRecord() incorrect type");
    return *(static_cast<SRecord*>(memory));
}

// Return pointer, it is never deleted
ScChannel* SValue::getScChannel() const {
    SCT_TOOL_ASSERT (isScChannel(), "SValue::getScChannel() incorrect type");
    return static_cast<ScChannel*>(channel);
}
// Return pointer, it is never deleted
ScInPort* SValue::getScInPort() const {
    SCT_TOOL_ASSERT (isScInPort(), "SValue::getScInPort() incorrect type");
    return static_cast<ScInPort*>(channel);
}
// Return pointer, it is never deleted
ScOutPort* SValue::getScOutPort() const {
    SCT_TOOL_ASSERT (isScOutPort(), "SValue::getScOutPort() incorrect type");
    return static_cast<ScOutPort*>(channel);
}

bool SValue::operator == (const SValue& rhs) const {
    // Use @exactEqual to check bit width and signness of integers
    // Do not check radix for integers
    if (isInteger() && rhs.isInteger()) {
        return exactEqual(*integer, *rhs.integer);
    }
    
    return (type == rhs.type && (type == otUnknown || 
            (type == otChannel && channel == rhs.channel) ||
            (isVariable() && rhs.isVariable() && 
             getVariable() == rhs.getVariable()) ||
            (isTmpVariable() && rhs.isTmpVariable() && 
             getTmpVariable() == rhs.getTmpVariable()) ||
            (isArray() && rhs.isArray() && 
             getArray() == rhs.getArray()) || 
            (isRecord() && rhs.isRecord() && 
             getRecord() == rhs.getRecord()) ||
            (isSimpleObject() && rhs.isSimpleObject() && 
             getSimpleObject() == rhs.getSimpleObject()) ));
}

bool SValue::operator != (const SValue& rhs) const {
    return (!this->operator ==(rhs));
}

// Get value as string
string SValue::asString(bool debug) const {
    if (isUnknown()) {
        return "Unknown";
    } else 
    if (isInteger()) {
        char radix = getRadix();
        if (radix == 10)
            return sc::APSintToString(getInteger(), 10);
        if (radix == 100) 
            return debug ? "zw0" : sc::APSintToString(getInteger(), 10);
        
        char prefix = radix == 16 ? 'x' : radix == 8 ? 'o' : 'b';
        return (prefix + sc::APSintToString(getInteger(), radix));
    } else 
    if (isScChannel()) {
        return channel->asString(debug);
    } else {
        return memory->asString(debug);
    }
}

bool SValue::getBoolValue() const 
{
    SCT_TOOL_ASSERT (type != otUnknown, "No value unexpected");
    
    // Pointer to NULL represented as variable with integer value 0
    if (isInteger()) {
        return getInteger().getBoolValue();
    }
    // Any non-integer object cannot be NULL
    return true;
}

bool SValue::isInClassHierarhy(const SValue& mval) 
{
    SCT_TOOL_ASSERT (isVariable() || isTmpVariable(), "Not variable value");
    SValue pval = (isVariable()) ? getVariable().getParent() : 
                                   getTmpVariable().getParent();
    if (pval == mval) {
        return true;
    }
    for (const auto& b : mval.getRecord().bases) {
        if (isInClassHierarhy(b)) {
            return true;
        }
    }
    return false;
}

// ===========================================================================

// -------------- SObject -------------- 
uint64_t SObject::id_gen = 0;

IMemory::Kind SObject::getObjectKind() const {
    return IMemory::Kind::otObject;
}

clang::QualType SObject::getType() const {
    return type;
}

void SObject::setType(clang::QualType type_) {
    type = type_;
}

string SObject::asString(bool debug) const {
    if (debug) {
        return ("obj_" + to_string(id)+(isOwner()?"(OWNER)":""));
    } else {
        SCT_TOOL_ASSERT (false, "SObject::asString no debug");
        return "";
    }    
}

// -------------- SArray -------------- 
IMemory::Kind SArray::getObjectKind() const {
    return IMemory::Kind::otArray;
}

string SArray::asString(bool debug) const {
    if (debug) {
        return ("ARR_" + to_string(getId()) + "[" +
                (unknown ? "UNKW" : to_string(offset)) + "]("+
                to_string(size) + ")");
    } else {
        SCT_TOOL_ASSERT (false, "SArray::asString no debug");
        return "";
    }    
}

// -------------- SRecord -------------- 
IMemory::Kind SRecord::getObjectKind() const {
    return IMemory::Kind::otRecord;
}

string SRecord::asString(bool debug) const {
    if (debug) {
        std::string pref = var ? (var.isTmpVariable() ? "TREC_" : "LREC_") : 
                                 "REC_";
        return (pref + to_string(getId())+ "("+ getBaseString(*this) +")"); 
    } else {
        SCT_TOOL_ASSERT (false, "SRecord::asString no debug");
        return "";
    }    
}

// -------------- SVariable -------------- 
IMemory::Kind SVariable::getObjectKind() const {
    return IMemory::Kind::otVariable;
}

clang::QualType SVariable::getType() const {
    return decl->getType();
}

string SVariable::asString(bool debug) const {
    if (debug) {
        //std::stringstream stream;
        //stream << std::hex << decl;
        return (parent.asString() + "." + decl->getDeclName().getAsString());
               //+"( "+stream.str()+" )");
    } else {
        return decl->getDeclName().getAsString();
    }
}

// -------------- STmpVariable -------------- 
uint64_t STmpVariable::id_gen = 0;

IMemory::Kind STmpVariable::getObjectKind() const {
    return IMemory::Kind::otTmpVariable;
}

clang::QualType STmpVariable::getType() const {
    return type;
}

string STmpVariable::asString(bool debug) const {
    if (debug) {
        return (parent.asString() + ".TMP_" + to_string(id));
    } else {
        return ("TMP_" + to_string(id));
    }
}

// ----------------------------------------------------------------------------
// Hash functions
namespace std {
    
std::size_t hash<sc::SVariable>::operator () (const sc::SVariable& obj) const 
{
    using std::hash;
    return (std::hash<const void*>()(obj.getDecl()) ^ 
            std::hash<sc::SValue>()(obj.getParent()));
}    

std::size_t hash<sc::STmpVariable>::operator () (const sc::STmpVariable& obj) const 
{
    using std::hash;
    return (std::hash<uint64_t>()(obj.getId()) ^ 
            std::hash<sc::SValue>()(obj.getParent()));
}    

std::size_t hash<sc::SValue>::operator () (const sc::SValue& obj) const {
    using std::hash;
    if (obj.isVariable()) {
        return std::hash<sc::SVariable>()(obj.getVariable());
    } else
    if (obj.isTmpVariable()) {
        return std::hash<sc::STmpVariable>()(obj.getTmpVariable());
    } else
    if (obj.isRecord()) {
        // For local record use variable 
        const auto& rec = obj.getRecord();
        if (rec.var) {
            if (rec.var.isVariable()) {
                return (std::hash<sc::SVariable>()(rec.var.getVariable()) ^ 
                        std::hash<size_t>()(rec.index));
            } else 
            if (rec.var.isTmpVariable())  {
                return (std::hash<sc::STmpVariable>()(rec.var.getTmpVariable()) ^ 
                        std::hash<size_t>()(rec.index));
            }
        }
        return std::hash<uint64_t>()(obj.getObjectPtr()->getId());
    } else
    if (obj.isObject()) {
        return std::hash<uint64_t>()(obj.getObjectPtr()->getId());
    } else
    if (obj.isInteger()) {
        return std::hash<uint64_t>()(obj.getInteger().getExtValue());
    } else 
    if (obj.isScChannel()) {
        return std::hash<uint64_t>()((uint64_t)obj.getScChannel());
    } else {
        return std::hash<uint64_t>()(0x0D10F437);
    }
}

}
