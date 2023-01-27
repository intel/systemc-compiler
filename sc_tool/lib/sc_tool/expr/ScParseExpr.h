/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/* 
 * SC Clang front end project.
 * 
 * SC CFG traverse abstract base class.
 * Provides only common methods to operate with state.
 * 
 * Author: Mikhail Moiseev
 */

#ifndef SCPARSEEXPR_H
#define SCPARSEEXPR_H

#include "sc_tool/cfg/ScState.h"
#include "sc_tool/utils/CppTypeTraits.h"

#include <clang/AST/Expr.h>
#include <clang/AST/ExprCXX.h>
#include <clang/AST/ASTContext.h>
#include "clang/Basic/SourceManager.h"
#include <clang/Analysis/CFG.h>

namespace sc {

class ScParseExpr {
protected:
    const clang::SourceManager&   sm;
    const clang::ASTContext& astCtx;
    
    /// Current block level, block inside CTHREAD main loop starts with 0
    unsigned level = 0;
    /// State singleton for process function analysis
    std::shared_ptr<ScState>  state;

    /// Current process is combinatorial
    bool isCombProcess;
    /// Current module value in synthesized code, 
    ///  if @modval is module, the same as @modval
    ///  if @modval if MIF, first parent module which is not MIF
    SValue  synmodval;
    /// Current module value, *this except for local record function/constructor
    SValue  modval;
    /// Current local record value, *this for local record function/constructor
    SValue  recval;
    
    /// Return value, temporary variable declared in parent function
    SValue returnValue;

    /// Check no value in statement parsing, no value checked and error reported
    /// only for design elaboration, for constant propagation not used
    bool checkNoValue;
    
    /// Inside of function parameters parsing, used to avoid function/constructor
    /// call inside of parameter s of another call
    bool inFuncParams = false;
    /// Do not add variable in ScParseRangeExpr 
    bool skipRangeVar = false;
    /// Current expression is LHS which could be modified (assigned), 
    /// required to clear value at bit/range access
    bool assignLHS = false;
    /// String literal unsigned/width passed down for parse it as a number
    bool strLiterUnsigned = true;
    unsigned strLiterWidth = 0;
    
    /// Record variable value from currently analyzed @DeclStmt,
    /// used in record constructor code generation
    SValue locrecvar = NO_VALUE;
    /// Is record variable already constructed with normal constructor
    /// Used in copy/move constructor to avoid double assignment of record fields
    bool locrecvarCtor = false;
    /// Temporary constructed record with InitListExpr and CXXTemporaryObjectExpr
    SValue temprec = NO_VALUE;
    
public:

    explicit ScParseExpr(const ScParseExpr& other)
    : sm(other.sm)
    , astCtx(other.astCtx)
    , state(other.state->clone())
    , modval(other.modval)
    , recval(other.recval)
    , checkNoValue(other.checkNoValue)
    {}

    explicit ScParseExpr(const clang::ASTContext& context_,
                         std::shared_ptr<ScState> state_, 
                         bool isCombProcess_,
                         const SValue& modval_,
                         bool checkNoValue_ = true,
                         SValue recval_ = NO_VALUE) :
        sm(context_.getSourceManager()), astCtx(context_), state(state_),
        isCombProcess(isCombProcess_),
        modval(modval_), recval(recval_), checkNoValue(checkNoValue_)
    {
        SCT_TOOL_ASSERT (state, "ScTraverse::ScTraverse state is null");
    }
    
    virtual ~ScParseExpr() {}
    
    /// Update @modval, required for temporal assertion in base classes
    void setModval(const SValue& val) {
        modval = val;
    }
    void setSynModval(const SValue& val) {
        synmodval = val;
    }
    
    // ---------------------------------------------------------------------------
    // State access functions, can be overloaded in inheritors
    
    /// Check if value is integer of ZeroWidth or variable of sct_zero_width 
    /// type or element of array of sct_zero_width 
    bool isScZeroWidth(const SValue& val);
    
    /// Get integer literal basis
    char getLiteralRadix(const clang::IntegerLiteral* intLiteral);
    
    /// Parse global/static constant and put its value to state
    virtual void parseGlobalConstant(const SValue& sVar);

    /// Variable value assignment or clear
    /// Used in assignment operator in value sensitive analysis
    /// Support references initialization and assignment
    /// if @rval is variable or object get its value, else use @rval as value
    ///    @rval is SC object : @lval -> @rval
    ///    otherwise          : @lval = @rval 
    void assignValueInState(const SValue& lval, const SValue& rval,
                ArrayUnkwnMode returnUnknown = ArrayUnkwnMode::amNoValue);
    /// Record fields values assignment or clear
    /// \param @lval -- record variable
    void assignRecValueInState(const SValue &lval, const SValue &rval,
                ArrayUnkwnMode returnUnknown = ArrayUnkwnMode::amNoValue);

    ///  Get @lval or value of @lval from the state 
    ///  if @lval is variable or object get its value, else use @lval as value 
    ///  Do @lval de-reference, no de-reference for returned value in @rval.
    virtual SValue getValueFromState(const SValue& lval, 
                ArrayUnkwnMode returnUnknown = ArrayUnkwnMode::amNoValue);

    /// Get channel value from the given value, @amFirstElement used inside
    /// \return channel value or NO_VALUE
    SValue getChannelFromState(const SValue& val);
    
    /// Get record/MIF value from the given value
    SValue getRecordFromState(const SValue& val,
                ArrayUnkwnMode returnUnknown = ArrayUnkwnMode::amNoValue);
    
    /// Create record/module object value and parse its field declarations, 
    /// no constructor function call. 
    /// \param var -- local variable/member owns this record
    /// \param index -- local element number in single/multi dimensional array
    /// \param checkConst -- get only constant variables in expression and 
    ///                      put value to state for constant variable only
    SValue createRecValue(const clang::CXXRecordDecl* recDecl, 
                          const SValue& parent, const SValue& var, 
                          bool parseFields, size_t index = 0, 
                          bool checkConst = true);
    
    /// Copy values of record fields from @rval to @lval
    /// \param lval -- target record value 
    /// \param rval -- source record value
    void copyRecFieldValues(const SValue& lval, const SValue& rval);
    
    /// Put record fields declarations into @codeWriter, used in generate code only 
    /// Used for copy/move record constructor where records fields must be declared
    /// \param lval -- record value to put into codeWriter
    void declareRecFields(const SValue& lval); 
    
    /// Generate record constructor including its base class constructor, 
    /// member initialization
    /// \return record value
    SValue parseRecordCtor(clang::CXXConstructExpr* expr, SValue parent,
                           SValue currecvar, bool analyzeRecordCtor);
    
    /// Store statement string for @stmt
    virtual void storeStmtStr(clang::Stmt* stmt) {};
    
    /// Store statement string for @nullptr
    virtual void storeStmtStrNull(clang::Stmt* stmt) {};
    
// ----------------------------------------------------------------------------
// Auxiliary functions
    
    /// Get all sub-expressions in initializer list 
    std::vector<clang::Expr*> getAllInitExpr(clang::Expr* expr,
                                             bool innerArray = false) const;
    
    /// Get AST value for constant or template parameter given as @lval
    /// \return true and integer value in @rval of false and NO_VALUE
    static bool getConstASTValue(const clang::ASTContext &context,
                                 const SValue &lval,
                                 SValue &rval);

    /// Get AST value for element of constant or template array given as @lval
    /// Only one-dimension arrays supported
    /// \return true and integer value in @rval of false and NO_VALUE
    static bool getConstASTArrElement(const clang::ASTContext &context,
                                      const SValue &lval,
                                      size_t index,
                                      SValue &rval);

    /// Recursive parsing multi-dimensional array
    void parseArrayDecl(const clang::QualType& type, SValue& val);

    /// Get @indx-th element in multi-dimensional array @aval
    SValue getArrElmByIndx(const clang::QualType &type,
                           size_t indx, const SValue &aval);

    /// Set @indx-th element in multi-dimensional array @aval to value @ival
    void setArrayElement(const clang::QualType& type, size_t indx, 
                         const SValue& aval, const SValue& ival);
    
    /// Parse any declaration, used for module fields and local variables.
    /// Create variable value for @decl and put its initial value into state
    /// \param argExpr    -- initialization argument expression, 
    ///                      if @nullptr initializer from declaration is used   
    /// \param declModval -- module value, modval for declaration statement
    ///                      but funcModval for method call (can differ from modval)
    /// \param checkConst -- get only constant variables in expression and 
    ///                      put value to state for constant variable only
    /// \return <declared value, initialization value(s)>
    std::pair<SValue, std::vector<SValue> >  
            parseValueDecl(clang::ValueDecl* decl,
                          const SValue& declModval, 
                          clang::Expr* argExpr = nullptr,
                          bool checkConst = true, 
                          bool removeSubValues = false);

    /// Evaluate statement from CFG : Block element or terminator
    virtual void chooseExprMethod(clang::Stmt *stmt, SValue &result);

protected:

    /// Get terminator condition
    const clang::Expr* getTermCond(const clang::Stmt* stmt);
    
    /// Evaluate a branch of expression tree (subexpression of CFG statement)
    /// @return result of evaluation
    virtual SValue evalSubExpr(clang::Expr *expr);


    /// Parse and evaluate one expression/statement as constant integer
    /// Overridden in inheritors
    /// \param checkConst -- get value for variable if it is constant only
    ///                      used in ScTraverseProc, not used in ScTraverseConst
    /// \param checkRecOnly -- get zero instead of unknown index for record/MIF array
    // \return <result, integer value of result>
    virtual std::pair<SValue, SValue>   
    evaluateConstInt(clang::Expr* expr, bool checkConst = true,
                     bool checkRecOnly = false) 
    {
        return std::make_pair(NO_VALUE, NO_VALUE);
    }
    
    /// Try to get integer from state.
    /// If no value, ScGenerateExpr tries to evaluate it with local instance of 
    /// parseValueExpr.
    virtual SValue evaluateConstInt(clang::Expr* expr, const SValue& val, 
                            bool checkConst = true) {
        return NO_VALUE;
    }

    /// Parse function call or constructor call parameters, 
    /// fill state with parameters with correspondent argument values
    virtual void prepareCallParams(clang::Expr* expr, const SValue& funcModval, 
                                   const clang::FunctionDecl* callFuncDecl) {}
    
    /// Register return value and prepare @lastContext, 
    /// used for methods and global functions, implemented in ScTraverseConst
    /// \param funcModval -- current module
    /// \param funcRecval -- current local record instance, if no local record
    ///                      put current module to here 
    virtual void prepareCallContext(clang::Expr* expr, 
                            const SValue& funcModval, 
                            const SValue& funcRecval, 
                            const clang::FunctionDecl* callFuncDecl, 
                            const SValue& retVal) {}

    /// Parse expression and return @val
    /// Used for literal
    virtual void parseExpr(clang::IntegerLiteral* expr, SValue& val);
    
    /// Constant expression, can contain literal inside
    virtual void parseExpr(clang::ConstantExpr* expr, SValue& val);
    
    /// Boolean literal
    virtual void parseExpr(clang::CXXBoolLiteralExpr* expr, SValue& val);
    
    /// @nullptr literal
    virtual void parseExpr(clang::CXXNullPtrLiteralExpr* expr, SValue& val);

    /// Convert string to integer SValue, >64bit supported 
    SValue stringToInt(std::string s, unsigned width, bool usigned);
    
    /// String literal (const char*)
    virtual void parseExpr(clang::StringLiteral* expr, SValue& val);
    
    /// Used for local variables access in left/right parts
    virtual void parseExpr(clang::DeclRefExpr* expr, SValue& val);
    
    /// Any access of member variable
    virtual void parseExpr(clang::MemberExpr* expr, SValue& val);

    /// Set root to determine module hierarchy
    virtual void parseExpr(clang::CXXThisExpr* expr, SValue& thisPtrVal);
    
    /// Used for implicit type cast and LValue to RValue cast.
    virtual void parseExpr(clang::ImplicitCastExpr* expr, SValue& rval, SValue& val);
    virtual void parseExpr(clang::ExplicitCastExpr* expr, SValue& rval, SValue& val);

    /// Parethesized expression, i.e. expression in "()"
    virtual void parseExpr(clang::ParenExpr* expr, SValue& val);

    /// Used for default argument value in function calls
    virtual void parseExpr(clang::CXXDefaultArgExpr* expr, SValue& val);
    
    /// Called from @parseCtorCfg and returns initialization value (@rval)
    /// Used for default initializer in constructor or in aggregate initialization
    virtual void parseExpr(clang::CXXDefaultInitExpr* expr, SValue& val) {}

    /// Used for default initializer in constructor or in aggregate initialization
    /// T{}
    virtual void parseExpr(clang::InitListExpr* expr, SValue& val) {}
    
    /// Used for default constructor 
    /// T()
    virtual void parseExpr(clang::CXXTemporaryObjectExpr* expr, SValue& val) {}
    
    /// C++ type default initializer (0), used for array filler 
    virtual void parseExpr(clang::ImplicitValueInitExpr* expr, SValue& val);
    
    /// CXX constructor, including @sc_port/@sc_signal
    virtual void parseExpr(clang::CXXConstructExpr* expr, SValue& val);
    
    /// Operator @new and @new[]
    virtual void parseExpr(clang::CXXNewExpr* expr, SValue& val) {}
    
    /// Operator delete
    virtual void parseExpr(clang::CXXDeleteExpr* expr, SValue& val) {}    
    
    /// Array index access operator []
    virtual void parseExpr(clang::ArraySubscriptExpr* expr, SValue& val) {}
    
    /// Declared variable may be initialized with expression inside
    /// Used for variable declaration with and without initialization
    virtual void parseDeclStmt(clang::Stmt* stmt, clang::ValueDecl* decl, 
                               SValue& val, clang::Expr* initExpr = nullptr) {}
    
    /// Parse field declaration without initialization to put into codeWriter, 
    /// \param lfvar -- field variable
    virtual void parseFieldDecl(clang::ValueDecl* decl, const SValue& lfvar) {}
    
    /// Parse array field declaration w/o initialization to put into codeWriter, 
    /// used in declaration array of records
    virtual void parseArrayFieldDecl(clang::ValueDecl* decl,const SValue& lfvar,
                                     const std::vector<size_t>& arrSizes) {}

    /// Parse statement and run evalSubExpr for each operand
    virtual void parseBinaryStmt(clang::BinaryOperator* stmt, SValue& val) {}

    /// Parse statement and run evalSubExpr for each operand
    virtual void parseCompoundAssignStmt(clang::CompoundAssignOperator* stmt, 
                                         SValue& val) {}
    
    /// Parse statement and run evalSubExpr for the operand
    virtual void parseUnaryStmt(clang::UnaryOperator* stmt, SValue& val) {}
    
    /// Ternary operator (...) ? ... : ...
    virtual void parseConditionalStmt(clang::ConditionalOperator* stmt, 
                                      SValue& val) {}
    
    /// Parse while statement
    virtual void parseWhile(clang::WhileStmt* stmt, SValue& val) {}

    /// Function call expression
    virtual void parseCall(clang::CallExpr* expr, SValue& val) {}

    /// Member function call expression
    virtual void parseMemberCall(clang::CXXMemberCallExpr* expr,
                                 SValue& tval, SValue& val) {}

    /// Operator call expression
    virtual void parseOperatorCall(clang::CXXOperatorCallExpr* expr, 
                                   SValue& tval, SValue& val) {}

    /// Return statement
    virtual void parseReturnStmt(clang::ReturnStmt* stmt, SValue& val) {}
    
    /// Transform temporary that is written to memory so that a reference can bind it
    virtual void parseExpr(clang::MaterializeTemporaryExpr* expr, SValue& val) {
        val = evalSubExpr(expr->getSubExpr());
    }

    /// Expression with cleanup
    virtual void parseExpr(clang::ExprWithCleanups* expr, SValue& val) {
        val = evalSubExpr(expr->getSubExpr());
    }

    /// Used to call constructor/destructor for temporary object
    virtual void parseExpr(clang::CXXBindTemporaryExpr* expr, SValue& val) {
        val = evalSubExpr(expr->getSubExpr());
    }
    
    /// Template parameter use as integer value
    virtual void parseExpr(clang::SubstNonTypeTemplateParmExpr* expr, SValue& val) {
        val = evalSubExpr(expr->getReplacement());
    }
    
    /// ...
    virtual void parseExpr(clang::ExpressionTraitExpr* expr, SValue& val) {}
    
    /// Incorrect statement, error in source code
    virtual void parseExpr(clang::OpaqueValueExpr* expr, SValue& val);
  
};
}

#endif /* SCPARSEEXPR_H */

