/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * CFG state class, contains objects and value in a CFG node.
 * 
 * Author: Mikhail Moiseev
 *
 */

#ifndef SCSTATE_H
#define SCSTATE_H

#include "sc_tool/cfg/SValue.h"
#include "sc_tool/elab/ScObjectView.h"
#include "sc_tool/utils/InsertionOrderSet.h"

#include <unordered_map>
#include <unordered_set>


// ===========================================================================
// State interface

namespace sc {

/// Verilog variable properties, used to generate current/next variable for 
/// registers 
struct VerilogVarTraits 
{
    // @COMBSIG is @sct_comb_signal with @clear false
    // @COMBSIGCLEAR is @sct_comb_signal with @clear true 
    // @CLEARSIG is @sct_clear_signal 
    // @READONLY_CDR is READONLY constant defined in reset section of CTHREAD 
    enum VarKind {REGISTER, COMB, READONLY, READONLY_CDR, COMBSIG, COMBSIGCLEAR, 
                  CLEARSIG};

    /// true for register variables, false for combinational/read-only variables
    VarKind kind;
    
    /// Access in reset section and after reset 
    enum AccessPlace {NOWHERE, IN_RESET, AFTER_RESET, BOTH};
    
    /// Access place for CTHREAD variables 
    AccessPlace place;

    /// @true if variable defined inside module body, 
    /// @false for process-local variables
    bool isModuleScope = false;
    /// Skip @next_var = @var assignment for this variable
    /// Is extra zero element of MIF array or non-zero element,
    /// only one zero element is used to print
    bool skipNextAssign = false;
    /// Constant variable which is translated into @localparam
    bool isConstVar = false;
    
    /// For registers holds name of register
    /// For module-level combinational variables, holds variable name
    /// For local combinational variables is empty
    std::optional<std::string> currName;

    /// For registers holds name of next-value combinational variable
    /// empty otherwise
    std::optional<std::string> nextName;
    
    /// MIF array element suffix, used in @var_next = @var assignment
    std::optional<std::string> mifElemSuffix;

    static inline AccessPlace getAccessPlace(bool inResetAccess, 
                                             bool afterResetAcess) {
        if (inResetAccess && afterResetAcess) {
            return BOTH;
        } else 
        if (inResetAccess) {
            return IN_RESET;
        } else
        if (afterResetAcess) {
            return AFTER_RESET;
        } else {
            return NOWHERE;
        }
    }
    
    VerilogVarTraits(VarKind kind, AccessPlace place,
                     bool isModuleScope, bool skipNextAssign, bool isConstVar,
                     std::optional<std::string> curr = std::nullopt,
                     std::optional<std::string> next = std::nullopt,
                     std::optional<std::string> suffix = std::nullopt) :
        kind(kind), place(place), isModuleScope(isModuleScope), 
        skipNextAssign(skipNextAssign), isConstVar(isConstVar),
        currName(curr), nextName(next), mifElemSuffix(suffix)
    {}

    template <typename OS>
    friend OS &operator << (OS &os, VerilogVarTraits v) {
        if (v.skipNextAssign) return os << "MIF_ZERO_OBJ";
        return os << "(" << (v.isModuleScope ? "MOD " : "LOC ")
                  << (v.kind == REGISTER ? "REG " : 
                      (v.kind == COMB ? "COMB " : 
                       (v.kind == COMBSIGCLEAR ? "COMBSIGC " : 
                        (v.kind == COMBSIG ? "COMBSIG " : 
                        (v.kind == READONLY ? "READONLY " : "READONLY_CDR ")))))
                  << (v.currName ? " NAME: " + *v.currName : "")
                  << (v.nextName ? " NEXT_NAME: " + *v.nextName : "")
                  << ")" << (v.mifElemSuffix ? *v.mifElemSuffix : "");
    }

    bool isCombSigClear() const { return kind == COMBSIGCLEAR; }
    bool isCombSig() const { return kind == COMBSIG; }
    bool isClearSig() const { return kind == CLEARSIG; }
    bool isRegister() const { return kind == REGISTER; }
    bool isReadOnly() const {return (kind == READONLY || kind == READONLY_CDR);}
    bool isReadOnlyCDR() const { return kind == READONLY_CDR; }
    bool isComb() const { return kind == COMB; }
    bool isConstVerVar() const {return isConstVar;}

    bool isAccessInReset() const { return place == BOTH || place == IN_RESET;}
    bool isAccessAfterReset() const { 
        return place == BOTH || place == AFTER_RESET;
    }
};

/// Get value from state mode for array unknown index access,
/// used in ScState::getValue() 
enum class ArrayUnkwnMode {
    amNoValue,          // Return NO_VALUE
    amArrayUnknown,     // Return the same array unknown element
    amFirstElement,     // Return first (zero index) element
    amFirstElementRec   // Return first element for record/MIF array 
                        // unknown index,NO_VALUE otherwise
};

/// State of class, structure or module
class ScState 
{
protected:
    /// State tuples <SValue, SValue>
    std::unordered_map<SValue, SValue>    tuples;
    
    /// Level for variable/temporary/object value declarations
    std::vector<std::vector<SValue> >     levels;
    unsigned maxLevel = 0;
    
    /// Is state dead
    bool    dead;

    /// Part of state that does not change during process analysis
    struct StaticState {
        /// Derived class for each base class instance
        std::unordered_map<SValue, SValue>    derived;
        /// Map from SValue to elaboration database Objects,
        /// Used to add registers to VeriloogModule based on UseDef analysis 
        std::unordered_map<SValue, sc_elab::ObjectView> sVal2ElabMap;
        /// Map from SValue to external name given in elaborator
        std::unordered_map<SValue, std::string> extrValNames;
        /// Verilog properties of SValues, use owner variable @SVariable for 
        /// all kinds of objects except channels where @ScChannel used
        std::unordered_map<SValue, const VerilogVarTraits> varTraits;
        /// Name of automatically-generated wait-state variable <CurrentVar, NextVar>
        std::pair<std::string, std::string> procStateName;
        /// Name of automatically-generated counter variable used for wait(N)
        std::pair<std::string, std::string> waitNVarName;
    };

    std::shared_ptr<StaticState>  staticState = std::make_shared<StaticState>();

    /// Values which has been defined at all paths, used to fill @readndef only
    /// It contains variables, specific array elements and record fields
    InsertionOrderSet<SValue>    defined;
    /// Declared variables, can be defined or not, used to avoid registers
    /// for declared but not defined variables, not included SC types
    InsertionOrderSet<SValue>    declared;
    
    /// All the collections below are filtered: 
    /// zero element for array, all fields for record
    /// Read before defined/declared at any path
    InsertionOrderSet<SValue>    readndef;
    /// Read non-initialized value in the cycle as declared, for CPP types only
    InsertionOrderSet<SValue>    readninit;
    /// Read in SVA expression, considered as @readndef to create register
    InsertionOrderSet<SValue>    readsva;
    /// Read at any path
    InsertionOrderSet<SValue>    read;
    /// Accessed for read or write at any path and in any MIF array elements
    InsertionOrderSet<SValue>    access;
    /// Any defined values including partially define arrays
    InsertionOrderSet<SValue>    arraydefined;
    /// Defined at all paths 
    InsertionOrderSet<SValue>    defallpath;
    /// Defined at some paths and not defined at some other paths, latches
    InsertionOrderSet<SValue>    defsomepath;
    
    /// FOR-loop internal counter variables, used to prevent its transformation to 
    /// register, which is forced for @SCT_ASSERT expression arguments
    std::unordered_set<SValue>   loopCntrVars;
    
    /// Parsing SVA argument mode, consider all read variables as not defined
    /// to make them registers, required as SVA generated in @always_ff
    bool parseSvaArg = false;
    
protected:
    /// Auxiliary value parser functions
    void parseParentForVar(SValue val, unsigned crossModule,
                           std::vector<SValue>& valStack) const; 
    void parseParentForRec(SValue val, unsigned crossModule,
                           std::vector<SValue>& valStack) const; 
    void parseParentForObj(SValue val, unsigned crossModule,
                           std::vector<SValue>& valStack) const; 
    
public:
    /// Parse value hierarchy to topmost module storing all intermediate values 
    /// in @valStack
    /// \param crossModule -- number of module/MIF border cross
    void parseValueHierarchy(SValue val, unsigned crossModule, 
                             std::vector<SValue>& valStack) const;

    // Check if the given element is member of the given record @recval
    // It can be not direct member, i.e.member of its member
    bool checkRecord(const SValue& val, const SValue& recval,
                     unsigned crossModule) const;
    
public:
    explicit ScState(bool dead_ = false) : dead(dead_) {}
    
    ScState(const ScState& other) = default;

    /// Initialize correct radix for member integer constants
    /// \return -- radix updated for some tuples
    template<class PARSE_VALUE>
    bool initConstRadix(PARSE_VALUE& parseValue) 
    {
        using namespace std;
        using namespace clang;

        // Several iterations to spread radix from used constants into evaluated 
        bool updated = false;

        for (auto& i : tuples) {
            const SValue& lval = i.first;

            if (lval.isVariable() && lval.getType().isConstQualified()) {
                // Skip constants which already have non-decimal radix
                SValue& rval = i.second;
                //cout << "lval " << lval << " rval " << rval << endl;
                if (!rval.isInteger() || rval.getRadix() != 10) continue;

                ValueDecl* decl = const_cast<ValueDecl*>(
                                        lval.getVariable().getDecl());
                VarDecl* vdecl = dyn_cast<VarDecl>(decl);
                FieldDecl* fdecl = dyn_cast<FieldDecl>(decl);
                //decl->dumpColor();

                bool hasInit = (vdecl && vdecl->hasInit()) || 
                               (fdecl && fdecl->hasInClassInitializer());
                if (!hasInit) continue;

                Expr* iexpr = vdecl ? vdecl->getInit() : 
                                      fdecl->getInClassInitializer();
                SValue ival = parseValue.evaluateConstInt(iexpr).second;
                //cout << "  ival " << ival << endl;

                if (ival.isInteger() && ival.getRadix() != 10) {
                    rval.setRadix(ival.getRadix()); 
                    updated = true;
                }
            }
        }
        return updated;
    }

    /// Fill derived classes for all base records of the given record @recval
    void fillDerived(const SValue& recval);
    
    /// Fill derived classes map for all records in the current state tuples
    void fillDerived();
    
    // Not used for now
    /// Set static class value for pointer to derived class
    //void updateStaticClasses();

    /// Check if state is dead
    bool isDead();

    /// Return number of items in state
    size_t size() const;
    
    /// Create clone of this state for IF branches
    ScState* clone() const;
    
    /// Join other state to this one in PHI function. If there are different 
    /// values for an object in two states, this tuple is removed.
    void join(ScState* other);

    /// Recursively set single/multidimensional array elements to NO_VALUE, 
    /// if tuple right part is:
    ///   array -- run this function for all its elements,
    ///   integer -- set it to NO_VALUE,
    ///   channel -- or object do nothing.
    void setArrayNoValue(const SValue& val);
    
    /// Set record array elements accessed at unknown index to NO_VALUE, 
    /// For array and pointers it gets variable, but set arrays elements/pointee
    void setArrayRecordNoValue(const SValue& val);
    
    /// Put @rval value for @lval if it is variable, record, array, object, 
    /// replace old value if exist
    /// \param deReference -- do de-reference for LValue and RValue references
    /// \param checkUnknwIndx -- check @lval has unknown index and erase tuple
    void putValue(const SValue& lval, const SValue& rval, 
                  bool deReference = true, bool checkUnknwIndx = true);
    
    /// Create deep copy in state of given value, returns clone of the value and 
    /// put clone of all the subvalue clones into state
    /// \param level -- value level used to remove tuples above the level
    /// \param parent -- parent for cloned value
    /// \param locvar -- owner variable for cloned value if it is local record
    SValue copyIntSubValues(const SValue& val, unsigned level,
                            const SValue& parent = NO_VALUE,
                            const SValue& locvar = NO_VALUE, size_t index = 0);
    
    /// Remove tuples with integer values starting with @val in left part and 
    /// recursively all tuples with its right parts, no array/record removed 
    /// Use to clean up state by write to array element at unknown index
    /// \param lval -- variable value
    void removeIntSubValues(const SValue& lval, bool doGetValue = true);
    
    /// Remove tuples with @val in left part and recursively all tuples with its 
    /// right parts, used to remove old record/array values at variable 
    /// re-declaration which required to get variable for value
    void removeSubValues(const SValue& val);
    
    /// Recursively create multi-dimensional array and its elements 
    /// at specified value level
    SValue createArrayInState(clang::QualType arrayType, unsigned level = 0);
    /// Recursively create multi-dimensional std::array and its elements 
    /// at specified value level
    SValue createStdArrayInState(clang::QualType arrayType, unsigned level = 0);

    /// Get value of @lval into @rval, set @rval into @NO_VALUE if no value for
    /// @lval found
    /// \param deReference -- do de-reference for LValue reference
    /// \param returnUnkwn -- return array unknown element, first element or NO_VALUE (false) 
    /// \return <value found, array with unknown index returned>
    std::pair<bool, bool> getValue(const SValue& lval, SValue& rval, 
                                   bool deReference = true,
                                   ArrayUnkwnMode returnUnknown = 
                                   ArrayUnkwnMode::amNoValue) const;

    SValue getValue(const SValue& lval) const;

    /// Remove tuple for given left part from state
    void removeValue (const SValue& lval);

    /// Do @lval dereference if required to get referenced variable
    /// \param keepConstRef -- do not de-reference constant reference if 
    ///                        it refers to constant, required for UseDef
    void getDerefVariable(SValue lval, SValue &llval, 
                          bool keepConstRef = false) const;

    /// Get derived class for given base class
    /// \param dval -- derived class or the same class if no derived class found
    /// \return true if derived class found 
    bool getDerivedClass(const SValue& bval, SValue& dval) const;
    
    /// Get most derived class (dynamic class) for given base class
    /// \param dval -- most derived class or the same class if no derived class found
    /// \return true if any derived class found 
    bool getMostDerivedClass(const SValue& bval, SValue& dval) const;

    /// Print @state to cout
    void print() const;
    void printSize() const;

    /// Add elaboration object for SValue
    void putElabObject(const SValue &sval, sc_elab::ObjectView objView,
                       const sc_elab::VerilogVar* chanRecFieldVar = nullptr);

    /// Put VerilogVarTraits for SValue
    void putVerilogTraits(const SValue &sval, VerilogVarTraits traits);

    /// Remove VerilogVarTraits. Usually we clear previously collected traits
    /// before starting each process analysis
    void clearVerilogTraits();

    /// Set name of wait state variable <PROC_STATE, PROC_STATE_next>
    void setProcStateName(const std::string& current, const std::string& next);
    /// Get name of wait state variable <PROC_STATE, PROC_STATE_next>
    const std::pair<std::string, std::string>&  getProcStateName() const;

    /// Set name of a counter variable used to model wait(N)
    void setWaitNVarName(const std::string& current, const std::string& next);
    /// Get name of a counter variable used to model wait(N)
    const std::pair<std::string, std::string>&  getWaitNVarName() const;

    /// Get elaboration object for SValue if exists
    std::optional<sc_elab::ObjectView> getElabObject(const SValue &sval) const;

    /// Get elaboration object map
    const std::unordered_map<SValue, std::string>& getExtrValNames() const;
    
    /// Get variable traits including names
    const std::unordered_map<SValue, const VerilogVarTraits>& getVarTraits() const;

    /// Is @val array variable or pointer which owns of an object, 
    /// return false for temporary variable
    bool isObjectOwner(const SValue& val) const;

    /// Get variable (not temporary variable) for the given value recursively
    /// For record it also checks derived records
    SValue getVariableForValue(const SValue& rval) const;
    
    /// Not used now
    /// Filter Used/Defined values replacing array element with zero element
    /// \return -- all record field values for a record field/record variable
    InsertionOrderSet<SValue> getZeroIndexAllFields(const SValue& val) const;

    /// Check value is array/record array element 
    bool isArrElem(const SValue& val, unsigned crossModule) const;

    /// Check value is array/record array element at unknown index which is 
    /// not current module/record
    /// \return <hasAnyArray, hasArrayAtUnknIndx>
    std::pair<bool, bool> isArrElemUnkwn(const SValue& val) const;
    
    /// Check if given variable has any temporary parent
    bool hasTempVariable(const SValue &val);
    
    /// Add declared but not initialized variable, not included SC types
    /// \return zero array variable for given variable @lval
    SValue declareValue(const SValue &lval);

    /// Add value to @defined for non-zero array elements, 
    /// for zero element @writeToValue with isDefined = true should be used
    void writeToArrElmValue(const SValue &lval);
    
    /// Add value to @defined
    /// \param isDefined -- all values for fields or array elements are defined 
    /// \return zero array variable for given value @lval
    SValue writeToValue(SValue lval, bool isDefined = false);

    /// Add value to read not defined if it was not defined yet
    /// \return zero array variable for given value @lval
    SValue readFromValue(SValue lval);
    
    /// Filter UseDef to remove non-used values eliminated in unused 
    /// statements removing
    void filterUseDef(const std::unordered_set<SValue>& defVals, 
                      const std::unordered_set<SValue>& useVals);
    
    /// Register FOR-loop counter variable
    void regForLoopCounter(const SValue& val) {
        if (val.isVariable()) {
            loopCntrVars.insert(val);
        }
    }
    
    /// Set parsing SVA argument mode, to consider read variables as not defined
    void setParseSvaArg(bool mode) {
        parseSvaArg = mode;
    }
    bool getParseSvaArg() {
        return parseSvaArg;
    }
    
    /// Get read not defined values
    /// \param includeSva -- add also @readsva
    const InsertionOrderSet<SValue> getReadNotDefinedValues(
                                                bool includeSva = true) const;
    const InsertionOrderSet<SValue> getReadNotInitValues() const;

    /// Get read values
    const InsertionOrderSet<SValue>& getReadValues() const;
    
    /// Get accessed values
    const InsertionOrderSet<SValue>& getAccessValues() const;
    
    /// Get read in process SVA values
    const InsertionOrderSet<SValue>& getSvaReadValues() const;

    /// Get all paths defined values
    const InsertionOrderSet<SValue>& getDefAllPathValues() const;
    
    /// Get some path defined values
    const InsertionOrderSet<SValue>& getDefSomePathValues() const;
    
    /// Get array partially define at least at one path
    const InsertionOrderSet<SValue>& getDefArrayValues() const;
    
    /// Get declared values
    const InsertionOrderSet<SValue>& getDeclaredValues() const;
    
    /// Remove single integer variables which exists in @defined 
    /// Used to remove member variables from state after preliminary CPA
    void removeDefinedValues(std::unordered_set<SValue> defined);

    /// Is the given value an array value or channel as array element, 
    /// does not work for array variable or record array
    /// \param unkwIndex -- index is non-determinable
    bool isArray(const SValue& val, bool& unkwIndex) const;

    /// Get bottommost array for given value which can be element in array or 
    /// field in record array element.
    /// \param unkwIndex -- unknown index for bottommost array
    /// \param checkRecOnly -- provide unkwIndex for record/MIF array only
    /// \return bottom array value or NO_VALUE
    SValue getBottomArrayForAny(const SValue& val, bool& unkwIndex,
                unsigned crossModule = 0, bool checkRecOnly = false) const;
    /// Get all array values for given value which can be element in array or 
    /// field in record array element.
    /// \return all array values or NO_VALUE
    std::vector<SValue> getAllMifArrays(const SValue& val, unsigned crossModule) const;
    
    /// Get topmost array and field declaration for given value which can be 
    /// element in array or field in record array element.
    /// Only one multidimensional array supported, no record array with array member
    /// \param eval -- element which specifies array 
    /// \param crossModule -- cross module/MIF border number
    /// \param decls -- field declarations of record element
    SValue getTopForAny(const SValue& eval, unsigned crossModule,
                             std::vector<const clang::ValueDecl*>& decls) const;
    
    /// Get first non-MIF module in @eval parent hierarchy
    SValue getSynthModuleValue(const SValue& eval, unsigned crossModule) const;

    /// Clear offset(s) of array element, can be applied for array and 
    /// array of records, supports multidimensional arrays and 
    /// record array with inner record 
    /// \param crossModule -- cross module/MIF border number
    /// \return first array(s) element or the same field as @val of first 
    ///         array(s) element
    SValue getFirstArrayElementForAny(const SValue& val, 
                                      unsigned crossModule = 0) const;
    
    /// Restore correct parent for record field accessed at unknown index(es)
    /// Required for multidimensional array and array in record array
    /// For T a[N][M] accessed at unknown indices there is ARR2[UNKW].a which is 
    /// corrected here to REC1.a, where state contains (ARR2[0], REC1) tuple
    void correctUnknownIndexValue(SValue& val) const;
    
    /// Get all elements of given record array recursively
    /// \return true if it is last lvalue, next one is unknown or integer
    bool getRecordArrayElements(const SValue& val, 
                               std::vector<SValue>& resvals,
                               const std::vector<const clang::ValueDecl*>& decls,
                               int declIndx) const;
    
    /// Return all elements of single/multidimensional array/record array 
    /// where the given value is stored as element
    std::vector<SValue> getAllRecordArrayElementsForAny(
                                const SValue& val, 
                                unsigned crossModule = 0) const;
    
    /// Get all fields for given record value with 
    InsertionOrderSet<SValue> getRecordFields(const SValue& recval) const;
    
    /// Get array indices for one/multidimensional array element
    /// \param eval -- element which specifies array 
    /// \param mval -- topmost array value
    /// \param indxs -- array indices, -1 for unknown index
    //void getArrayIndices(const SValue& eval, SValue& mval, 
    //                     std::vector<int>& indxs) const;
    
    /// Get all elements in sub-arrays for given array
    /// Not used for now
    std::vector<SValue> getSubArrayElements(const SValue& val) const;

    /// Compare state tuples and set NO_VALUE if tuples are different 
    void compareAndSetNovalue(ScState* other);

    /// Compare state tuples with #other
    bool compare(ScState* other) const;

    /// Set level for variable/object value, 
    /// used to remove local variables at statement/scope/function exit
    void setValueLevel(const SValue& val, unsigned level);

    /// Erase variable/object value tuples if level great than given one,
    /// used to remove local variables at statement/scope/function exit
    void removeValuesByLevel(unsigned level);
    
    /// Print levels info
    void printLevels();

    /// Clear #levels and #levelNum
    void clearValueLevels();

    /// Clear use-def results
    void clearReadAndDefinedVals();

    /// Checking for tuple with NO_VALUE in right part, assert violation if yes
    void checkNoValueTuple();
    
    /// Cross module/MIF border number for looking record array first element
    static const unsigned MIF_CROSS_NUM = 4;
    
    /// Get zero array element for unknown record/MIF array element which is given 
    /// with its field @val, if @val is reference
    /// Required for Rec/MIF method reference parameters for unknown array element
    /// to be considered as zero element to do dereference
    /// \return the correspondent field of the array zero element or @val itself
    SValue getZeroRecArrRefField(SValue val) const; 
    
    /// Is @val a field in any record including record array field
    /// \return parent if the value is a field or NO_VALUE
    bool isRecField(const SValue& val) const;

public:    
    /// Check is @val is field of local record
    static bool isLocalRecField(const SValue& val);

    /// Get name prefix for local record
    static std::string getLocalRecName(const SValue& val);
    
    /// Is constant variable/object or field of local record is constant variable/object
    /// Field of constant record array is not considered as it does not make sense
    static bool isConstVarOrLocRec(const SValue& val);
    
    /// Compare two state, should not be used, use compare() instead 
    static bool compareStates(const ScState* bigger, const ScState* other);
  
    /// Get APSInt from ValueView
    static llvm::APSInt getIntFromView(bool isSigned, sc_elab::ValueView valueView);

    /// Check if value is candidate to localparam: non-constant, member variable, 
    /// scalar of integral type, not defined in this process
    static bool isMemberPrimVar(const SValue& val, const ScState* state);
    
};

}

#endif /* SCSTATE_H */

