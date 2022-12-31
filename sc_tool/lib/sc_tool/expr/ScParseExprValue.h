/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * SC Clang front-end project.
 * Traverse CFG with value extraction abstract class.
 *  
 * File:   ScTraverseValue.h
 * Author: Mikhail Moiseev
 */

#ifndef SCPARSEEXPRVALUE_H
#define SCPARSEEXPRVALUE_H

#include "sc_tool/expr/ScParseExpr.h"
#include "llvm/ADT/APSInt.h"

namespace sc {
/// Statement with last function call, <Call statement, Stmt>
typedef std::pair<const clang::Stmt*, const clang::Stmt*>     ContextStmt;
}

namespace std {
/// Hash function for @ContextStmt
template<>
struct hash<sc::ContextStmt>
{
    std::size_t operator () (const sc::ContextStmt& obj) const {
        using std::hash;
        return ( std::hash<void*>()((void*)obj.first)^
            std::hash<void*>()((void*)obj.second) );
    }
};
} // namespace std

namespace sc {

/// Evaluates statements and expressions and puts results to State.
/// This class does not traverse CFG, CFG traversal should be implemented in
/// derived classes.
class ScParseExprValue : public ScParseExpr
{
protected:
    /// Evaluation precision 
    uint32_t EPRECISION = 64;

    /// Current function 
    const clang::FunctionDecl* funcDecl = nullptr;

    /// Current top level statement analyzed
    clang::Stmt* currStmt = nullptr;
    /// Current statements properties
    /// Current statement is mandatory required, cannot be removed
    bool isRequiredStmt;
    bool isUserCallStmt;
    bool isAssignStmt;
    bool isSideEffStmt;
    
    /// Statements for variable value which define/use the variable
    std::unordered_map<SValue, std::unordered_set<clang::Stmt*>> defVarStmts;
    std::unordered_map<SValue, std::unordered_set<clang::Stmt*>> useVarStmts;
    
    /// Constant evaluation mode for CPA or code generation, prevent user 
    /// defined function calls from @evaluateConstInt()/@checkConstRefValue()
    bool evaluateConstMode = false;
    
    /// Processing module scope SCT_ASSERT
    bool moduleSctAssert = false;
    
    /// The following loop is alive, i.e. has at least one iteration
    bool aliveLoop = false;

    /// Ignore stored statement value @stmtStoredValue in 
    /// ScTraverseConst::chooseExprMethod(), used in CPA to get 
    /// variable value for function argument
    bool ignoreStored = false;
    
    /// Variable registered as latched by @sct_assert_latch()
    std::unordered_set<SValue>    assertLatches;
    
    /// Parsed condition value stored to avoid re-parse in &&/|| and ?
    std::unordered_map<clang::Stmt*, SValue>   condStoredValue;
    
    /// Current function has one return from function scope and return statement
    bool simpleReturnFunc = true;
    clang::Stmt* returnStmtFunc = nullptr;
    /// Current function and all called functions change some non-local
    /// variables/channels through parameters or directly
    bool sideEffectFunc = false;

public:
    explicit ScParseExprValue(const clang::ASTContext& context_,
                              std::shared_ptr<ScState> state_, 
                              bool isCombProcess_,
                              const SValue& modval_, 
                              bool checkNoValue_ = true,
                              SValue recval_ = NO_VALUE) : 
        ScParseExpr(context_, state_, isCombProcess_, modval_, checkNoValue_, recval_)
    {}

    virtual ~ScParseExprValue() {}
    
    /// Wrappers around similar methods of @state, 
    /// register variable in @defVarStmts/@useVarStmts
    void declareValue(const SValue& var);
    void writeToValue(const SValue& lval, bool isDefined = false);
    void readFromValue(const SValue& lval);

    /// Parse SCT_ASSERT in module scope to fill read value
    void parseSvaDecl(const clang::FieldDecl* fdecl);
    
    /// Set evaluation precision in bit
    void setEvalPrecision(uint32_t val) { EPRECISION = val; }
    
    /// Parse and evaluate one expression/statement as constant integer
    /// \param checkConst -- get value for variable if it is constant only
    ///                      used in ScTraverseProc, not used in ScTraverseConst
    /// \param checkRecOnly -- get zero instead of unknown index for record/MIF array
    /// \return <result, integer value of result>
    std::pair<SValue, SValue> evaluateConstInt(SValue val, bool checkConst,
                                            bool checkRecOnly);
    std::pair<SValue, SValue> evaluateConstInt(clang::Expr* expr, 
                                            bool checkConst = true,
                                            bool checkRecOnly = false) override;
    
    /// Try to get integer from state, return NO_VALUE if not.
    SValue evaluateConstInt(clang::Expr* expr, const SValue& val, 
                            bool checkConst = true) override;
    
    /// Store ternary statement condition for SVA property
    virtual void putSvaCondTerm(const clang::Stmt* stmt, SValue val) {}
    
    /// Check if @val is integer or any kind of RValue 
    /// (constant, constant array element)
    bool checkConstRefValue(SValue val);

    /// Parse function call or constructor call parameters, 
    /// fill state with parameters with correspondent argument values
    void prepareCallParams(clang::Expr* expr, const SValue& funcModval, 
                           const clang::FunctionDecl* callFuncDecl) override;

    void setModuleSctAssert() {
        moduleSctAssert = true;
    } 
    
    /// Return registered latches by @sct_assert_latch()
    const std::unordered_set<SValue> getAssertLatches() const {
        return assertLatches;
    }
    
protected:    
    
    /// Declared variable may be initialized with expression inside
    /// Used for variable declaration with and without initialization
    /// \param initExpr -- record/record field initializer, 
    ///                    not used for normal variables
    void parseDeclStmt(clang::Stmt* stmt, clang::ValueDecl* decl, 
                       SValue& val, clang::Expr* initExpr) override;

    /// Used for local variables usage like assignment statement in left/right parts
    void parseExpr(clang::DeclRefExpr* expr, SValue& val) override;
    
    // Any access of member variable
    void parseExpr(clang::MemberExpr* expr, SValue& val) override;
    
    /// Used for default initializer in constructor or in aggregate initialization
    void parseExpr(clang::CXXDefaultInitExpr* expr, SValue& val) override;

    /// Used for default initializer in constructor or in aggregate initialization
    /// T{}
    void parseExpr(clang::InitListExpr* expr, SValue& val) override;
    
    /// Used for construction temporary record object 
    /// T()
    void parseExpr(clang::CXXTemporaryObjectExpr* expr, SValue& val) override;

    /// CXX constructor
    void parseExpr(clang::CXXConstructExpr* expr, SValue& val) override;

    /// Operator @new and @new[]
    void parseExpr(clang::CXXNewExpr* expr, SValue& val) override;
        
    /// Operator delete
    void parseExpr(clang::CXXDeleteExpr* expr, SValue& val) override;
    
    /// Common function for operator[] in @ArraySubscriptExpr and @CXXOperatorCall
    SValue parseArraySubscript(clang::Expr* expr, const SValue& bval, 
                               const SValue& ival);
    /// Array index access operator []
    void parseExpr(clang::ArraySubscriptExpr* expr, SValue& val) override;
    
    /// Used for explicit/implicit type cast and LValue to RValue cast
    void parseImplExplCast(clang::CastExpr* expr, SValue& rval, SValue& val);
    void parseExpr(clang::ImplicitCastExpr* expr, SValue& rval, SValue& val) override;
    void parseExpr(clang::ExplicitCastExpr* expr, SValue& rval, SValue& val) override;
    
    /// General binary statements including assignment
    /// Parse statement and run evalSubExpr for each operand
    void parseBinaryStmt(clang::BinaryOperator* stmt, SValue& val) override;

    /// Parse statement and run evalSubExpr for each operand
    void parseCompoundAssignStmt(clang::CompoundAssignOperator* stmt, SValue& val) override;

    /// Parse statement and run evalSubExpr for the operand
    void parseUnaryStmt(clang::UnaryOperator* stmt, SValue& val) override;

    /// Parse unary increment/decrement and operator ++/--
    SValue parseIncDecStmt(const SValue& rval, bool isPrefix, bool isIncrement);
    
    /// Ternary operator (...) ? ... : ...
    void parseConditionalStmt(clang::ConditionalOperator* stmt, SValue& val) override;

    /// Get value for and_reduce, or_reduce and other reduce functions
    SValue getReducedVal(const std::string& fname, const SValue& rval); 
    
    /// Get type width, sign and APSInt value for arbitrary @val, that is 
    /// possible if @val is Integer or Variable/Object has Integer value
    /// \return true if that is possible
    bool getIntValueInfo(const SValue& val, std::pair<size_t, bool>& info, 
                         llvm::APSInt& intVal);
    /// Get value for concat()
    SValue getConcatVal(const SValue& fval, const SValue& sval);

    /// Function call expression
    void parseCall(clang::CallExpr* expr, SValue& returnVal) override;

    /// Member function call expression, used for general function call
    /// This method is override for module/port/signal/clock special methods
    /// \param tval -- this value for user method
    /// \param val -- evaluated value/return value for user method
    void parseMemberCall(clang::CXXMemberCallExpr* callExpr, SValue& tval,
                         SValue& val) override;

    /// Operator call expression
    void parseOperatorCall(clang::CXXOperatorCallExpr* expr, SValue& tval,
                           SValue& val) override;

    /// Return statement
    void parseReturnStmt(clang::ReturnStmt* stmt, SValue& val) override;
    
    void parseExpr(clang::ExpressionTraitExpr* expr, SValue& val) override;
    
// ---------------------------------------------------------------------------
// Auxiliary methods
public:
    /// Pointer dereference preserving unknown array index, does null/dangling 
    /// pointer error reporting  
    /// \return pointe value
    SValue derefPointer(const SValue& rval, clang::Stmt* stmt, bool checkPtr);

    /// Try to calculate range or bit selection 
    /// \return integer value or NO_VALUE
    SValue evalRangeSelect(const clang::Expr* expr, SValue val, SValue hi, 
                           SValue lo);

    /// Get #val from state, if it is not an Integer return Unknown
    SValue getIntOrUnkwn(SValue val);
    
    /// Logical not "!"
    SValue evalUnaryLNot(const SValue& val);

    /// Bitwise not "~" with extension to @bitWidth bits
    SValue evalUnaryBNot(const SValue& val, unsigned bitWidth);

};

} // namespace sc


#endif /* SCPARSEEXPRVALUE_H */

