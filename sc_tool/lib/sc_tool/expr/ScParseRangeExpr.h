/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/* 
 * SC Clang front end project. 
 * SC type range expression evaluation class.
 * 
 * File:   ScParseRangeExpr.h
 * Author: Mikhail Moiseev
 */

#ifndef SCPARSERANGEEXPR_H
#define SCPARSERANGEEXPR_H

#include "sc_tool/expr/ScParseExprValue.h"
#include "clang/AST/Decl.h"
#include "clang/AST/Expr.h"
#include "clang/AST/Type.h"

namespace sc {

/// Evaluate range expression with substitution zero for all variables w/o value
/// and calculating number of such variables
class ScParseRangeExpr : public ScParseExprValue 
{
public:
    explicit ScParseRangeExpr(const clang::ASTContext& context_, 
                              std::shared_ptr<ScState> state_, 
                              bool isCombProcess_,
                              const SValue& modval_) :
        ScParseExprValue(context_, state_, isCombProcess_, modval_)
    {}
    
    /// If @lval is integer place it to @rval else place 0
    /// \return false if no value for @lval exists
    virtual SValue getValueFromState(const SValue& lval,
                    ArrayUnkwnMode returnUnknown = ArrayUnkwnMode::amNoValue)
    {
        // Skip updating @variables, required for @parseGlobalConstant() if 
        // there is a function call like std::max/std::min
        if (skipRangeVar) {
            return ScParseExpr::getValueFromState(lval, returnUnknown);
        }
        
        SValue rval;
        
        if (lval.isVariable() || lval.isTmpVariable()) {
            // Temporary variable cannot be constant type
            bool isConst = lval.isVariable() && 
                           lval.getVariable().getType().isConstQualified();
            
            if (isConst) {
                // Try to get constant value from AST
                if (getConstASTValue(astCtx, lval, rval)) {
                    return rval;
                }
                
                // Try to get integer value from the state
                state->getValue(lval, rval);
                if (!rval.isInteger()) {
                    variables.push_back(lval);
                    rval = SValue(llvm::APSInt(64, false), 10);
                }
            } else {
                rval = SValue(llvm::APSInt(64, false), 10);
                variables.push_back(lval);
            }
            
        } else 
        if (lval.isObject()) {
            rval = SValue(llvm::APSInt(64, false), 10);
            variables.push_back(lval);
            
        } else 
        if (lval.isInteger()) {
            rval = lval;
        }
        
        return rval;
    }
    
    /// Parse and evaluate one expression/statement, 
    /// if result is non-constant variable, replace it with zero value
    /// \return true if expression is evaluated to a constant value
    virtual bool evaluate(const clang::Stmt* stmt, SValue& val)
    {
        variables.clear();

        // Parse @stmt with @chooseExprMethod
        SValue lval;
        ScParseExprValue::chooseExprMethod(const_cast<clang::Stmt *>(stmt),
                                           lval);
        val = getValueFromState(lval);
        
        return (variables.size() == 0);
    }
    
    /// Get non-constant variable number
    const std::vector<SValue>& getVariables() {
        return variables;
    }
    
protected:
    // Range variables which are non-constant
    std::vector<SValue>     variables;
    
};

}


#endif /* SCPARSERANGEEXPR_H */

