/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Code writer for SC statements.
 *
 * Author: Mikhail Moiseev
 */

#ifndef SCVERILOGWRITER_H
#define SCVERILOGWRITER_H

#include "sc_tool/cfg/ScState.h"
#include "llvm/ADT/Optional.h"

namespace std {

template<> 
struct hash< std::pair<sc::SValue, bool> >
{
    std::size_t operator () (const std::pair<sc::SValue, bool>& obj) const;
};

template<> 
struct hash< std::pair<std::pair<sc::SValue, bool>, sc::SValue> >
{
    std::size_t operator () (
        const std::pair<std::pair<sc::SValue, bool>, sc::SValue>& obj) const;
};

template<> 
struct hash< std::pair<std::string, sc::SValue> >
{
    std::size_t operator () (const std::pair<std::string, 
                             sc::SValue>& obj) const;
};

}

namespace sc {
    
enum class CastSign {
    NOCAST = 0, 
    UCAST = 1, 
    SCAST = 2,  // original signed cast added in ImplicitCast operator
    SACAST = 3  // artificially signed cast added in VerilogWriter, signed+1bit
};
    
enum class ExprSign {
    NOEXPR = 0, 
    SEXPR = 1, 
    UEXPR = 2,
    LEXPR = 3
};

/// Term string, flags and range
struct TermInfo 
{
    // <readName, writeName>
    std::pair<std::string, std::string> str;
    bool        isChannel;      // Used to choose non-blocking assignment
    // General width of any kind of term:
    // - Literal width based on value bit needed
    // - Variable width based on type width 
    // - Expression width evaluated from argument widths, 0 if unknown
    size_t      exprWidth;      // Used to avoid index in partial select and 
                                // for type extension in some binary operations
    size_t      minCastWidth;   // From explicit cast, minimal width to narrow value
    size_t      lastCastWidth;  // From explicit cast/extend width/replace variable, 
                                // last cast for concatenation, applied depends on
                                // @explCast for @getExprWidth or make literal only 
    bool        explCast;       // Explicit cast, used to apply SACAST and skip
                                // NOCAST -> SCAST transformation
    CastSign    castSign;       // From implicit/explicit casts, signedness for
                                // literals/variables/expressions 
    ExprSign    exprSign;       // Signed/unsigned expression (unary/binary) flag
    char        literRadix;     // Term literal radix or 0 if not literal
    bool        simplTerm;      // Term can be used w/o brackets anywhere
    bool        incrWidth;      // Increase result width of operand width, 
                                // some binary and unary operators       
    
    TermInfo(const std::pair<std::string, std::string>& s, 
             size_t exprWidth_, bool isChannel_) : 
        str(s), isChannel(isChannel_), 
        exprWidth(exprWidth_), minCastWidth(0), lastCastWidth(0), explCast(false),
        castSign(CastSign::NOCAST), exprSign(ExprSign::NOEXPR), 
        literRadix(0), simplTerm(true), incrWidth(false)
    {}
};

/// State sensitive part of @ScVerilogWriter which need to be stored/restored
/// for analysis finish/start at @wait() 
struct ScVerilogWriterContext
{
    /// Record name string, used in record method call as field prefix/suffix,
    /// required to provide indices as they can be not stored in field value 
    /// <record value, record expression>
    std::pair<SValue, std::string> recordValueName;
    /// Expression/variable strings for reference type value
    std::unordered_map<SValue, std::pair<std::string, 
                                         std::string> > refValueDecl;
    /// Expression/variable strings for pointer type value
    std::unordered_map<SValue, std::pair<std::string, 
                                         std::string> > ptrValueDecl;
};

/// Verilog code generator
class ScVerilogWriter {

public:

    explicit ScVerilogWriter(
                const clang::SourceManager& sm_,
                bool isCombProcess_,
                const std::unordered_map<SValue, std::string>& extrValNames_,
                const std::unordered_map<SValue, 
                const VerilogVarTraits>& varTraits_,
                const std::pair<std::string, std::string>& waitNVarName_) : 
        sm(sm_), 
        isCombProcess(isCombProcess_),  
        extrValNames(extrValNames_),
        varTraits(varTraits_),
        waitNVarName(waitNVarName_)
    {
        using namespace std;
    
        // Fill all external names
        for (const auto& i : extrValNames) {
            extrNames.insert(i.second);
        }
        for (const auto& i : varTraits) {
            if (i.second.currName) {
                extrNames.insert(i.second.currName.getValue());
            }
            if (i.second.nextName) {
                extrNames.insert(i.second.nextName.getValue());
            }
        }
        
        /*cout << "---------- extrNames -----------" << endl;
        for (auto&& i : extrValNames) {
            cout << i.first << " : " << i.second << endl;
        }
        cout << "--------------------------------" << endl;*/
    }
    
    /// Copy constructor from module code write which contains some non-local
    /// variable and their declarations
    explicit ScVerilogWriter(const ScVerilogWriter& rhs) :
        sm(rhs.sm),
        isCombProcess(rhs.isCombProcess),
        isClockThreadReset(rhs.isClockThreadReset),
        extrValNames(rhs.extrValNames),
        varTraits(rhs.varTraits),
        extrNames(rhs.extrNames),
        waitNVarName(rhs.waitNVarName),
        varNameIndex(rhs.varNameIndex),
        varIndex(rhs.varIndex)
    {}    
    
    virtual ~ScVerilogWriter() {}
    
    /// Set skip put term mode, @terms not updated by put... functions
    void setSkipTerm(bool skip) {
        SCT_TOOL_ASSERT (skip || skipTerm > 0,
                         "Clear skip term if it is already zero");
        skipTerm = skip ? skipTerm+1 : skipTerm-1;
    }
    
    /// Set clocked thread reset section
    void setResetSection(bool isReset) {
        isClockThreadReset = isReset;
    }
    
    /// Set start of assert arguments parsing
    void setParseSvaArg(bool isAssert) {
        parseSvaArg = isAssert;
    }
    bool isParseSvaArg() {
        return parseSvaArg;
    }
    
    /// Set skip sign cast mode
    void setSkipSignCast(bool skipCast) {
        skipSignCast = skipCast;
    }
    bool isSkipSignCast() {
        return skipSignCast;
    }
    
    /// Get member combinational variables assigned/used in reset section
    const InsertionOrderSet<SValue>& getExtrCombVarUsedRst() const {
        return extrCombVarUsedRst;
    }

    /// Clear to do not have these declaration in @always_comb process
    void clearExtrCombVarUsedRst() {
        extrCombVarUsedRst.clear();
    }
    
    /// Update external names from constant propagator, required for static constants
    void updateExtrNames(const std::unordered_map<SValue, std::string>& valNames);
    
    /// Update external name strings after process analysis, required to add 
    /// empty sensitive METHOD local variables to provide name uniqueness
    void updateExtrNames(const std::unordered_set<std::string>& valNames);
    
    std::unordered_set<std::string>& getExtrNames() {
        return extrNames;
    }
    
    /// Set @forLoopInit
    void setForLoopInit(bool mode) {
        forLoopInit = mode;
    }
    
    bool isForLoopInit() {
        return forLoopInit;
    }
    
    /// Set empty sensitivity for method process
    void setEmptySensitivity(bool empty) {
        emptySensitivity = empty;
    }
    
    /// Is empty sensitivity for method process
    bool isEmptySensitivity() {
        return emptySensitivity;
    }
    
    //=======================================================================
protected:    
    /// Get canonical type string for the given @type, return element type for array
    std::string getVarDeclVerilog(const clang::QualType& type, 
                                  const std::string& varName,
                                  const clang::Stmt* init = nullptr);
    
    /// Get index to provide unique name for local variable
    std::string getUniqueName(const std::string& origVarName);

    // Check if variable value is not registered in varTraits and extrValNames 
    bool isLocalVariable(const SValue& val);
    
    /// Get unique read and write names for variable in scope
    /// \param recvar -- used to provide unique name inside of record instance,
    ///                  required for inner records only
    /// \return <readName, writeName>
    std::pair<std::string, std::string> getVarName(const SValue& val);
    
    /// Get concatenated name string for given vector of record variables
    //std::string getRecordName(const std::vector<SValue>& recvars);
    
    /// Get indices suffix from record name or empty string
    std::string getIndexFromRecordName(std::string recName) const;
    
    /// Get name for ScObject, used for port/signal
    /// \param cval -- channel value
    /// \return <readName, writeName>
    std::pair<std::string, std::string> getChannelName(const SValue& cval);
    
    /// Return true if @val is register in @varTraits, only in splitting thread mode
    bool isRegister(const SValue& val);

    /// Check @sc_comb_sig with CLEAR flag 
    bool isCombSig(const SValue& val);
    
    /// Check @sc_comb_sig without CLEAR flag 
    bool isCombSigClear(const SValue& val);

    // Calculate outlined brackets number
    unsigned getBracketNum(const std::string& s);
    
    /// Check if string is in brackets, return true if it is
    bool isTermInBrackets(const std::string& s);
    
    /// Remove all leading "(" and tailing ")" brackets in the given string
    /// Returns the same string reference
    std::string removeBrackets(const std::string& s);
    
    /// Remove minus at first position
    std::string removeLeadMinus(const std::string& s);
    /// Add minus at first position
    std::string addLeadMinus(const std::string& s);

    /// Remove cast prefix up to "'" char
    //std::string removeCastPrefix(const std::string& s);
    
    /// Extract signed or unsigned value from given literal string 
    /// \return <unsigned, signed> optional values
    std::pair<llvm::Optional<uint64_t>, llvm::Optional<int64_t>> getLiteralVal(
                                            const std::string& literStr);

    /// Get absolute value of given literal statements
    uint64_t getLiteralAbs(const clang::Stmt* stmt);

    /// Make literal term string in sized form if required
    /// \param addNegBrackets -- add brackets for negative literal
    std::string makeLiteralStr(const std::string& literStr, char radix,
                               size_t minCastWidth, size_t lastCastWidth,
                               CastSign castSign, bool addNegBrackets);
    
    /// Make non-literal term string with sign cast if required
    /// \param castSign -- sign cast applied to add @signed
    std::string makeTermStr(const std::string& termStr, size_t minCastWidth, 
                            size_t lastCastWidth, CastSign castSign);

    /// Get @stmt string as RValue, cast optionally applied
    /// \param skipCast       -- do not add cast for non-literal, 
    ///                          required for bit/range select argument
    /// \param addNegBrackets -- add brackets for negative literal, 
    ///                          used for binary, unary 
    std::pair<std::string, std::string> getTermAsRValue(
                                            const clang::Stmt* stmt, 
                                            bool skipCast = false, 
                                            bool addNegBrackets = false,
                                            bool doSignCast = false,
                                            bool doConcat = false);
    
    /// Put/replace string into @terms
    void putString(const clang::Stmt* stmt, 
                   const TermInfo& info);

    /// Put/replace string into @terms with given flags
    void putString(const clang::Stmt* stmt, 
                   const std::pair<std::string, std::string>& s, 
                   size_t exprWidth, bool isChannel = false);

    /// Put/replace the same string into @terms with empty flags and no range
    void putString(const clang::Stmt* stmt, const std::string& s, 
                   size_t exprWidth, bool isChannel = false);
    
    /// Add string into @terms string with empty flags, no range and no channel
    void addString(const clang::Stmt* stmt, const std::string& s);
    
    void clearLiteralTerm(const clang::Stmt* stmt);
    
    // Clear @simpleTerm for given stmt, 
    // that means this term needs brackets in compound assignment
    void clearSimpleTerm(const clang::Stmt* stmt);
    
    void setExprSign(const clang::Stmt* stmt, bool sign, bool liter = false);

    /// Set incrWidth for given stmt, 
    /// that means result width is bigger than operand data width 
    void setIncrWidth(const clang::Stmt* stmt);

    /// Put assignment string, record field supported
    void putAssignBase(const clang::Stmt* stmt, const SValue& lval, 
                      std::string lhsName, std::string rhsName, 
                      size_t width, bool isChannel = false);
    
//============================================================================

public:
    /// Get incrWidth for given stmt
    bool isIncrWidth(const clang::Stmt* stmt) const;

    /// Get expression data width from @exprWidth 
    /// \param doConcat -- get expression width for concatenation
    /// \return @exprWidth for given statement or 0 if width unknown
    size_t getExprWidth(const clang::Stmt* stmt, bool doConcat = false);
    
    /// Get minimal width to avoid part/bit selection outside of variable width
    /// \return @exprWidth for given statement or 0 if width unknown
    size_t getMinExprWidth(const clang::Stmt* stmt);
    
    /// Get expression data width from @lastCast or @exprWidth after that or 
    /// type information at the end
    /// \param getMinWidth -- get minimal width of @minCast and @exprWidth
    /// \return expression/type width or 64 with error reporting
    size_t getExprTypeWidth(const clang::Expr* expr, size_t defWidth = 64);

    /// Get minimal expression data width as minimal of @minCast and @exprWidth
    /// \return expression/type width or 64 with error reporting
    size_t getMinExprTypeWidth(const clang::Expr* expr, size_t defWidth = 64);

    /// Get record array indices string
    std::string getRecordIndxs(const std::vector<SValue>& recarrs);
    
    /// Used for statements which produces nothing, like @ImplicitCastExpr
    void copyTerm(const clang::Stmt* srcStmt, const clang::Stmt* stmt);

    /// Append term string of scrStmt to existing string of stmt, 
    /// used for record parameter which has multiple fields
    void addTerm(const clang::Stmt* srcStmt, const clang::Stmt* stmt);

    /// Used for statements in brackets
    void copyTermInBrackets(const clang::Stmt* srcStmt, const clang::Stmt* stmt);

    /// Used to remove brackets, for example for channel access
    void copyTermRemoveBrackets(const clang::Stmt* srcStmt, 
                                const clang::Stmt* stmt);
    
    /// Used for explicit type cast statements, inheritors of @ExplicitCastExpr
    void putTypeCast(const clang::Stmt* srcStmt, const clang::Stmt* stmt,
                     const clang::QualType& type);

    /// Set cast width for variables/expressions replaced by value,
    /// used in concatenation
    void setReplacedCastWidth(const clang::Stmt* stmt, 
                              const clang::QualType& type);

    /// Extend type width for arithmetic operation  argument self-determined in SV,
    /// this is type cast to given @width
    void extendTypeWidth(const clang::Stmt* stmt, const size_t width);
    
    /// Used for literals
    void putLiteral(const clang::Stmt* stmt, const SValue& val);
    
    /// Put local variable (non-array) declaration with possible initialization
    /// \param init -- initialization expression, can be @nullptr
    /// \param replaceConstEnable -- constant variable declaration can be not
    ///                              generated if it replaced with value,
    ///                              that possible for non-reference constants 
    ///                              with evaluated initialization value
    void putVarDecl(const clang::Stmt* stmt, const SValue& val, 
                    const clang::QualType& type, const clang::Expr* init,
                    bool replaceConstEnable = false);

    /// Array declaration statement w/o initialization 
    void putArrayDecl(const clang::Stmt* stmt, const SValue& val, 
                      const clang::QualType& type, 
                      const std::vector<std::size_t>& arrSizes);
    
    /// Put string of @init statement to use instead of the reference variable
    /// Used for any non-constant reference 
    void storeRefVarDecl(const SValue& val, const clang::Expr* init);

    /// Put local reference variable (non-array) declaration with initialization
    /// Used for rval temporary variable with constant reference value only
    void storeRefVarDecl(const SValue& val, const SValue& rval);
    
    /// Put string of @init statement to use instead of the pointer variable
    /// Used for array of pointers at unknown index, it cannot be de-referenced
    void storePointerVarDecl(const SValue& val, const clang::Expr* init);

    /// Put string for pointer variable into @ptrValueDecl, 
    /// @rval can be pointee variable or another pointer as well
    void storePointerVarDecl(const SValue& val, const SValue& rval, 
                           const SValue& cval = NO_VALUE);
    
    /// Any access to member/local variable/local record field variable 
    /// and any other expression
    /// \param recarr -- record array, used to get indices string
    void putValueExpr(const clang::Stmt* stmt, const SValue& val,
                      const SValue& recarr = NO_VALUE,
                      bool elemOfMifArr = false, bool elemOfRecArr = false,
                      const std::string& refRecarrIndxStr = "");
     
    /// Any access to member/local variable/local record field variable 
    /// and any other expression
    /// \param recarrs -- vector of record arrays, used to get indices string
    /// \param elemOfMifArr -- put member of current element of MIF array 
    /// \param elemOfRecArr -- put member of a element of a record/MIF array 
    void putValueExpr(const clang::Stmt* stmt, const SValue& val, 
                      const std::vector<SValue>& recarrs, 
                      bool elemOfMifArr, bool elemOfRecArr,
                      const std::string& refRecarrIndxStr = "");

    /// Any access to channel
    /// \param recarr -- record array, used to get indices string
    void putChannelExpr(const clang::Stmt* stmt, const SValue& val,
                        const SValue& recarr = NO_VALUE,
                        bool elemOfMifArr = false, bool elemOfRecArr = false);

    /// Any access to channel
    /// \param recarrs -- vector of record arrays, used to get indices string
    /// \param elemOfMifArr -- put member of current element of MIF array 
    /// \param elemOfRecArr -- put member of a element of a record array 
    void putChannelExpr(const clang::Stmt* stmt, const SValue& cval,
                        const std::vector<SValue>& recarrs, 
                        bool elemOfMifArr, bool elemOfRecArr);
    
    /// Try to put local reference type variable, return true if @val is 
    /// local reference
    bool putLocalRefValueExpr(const clang::Stmt* stmt, const SValue& val);
    
    /// Try to put local pointer parameter variable, return true if @val is 
    /// local pointer parameter
    bool putLocalPtrValueExpr(const clang::Stmt* stmt, const SValue& val);
    
    /// Assignment statement, used for general purpose
    void putAssign(const clang::Stmt* stmt, const SValue& lval, 
                   const clang::Expr* lhs, const clang::Expr* rhs);
    
    /// Assignment statement, for non-channel @lval only 
    /// Use when there is no expression for @lval, i.e. if @lval is artificial
    void putAssign(const clang::Stmt* stmt, const SValue& lval, 
                   const clang::Expr* rhs);
    
    /// Assignment for record variable (record copy)
    /// \param lvar & lrec -- LHS record variable and record value
    /// \param rvar & rrec -- RHS record variable and record value
    /// \param lrecSuffix & rrecSuffix -- LHS an RHS record indices suffix
    void putRecordAssign(const clang::Stmt* stmt, 
                         const SValue& lvar, const SValue& lrec, 
                         const SValue& rvar, const SValue& rrec,
                         const std::string& lrecSuffix,
                         const std::string& rrecSuffix);
    
    /// Put array element initialization, used for array initializer list for 
    /// local variable
    void putArrayElemInit(const clang::Stmt* stmt, const SValue& bval, 
                          const SValue& ival, const clang::Expr* iexpr);
    
    /// Put array element initialization with zero
    /// \param bval -- array variable
    /// \param ival -- array index integer
    void putArrayElemInitZero(const clang::Stmt* stmt, const SValue& bval, 
                              const SValue& ival);
    
    /// Add array subscript index into @arraySubIndices
    void addSubscriptIndex(const SValue& bval, const clang::Expr* indx);
    
    /// Get string from indices "[index1][index2]..." stored in @arraySubIndices
    std::string getIndexString(const SValue& val);

    /// Array index access operator []
    /// \param base  -- base expression 
    /// \param index -- index expression 
    void putArrayIndexExpr(const clang::Stmt* stmt, const clang::Expr* base,
                           const clang::Expr* index);
    
    /// Constant and variable based range part-select
    /// \param useDelta -- use "+:" if true, or ":" if false 
    void putPartSelectExpr(const clang::Stmt* stmt, const SValue& val,
                           const clang::Expr* base,
                           const clang::Expr* hindx, 
                           const clang::Expr* lindx,
                           bool useDelta);

    /// Constant and variable based bit part-select
    /// @param base  -- base expression 
    /// @param index -- index expression 
    void putBitSelectExpr(const clang::Stmt* stmt, const SValue& val,
                          const clang::Expr* base,
                          const clang::Expr* index);
    
    /// Binary operators "+", "-", "*", "/", "%", "||", "&&", "&", "|", "^",
    /// "<<", ">>", >", "<", ">=", "<=", "==", "!="
    void putBinary(const clang::Stmt* stmt, std::string opcode, 
                   const clang::Expr* lhs, const clang::Expr* rhs);
    
    /// Compound assignment operators "+=", "-=", "*=", "/=", "%=", "&=", "|=",
    /// "^=", "<<=", ">>="
    void putCompAssign(const clang::Stmt* stmt, std::string opcode, 
                       const SValue& lval,
                       const clang::Expr* lhs, const clang::Expr* rhs);
    
    /// Unary operators "++", "--", ...
    void putUnary(const clang::Stmt* stmt, std::string opcode, 
                  const clang::Expr* rhs, bool isPrefix = true);
    
    /// Put sign cast for literals and expressions
    void putSignCast(const clang::Stmt* stmt, CastSign castSign);
    
    /// Ternary statement ?
    void putCondStmt(const clang::Stmt* stmt, const clang::Stmt* cond, 
                     const clang::Stmt* lhs, const clang::Stmt* rhs);
    
    /// Concatenation statement
    void putConcat(const clang::Stmt* stmt, const clang::Stmt* first, 
                   const clang::Stmt* second);

    /// Put function call parameter initialization with the corresponding argument
    /// \param stmt is function call expression 
    void putFCallParam(const clang::Stmt* stmt, const SValue& pval, 
                       const clang::Expr* arg);
    
    /// Put empty string for function w/o parameters, required for function call
    /// in right part of && / || expression
    void putEmptyFCallParam(const clang::Stmt* stmt);
    
    /// Put wait(int n) counter assignment
    void putWaitNAssign(const clang::Stmt* stmt, const clang::Expr* waitn);
    
    void putClockEdge(const clang::Stmt* stmt, const clang::Stmt* clock, 
                      bool posEdge, bool negEdge);
    
    /// Put SVA for @sct_assert
    void putAssert(const clang::Stmt* stmt, const clang::Expr* arg,
                   const std::string& msgStr);
    
    /// Put temporal assertion in clock thread 
    void putTemporalAssert(const clang::Stmt* stmt, 
                           const clang::Expr* lhs,
                           const clang::Expr* rhs,
                           const std::string& timeStr,
                           const clang::Expr* event = nullptr);

    
    //=========================================================================
    
    /// Clear before next statement 
    void startStmt();

    /// Remove statement from terms
    void clearStmt(const clang::Stmt* stmt);
    
    void printTerms() {
        using namespace std;
        cout << "Terms -----------------" << hex << endl;
        for (auto i : terms) {
            cout << "  " << i.first << endl;
        }
        cout << dec;
    }

    
    /// Get string for @stmt, which may be sub-expression
    /// \return expression string to read
    llvm::Optional<std::string> getStmtString(const clang::Stmt* stmt);

    /// Get string for IF statement
    std::string getIfString(const clang::Expr* cexpr);

    /// Get std::string for SWITCH statement
    std::string getSwitchString(const clang::Expr* cexpr);

    /// Get std::string for FOR statement
    std::string getForString(const clang::Stmt* init, const clang::Expr* cexpr, 
                             const clang::Expr* incr);

    /// Get std::string for WHILE statement
    std::string getWhileString(const clang::Expr* cexpr);

    /// Store break statement
    std::string getBreakString();

    /// Store continue statement
    std::string getContinueString();
    
    std::string getTabSymbol() {
        return TAB_SYM;
    }
    
    //=========================================================================

    // Control @keepArrayIndices flag
    bool isKeepArrayIndices() {return keepArrayIndices;}
    void setKeepArrayIndices() {keepArrayIndices = true;}
    void resetKeepArrayIndices() {keepArrayIndices = false;}
    
    void setRecordName(const SValue& val, std::string str) {
        recordValueName = std::pair<SValue,std::string>(val, str);
    }
    std::pair<SValue,std::string> getRecordName() {
        return recordValueName;
    }
    void setMIFName(const SValue& val, std::string str) {
        MIFValueName = std::pair<SValue,std::string>(val, str);
    }
    std::pair<SValue,std::string> getMIFName() {
        return MIFValueName;
    }
    
    //=========================================================================
    /// Print local variable declaration, used in @always_comb 
    void printLocalDeclaration(std::ostream &os, const ScState* state);

    /// Print local combinational variable declaration, 
    /// used in @always_ff reset section
    void printResetCombDecl(std::ostream &os);
        
    /// Print variable declaration for given variable
    void printDeclString(std::ostream &os, const SValue& val, 
                         const std::string& sizeSuff = "");
    
    /// Get declared variables/constants not replaced by integer values
    inline std::unordered_set<SValue> getNotReplacedVars() {
        return notReplacedVars;
    }
    
    /// Get initializer statement for constant/variables locally declared
    inline std::unordered_map<SValue, 
           std::unordered_set<const clang::Stmt*>> getVarAssignStmts() {
        return varAssignStmts;
    }
    
    /// Register assignment statement, for declared variables only
    /// \param val -- variable/constant value
    /// \param stmt -- initializer statement for the variable/constant, may be null 
    inline void putVarAssignStmt(const SValue& val, const clang::Stmt* stmt) {
        if (stmt) {
            auto i = varAssignStmts.find(val);
            if (i == varAssignStmts.end()) {
                varAssignStmts.emplace(val, std::unordered_set<
                                       const clang::Stmt*>({stmt}));
            } else {
                i->second.insert(stmt);
            }
        }
    }
    
    //=========================================================================
    /// Store this state in finish of analysis at wait()
    ScVerilogWriterContext serialize() {
        ScVerilogWriterContext ctx;
        ctx.recordValueName = recordValueName;
        ctx.refValueDecl = refValueDecl;
        ctx.ptrValueDecl = ptrValueDecl;
        return ctx;
    }
    
    /// Restore this state in start of analysis after wait() 
    void deserialize(const ScVerilogWriterContext& ctx) {
        recordValueName = ctx.recordValueName;
        refValueDecl = ctx.refValueDecl;
        ptrValueDecl = ctx.ptrValueDecl;
    }
    
protected:
    /// Blocking and non-blocking assignment symbols: "=" and "<="
    const std::string ASSIGN_SYM = " = ";
    const std::string NB_ASSIGN_SYM = " <= ";
    const std::string ASSIGN_STMT_SYM = "assign ";
    /// Constructed name suffix symbol
    const char NAME_SUFF_SYM = '_';
    /// Next variable in thread suffix
    const std::string NEXT_VAR_SUFFIX = "_next";
    /// Verilog keyword suffix
    const std::string VERILOG_KEYWORD_SUFFIX = "_v";
    // Tabulation
    std::string TAB_SYM = "    ";

    const clang::SourceManager    &sm;
    
    /// Combinatorial process
    const bool isCombProcess;
    /// Is reset section of clocked thread process
    bool isClockThreadReset = false;
    /// Start of assert in process flag used to provide next name for non-channels
    bool parseSvaArg = false;
    /// Skip term mode, @terms not updated by put... functions
    /// This is counter to consider multiple calls of setSkipTerm function
    unsigned skipTerm = 0;
    /// No sign cast mode, used for array/partial select indices
    bool skipSignCast = false;
    /// FOR loop initialization mode
    bool forLoopInit = false;
    /// Method with NO sensitivity list, 
    /// Verilog assignment statement generated for such method
    bool emptySensitivity = false;
    /// Keep array indices, i.e. do not erase them from @arraySubIndices in 
    /// getIndexString(), required for record array with inner record access
    bool keepArrayIndices = false;
    /// Record name string, used in record method call as field prefix/suffix,
    /// required to provide indices as they can be not stored in field value 
    /// <record value, record expression>
    std::pair<SValue, std::string> recordValueName{NO_VALUE, ""};
    /// MIF name string, used in access MIF members from its process,
    /// set up at start of MIF process generation
    /// <record value, record expression>
    std::pair<SValue, std::string> MIFValueName{NO_VALUE, ""};

    /// Variable value to external name collection
    std::unordered_map<SValue, std::string> extrValNames;
    /// Verilog properties of SValues including names
    std::unordered_map<SValue, const VerilogVarTraits> varTraits;
    /// All external names, used to check uniqueness of local variable name
    std::unordered_set<std::string> extrNames;
    /// Name of automatically-generated counter variable used for wait(N)
    std::pair<std::string, std::string> waitNVarName;
    
    /// Current statement terms(sub-statements) and pair of <string, arrayFCall> 
    std::unordered_map<const clang::Stmt*, TermInfo> terms;
    /// Variable name index, <variable name, last used index> 
    std::unordered_map<std::string, unsigned> varNameIndex;
    /// Variable value name dictionary, <<variable, is next>, name index> 
    std::unordered_map<std::pair<SValue, bool>, std::string> varIndex;
    /// Declaration for local variables
    std::vector< std::pair<SValue, std::string> > localDeclVerilog;
    /// Variables and constants not replaced by integer values, 
    /// in future any objects which name is used in generated code
    std::unordered_set<SValue> notReplacedVars;
    /// Variable assignment statements, used to remove variable initialization 
    /// for removed variables/constants, currently for local variables only
    std::unordered_map<SValue, std::unordered_set<
                       const clang::Stmt*>> varAssignStmts;
    /// Member combinational variables assigned/used in reset section, 
    /// stored to add local declaration in reset section to distinguish it
    /// from module declaration to avoid multiple process variable modification
    InsertionOrderSet<SValue> extrCombVarUsedRst;
    /// Array indices for current subscript statement, 
    /// used for record/MIF array access
    std::vector<std::pair<SValue, const clang::Stmt*> >  arraySubIndices;
    /// Expression/variable strings for reference type value
    std::unordered_map<SValue, std::pair<std::string, 
                                         std::string> > refValueDecl;
    /// Expression/variable strings for pointer type value
    std::unordered_map<SValue, std::pair<std::string, 
                                         std::string> > ptrValueDecl;
    
    /// LHS names assigned in empty sensitive methods, used to report duplicate
    /// assignment error
    std::unordered_set<std::string> emptySensLhsNames;
};

}

#endif /* SCVERILOGWRITER_H */

