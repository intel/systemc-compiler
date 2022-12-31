/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * SC Clang front end project. 
 * SC expression parser and code generator class.
 *  
 * File:   ScGenerateExpr.h
 * Author: Mikhail Moiseev
 */

#ifndef SCGENERATEEXPR_H
#define SCGENERATEEXPR_H

#include "sc_tool/scope/ScVerilogWriter.h"
#include "sc_tool/expr/ScParseExpr.h"
#include "llvm/ADT/Optional.h"
#include <unordered_map>

namespace sc {

// Process function analyzer, @state is cloned and owned by ScTraverseProc 
class ScGenerateExpr : public ScParseExpr {
public:
             
    explicit ScGenerateExpr(const clang::ASTContext& context_, 
                            std::shared_ptr<ScState> state_, 
                            bool isCombProcess_, const SValue& modval_,
                            ScVerilogWriter* codeWriter_);

    virtual ~ScGenerateExpr() {}
    protected:
    // ------------------------------------------------------------------------
    // Utility functions
    
    /// Report warning if @val is register or pointer/reference to register
    void reportReadRegister(const SValue& val, const clang::Expr* expr);
    
    /// Find any kind of assignment inside of give expression
    /// \return <assignment statement, LHS statement of the assignment>
    std::pair<clang::Expr*, clang::Expr*> findAssignmentStmt(clang::Expr* expr);

    /// Check if argument is narrowing integral to boolean cast
    bool isIntToBoolCast(const clang::Expr* expr);

    /// Check if argument is SC channel
    bool isArgChannel(const clang::Expr* argExpr);
    
    /// Check if statement is operator of IO stream cin/cout
    bool isIoStreamStmt(const clang::Stmt* stmt);

    /// Check if stmt is wait(n) 
    bool isWaitNStmt(const clang::Stmt* stmt);
    
    /// Returns true is given statement is call of @sct_assert_in_proc_func()
    bool isTemporalAssert(const clang::Stmt* stmt) const;
    
    /// Returns true if statement is call of @sct_assert()
    bool isSctAssert(const clang::Stmt* stmt) const;
    
    /// Check if function has wait() inside, implemented in ScTraverseProc
    virtual bool isWaitInFunc(const clang::FunctionDecl* decl) = 0;

    /// Check if current function has wait() inside, implemented in ScTraverseProc
    virtual bool isWaitInCurrFunc() = 0;

    /// Get string from char* argument
    llvm::Optional<std::string> getStringFromArg(clang::Expr* argExpr);

    /// Get terminator condition, implemented in ScTraverseProc
    /// \param val  -- at all iterations including first
    /// \param fval -- at first iteration only, used for loop with wait()
    virtual void getTermCondValue(const clang::Stmt* stmt, SValue& val, 
                                  SValue& fval) = 0;

    /// Parse and evaluate condition expression as constant integer, operates
    /// with constants only, no @keepConstVariables/@calcPartSelectArg checked
    /// \param val        -- integer if returned true or 
    ///                      some other evaluated value else
    /// \return <result, integer value of result>
    std::pair<SValue, SValue>  evaluateConstIntNoCheck(clang::Expr* expr);

    /// Parse and evaluate one expression/statement as constant integer
    /// returns NO_VALUE for expression with references
    /// \param val        -- integer if returned true or 
    ///                      some other evaluated value else
    /// \param checkConst -- get value for variable if it is constant only
    ///                      used in ScTraverseProc, not used in ScTraverseConst
    /// \return <result, integer value of result>
    std::pair<SValue, SValue> evaluateConstInt(
            clang::Expr* expr, bool checkConst = true,
            bool checkRecOnly = false) override;

    /// The same as previous one, just return integer value only
    /// \return <integer value of result>
    SValue evaluateConstInt(clang::Expr* expr, const SValue& val, 
                            bool checkConst = true) override;
    
    /// Evaluate variable dependent range (@hexpr, @lexpr) expression and put 
    /// evaluated values to generate part-selection code
    /// \return if part-selection with variables is required (true) or 
    ///         constant range is used 
    bool evaluateRangeExpr(clang::Expr* hexpr, clang::Expr* lexpr);

    /// Get time argument(s) of get_lo_time() and return lo/hi time pair
    //llvm::Optional<std::pair<int, int>> getTimeFromCall(clang::Expr* expr);

    /// Parse SCT_ASSERT in module scope and put assertion string into codeWriter
    /// \return statement for which assertion string is stored
    const clang::Stmt* parseSvaDecl(const clang::FieldDecl* fdecl);
    
    /// Generate function parameter assignments
    void prepareCallParams(clang::Expr* expr, const SValue& funcModval, 
                           const clang::FunctionDecl* callFuncDecl) override;
    
    /// Get vector with parent record values for given field variable or record value
    /// For record array it get record value from its most inner zero element
    /// \param val -- record field variable or record value
    /// \return -- vector with record values in hierarchy
    std::vector<SValue> getRecVector(const SValue& val);
    
    /// Put any member expression specified in @val for @stmt 
    void putMemberExpr(const clang::Expr* expr, const SValue& val,
                       const std::string& refRecarrIndxStr);

    /// Put any pointer variable de-referenced string, including pointer parameter
    /// Used everywhere pointer de-referenced or pointer method called
    /// \param lvar -- variable for value lval, lvar = getVariableForValue(lval)
    /// \param lval -- pointer value
    /// \param rval -- pointee value, state->getValue(lval, rval)
    void putPtrDerefExpr(const clang::Expr* expr, const SValue& lvar,
                         const SValue& lval, const SValue& rval);

    
    // ------------------------------------------------------------------------
    // Parse statement functions

    /// Integer literal
    void parseExpr(clang::IntegerLiteral* expr, SValue& val) override;
    
    /// Constant expression
    void parseExpr(clang::ConstantExpr* expr, SValue& val) override;
    
    /// Bool literal
    void parseExpr(clang::CXXBoolLiteralExpr* expr, SValue& val) override;

    /// @nullptr literal
    void parseExpr(clang::CXXNullPtrLiteralExpr* expr, SValue& val) override;
    
    /// String literal (const char*)
    void parseExpr(clang::StringLiteral* expr, SValue& val) override;
    
    /// Any access of member variable
    void parseExpr(clang::MemberExpr* expr, SValue& val) override;
    
    /// Set root to determine module hierarchy
    void parseExpr(clang::CXXThisExpr* expr, SValue& val) override;

    /// Used for local variables usage like assignment statement in left/right parts
    void parseExpr(clang::DeclRefExpr* expr, SValue& val) override;

    /// Used for implicit type cast and LValue to RValue cast
    void parseExpr(clang::ImplicitCastExpr* expr, SValue& rval, SValue& val) override;
    void parseExpr(clang::ExplicitCastExpr* expr, SValue& rval, SValue& val) override;
    
    /// Parenthesized expression, i.e. expression in "()"
    void parseExpr(clang::ParenExpr* expr, SValue& val) override;

    /// Used for default initializer in constructor or in aggregate initialization
    void parseExpr(clang::CXXDefaultInitExpr* expr, SValue& val) override;
 
    /// Used for default argument value in function calls
    void parseExpr(clang::CXXDefaultArgExpr* expr, SValue& val) override;

    /// Used for default initializer in constructor or in aggregate initialization
    /// T{}
    void parseExpr(clang::InitListExpr* expr, SValue& val) override;
    
    /// Used for construction temporary record object 
    /// T()
    void parseExpr(clang::CXXTemporaryObjectExpr* expr, SValue& val) override;
    
    /// CXX constructor, including @sc_port/@sc_signal
    void parseExpr(clang::CXXConstructExpr* expr, SValue& val) override;
    
    /// Operator @new and @new[]
    void parseExpr(clang::CXXNewExpr* expr, SValue& val) override;

    /// Operator delete
    void parseExpr(clang::CXXDeleteExpr* expr, SValue& val) override;

    /// Common function for operator[] in @ArraySubscriptExpr and @CXXOperatorCall
    SValue parseArraySubscript(clang::Expr* expr, clang::Expr* baseExpr, 
                               clang::Expr* indxExpr);
    /// Array index access operator []
    void parseExpr(clang::ArraySubscriptExpr* expr, SValue& val) override;
    
    /// Transform temporary that is written to memory so that a reference can bind it
    void parseExpr(clang::MaterializeTemporaryExpr* expr, SValue& val) override;

    /// Expression with cleanup
    void parseExpr(clang::ExprWithCleanups* expr, SValue& val) override;
    
    /// Represents binding an expression to a temporary.
    /// Used to call destructor for temporary object
    void parseExpr(clang::CXXBindTemporaryExpr* expr, SValue& val) override;
    
    /// Template parameter use as integer value
    void parseExpr(clang::SubstNonTypeTemplateParmExpr* expr, SValue& val) override;
    
    /// Used for local variable declaration
    void parseDeclStmt(clang::Stmt* stmt, clang::ValueDecl* decl, 
                       SValue& val, clang::Expr* initExpr) override;
    
    /// Parse field declaration without initialization to put into codeWriter, 
    /// \param lfvar -- field variable
    void parseFieldDecl(clang::ValueDecl* decl, const SValue& lfvar) override;

    /// Parse array field declaration w/o initialization to put into codeWriter, 
    /// used in declaration array of records
    void parseArrayFieldDecl(clang::ValueDecl* decl, const SValue& lfvar,
                             const std::vector<size_t>& arrSizes) override;

    /// Parse statement and run @chooseExprMethod for each operand
    void parseBinaryStmt(clang::BinaryOperator* stmt, SValue& val) override;
   
    /// Parse statement and run @chooseExprMethod for each operand
    void parseCompoundAssignStmt(clang::CompoundAssignOperator* stmt,
                                         SValue& val) override;
    
    /// Parse statement and run @chooseExprMethod for the operand
    void parseUnaryStmt(clang::UnaryOperator* stmt, SValue& val) override;
    
    /// Operator call expression
    void parseOperatorCall(clang::CXXOperatorCallExpr* expr, SValue& tval,
                           SValue& val) override;

    /// Member function call expression
    void parseMemberCall(clang::CXXMemberCallExpr* expr, SValue& tval,
                         SValue& val) override;
  
    /// Function call expression
    void parseCall(clang::CallExpr* expr, SValue& val) override;
    
    /// Return statement
    void parseReturnStmt(clang::ReturnStmt* stmt, SValue& val) override;
    
    /// Ternary operator (...) ? ... : ...
    void parseConditionalStmt(clang::ConditionalOperator* stmt, SValue& val) override;
    
    /// Choose and run DFS step in accordance with expression type.
    /// Remove sub-statements from generator
    void chooseExprMethod(clang::Stmt *stmt, SValue &val) override;
    
public:
    /// Parse one statement, clear @terms and run @chooseExprMethod
    llvm::Optional<std::string> parse(const clang::Stmt* stmt);
    
    /// Parse block terminator statement 
    llvm::Optional<std::string> parseTerm(const clang::Stmt* stmt,
                                          const SValue& termCond,
                                          bool artifIf = false); 
    
protected:
    /// Code writer, used for process function and called functions
    /// Not need to save/restore in process context
    ScVerilogWriter*   codeWriter;
    
    /// Returned pointed object to replace temporary variable returned for
    /// functions returns pointer
    SValue returnPtrVal = NO_VALUE;
    
    /// Array indices for for record array parameters passed by reference
    /// <record value, indices string>
    std::unordered_map<SValue, std::string>  recarrRefIndices;

    /// Suffix string with record array parameter passed by reference indices
    std::string refRecarrIndx = "";

    /// Name for temporal assertion    
    llvm::Optional<std::string> assertName = llvm::None;
    
    /// Function calls replaced by constant
    std::unordered_map<clang::Stmt*, clang::Stmt*> constReplacedFunc;
};
}

#endif /* SCGENERATEEXPR_H */

