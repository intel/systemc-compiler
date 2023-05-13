/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Author: Mikhail Moiseev
 */

#include "sc_tool/cfg/ScState.h"

#include "sc_tool/elab/ScVerilogModule.h"
#include "sc_tool/utils/ScTypeTraits.h"
#include "sc_tool/utils/CppTypeTraits.h"
#include "sc_tool/utils/CheckCppInheritance.h"
#include "sc_tool/utils/DebugOptions.h"
#include "sc_tool/diag/ScToolDiagnostic.h"
#include "sc_tool/ScCommandLine.h"
#include <vector>
#include <sstream>
#include <iostream>

// ===========================================================================
// ScState implementation 

using namespace sc;
using namespace std;
using namespace clang;
using namespace llvm;

// ===========================================================================
// Value hierarchy parsing down to up

void ScState::parseParentForVar(SValue val, unsigned crossModule,
                                std::vector<SValue>& valStack) const
{
    SCT_TOOL_ASSERT (val.isVariable(), "Not variable value");
    valStack.push_back(val);
    
    // Traverse up through parent which are record variables up to array
    SValue pval = val.getVariable().getParent();
    
    parseValueHierarchy(pval, crossModule, valStack);
}

void ScState::parseParentForObj(SValue val, unsigned crossModule,
                                vector<SValue>& valStack) const
{
    SCT_TOOL_ASSERT(val.isObject()||val.isScChannel(), "Incorrect object type");
    valStack.push_back(val);
    
    // Continue traverse up with zero element
    if (val.isArray()) {
        val.getArray().setOffset(0);
    }
    
    // Get variable which contains this object
    bool found = false;
    for (const auto& i : tuples) {
        if (i.second == val && isObjectOwner(i.first)) {
            val = i.first; found = true; break;
        }
    }
    
    if (found) {
        parseValueHierarchy(val, crossModule, valStack);
    }
}

void ScState::parseParentForRec(SValue val, unsigned crossModule,
                                vector<SValue>& valStack) const
{
    SCT_TOOL_ASSERT(val.isRecord(), "No record value in parseParentForRec()");
    
    // Limit module/MIF cross number to cope with cycle dependencies
    if (val.isRecord() && isScModuleOrInterface(val.getType(), true)) {
        if (crossModule == 0) return;
        crossModule -= 1;
    }
    
    // Go up from base class to inheritor which can be stored in array
    SValue dval;
    getMostDerivedClass(val, dval);

    parseParentForObj(dval, crossModule, valStack);
}

// ===========================================================================
// Public methods

// Parse value hierarchy to topmost module storing all intermediate values 
void ScState::parseValueHierarchy(SValue val, unsigned crossModule,
                                  vector<SValue>& valStack) const
{
    if (val.isVariable()) {
        parseParentForVar(val, crossModule, valStack);

    } else 
    if (val.isRecord()) {
        parseParentForRec(val, crossModule, valStack);

    } else 
    if (val.isObject() || val.isScChannel()) {
        parseParentForObj(val, crossModule, valStack);
    }
}

// Check if the given element is member of the given record @recval
// It can be not direct member, i.e.member of its member
bool ScState::checkRecord(const SValue& val, 
                          const SValue& recval,
                          unsigned crossModule) const
{
    if (!recval.isRecord()) return false;
    
    vector<SValue> valStack;
    parseValueHierarchy(val, crossModule, valStack);
    
    // Compare most derived classes
    SValue drecval;
    getMostDerivedClass(recval, drecval);
    
    // Return first (most bottom) array or no value
    for (auto mval : valStack) {
        if (mval == drecval) {
            return true;
        }
    }
    return false;
}


// ===========================================================================
// Public methods

void ScState::fillDerived(const SValue& recval)
{
    for (auto& b : recval.getRecord().bases) {
        auto j = staticState->derived.find(b);
        if (j == staticState->derived.end()) {
            // Put derived class for new base class
            staticState->derived.emplace(b, recval);
        } else {
            // Do nothing, that is possible in multiple inheritance,
            // not important what derived class is stored
        }
        // Recursively pass deep into base classes
        fillDerived(b);
    }
}

void ScState::fillDerivedClasses(const SValue &dynmodval) 
{
    for (const auto& i : tuples) {
        // Check all records which can be only in right part of tuple
        if (i.second.isRecord()) {
            fillDerived( i.second );
        }
    }
}

// Not used for now
// This function does not support sc_vector
/*void ScState::updateStaticClasses()
{
    for (const auto& i : tuples) {
        // Try to get record value 
        SValue tval; SValue recval = i.first;
        do {
            tval = recval;
            getValue(tval, recval, false);
        } while (!recval.isRecord() && !recval.isUnknown());
        
        if ( recval.isRecord() ) {
            // Get variable/pointer type
            QualType type = (i.first.isVariable() || i.first.isTmpVariable()) ?
                             i.first.getVariable().getType() : 
                             i.first.getObjectPtr()->getType();
            // Skip @sc_port<IF> cannot points to static
            if (isScPort(type)) {
                continue;
            }
            // Get class type from variable type (static class)
            while (!isUserDefinedClass(type)) {
                // TODO: add vector support here 
                if (isPointer(type) || type->isArrayType()) {
                    // Qualifiers not important here
                    type = QualType(type->getPointeeOrArrayElementType(), 0);
                }
            }
            //cout << "final type " << type.getAsString() << endl;
            
            if (isUserDefinedClass(type)) {
                // Get record value for static class type
                SValue sval = getBaseClass(recval, type);
                
                if (sval.isRecord()) {
                    // Replace record value with static class value
                    // No references possible here
                    putValue(tval, sval, false);
                    
                } else {
                    // Do nothing, it can be dynamic object with record value
                }
            } else {
                SCT_TOOL_ASSERT(false, "No class type for variable/pointer to class");
            }
        }
    }
}*/

bool ScState::isDead() {
    return dead;
}

size_t ScState::size() const {
    return tuples.size();
}

ScState* ScState::clone() const {
    return (new ScState(*this));
}

void ScState::join(ScState* other) 
{
    // Do nothing for @other dead state
    if (other->isDead()) return;

    // If this is dead state and @other is not dead
    if (dead) {
        tuples.swap(other->tuples);
        levels.swap(other->levels);
        maxLevel = other->maxLevel;
        staticState.swap(other->staticState);
        defined.swap(other->defined);
        declared.swap(other->declared);
        readndef.swap(other->readndef);
        readninit.swap(other->readninit);
        readsva.swap(other->readsva);
        read.swap(other->read);
        defallpath.swap(other->defallpath);
        defsomepath.swap(other->defsomepath);
        arraydefined.swap(other->arraydefined);
        loopCntrVars.swap(other->loopCntrVars);
        dead = false;
        return;
    }

    // Both states are not dead
    // Levels and maxLevel not changed here, extra values will be removed later
    auto ti = tuples.begin();
    while (ti != tuples.end()) {
        auto j = other->tuples.find(ti->first);

        if (j == other->tuples.end()) {
            // No tuple for this object found in @other, remove this tuple
            ti = tuples.erase(ti);

        } else {
            // There are tuples for this object in @this and @other states
            if (ti->second == j->second) {
                // Checking levels are the same not required
                ++ti;
            } else {
                // Different values, remove this tuple
                //cout << "Diff " << ti->first << " : " << ti->second << " " << j->second << endl;
                ti = tuples.erase(ti);
            }
        }
    }

    // Remain @defined value if it exists in both states
    auto di = defined.begin();
    while (di != defined.end()) {
        if (other->defined.count(*di) == 0) {
            di = defined.erase(di);
        } else {
            ++di;
        }
    }
    
    // The same for declared as it used for @readndef
    auto de = declared.begin();
    while (de != declared.end()) {
        if (other->declared.count(*de) == 0) {
            de = declared.erase(de);
        } else {
            ++de;
        }
    }
    
    // Remain @defallpath value if it exists in both states
    auto da = defallpath.begin();
    while (da != defallpath.end()) {
        if (other->defallpath.count(*da) == 0) {
            defsomepath.insert(*da);
            da = defallpath.erase(da);
        } else {
            ++da;
        }
    }
    if (defallpath.size() != other->defallpath.size()) {
        for (auto& od : other->defallpath) {
            if (defallpath.count(od) == 0) {
                defsomepath.insert(od);
            }
        }
    }
    // Get values from other state to keep them
    defsomepath.insert(cbegin(other->defsomepath), cend(other->defsomepath));
    
    // Remain value if it exists in any state
    readndef.insert(cbegin(other->readndef), cend(other->readndef));
    readninit.insert(cbegin(other->readninit), cend(other->readninit));
    readsva.insert(cbegin(other->readsva), cend(other->readsva));
    read.insert(cbegin(other->read), cend(other->read));
    arraydefined.insert(cbegin(other->arraydefined), cend(other->arraydefined));
    loopCntrVars.insert(cbegin(other->loopCntrVars), cend(other->loopCntrVars));
}

// Recursively set single/multidimensional array elements to NO_VALUE 
void ScState::setArrayNoValue(const SValue& val) 
{
    //cout << "----- setArrayNoValue erase tuples for val " << val << endl;
    //if (val.getTypePtr()) val.getType()->dump();
    
    // Clear @unknown flag 
    SValue aval = val;
    for (size_t i = 0; i < aval.getArray().getSize(); i++) {
        aval.getArray().setOffset(i);
        removeIntSubValues(aval);
    }
}

// Set record array elements accessed at unknown index to NO_VALUE, 
// For array and pointers it gets variable, but set arrays elements/pointee
void ScState::setArrayRecordNoValue(const SValue& val) 
{
    //cout << "------ getAllRecordArrayElements erase tuples for val " << val << " :" << endl;
    
    // Get record from pointer
    SValue tval = val;
    while (tval.isSimpleObject()) {
        SValue ttval = getValue(tval);
        tval = ttval;
    }

    auto recElems = getAllRecordArrayElementsForAny(tval);
    
    // Remove elements from state
    for (const auto& i : recElems) {
        removeIntSubValues(i);
    }
}

void ScState::putValue(const SValue& lval, const SValue& rval, bool deReference,
                       bool checkUnknwIndx)
{
    // It is prohibited to put value into dead state
    SCT_TOOL_ASSERT (!dead, "Put value to dead state");
    //cout << "putValue lval " << lval << endl;
    
    if (lval.isVariable() || lval.isTmpVariable() || lval.isObject()) {
        // Get de-referenced variable for @lval
        SValue llval = lval;
        if (deReference) {
            getDerefVariable(lval, llval);
        }

        if (checkUnknwIndx) {
            // For normal array fill all elements and sub-arrays with NO_VALUE
            bool unkwIndex;
            bool isArr = isArray(llval, unkwIndex);
            if (isArr && unkwIndex) {
                setArrayNoValue(llval);
                return;
            }

            // For record array fill all elements and sub-arrays with NO_VALUE
            // @llval is field
            bool isRecArr = getBottomArrayForAny(llval, unkwIndex);
            
            if (isRecArr && unkwIndex) {
                setArrayRecordNoValue(llval);
                return;
            }
        }

        SValue rrval = rval;
        
        if (rval.isVariable() || rval.isTmpVariable() || rval.isObject()) {
            // For reference RValue type do automatic de-reference
            if (deReference) {
                getDerefVariable(rval, rrval);
            }
        } else 
        if (rval.isInteger()) {
            QualType type = lval.getType();

            if (!isPointerOrScPort(type)) {
                // Cast @rval integer value to @lval variable width
                if (type->isReferenceType()) {
                    type = type.getNonReferenceType();
                }

                if (isa<AutoType>(type)) {
                    rrval = rval;
                    
                } else 
                if (isBoolType(type)) {
                    // Boolean type is special case 
                    rrval = SValue(SValue::boolToAPSInt(rval.getBoolValue()), 10);
                    
                } else 
                if (auto pair = getIntTraits(type, true)) {
                    size_t width = pair.getValue().first;
                    bool isUnsigned = pair.getValue().second;

                    rrval = SValue(extrOrTrunc(rval.getInteger(), width, 
                                   isUnsigned), rval.getRadix());
                    //cout << "   rrval " << rrval << " width " << width << endl;
                } else {
                    type->dump();
                    SCT_TOOL_ASSERT(false, "Cannot determine integral type width");
                }
            }
        }
        
        if (rrval.isUnknown()) {
            // Do not push NO_VALUE tuple to state, NO_VALUE is anyway returned 
            // in getValue() for LValue without tuple in state
            tuples.erase(llval);
            
            if (DebugOptions::isEnabled(DebugComponent::doState)) {
                cout << "putValue (" << lval << ", NO_VALUE)" << endl;
            }
            
        } else {
            // Try to put new value, including reference initialization
            auto pair = tuples.emplace(lval, rrval);

            if (!pair.second) {
                if (deReference && lval.isReference()) {
                    // For reference LValue type do automatic de-reference
                    // No constant reference considered as it cannot be reassigned
                    llval = pair.first->second;
                    pair = tuples.emplace(llval, rrval);

                    if (!pair.second) {
                        // Replace value for de-referenced variable
                        pair.first->second = rrval;
                    }
                } else {
                    // Replace value 
                    pair.first->second = rrval;
                }
            }
            if (DebugOptions::isEnabled(DebugComponent::doState)) {
                cout << "putValue (" << lval << ", " << rrval << ")" << endl;
        } 
        } 
    } else {
        if (DebugOptions::isEnabled(DebugComponent::doState)) {
            cout << "putValue (" << lval << ", " << rval << ")" << endl;
    }
}
}

// Create deep copy in state of given value, returns clone of the value and 
// put clone of all the subvalue clones into state
// \param level -- value level used to remove tuples above the level
// \param parent -- parent for cloned value
// \param locvar -- owner variable for cloned value if it is local record
SValue ScState::copyIntSubValues(const SValue& val, unsigned level,
                                 const SValue& parent, const SValue& locvar, 
                                 size_t index) 
{
    //cout << "copyIntSubValues " << val << endl;
    // No copy for integer value
    if (val.isUnknown() || val.isInteger()) {
        return val;
    }

    // Clone value object with new ID, i.e. cval != val
    SValue cval(val.clone(parent, locvar, index));
    
    if (val.isArray()) {
        SValue aval = val;
        size_t arrSize = val.getArray().getSize();
        
        for (size_t i = 0; i < arrSize; ++i) {
            aval.getArray().setOffset(i);
            cval.getArray().setOffset(i);
            // Recursively copy value of the element 
            if (SValue rval = copyIntSubValues(getValue(aval), level, 
                                               parent, locvar, i)) 
            {
                tuples.emplace(cval, rval);
                setValueLevel(cval, level);
            }
        }
        if (val.getArray().isUnknown()) {
            cval.getArray().setUnknownOffset();
        } else {
            cval.getArray().setOffset(val.getArray().getOffset());
        }
        
    } else 
    if (val.isRecord()) {
        auto recDecl = val.getType()->getAsCXXRecordDecl();
    
        for (auto fieldDecl : recDecl->fields()) {
            SValue fval(fieldDecl, val);
            SValue cfval(fieldDecl, cval);
            // Recursively copy value of the field
            if (SValue rval = copyIntSubValues(getValue(fval), level, 
                                               cval, cfval)) 
            {
                tuples.emplace(cfval, rval);
                setValueLevel(cfval, level);
            }
        }
        
    } else 
    if (val.isVariable() || val.isTmpVariable()) {
        if (SValue rval = copyIntSubValues(getValue(val), level, 
                                           parent, cval)) 
        {
            tuples.emplace(cval, rval);
            setValueLevel(cval, level);
        }
    } else 
    if (val.isSimpleObject()) {
        if (SValue rval = copyIntSubValues(getValue(val), level, 
                                           parent, locvar, index)) 
        {
            tuples.emplace(cval, rval);
            setValueLevel(cval, level);
        }
    }
    
    return cval;
}


// Remove tuples with integer values starting with @val in left part and 
// recursively all tuples with its right parts, no array/record removed 
// Use to clean up state by write to array element at unknown index
// \param lval -- variable value
void ScState::removeIntSubValues(const SValue& lval, bool doGetValue) 
{
    //cout << "removeIntSubValues " << lval << endl;
    SValue rval = lval;
    if (doGetValue) {
        getValue(lval, rval, true, ArrayUnkwnMode::amFirstElement);
    }
    
    if (rval.isInteger()) {
        //cout << "   >>> " << lval << endl;
        tuples.erase(lval);
        
    } else 
    if (rval.isArray()) {
        SValue aval = rval;
        size_t arrSize = aval.getArray().getSize();
        
        for (size_t i = 0; i < arrSize; ++i) {
            aval.getArray().setOffset(i);
            removeIntSubValues(aval);
        }
    } else 
    if (rval.isRecord()) {
        auto fields = getRecordFields(rval);
        for (const SValue& fval : fields) {
            removeIntSubValues(fval);
        }
        
    } else 
    if (rval.isSimpleObject() || rval.isVariable() || rval.isTmpVariable()) {
        removeIntSubValues(rval);
    }
}

// Remove tuples with @val in left part and recursively all tuples with its 
// right parts, used to remove old record/array values at variable 
// re-declaration which required to get variable for value
void ScState::removeSubValues(const SValue& val) 
{
    //cout << "removeSubValues " << val << endl;
    if (val.isArray()) {
        SValue aval = val;
        size_t arrSize = aval.getArray().getSize();
        
        for (size_t i = 0; i < arrSize; ++i) {
            aval.getArray().setOffset(i);
            
            auto j = tuples.find(aval);
            if (j != tuples.end()) {
                removeSubValues(j->second);
            }
        }
        
        tuples.erase(val);
        
    } else 
    if (val.isRecord()) {
        auto fields = getRecordFields(val);
        for (const SValue& fval : fields) {
            removeSubValues(fval);
        }
        
        tuples.erase(val);
        
    } else
    if (val.isSimpleObject()) {
        auto j = tuples.find(val);
        if (j != tuples.end()) {
            removeSubValues(j->second);
        }
        
        tuples.erase(val);
    }
}

SValue ScState::createArrayInState(clang::QualType arrayType, unsigned level)
{
    if (arrayType->isArrayType()) {
        SCT_TOOL_ASSERT (arrayType->isConstantArrayType(),
                         "createArrayInState : Non-constant array type ");

        // Create stack array, VarDecl parsed in SValue constructor
        QualType elmType = dyn_cast<clang::ArrayType>(arrayType)->getElementType();
        size_t size = getArraySize(arrayType);

        // Create array object
        SValue arrayRootVal = SValue(elmType, size, 0);

        // Fill array with elements
        for (unsigned i = 0; i < size; i++) {
            // Recursive call to create sub-arrays
            SValue eval = createArrayInState(elmType, level);
            // Put @NO_VALUE or sub_array value into @aval
            arrayRootVal.getArray().setOffset(i);
            putValue(arrayRootVal, eval, false); //@val -> @eval
            setValueLevel(arrayRootVal, level);
        }
        if (arrayRootVal.getArray().getSize() > 0) {
            arrayRootVal.getArray().setOffset(0);
        }
        return arrayRootVal;
        
    } else {
        return NO_VALUE;
    }
}

SValue ScState::createStdArrayInState(clang::QualType arrayType, unsigned level)
{
    if (isStdArray(arrayType)) {
        auto elmType = getTemplateArgAsType(arrayType, 0);
        SCT_TOOL_ASSERT (elmType, "createStdArrayInState : No type inferred");
        auto sizeArg = sc::getTemplateArg(arrayType, 1);
        SCT_TOOL_ASSERT (sizeArg, "createStdArrayInState : No size inferred");
        size_t size = sizeArg->getAsIntegral().getZExtValue();
        
        // Create array object
        SValue arrayRootVal = SValue(*elmType, size, 0);
        
        // Fill array with elements
        for (unsigned i = 0; i < size; i++) {
            // Recursive call to create sub-arrays
            SValue eval = createStdArrayInState(*elmType, level);
            // Put sub_array value into @aval
            arrayRootVal.getArray().setOffset(i);
            putValue(arrayRootVal, eval, false); //@val -> @eval
            setValueLevel(arrayRootVal, level);
        }
        if (arrayRootVal.getArray().getSize() > 0) {
            arrayRootVal.getArray().setOffset(0);
        }
        return arrayRootVal;
        
    } else {
        return NO_VALUE;
    }
}

// \return <value found, array with unknown index returned>
std::pair<bool, bool> 
ScState::getValue(const SValue& lval, SValue& rval, bool deReference,
                  ArrayUnkwnMode returnUnknown) const
{
    if (lval.isVariable() || lval.isTmpVariable() || lval.isObject()) {
        // Get de-referenced variable for @lval
        SValue llval = lval;
        if (deReference) {
            getDerefVariable(lval, llval);
        }

        bool unkwIndex;
        bool isArr = isArray(llval, unkwIndex);
        isArr = isArr || getBottomArrayForAny(llval, unkwIndex);
        
        // For array access at unknown index any element could be accessed
        if (isArr && unkwIndex) {
            bool unkwIndexRec = false;
            if (returnUnknown == ArrayUnkwnMode::amFirstElementRec) {
                getBottomArrayForAny(llval, unkwIndexRec, 0, true);
            }

            if (unkwIndexRec) {
                llval = getFirstArrayElementForAny(llval);
                
            } else {
                switch (returnUnknown) {
                    case ArrayUnkwnMode::amNoValue:
                    case ArrayUnkwnMode::amFirstElementRec:
                        rval = NO_VALUE;
                        return std::pair<bool, bool>(false, false);

                    case ArrayUnkwnMode::amArrayUnknown:
                        rval = llval;
                        return std::pair<bool, bool>(true, true);

                    case ArrayUnkwnMode::amFirstElement:
                        llval = getFirstArrayElementForAny(llval);
                        break;
                }
            }
        }
        
        auto i = tuples.find(llval);

        if (i != tuples.end()) {
            rval = i->second;
            if (DebugOptions::isEnabled(DebugComponent::doState)) {
                cout << "getValue (" << llval.asString() << ", " << rval.asString() << ")" << endl;
            }
            return std::pair<bool, bool>(true, false);
        }
    }
    
    if (DebugOptions::isEnabled(DebugComponent::doState)) {
        cout << "getValue NO_VALUE for " << lval.asString() << endl;
    }
    
    rval = NO_VALUE;
    return std::pair<bool, bool>(false, false);
}

// TODO: add bool deReference
SValue ScState::getValue(const SValue& lval) const
{
    SValue res;
    getValue(lval, res, false);
    return res;
}

void ScState::removeValue(const SValue& lval)
{
    tuples.erase(lval);
}

// Do @lval dereference if required to get referenced variable
void ScState::getDerefVariable(const SValue& lval, SValue& rval, 
                               bool keepConstRef) const
{
    if (lval.isConstReference()) {
        // Constant reference can refer to variable/object or contain RValue
        auto i = tuples.find(lval);
        if (i != tuples.end()) {
            // If RValue is constant/constant array element (even at unknown index)
            // then keep reference, required for UseDef
            if (i->second.isInteger() || i->second.isUnknown() || 
                (keepConstRef && ScState::isConstVarOrLocRec(i->second))) {
                rval = lval;
            } else {
                // Do de-reference
                rval = i->second;
            }
        } else {
            rval = lval;
        }
        
    } else 
    if (lval.isReference()) {
        // Non-constant reference must refer to valid variable/object
        auto i = tuples.find(lval);
        SCT_TOOL_ASSERT (i != tuples.end(), "No variable/value for reference");
        SCT_TOOL_ASSERT (!i->second.isInteger() && !i->second.isUnknown(), 
                         "Incorrect reference initializer");
        // Do de-reference
        rval = i->second;
        
    } else {
        // No de-reference required
        rval = lval;
    }
}

// Get derived class for given base class
bool ScState::getDerivedClass(const SValue& bval, SValue& dval) const
{
    auto i = staticState->derived.find(bval);
    if (i != staticState->derived.end()) {
        dval = i->second;
        return true;
    }
    dval = bval;
    return false;
}

// Get most derived class (dynamic class) for given base class
bool ScState::getMostDerivedClass(const SValue& bval, SValue& dval) const
{
    SValue val = bval; bool found = false;
    
    while ( getDerivedClass(val, dval) ) {
        val = dval;
        found = true;
    }
    return found;
}

void ScState::print() const {
    cout << "---------------- STATE ----------------" << endl;
    if (dead) {
        cout << "DEAD STATE" << endl;
        return;
    }
    cout << "----- tuples :" << endl;
    for (const auto& i : tuples) {
        cout << "(" << i.first 
             //<< " #" << (i.first.isVariable() ? i.first.getVariable().getDecl() : 0)
             //<< (levels.count(i.first) ? "_L"+to_string(levels.at(i.first)) : "")
             << ", " << i.second << ")"<< endl;
    }
    /*cout << "----- defined :" << endl;
    for (const auto& i : defined) {
        cout << i.asString() << ", "<< endl;
    }
    cout << "----- defined all paths :" << endl;
    for (const auto& i : defallpath) {
        cout << i.asString() << ", "<< endl;
    }
    cout << "----- declared :" << endl;
    for (const auto& i : declared) {
        cout << i.asString() << ", "<< endl;
    }
    cout << "----- read not defined :" << endl;
    for (const auto& i : readndef) {
        cout << i.asString() << ", "<< endl;
    }
//    cout << "----- read not defined in SVA :" << endl;
//    for (const auto& i : readsva) {
//        cout << i.asString() << ", "<< endl;
//    }
    cout << "----- read :" << endl;
    for (const auto& i : read) {
        cout << i.asString() << ", "<< endl;
    }
//    cout << "----- latches :" << endl;
//    for (const auto& i : defsomepath) {
//        cout << i.asString() << ", "<< endl;
//    }
    cout << "----- array some elements defined :" << endl;
    for (const auto& i : arraydefined) {
        cout << i.asString() << ", "<< endl;
    }*/
//    cout << "----- derived :" << endl;
//    for (auto& i : staticState->derived) {
//        cout << "(" << i.first.asString() << ", " << i.second.asString() << ")"<< endl;
//    }
//    cout << "----- VerilogVarTraits:" << endl;
//    for (auto& i : staticState->varTraits) {
//        cout << "    " << i.first << " : " << i.second << endl;
//    }
//    cout << "----- SVal2Obj:" << endl;
//    for (auto& i : staticState->sVal2ElabMap) {
//        cout << "    " << i.first << " : " << i.second.getID() << endl;
//    }
//    cout << "----- Obj2SVal:" << endl;
//    for (auto& i : staticState->elab2SValMap) {
//        cout << "    " << i.first.getID() << " : " << i.second << endl;
//    }
    cout << endl;
}

void ScState::printSize() const {
    cout << "STATE SIZE : " << dec << "tuples: " <<
            std::count_if(cbegin(tuples), cend(tuples),
                [](const auto& p){return p.second.isUnknown();}) 
            << " / " << tuples.size() << "\n"
            << "    levels: " << levels.size() << "\n"
            << "    defined: " << defined.size() << "\n"
            << "    declared: " << declared.size() << "\n"
            << "    readndef: " << readndef.size() << "\n"
            << "    readninit: " << readninit.size() << "\n"
            << "    readsva: " << readsva.size() << "\n"
            << "    read: " << read.size() << "\n"
            << "    defallpath: " << defallpath.size() << "\n"
            << "    defsomepath: " << defsomepath.size() << endl;
}

void ScState::putElabObject(const SValue& sval, sc_elab::ObjectView objView,
                            const sc_elab::VerilogVar* chanRecFieldVar) 
{
    using std::cout; using std::endl;
    //cout<< "putElabObject objView " << objView.getDebugString() << endl;
    // Only first @objView for the value is stored, next are ignored
    staticState->sVal2ElabMap.emplace(sval, objView);
    
    if (auto elabObj = objView.getVerilogNameOwner()) {
        // Get Verilog variables created for object, if vector is empty
        // that means object has no Verilog representation
        auto vars = elabObj->getVerilogVars();
        bool isVar = vars.size() != 0 || chanRecFieldVar;
        
//        cout << "    vars for sval " << sval << " : ";
//        for (auto verVar : vars) {
//            cout << "  " << verVar.var->getName();
//        }
//        cout << endl;

        if (isVar) {
            auto var = chanRecFieldVar ? chanRecFieldVar : vars[0].var;

            if (staticState->extrValNames.count(sval) == 0) {
                //std::cout << "Name for " << sval  << " (Id#" << elabObj->getID() << ")"
                //           << " is " << vars[0].var->getName() << std::endl;
                staticState->extrValNames.emplace(sval, var->getName());
                
                // Add name for channel object also, it is taken by channel object
                SValue ssval = getValue(sval);
                if (ssval.isScChannel()) {
                    staticState->extrValNames.emplace(ssval, var->getName());
                }
            } else {
                // That is possible as there are multiple static constant fields
                // for 1)channels and 2)template with several same class 
                // parameters which contains static constant field
                if (objView.isStatic() && objView.isConstant()) {
                    // That is OK for template parameter class
                } else {
                    std::cout << "Duplicate state value " << sval 
                              << " name is " << var->getName() << std::endl;
                    SCT_TOOL_ASSERT (false, "Duplicate value in elaborator variables");
                }
            }
        } else {
            // No name for module variables, others should have one name
            if (DebugOptions::isEnabled(DebugComponent::doGenState)) {
                cout << "No external name for " << sval << endl;
            }
        }
    }
}

void ScState::putVerilogTraits(const SValue &sval, VerilogVarTraits traits)
{
    //cout << "putVerilogTraits " << sval << "  " << traits << endl;
    staticState->varTraits.emplace(sval, traits);
}

void ScState::clearVerilogTraits()
{
    staticState->varTraits.clear();
}

void ScState::setProcStateName(const std::string& current, const std::string& next)
{
    staticState->procStateName = {current, next};
}

const std::pair<std::string, std::string>& ScState::getProcStateName() const
{
    return staticState->procStateName;
}

void ScState::setWaitNVarName(const std::string& current, const std::string& next)
{
    staticState->waitNVarName = {current, next};
}

const std::pair<std::string, std::string>&  ScState::getWaitNVarName() const
{
    return staticState->waitNVarName;
}

llvm::Optional<sc_elab::ObjectView> ScState::getElabObject(const SValue &sval) const
{
    auto searchRes = staticState->sVal2ElabMap.find(sval);
    if (searchRes != staticState->sVal2ElabMap.end()) {
        return searchRes->second;
    }
    return llvm::None;
}

const unordered_map<SValue, string>& ScState::getExtrValNames() const {
    return staticState->extrValNames;
}

const unordered_map<SValue, const VerilogVarTraits>& ScState::getVarTraits() const {
    return staticState->varTraits;
}

// Is @val array variable or pointer which owns of an object, 
// \return false for @sc_port type variable and temporary variable
bool ScState::isObjectOwner(const SValue& val) const 
{
    // Array value is always owner of sub-array
    if (val.isArray()) {
        return true;
    }
    // Array typed variable is always owner
    if (val.getTypePtr() && val.getTypePtr()->isArrayType()) {
        return true;
    }
    
    // Temporary variable not considered as object owner
    if (val.isVariable()) {
        // @sc_port is not owner of the MIF object
        if (isScPort(val.getType())) {
            return false;
        }
        // Check if it is global pointer variable
        if (val.getTypePtr()->isPointerType()) {
            const ValueDecl* valDecl = val.getVariable().getDecl();
            const DeclContext* declContext = valDecl->getDeclContext();
            bool isLocalVar = isa<FunctionDecl>(declContext);
            return !isLocalVar;
            
        } else {
            // Local variable owns record
            return true;
        }
    }
    
    // Simple object is pointer and it owns pointee if it assigned at 
    // elaboration phase only, that marked with @owner flag
    if (val.isSimpleObject() && val.getSimpleObject().isOwner()) {
        return true;
    }
    
    return false;
}

// Get variable (not temporary variable) for the given value recursively
// For record it also checks derived records
SValue ScState::getVariableForValue(const SValue& rval) const
{
    //cout << "getVariableForValue for " << rval.asString() << endl;
    
    if (rval.isUnknown()) {
        return NO_VALUE;
    }
    
    // Do not return temporary variable
    if (rval.isVariable()) {
        //cout << "Owner variable " << rval << endl;
        return rval;
        
    } else {
        SValue rrval = rval;
        // Reset array offset as variable points to first element
        if (rrval.isArray()) {
            rrval.getArray().setOffset(0);
        }
        
        bool hasDerived;
        do {
            // Recursively find topmost variable
            for (const auto& i : tuples) {
                // Check left value is object owner, any object has exact one owner
                //cout << "i.first " << i.first << endl;

                if (i.second == rrval && isObjectOwner(i.first)) {
                    return getVariableForValue(i.first);
                }
            }
            // Try to get derived class value to find variable for it 
            hasDerived = false;
            if (rrval.isRecord()) {
                SValue brval = rrval;
                hasDerived = getDerivedClass(brval, rrval);
            }
        } while (hasDerived);

        //cout << "No owner found" << endl;
        return NO_VALUE;
    }
}

// Not used now
// Filter Used/Defined values replacing array element with zero-index element
// \return -- all record field values for a record field/record variable
InsertionOrderSet<SValue> ScState::getZeroIndexAllFields(const SValue& val) const 
{
    //cout << "---- getZeroIndexAllFields val " << val << endl; 
    InsertionOrderSet<SValue> res;
    SValue mval = val; 

    // Check for local variable in MIF/record function 
    bool isLocVar = false;
    if (mval.isVariable()) {
        isLocVar = !isa<FieldDecl>(mval.getVariable().getDecl());
    }
    
    // Get first element of array
    if (mval.isVariable()) {
        mval = getFirstArrayElementForAny(mval);
        //cout << "   frst elm mval " << mval << endl;
    }
    
    // Variable is record, not record field
    bool isRec = isUserClass(mval.getType());
    bool isRecArr = !isRec && isUserDefinedClassArray(mval.getType(), false);
    
    // Try to get record for variable or parent record for field variable
    SValue recval;
    if (mval.isVariable()) {
        if (isRec) {
            // Record variable in copy/assign
            getValue(mval, recval);
            // Local variable works only for record field
            isLocVar = false;   
            
        } else 
        if (isRecArr) {
            // Record array variable in copy/assign 
            while (mval && !mval.isRecord()) {
                getValue(mval, recval);
                mval = recval;
            }
            // Local variable works only for record field
            isLocVar = false;
            
        } else {
            // Record field or local function variable
            recval = mval.getVariable().getParent();
        }
    }
    //cout << "   recval " << recval << endl;

    // Do not add all fields of module/MIF
    QualType recType = recval.getType();
    bool isModuleOrMIF = recType.getTypePtrOrNull() &&
                         isScModuleOrInterface(recType, true);

    if (recval.isRecord() && !isModuleOrMIF && !isLocVar) {
        // Gets all fields for local record or for record copy/assign
        //cout << "   recval " << recval << " record, add fields " << endl;
        InsertionOrderSet<SValue> fields = getRecordFields(recval);
        res.insert(fields.begin(), fields.end());

    } else {
        // Other kind variable
        //cout << "   mval " << mval << " not a record " << endl;
        res.insert(mval);
    }
    
    return res;
}

// Check value is array/record array element 
bool ScState::isArrElem(const SValue& val, unsigned crossModule) const
{
    // Get object and variable values into @valStack, 0 -- no cross module
    std::vector<SValue> valStack;
    parseValueHierarchy(val, crossModule, valStack);
    
    for (const SValue& mval : valStack) {
        if (mval.isArray()) {
            return true;
        }
    }
    return false;
}

// Check value is array/record array element at unknown index which is 
// not current module/record
// \return <hasAnyArray, hasArrayAtUnknIndx>
std::pair<bool, bool> ScState::isArrElemUnkwn(const SValue& val) const
{
    // Get object and variable values into @valStack, 0 -- no cross module
    std::vector<SValue> valStack;
    parseValueHierarchy(val, 0, valStack);
    
    // Get number of array with unknown index values in the value hierarchy
    SValue aval;
    bool hasArray = false;
    unsigned unkwIndxNum = 0; 
    
    for (const SValue& mval : valStack) {
        if (mval.isArray()) {
            if (mval.getArray().isUnknown()) {
                unkwIndxNum += 1; 
                aval = mval;
            }
            hasArray = true;
        }
    }
    
    //cout << "isNotArrElemUnkwn " << val << " aval " << aval << endl;
    
    if (unkwIndxNum == 0) {
        //cout << "   " << val << " NOT array unknw" << endl;
        return std::make_pair(hasArray, false);
        
    } else 
    if (unkwIndxNum == 1) {
        // One unknown index, check if it can be ignored 
        // Check for local variable in MIF/record function 
        bool isLocVar = false;
        if (val.isVariable()) {
            isLocVar = !isa<FieldDecl>(val.getVariable().getDecl());
        }

        // Get zero element of array
        aval.getArray().setOffset(0);
        
        // Try to get record at unknown index
        SValue recval; getValue(aval, recval);
        if (!recval.isRecord()) getValue(recval, recval); 
        //cout << "Record value recval " << recval << endl;
        
        // Check for record at unknown index is module
        QualType recType = recval.getType();
        bool isModule = recType.getTypePtrOrNull() && isScModule(recType, true);

        // Module field and local function variable considered as not in array
        if (recval.isRecord() && (isModule || isLocVar)) {
            //cout << "   " << val << " NOT array (MIF/Rec) " << endl;
            return std::make_pair(true, false);
        }
        //cout << "   " << val << " array unknw" << endl;
    }
    
    return std::make_pair(true, true);
}

// Add declared but not initialized variable, not included SC types
void ScState::declareValue(const SValue &lval) 
{
    declared.insert(lval);
}

// Add value to @defined for non-zero array elements
void ScState::writeToArrElmValue(const SValue& lval) 
{
    // Array element cannot be reference
    defined.insert(lval);
}

// Add value to @defined
// \param isDefined -- all values for fields or array elements are defined 
SValue ScState::writeToValue(SValue lval, bool isDefined) 
{
    //cout << "writeToValue lval " << lval << endl;
    SValue llval; 
    // Remove reference, Use/Def analysis works for referenced objects
    // Return constant reference if it refers to constant
    getDerefVariable(lval, llval, true); lval = llval;
    // Get variable for channels and objects
    if (lval.isObject() || lval.isScChannel()) {
        llval = getVariableForValue(lval);
    }

    // No Use/Def analysis for non-constant reference
    if (!llval.isVariable()) return NO_VALUE;
    if (llval.isReference() && !llval.isConstReference()) return NO_VALUE;
    //cout << "   lval " << lval << " llval " << llval << " isDefined " << isDefined << endl;
    
    // Get zero index element if required
    SValue zeroVal = getFirstArrayElementForAny(llval);

    // Add array defined for any array/non-array value
    arraydefined.insert(zeroVal);

    if (DebugOptions::isEnabled(DebugComponent::doUseDef)) {
        cout << "   add to array_defined " << zeroVal << endl;
    }

    if (isDefined) {
        // Function non-reference parameter including array is fully defined 
        // REC add all fields, ARR add all elements
        defined.insert(lval);
            
        defallpath.insert(zeroVal);
        defsomepath.erase(zeroVal);

        if (DebugOptions::isEnabled(DebugComponent::doUseDef)) {
            cout << "   add to defined param/first elem lval " << lval << endl;
        }
    } else {
        auto arrFlags = isArrElemUnkwn(lval); // hasAnyArray|hasArrayAtUnknIndx

        // For value which is not array element at unknown index
        if (!arrFlags.second) {
            // REC specific field, ARR specific element
            defined.insert(lval);

            // Add all/some path defined for non-array value only 
            if (!arrFlags.first) {
                SValue fval = getFirstArrayElementForAny(llval);
                defallpath.insert(fval);
                defsomepath.erase(fval);
            }

            if (DebugOptions::isEnabled(DebugComponent::doUseDef)) {
                cout << "   add to defined lval " << lval << endl;
            }
        } else {
            // No define for array, difficult to prove all elements are defined 
        }
    }
    return zeroVal;
}

SValue ScState::readFromValue(SValue lval)
{
    //cout << "readFromValue lval " << lval << endl;
    SValue llval; 
    // Remove reference, Use/Def analysis works for referenced objects
    // Return constant reference if it refers to constant
    getDerefVariable(lval, llval, true); lval = llval;
    // Get variable for channels and objects
    if (lval.isObject() || lval.isScChannel()) {
        llval = getVariableForValue(lval);
    }

    // No Use/Def analysis for non-constant reference
    if (!llval.isVariable()) return NO_VALUE;
    if (llval.isReference() && !llval.isConstReference()) return NO_VALUE;
    //cout << "   lval " << lval << " llval " << llval << endl;

    // Get zero index element if required
    SValue zeroVal = getFirstArrayElementForAny(llval);

    bool isDefined = false;
    bool isDeclared = false;
    
    // Consider SVA arguments except FOR-loop counter as not defined 
    // to make them registers
    // For value which is not array element at unknown index
    bool forLoopCounter = loopCntrVars.count(llval); 
    if ((!parseSvaArg || forLoopCounter) && !isArrElemUnkwn(lval).second) {
        // Check non-array element variable or record field is defined or declared
        isDefined = defined.count(lval);
        isDeclared = declared.count(lval);

        if (DebugOptions::isEnabled(DebugComponent::doUseDef)) {
            cout << "   check defined lval " << lval << " isDefined " 
                 << isDefined << " isDeclared " << isDeclared <<  endl;
        }
    }

    if (!isDefined && !isDeclared) {
        if (parseSvaArg) {
            readsva.insert(zeroVal);
        } else {
            readndef.insert(zeroVal);
        }
        
        if (DebugOptions::isEnabled(DebugComponent::doUseDef)) {
            cout << "   add to readNotDefined " << zeroVal << endl;
        }
    }
    
    // Read non-initialized CPP variables in the same cycle where declared
    // to report warning
    if (isDeclared && !isDefined && !parseSvaArg) {
        readninit.insert(zeroVal);
    }

    read.insert(zeroVal);

    if (DebugOptions::isEnabled(DebugComponent::doUseDef)) {
        cout << "   add to read " << zeroVal << endl;
    }
    return zeroVal;
}

// Filter UseDef to remove non-used values eliminated in unused statements removing
void ScState::filterUseDef(const std::unordered_set<SValue>& defVals, 
                           const std::unordered_set<SValue>& useVals) 
{
    //cout << "filterUseDef" << endl;
    auto filter = [](InsertionOrderSet<SValue>& ovals, 
                     const std::unordered_set<SValue>& fvals) 
    {
        for (auto i = ovals.begin(); i != ovals.end(); ) {
            if (fvals.count(*i) == 0) {
                i = ovals.erase(i);
                //cout << "  " << *i << endl;
            } else {
                ++i;
            }
        }
    };
    
    filter(arraydefined, defVals);
    filter(read, useVals);
    filter(readndef, useVals);
    filter(readsva, useVals);
}

const InsertionOrderSet<SValue> ScState::getReadNotDefinedValues(
                                                    bool includeSva) const {
    if (!includeSva) return readndef;
    
    InsertionOrderSet<SValue> res(readndef);
    res.insert(readsva.begin(), readsva.end());
    return res;
}

const InsertionOrderSet<SValue> ScState::getReadNotInitValues() const {
    return readninit;
}

const InsertionOrderSet<SValue>& ScState::getReadValues() const {
    return read;
}

const InsertionOrderSet<SValue>& ScState::getSvaReadValues() const {
    return readsva;
}

const InsertionOrderSet<SValue>& ScState::getDefAllPathValues() const {
    return defallpath;
}

const InsertionOrderSet<SValue>& ScState::getDefSomePathValues() const {
    return defsomepath;
}

const InsertionOrderSet<SValue>& ScState::getDefArrayValues() const {
    return arraydefined;
}

const InsertionOrderSet<SValue>& ScState::getDeclaredValues() const {
    return declared;
}

// Remove single integer variables which exists in @defined 
// Used to remove member variables from state after preliminary CPA
void ScState::removeDefinedValues(std::unordered_set<SValue> defined) 
{
    auto i = tuples.begin();
    while (i != tuples.end()) {
        // Remove tuple for integer variable only
        if (i->first.isVariable() && i->second.isInteger()) {
            // Get zero index element including record arrays
            SValue zeroVal = getFirstArrayElementForAny(i->first);

            if (defined.count(zeroVal) == 0) {
                ++i;
            } else {
                i = tuples.erase(i);
            }
        } else {
            ++i;
        }
    }
}

// Is the given value an array value or channel as array element, 
// does not work for array variable or record array
// \param unkwIndex -- index is non-determinable
bool ScState::isArray(const SValue& val, bool& unkwIndex) const 
{
    SValue mval = val;
    unkwIndex = false;

    // Checking for channels/channel pointers array 
    if (mval.isScChannel()) {
        do {
            bool found = false;
            for (const auto& i : tuples) {
                if (i.second == mval && isObjectOwner(i.first)) {
                    mval = i.first; found = true; break;
                }
            }

            if (!found) return false;
            
        } while (mval.isSimpleObject());
    }
    
    //cout << "isArray mval " << mval;
    
    bool isArr = false;
    while (mval.isArray()) {
        unkwIndex = unkwIndex || mval.getArray().isUnknown();
        isArr = true;

        auto i = tuples.find(mval);
        mval = (i != tuples.end()) ? i->second : NO_VALUE;
        //cout << "   " << mval; 
    }
    //cout << " " << isArr << endl;
    
    return isArr;
}

// Get bottommost array for given value which can be element in array or 
// field in record array element.
// \param unkwIndex -- unknown index for bottommost array
// \param checkRecOnly -- provide unkwIndex for record/MIF array only
// \return bottom array value or NO_VALUE
SValue ScState::getBottomArrayForAny(const SValue& eval, bool& unkwIndex,
                                     unsigned crossModule, bool checkRecOnly) const
{
    //cout << "getBottomArrayForAny eval " << eval << endl;
    
    // Get object and variable values into @valStack
    std::vector<SValue> valStack;
    parseValueHierarchy(eval, crossModule, valStack);

    // Get unknown from all arrays
    unkwIndex = false;
    for (const SValue& mval : valStack) {
        if (mval.isArray()) {
            if (checkRecOnly) {
                QualType elmtype = getArrayElementType(mval.getType());
                if (isPointer(elmtype)) {
                    elmtype = elmtype->getPointeeType();
                }
                if (!isScModularInterface(elmtype) &&
                    !isUserClass(elmtype)) continue;
            }
            unkwIndex = unkwIndex || mval.getArray().isUnknown();
        }
    }
    
    // Return first (most bottom) array or no value
    for (const SValue& mval : valStack) {
        if (mval.isArray()) {
            return mval;  // Bottom array
        }
    }
    return NO_VALUE;
}

std::vector<SValue> ScState::getAllMifArrays(const SValue& val, unsigned crossModule) const
{
    //cout << "getAllMifArrays val " << val << endl;
    
    // Get object and variable values into @valStack
    std::vector<SValue> valStack;
    parseValueHierarchy(val, crossModule, valStack);

    // Get unknown from all arrays
    std::vector<SValue> res;
    for (const SValue& mval : valStack) {
        if (isScModule(mval.getType())) break;
        
        if (mval.isArray()) {
            res.push_back(mval);
            //cout << "  " << mval << endl;
        }
    }
    std::reverse(res.begin(), res.end());
    return res;
}

// Get topmost value and field declarations for given value which can be 
// element in array or field in record array element.
// Only one multidimensional array supported, no record array with array member
// \param eval -- element which specifies array 
// \param crossModule -- cross module/MIF border number
// \param decls -- field declarations of record element variables
SValue ScState::getTopForAny(const SValue& eval, unsigned crossModule,
                             vector<const ValueDecl*>& decls) const
{
    //cout << "   parseArrayElement " << endl;
    vector<SValue> valStack;
    parseValueHierarchy(eval, crossModule, valStack);
    
//    cout << "valStack: " << endl;
//    for (const SValue& val : valStack) {
//        cout << "  " << val << endl;;
//    }
//    cout << endl;
    
    // Put declaration for variable or null for others
    for (const SValue& val : valStack) {
        if (val.isVariable()) {
            decls.push_back(val.getVariable().getDecl());
        } else {
            decls.push_back(nullptr);
        }
    }
    // Last declaration is not required
    decls.pop_back();
    
    // Return topmost value
    return valStack.back();
}

// Get first non-MIF module in @eval parent hierarchy
SValue ScState::getSynthModuleValue(const SValue& eval, unsigned crossModule) const
{
    //cout << "   parseArrayElement " << endl;
    vector<SValue> valStack;
    parseValueHierarchy(eval, crossModule, valStack);
    
    /*cout << "valStack: ";
    for (const SValue& val : valStack) {
        cout << " " << val << endl;
    }
    cout << endl;*/
    
    SValue mval;
    for (const SValue& val : valStack) {
        if (val.isRecord() && isScModule(val.getType(), false)) {
            mval = val;
            break;
        }
    }
    //cout << "   mval " << mval << endl;
    return mval;
}

// Clear offset(s) of array element, can be applied for array and 
// array of records, supports multidimensional arrays and 
// record array with inner record 
SValue ScState::getFirstArrayElementForAny(const SValue& val, 
                                           unsigned crossModule) const
{
    SValue mval = val;
    correctUnknownIndexValue(mval);
    //cout << "correctUnknownIndexValue val " << val << " mval " << mval << endl;
    
    std::vector<const clang::ValueDecl*> decls;
    mval = getTopForAny(mval, crossModule, decls);
    
//    cout << "  getTopArrayForAny val " << val << " mval " << mval << endl;
//    for (auto i = decls.rbegin(); i != decls.rend(); ++i) {
//        cout << "    " << hex << (uint64_t)(*i) << dec << endl;
//    }
    
    // Traverse down recovering variable declaration and clearing array offset
    //cout << "  Recovering zero value " << endl;
    for (auto i = decls.rbegin(); i != decls.rend(); ++i) {
        if (*i) {
            mval = SValue(*i, mval);
            // Restore correct parent module which can be changed
            correctParentBaseClass(mval);
            //cout << "    " << mval << endl;
        } else {
            // Clear array index at get array value or pointer de-reference
            mval = getValue(mval);
            //cout << "    " << mval << endl;
        }
    }
    
    //cout << " getFirstArrayElementForAny " << val << " " << mval << endl;
    return mval;
}

// Restore correct parent for record field accessed at unknown index(es)
// Required for multidimensional array and array in record array
// For T a[N][M] accessed at unknown indices there is ARR2[UNKW].a which is 
// corrected here to REC1.a, where state contains (ARR2[0], REC1) tuple
void ScState::correctUnknownIndexValue(SValue& val) const 
{
    //cout << "correctUnknownIndexValue " << val << endl;
    if (!val.isVariable()) return; 
    
    SValue parent = val.getVariable().getParent();
    //cout << "  parent " << parent << endl;
    if (parent.isVariable() && 
        parent.getVariable().getDecl() != val.getVariable().getDecl()) {
        correctUnknownIndexValue(parent);
        parent = getValue(parent);
        //cout << "  parent (1) " << parent << endl;
    }
    while (parent.isArray() || parent.isSimpleObject()) {
        if (parent.isArray()) parent.getArray().clearUnknown();
        parent = getValue(parent);
        //cout << "  parent (2) " << parent << endl;
    }
        
    val = SValue(val.getVariable().getDecl(), parent);
    // Restore correct parent module which can be changed
    correctParentBaseClass(val);
    //cout << "  result " << val << endl;
}

// Get all elements of given record array recursively
// \return true if it is last lvalue, next one is unknown or integer
bool ScState::getRecordArrayElements(const SValue& val, vector<SValue>& resvals,
                                     const std::vector<const clang::ValueDecl*>& decls,
                                     int declIndx) const
{
    if (val.isArray()) {
        SValue aval = val;
        // Get all elements as there is no index known
        for (size_t i = 0; i < aval.getArray().getSize(); i++) {
            aval.getArray().setOffset(i);
            SValue rval = getValue(aval);
            // Do not consider array of pointers as it cannot be written
            if (getRecordArrayElements(rval, resvals, decls, declIndx)) {
                resvals.push_back(aval);
            }
        }
        return false;
    } else 
    if (val.isRecord()) {
        if (declIndx < 0) {
            // All record fields are written
            return true;
            
        } else {
            //auto decl = decls.back(); decls.pop_back();
            auto decl = decls[declIndx]; declIndx--;
            SValue rval = SValue(decl, val);
            // Restore correct parent module which can be changed
            correctParentBaseClass(rval);
            // @val is variable which is lvalue, so do not check result
            getRecordArrayElements(rval, resvals, decls, declIndx);
            return false;
        }
    } else 
    if (val.isSimpleObject() || val.isVariable()) {
        SValue rval = getValue(val);
        if (getRecordArrayElements(rval, resvals, decls, declIndx)) {
            resvals.push_back(val);
        }
        return false;
        
    } else 
    if (val.isSimpleObject()) {
        // Do nothing
        return false;
    
    } else {
        // Do nothing
        return true;
    }
}

// TODO: update me
// Return all elements of single/multidimensional array/record array 
// where the given value is stored as element
vector<SValue> ScState::getAllRecordArrayElementsForAny(const SValue& val,
                                                unsigned crossModule) const
{
    vector<SValue> valStack;
    parseValueHierarchy(val, crossModule, valStack);
    
    // Get topmost array with unknown index
    SValue mval = val;
    for (const SValue& v : valStack) {
        if (v.isArray() && v.getArray().isUnknown()) {
            mval = v;
        }
    }
//    cout << "valStack: ";
//    for (const SValue& val : valStack) {
//        cout << " " << val;
//    }
//    cout << endl;

    SCT_TOOL_ASSERT (mval.isArray() && mval.getArray().isUnknown(), "");
    
    // Get all declaration before @mval
    std::vector<const clang::ValueDecl*> decls;
    for (const SValue& v : valStack) {
        if (v == mval) break;
        if (v.isVariable()) {
            decls.push_back(v.getVariable().getDecl());
        }
    }
    //cout << "   mval " << mval << endl;
    
    // Recursively traverse all the array elements and put them into @recvals
    vector<SValue> recvals;
    if (mval.isArray()) {
        getRecordArrayElements(mval, recvals, decls, decls.size()-1);
//        cout << "getRecordArrayElements: " << endl;
//        for (auto& v : recvals) {
//            cout << "  " << v << endl;
//        }
    }
    
    return recvals;
}

// Get all fields for given record value with 
InsertionOrderSet<SValue> ScState::getRecordFields(const SValue& recval) const
{
    //cout << endl << "getRecordFields for " << recval << endl << "   ";
    InsertionOrderSet<SValue> res;
    auto recDecl = recval.getType()->getAsCXXRecordDecl();
    
    for (auto fieldDecl : recDecl->fields()) {
        SValue fval(fieldDecl, recval);
        res.insert(fval);
        //cout << " fval " << fval << ",";
    }
    //cout << endl;
    
    return res;
}

// Get array indices for one/multidimensional array element
// \param eval -- element which specifies array 
// \param mval -- topmost array value
// \param indxs -- array indices, -1 for unknown index
/*void ScState::getArrayIndices(const SValue& eval, SValue& mval, 
                              std::vector<int>& indxs) const
{
    //cout << "getArrayInices eval " << eval << endl;
    mval = eval;
    
    // Traverse up through arrays at first index to topmost array
    //cout << "   indxs ";
    while (mval.isArray()) {
        if (mval.getArray().isUnknown()) { 
            indxs.push_back(-1);
        } else {
            indxs.push_back(mval.getArray().getOffset());
            //cout << " [" <<  mval.getArray().getOffset() << "]";
        }
        mval.getArray().setOffset(0);
        
        bool found = false;
        for (const auto& i : tuples) {
            if (i.second == mval && isObjectOwner(i.first)) {
                mval = i.first; found = true; break;
            }
        }
        SCT_TOOL_ASSERT (found, "No array in state");
    }
    //cout << endl;
    
    // Reverse indices order
    std::reverse(indxs.begin(), indxs.end());
    // @mval is topmost array if exists
    mval = getValue(mval); 
}*/

// Get all elements in sub-arrays for given array
std::vector<SValue> ScState::getSubArrayElements(const SValue& val) const 
{
    vector<SValue> res;
    SValue mval = getValue(val);
    
    if (mval.isArray()) {
        // Work with sub-arrays
        for (size_t i = 0; i < mval.getArray().getSize(); ++i) {
            mval.getArray().setOffset(i);
            vector<SValue> evals = getSubArrayElements(mval);
            for (const SValue& eval : evals) {
                res.push_back(eval);
            }
        }
    } else {
        res.push_back(val);
    }
    
    return res;
}


void ScState::compareAndSetNovalue(ScState* other) 
{
    if (dead != other->dead) {
        SCT_TOOL_ASSERT (false, "Not implemented yet");
    }
    //cout << "compareAndSetNovalue: " << endl;
    auto i = tuples.begin();
    while (i != tuples.end()) {
        // Ignore temporary variables
        if (i->first.isVariable() || i->first.isObject()) {
            const QualType& type = i->first.getType();
            
            // Skip references and constants, erase tuple for different values
            if (!i->first.isReference() && !type.isConstQualified()) {
                auto j = other->tuples.find(i->first);
                if (j == other->tuples.end() || i->second != j->second) {    
                    //cout << "    " << i->first << endl;
                    i = tuples.erase(i);
                    continue;
                }
            }
        }
        i++;
    }
}

// ===========================================================================
// Static methods

// Is field in any record
SValue ScState::isRecField(const SValue& val) 
{
    if (val.isVariable()) {
        SValue parent = val.getVariable().getParent();
        
        if (parent.isRecord() && !isScModuleOrInterface(parent.getType())) {
            return parent;
        }
    }
    return NO_VALUE;
}

// Is field in local record
SValue ScState::isLocalRecField(const SValue& val) 
{
    if (val.isVariable()) {
        SValue parent = val.getVariable().getParent();
        
        if (parent.isRecord() && parent.getRecord().isLocal()) {
            return parent;
        }
    }
    return NO_VALUE;
}

// Get name prefix for local record
std::string ScState::getLocalRecName(const SValue& val) 
{
    if (val.isVariable()) {
        SValue parent = val.getVariable().getParent();
        
        if (parent.isRecord() && parent.getRecord().isLocal()) {
            return parent.getRecord().var.asString(false);
        }
    }
    return "";
}

// Is constant variable/object or field of local record is constant variable/object
bool ScState::isConstVarOrLocRec(const SValue& val) 
{
    // Empty type returned for channel value
    if (val.getType().isNull()) return false;
    
    bool isConst = val.getType().isConstQualified();
    
    if (const SValue& pval = ScState::isLocalRecField(val)) {
        const SValue& pvar = pval.getRecord().var;
        isConst = isConst || pvar.getType().isConstQualified();
    }

    return isConst;
}


bool ScState::compareStates(const ScState* bigger, const ScState* other) 
{
    for (const auto& i : bigger->tuples) {
        // Ignore temporary variables
        if (i.first.isVariable() || i.first.isObject()) {
            const QualType& type = i.first.getType();

            // Skip references and constants
            if (!i.first.isReference() && !type.isConstQualified()) {
                auto j = other->tuples.find(i.first);
                if (j == other->tuples.end() || i.second != j->second) {
                    //cout << "Diff values for " << i.first.asString() << " : " 
                    //     << i.second.asString() << " and " << other->getValue(i.first).asString() << endl;
                    return false;
                }
            }
        }
    }
    return true;
}

llvm::APSInt ScState::getIntFromView(bool isSigned, sc_elab::ValueView valueView)  
{
    size_t bitwidth = valueView.bitwidth();
    
    if (valueView.int64Val()) {
        return llvm::APSInt(llvm::APInt(bitwidth,*valueView.int64Val()), !isSigned);
    } else 
    if (valueView.uint64Val()) {
        return llvm::APSInt(llvm::APInt(bitwidth,*valueView.uint64Val()), !isSigned);
    }
}

bool ScState::isMemberPrimVar(const SValue& val, const ScState* state) 
{
    // Skip non-variables
    if (!val.isVariable()) return false;

    // Skip pointers and arrays
    const QualType& type = val.getType();
    bool isPtr = sc::isPointer(type);
    bool isRef = sc::isReference(type);
    bool isArr = sc::isArray(type);
    if (isArr || isPtr || isRef) return false;

    // Skip constant variable it already declared as @localparam
    bool isConst = type.isConstQualified();
    if (isConst) return false;

    // Class field must be in this map, no local variables here
    if (auto elabObj = state->getElabObject(val)) {
        // Skip array and record elements
        vector<SValue> valStack;
        state->parseValueHierarchy(val, 0, valStack);
        bool skip = false;
        for (const SValue& mval : valStack) {
            if (mval.isArray() || mval.isRecord()) {
                skip = true; break;
            }
        }
        if (skip) return false;

        //cout << "  elabObj" << elabObj->getDebugString() << endl;
        if (auto elabOwner = elabObj->getVerilogNameOwner()) {
            elabObj = elabOwner;
        }

        // Skip non-primitive objects
        return (elabObj->isPrimitive() && !elabObj->isChannel());
    }
    
    return false;
}

bool ScState::compare(ScState* other) const 
{
    // It needs to compare twice as one state can contains no tuple 
    // for a variable, but other can contain
    return ( ScState::compareStates(this, other) && 
             ScState::compareStates(other, this) );
}

void ScState::setValueLevel(const SValue& val, unsigned level)
{
    // Skip record, integer and channel values (cannot be in left part of tuple)
    if (val.isRecord() || val.isInteger() || val.isScChannel()) return;
    
    while (levels.size() <= level) {
        levels.emplace_back();
    }
    levels[level].push_back(val);
    maxLevel = (maxLevel > level) ? maxLevel : level;
    //cout << "Set value val " << val << " level " << level <<  endl;
}

void ScState::removeValuesByLevel(unsigned level)
{
    //cout << "Remove values by level " << level << ", maxLevel " << maxLevel << endl;
    //printLevels();
    //cout << "-------------" << endl;
    while (maxLevel > level) {
        for (const auto& val : levels[maxLevel]) {
            // Do not remove references as they required after CPA to determine
            // UseDef variables
            if (!val.isReference()) {
                tuples.erase(val);
                //cout << "   " << val << " #" << (val.isVariable() ? val.getVariable().getDecl() : 0)<< endl;
            }
        }
        levels[maxLevel].clear();
        maxLevel--;
    }
    
    //print();
}

void ScState::printLevels() 
{
    unsigned i = 0;
    for (auto& lv : levels) {
        cout << "Level " << i++ << endl;
        for (auto& val : lv) {
            cout << "   " << val.asString() << endl;
        }
    }
}

void ScState::clearValueLevels()
{
    levels.clear();
    maxLevel = 0;
}

void ScState::clearReadAndDefinedVals()
{
    defined.clear();
    declared.clear();    
    readndef.clear();
    readninit.clear();
    readsva.clear();
    read.clear();
    defallpath.clear();
    defsomepath.clear();
    arraydefined.clear();
    loopCntrVars.clear();
}

void ScState::checkNoValueTuple() 
{
    for (const auto& i : tuples) {
        SCT_TOOL_ASSERT (!i.second.isUnknown(), "NO_VALUE in state");
    }
}

