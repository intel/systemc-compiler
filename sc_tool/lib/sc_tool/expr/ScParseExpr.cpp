/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Author: Mikhail Moiseev
 */

#include "sc_tool/utils/CheckCppInheritance.h"
#include "sc_tool/utils/ScTypeTraits.h"
#include "sc_tool/utils/CppTypeTraits.h"
#include "sc_tool/diag/ScToolDiagnostic.h"
#include "sc_tool/expr/ScParseExpr.h"
#include "clang/AST/Type.h"

namespace sc {
    
using namespace std;
using namespace clang;

// Get integer literal basis
char ScParseExpr::getLiteralRadix(const IntegerLiteral* intLiteral)
{
    // Source Location of current int literal
    SourceLocation loc = intLiteral->getLocation();
    const char* intStr = sm.getCharacterData(loc, nullptr);

    if (intStr[0] == '0') {
        if (intStr[1] == 'x' || intStr[1] == 'X') {
            return 16;
        } else 
        if (intStr[1] == 'b') {
            return 2;
        } else 
        if (isdigit(intStr[1])) {
            return 8;
        }
    }
    return 10;
}

// Parse global/static constant and put its value to state
SValue ScParseExpr::parseGlobalConstant(const SValue &val)
{
    SCT_TOOL_ASSERT (val.isVariable(), "No variable value");
    
    // Set skip variable in @ScParseRangeExpr
    skipRangeVar = true;
    SValue res = parseValueDecl(const_cast<clang::ValueDecl*>(
                                val.getVariable().getDecl()),
                                NO_VALUE, nullptr, false);
    skipRangeVar = false;

    if (res.isVariable()) {
        res = state->getValue(res);
    }

    return res;
}

void ScParseExpr::assignValueInState(const SValue &lval, const SValue &rval,
                                     ArrayUnkwnMode returnUnknown)
{
    using std::cout; using std::endl;
    //cout << "assignValueInState " << lval << " = " << rval << endl;
    
    if (rval.isInteger() || rval.isScChannel() || rval.isRecord() || 
        rval.isUnknown()) {
        // If @rval is integer/SC object/NO_VALUE put it as value
        state->putValue(lval, rval);

    } else 
    if (rval.isVariable() || rval.isTmpVariable() || rval.isObject()) {
        // Check @lval is reference and it is initialized
        SValue llval;
        if (!lval.isReference() || 
            state->getValue(lval, llval, false, returnUnknown).first) {
            // Put @rval value to @lval non-reference/initialized reference
            SValue rrval;
            state->getValue(rval, rrval, true, returnUnknown);   // reference considered inside
            //cout << "   rrval " << rrval << endl;
            state->putValue(lval, rrval);

        } else {
            // @lval reference initialization with @rval`s variable
            SValue rrval;
            // Do @rval de-reference if required
            state->getDerefVariable(rval, rrval);
            state->putValue(lval, rrval, false);
        }
    }
}

SValue ScParseExpr::getValueFromState(const SValue &lval,
                                      ArrayUnkwnMode returnUnknown)
{
    //using namespace std;
    SValue rval;
    
    if (lval.isVariable()) {
        SValue llval;
        // Do @lval de-reference if required
        state->getDerefVariable(lval, llval);
        // Try to get value from state
        state->getValue(llval, rval, false, returnUnknown);
        //cout << "getValueFromState llval " << llval << " rval " << rval << endl;
            
        if (rval.isUnknown() && llval.isVariable()) {
            // Try to get constant value from AST, for variable only
            // Required for constant reference parameter with default value
            bool isConst = llval.getType().isConstQualified();
            if (isConst) {
                rval = ScParseExpr::parseGlobalConstant(lval);
            }
        }
        
    } else 
    if (lval.isInteger() || lval.isScChannel() || lval.isRecord()) {
        rval = lval;

    } else 
    if (lval.isTmpVariable() || lval.isObject()) {
        // De-reference inside of @getValue()
        state->getValue(lval, rval, true, returnUnknown);
    }

    return rval;
}

// Get channel value from the given value, @amFirstElement used inside
// \return channel value or NO_VALUE
SValue ScParseExpr::getChannelFromState(const SValue &val)
{
    //std::cout << "getChannelFromState for " << val << std::endl;
    
    // Try to get channel for given value
    SValue lval = val;
    while (lval && !lval.isScChannel()) {
        SValue llval = lval;
        state->getValue(llval, lval, true, ArrayUnkwnMode::amFirstElement);
    }
    if (lval.isScChannel()) {
        //std::cout << "   " << lval << std::endl;
        return lval;
    }

    // Check first element in array if there is no value for given value
    vector<const ValueDecl*> decls;
    // Return most bottom array value or NO_VALUE
    lval = state->findArray(val, 0, decls); 
    if (lval.isArray()) {
        lval.getArray().setOffset(0);
    }
    while (lval && !lval.isScChannel()) {
        SValue llval = lval;
        state->getValue(llval, lval, true, ArrayUnkwnMode::amFirstElement);
    }
    //std::cout << "   " << lval << std::endl;
    
    return lval;
}

SValue ScParseExpr::getRecordFromState(const SValue& val,
                                       ArrayUnkwnMode returnUnknown)
{
    SValue lval = val;

    while (lval && !lval.isRecord()) {
        SValue llval = lval;
        auto flags = state->getValue(llval, lval, true, returnUnknown);
        // If @getValue() returns array with unknown index exit from loop
        if (flags.second) break;
    }
    
    return lval;
}

bool ScParseExpr::getConstASTValue(const clang::ASTContext &context,
                                   const SValue &lval,
                                   SValue &rval)
{
    // Can get value from AST for AST variable only
    if (!lval.isVariable()) {
        rval = NO_VALUE;
        return false;
    }
    // Array in template or other constant array not in state
    // Try to get value from AST
    auto valDecl = lval.getVariable().getDecl();
    bool isConst = lval.getVariable().getType().isConstQualified();

    // @valDecl can be NULL for temporary variable
    if (valDecl && isConst) {
        auto varDecl = llvm::dyn_cast<const clang::VarDecl>(valDecl);
        auto fldDecl = llvm::dyn_cast<const clang::FieldDecl>(valDecl);
        auto initExpr = (varDecl) ? varDecl->getAnyInitializer() : 
                        (fldDecl) ? fldDecl->getInClassInitializer() : nullptr;

        clang::Expr::EvalResult evalResult;
        bool evaluated =
            initExpr && initExpr->EvaluateAsRValue(evalResult, context);

        if (evaluated && evalResult.Val.isInt()) {
            rval = SValue(evalResult.Val.getInt(), 10);
            return true;
        }
    }
    rval = NO_VALUE;
    return false;
}

bool ScParseExpr::getConstASTArrElement(const clang::ASTContext &context,
                                        const SValue &lval,
                                        size_t index,
                                        SValue &rval)
{
    // Can get value from AST for AST variable only
    if (!lval.isVariable()) {
        rval = NO_VALUE;
        return false;
    }
    // Array in template or other constant array not in state
    // Try to get value from AST
    auto valDecl = lval.getVariable().getDecl();
    bool isConst = lval.getVariable().getType().isConstQualified();

    // @valDecl can be NULL for temporary variable
    if (valDecl && isConst) {
        auto varDecl = llvm::dyn_cast<const clang::VarDecl>(valDecl);
        auto fldDecl = llvm::dyn_cast<const clang::FieldDecl>(valDecl);
        auto initExpr = (varDecl) ? varDecl->getAnyInitializer() : 
                        (fldDecl) ? fldDecl->getInClassInitializer() : nullptr;

        clang::Expr::EvalResult evalResult;
        bool evaluated =
            initExpr && initExpr->EvaluateAsRValue(evalResult, context);

        if (evaluated) {
            auto elmVal = evalResult.Val.getArrayInitializedElt(index);
            if (elmVal.isInt()) {
                rval = SValue(elmVal.getInt(), 10);
                return true;
            }
        }
    }
    rval = NO_VALUE;
    return false;
}

void ScParseExpr::parseArrayDecl(const clang::QualType &type, SValue &val)
{
    val = state->createArrayInState(type, level);
}

SValue ScParseExpr::getArrElmByIndx(const clang::QualType &type,
                                    size_t indx, const SValue &aval)
{
    std::vector<size_t> arrInds = getArrayIndices(type, indx);
    size_t lastIndx = arrInds.back();
    arrInds.pop_back();

    SValue val(aval);
    for (auto i : arrInds) {
        val.getArray().setOffset(i);
        val = getValueFromState(val);
    }

    // Get last array object with last index offset
    val.getArray().setOffset(lastIndx);
    return val;
}

void ScParseExpr::setArrayElement(const clang::QualType &type,
                                  size_t indx,
                                  const SValue &aval,
                                  const SValue &ival)
{
    SValue val = getArrElmByIndx(type, indx, aval);
    assignValueInState(val, ival);
}

// Parse any declaration, used for module fields and local variables.
// Create variable value for @decl and put its initial value into state
SValue ScParseExpr::parseValueDecl(clang::ValueDecl* decl,
                                   const SValue& declModval, 
                                   clang::Expr* argExpr, bool checkConst, 
                                   bool removeSubValues)
{
    using namespace clang;
    using namespace llvm;
    using namespace std;
    // Variable or array variable
    SValue val(decl, declModval);
    state->setValueLevel(val, level);
    QualType type = decl->getType();
    
    // Remove all sub-values of declared @val, which created at previous pass
    // of this declaration statement, used in ScGenerateExpr only
    if (removeSubValues) {
        SValue rval;
        state->getValue(val, rval, false);
        state->removeSubValues(rval);
    }

    auto varDecl = dyn_cast<VarDecl>(decl);
    auto fieldDecl = dyn_cast<FieldDecl>(decl);
    SCT_TOOL_ASSERT (varDecl || fieldDecl, "No variable or field declaration");

    // @hasInit is false for inner record as it is field w/o initializer
    bool hasInit = (varDecl && varDecl->hasInit()) || 
                   (fieldDecl && fieldDecl->hasInClassInitializer());
    Expr* iexpr = argExpr ? argExpr : (hasInit ? 
                  (varDecl ? varDecl->getInit() : 
                   fieldDecl->getInClassInitializer()) : nullptr);
    
    if (type->isArrayType()) {
        // Array value
        SValue aval;
        // Parse multi-dimensional array, create sub-array values and
        // put them into state, set level for all elements inside
        parseArrayDecl(type, aval); // @aval initialized inside
        // Both @val and @aval are not references
        state->putValue(val, aval, false); // @val -> @aval

        bool isConst = type.isConstQualified();
        //cout << "parseValueDecl val " <<  val << " aval " << aval << endl;

        if (auto recType = getUserDefinedClassFromArray(type)) {
            // Put record values into array elements
            // Use @recval for local record field declarations
            SValue parent = recval ? recval : modval;
            auto recDecl = recType.getValue()->getAsCXXRecordDecl();
            // Record for zero array element
            SValue zeroRec;
            
            // No record constructor or field initialization supported for array
            auto elmnum = getArrayElementNumber(type);
            for (unsigned i = 0; i < elmnum; i++) {
                // Set @i-th element in multi-dimensional array
                // Add local variable and index in array to compare records
                // No set level required for record value 
                SValue rval = createRecValue(recDecl, parent, val, i);
                setArrayElement(type, i, aval, rval);
                
                if (i == 0) zeroRec = rval;
            }
            
            // Get array sizes, one size for single dimensional array
            auto arrSizes = getArraySizes(type);
            
            // Put field declarations to @localDeclVerilog for zero array element
            for (auto fdecl : recDecl->fields()) {
                bool isRecord = isUserDefinedClass(fdecl->getType());

                if (!isRecord) {
                    SValue fval(fdecl, zeroRec);
                    parseArrayFieldDecl(fdecl, fval, arrSizes);
                    
                } else {
                    SCT_INTERNAL_ERROR(decl->getBeginLoc(), 
                                       "Inner record not supported yet");
                }
            }
                
        } else
        if (iexpr && (!checkConst || isConst)) {
            // Put values of elements, that also works well for SC data types
            // Remove ExprWithCleanups wrapper
            if (auto cleanExpr = dyn_cast<ExprWithCleanups>(iexpr)) {
                iexpr = cleanExpr->getSubExpr();
            }
            
            if (auto init = dyn_cast<InitListExpr>(iexpr)) {
                for (unsigned i = 0; i < init->getNumInits(); i++) {
                    // Try to evaluate expression as integer constant
                    SValue ival;
                    Expr *iiexpr = const_cast<Expr *>(init->getInit(i));
                    evaluateConstInt(iiexpr, ival, checkConst);
                    // Set @i-th element in multi-dimensional array
                    setArrayElement(type, i, aval, ival);
                }
            } else 
            if ( isScIntegerArray(type, false) ) {
                // Put zero value for @iexpr
                SValue zval(APSInt(APInt(64, 0), true), 10);
                auto elmnum = getArrayElementNumber(type);
                for (unsigned i = 0; i < elmnum; i++) {
                    // Set @i-th element in multi-dimensional array
                    setArrayElement(type, i, aval, zval);
                }
            } else 
            if (isa<ArrayInitLoopExpr>(iexpr)) {
                // Do nothing here, ArrayInitLoopExpr used for array copy/move
                
            } else {
                SCT_INTERNAL_FATAL(iexpr->getBeginLoc(), 
                                 string("Unsupported initializer ")+ 
                                 iexpr->getStmtClassName());
            }
        }
    } else {
        // Single variable
        bool isRef = type->isReferenceType();
        bool isPtr = type->isPointerType();
        bool isConst = (isRef) ? type.getNonReferenceType().
                        isConstQualified() : type.isConstQualified();
        bool isRecord = isUserDefinedClass(type);

        if (isRef) {
            // It needs to put reference value into state
            if (iexpr) {
                // Do not evaluate constant reference as it can be used in
                // range/bit selection expression, even if @ival is unknown @NO_VALUE
                SValue ival = evalSubExpr(const_cast<Expr*>(iexpr));

                // Reference can be to array element or dynamic object
                // Channels can be for constant reference parameter
                if (!ival.isVariable() && !ival.isTmpVariable() &&
                    !ival.isArray() && !ival.isSimpleObject() &&
                    !((ival.isInteger() || ival.isUnknown() || 
                       ival.isScChannel()) && isConst))
                {
                    SCT_TOOL_ASSERT (false, "Incorrect reference initialization");
                }

                // Initializer value dereference required as @putValue() 
                // run with @deReference = false
                SValue iival;
                state->getDerefVariable(ival, iival);
                // Put variable for reference
                state->putValue(val, iival, false);
                
                // Check argument is array element at unknown index
                bool unkwIndex;
                bool isArr = state->isArray(iival, unkwIndex);
                if (isArr && unkwIndex) {
                    ScDiag::reportScDiag(argExpr->getBeginLoc(), 
                                         ScDiag::SYNTH_ARRAY_ELM_REFERENCE);
                }

            } else {
                ScDiag::reportScDiag(argExpr->getSourceRange().getBegin(),
                                     ScDiag::CPP_REFER_NO_INIT) << val.asString();
            }
        } else 
        if (isPtr) {
            if (iexpr) {
                // It needs to put pointer value into state
                SValue ival;
                if (evaluateConstInt(iexpr, ival, checkConst)) {
                    // Got integer constant for pointer
                    if (!ival.getInteger().isNullValue()) {
                        ScDiag::reportScDiag(argExpr->getBeginLoc(),
                                             ScDiag::SYNTH_POINTER_NONZERO_INIT)
                                             << val.asString();
                    }
                } else {
                    ival = evalSubExpr(const_cast<Expr*>(iexpr));
                }

                // Put pointer value, it needs to get value from @ival here
                // Use @amArrayUnknown for channel pointer array element at 
                // unknown index initialized/passed to function
                assignValueInState(val, ival, ArrayUnkwnMode::amArrayUnknown);
                
                // Check argument is array element at unknown index
                bool unkwIndex;
                bool isArr = state->isArray(ival, unkwIndex);
                if (isArr && unkwIndex) {
                    ScDiag::reportScDiag(argExpr->getBeginLoc(), 
                                         ScDiag::SYNTH_ARRAY_ELM_REFERENCE);
                }
                
            } else {
                ScDiag::reportScDiag(ScDiag::SYNTH_POINTER_NO_INIT)
                                     << val.asString();
            }
        } else 
        if (isRecord) {
            // User defined class non-module
            //cout << "-------- iexpr for record " << iexpr << " val " << val << endl;
            // Enable record constructor processing
            locrecvar = val;
            
            // Process record constructor, where fields are created and declared
            // Do not evaluate value for inner record as two ctors not supported
            if (iexpr) {
                SValue ival = evalSubExpr(const_cast<Expr*>(iexpr));
                //cout << "Record value is " << ival << endl;

                // Put record for record variable, value from @parseRecordCtor replaced
                assignValueInState(val, ival);
            }
            
        } else 
        if ((!checkConst || isConst) && iexpr) {
            // Try to evaluate expression as integer constant
            // Not applied to reference
            SValue ival;
            evaluateConstInt(iexpr, ival, checkConst);

            // Check builtin, enumeration or SC integer type
            if (isAnyInteger(decl->getType())) {
                assignValueInState(val, ival);
                
            } else {
                ScDiag::reportScDiag(decl->getBeginLoc(),
                                     ScDiag::SYNTH_TYPE_NOT_SUPPORTED) << 
                                     decl->getType();
            }
        }
    }
    return val;
}

void ScParseExpr::chooseExprMethod(clang::Stmt *stmt, SValue &val)
{
    using namespace clang;
    if (auto expr = dyn_cast<CompoundAssignOperator>(stmt)) {
        parseCompoundAssignStmt(expr, val);
    }
    else if (auto expr = dyn_cast<BinaryOperator>(stmt)) {
        parseBinaryStmt(expr, val);
    }
    else if (auto expr = dyn_cast<UnaryOperator>(stmt)) {
        parseUnaryStmt(expr, val);
    }
    else if (auto expr = dyn_cast<ConditionalOperator>(stmt)) {
        parseConditionalStmt(expr, val);
        
    } else 
    if (auto expr = dyn_cast<DeclStmt>(stmt)) {
        SCT_TOOL_ASSERT (expr->isSingleDecl(), "Declaration group not supported");
        auto decl = dyn_cast<ValueDecl>(expr->getSingleDecl());
        SCT_TOOL_ASSERT (decl, "No ValueDecl in DeclStmt");
        parseDeclStmt(expr, decl, val);
        
    } else 
    if (auto expr = dyn_cast<ArraySubscriptExpr>(stmt)) {
        parseExpr(expr, val);
    }
    else if (auto expr = dyn_cast<IntegerLiteral>(stmt)) {
        parseExpr(expr, val);
    }
    else if (auto expr = dyn_cast<CXXBoolLiteralExpr>(stmt)) {
        parseExpr(expr, val);
    }
    else if (auto expr = dyn_cast<CXXNullPtrLiteralExpr>(stmt)) {
        parseExpr(expr, val);
    }
    else if (auto expr = dyn_cast<ImplicitCastExpr>(stmt)) {
        parseExpr(expr, val);
    }
    else if (auto expr = dyn_cast<ExplicitCastExpr>(stmt)) {
        parseExpr(expr, val);
    }
    else if (auto expr = dyn_cast<ParenExpr>(stmt)) {
        parseExpr(expr, val);
    }
    else if (auto expr = dyn_cast<MemberExpr>(stmt)) {
        parseExpr(expr, val);
    }
    else if (auto expr = dyn_cast<CXXThisExpr>(stmt)) {
        parseExpr(expr, val);
    }
    else if (auto expr = dyn_cast<DeclRefExpr>(stmt)) {
        parseExpr(expr, val);
    }
    else if (auto expr = dyn_cast<CXXOperatorCallExpr>(stmt)) {
        parseOperatorCall(expr, val);
    }
    else if (auto expr = dyn_cast<CXXMemberCallExpr>(stmt)) {
        parseMemberCall(expr, val);
    }
    else if (auto expr = dyn_cast<CallExpr>(stmt)) {
        parseCall(expr, val);
    }
    else if (auto expr = dyn_cast<ReturnStmt>(stmt)) {
        parseReturnStmt(expr, val);
    }
    else if (auto expr = dyn_cast<CXXDefaultInitExpr>(stmt)) {
        parseExpr(expr, val);
    }
    else if (auto expr = dyn_cast<CXXDefaultArgExpr>(stmt)) {
        parseExpr(expr, val);
    }
    else if (auto expr = dyn_cast<InitListExpr>(stmt)) {
        parseExpr(expr, val);
    }
    else if (auto expr = dyn_cast<CXXConstructExpr>(stmt)) {
        parseExpr(expr, val);
    }
    else if (auto expr = dyn_cast<CXXNewExpr>(stmt)) {
        parseExpr(expr, val);
    }
    else if (auto expr = dyn_cast<CXXDeleteExpr>(stmt)) {
        parseExpr(expr, val);
    }
    else if (auto expr = dyn_cast<OpaqueValueExpr>(stmt)) {
        parseExpr(expr, val);
    }
    else if (auto expr = dyn_cast<ExpressionTraitExpr>(stmt)) {
        parseExpr(expr, val);
    }
    else if (auto expr = dyn_cast<MaterializeTemporaryExpr>(stmt)) {
        parseExpr(expr, val);
    }
    else if (auto expr = dyn_cast<ExprWithCleanups>(stmt)) {
        parseExpr(expr, val);
    }
    else if (auto expr = dyn_cast<CXXBindTemporaryExpr>(stmt)) {
        parseExpr(expr, val);
    }
    else if (auto expr = dyn_cast<SubstNonTypeTemplateParmExpr>(stmt)) {
        parseExpr(expr, val);
    }
    else if (isa<FloatingLiteral>(stmt) || isa<StringLiteral>(stmt)) {
        // Ignoring unsupported literals (May be used in debug output)
        val = NO_VALUE;
    }
    else {
        stmt->dumpColor();
        SCT_INTERNAL_FATAL(stmt->getBeginLoc(), "Unsupported statement");
    }
}

//-----------------------------------------------------------------------------

// Get terminator condition
const clang::Expr* ScParseExpr::getTermCond(const clang::Stmt* stmt) 
{
    using namespace clang;
    
    if (auto ifStmt = dyn_cast<const IfStmt>(stmt)) {
        return ifStmt->getCond();
    } else 
    if (auto binStmt = dyn_cast<const BinaryOperator>(stmt)) {
        return binStmt->getLHS();
    } else 
    if (auto condStmt = dyn_cast<const ConditionalOperator>(stmt)) {
        return condStmt->getCond();
    } else 
    if (auto switchStmt = dyn_cast<const SwitchStmt>(stmt)) {
        return switchStmt->getCond();
    } else 
    if (auto forStmt = dyn_cast<const ForStmt>(stmt)) {
        return forStmt->getCond();
    } else 
    if (auto whileStmt = dyn_cast<const WhileStmt>(stmt)) {
        return whileStmt->getCond();
    } else 
    if (auto doStmt = dyn_cast<const DoStmt>(stmt)) {
        return doStmt->getCond();
    } else {
        // Do nothing, it can be @break, @continue
    }
    return nullptr;
}


SValue ScParseExpr::evalSubExpr(clang::Expr *expr)
{
    SValue res;
    chooseExprMethod(expr, res);
    return res;
}

void ScParseExpr::parseExpr(clang::IntegerLiteral* expr, SValue& val)
{
    val = SValue(llvm::APSInt(expr->getValue(), expr->getType()->
                 isUnsignedIntegerType()), getLiteralRadix(expr));
}

void ScParseExpr::parseExpr(clang::CXXBoolLiteralExpr* expr, SValue& val) 
{
    val = SValue(SValue::boolToAPSInt(expr->getValue()), 10);
}

void ScParseExpr::parseExpr(clang::CXXNullPtrLiteralExpr* expr, SValue& val) 
{
    // Use zero integer as NULL
    val = SValue(llvm::APSInt(64, true), 10);
}

// Used for local variables access in left/right parts
void ScParseExpr::parseExpr(clang::DeclRefExpr* expr, SValue& val)
{
    using namespace std;
    using namespace llvm;
    auto* decl = expr->getDecl();

    if (auto* enumConstDecl = dyn_cast<clang::EnumConstantDecl>(decl)) {
        // Enum constants are not stored in state, evaluated immediately
        val = SValue(enumConstDecl->getInitVal(), 10);
        
    } else {
        // Change to decl->isCXXClassMember();
        if (isa<clang::RecordDecl>(decl->getDeclContext())) {
            // Record field
            if (auto* varDecl = dyn_cast<clang::VarDecl>(decl)) {
                if (varDecl->isStaticDataMember()) {
                    // Static data member
                    val = SValue(decl, NO_VALUE);
                            
                } else {
                    // Regular data member
                    val = SValue(decl, modval);
                }
            } else {
                SCT_TOOL_ASSERT (false, "Record has not variable declaration");
            }

        } else 
        if (!isa<clang::FunctionDecl>(decl->getDeclContext())) {
            // Global variable
            val = SValue(decl, NO_VALUE);
            
        } else 
        if (isUserDefinedClass(decl->getType())) {
            // Local record value 
            val = SValue(decl, modval);
            //cout << "Local record val " << val << ", modval " << modval << endl;
            
        } else {
            // Function local variable
            val = SValue(decl, modval);
        }
    }
}

// Create record/module object value and parse its field declarations
// Used for arrays of record elements
SValue ScParseExpr::createRecValue(const clang::CXXRecordDecl* recDecl, 
                                   const SValue& parent, const SValue& var, 
                                   size_t index) 
{
    using namespace clang;
    //cout << "createRecValue for var " << var << " index " << index << endl;

    std::vector<SValue> bases;
    for (auto base : recDecl->bases()) {
        auto type = base.getType();
        SValue bval = createRecValue(type->getAsCXXRecordDecl(), parent, var, 
                                     index);
        bases.push_back(bval);
    }

    // Record value
    SValue currec = SValue(QualType(recDecl->getTypeForDecl(), 0), 
                           bases, parent, var, index); 
    
    // Current module in @recval required for field initialization
    SValue lastRecval(recval); recval = currec;

    // Fill field values into state, required for array member of record array
    for (auto fieldDecl : recDecl->fields()) {
        parseValueDecl(fieldDecl, currec, nullptr);
    }
    
    // Restore current module before parse constructor call
    recval = lastRecval;

    return currec;
}

// Create record value for copy constructor with creating tuples for its fields 
// with copied values
// \return record value
SValue ScParseExpr::createRecordCopy(CXXConstructExpr* expr, 
                                     const SValue& val, const SValue& var) 
{
    //cout << "createRecordCopy of " << val << " for var " << var << endl;
    
    // Always create local record, copy of global record is local record
    SValue cval = SValue(val.getType(), val.getRecord().bases, 
                         val.getRecord().parent, var);
    state->putValue(var, cval);
    
    auto recDecl = val.getType()->getAsCXXRecordDecl();
    
    for (auto fieldDecl : recDecl->fields()) {
        // Get field value of copied record
        SValue fval(fieldDecl, val);
        SValue cfval(fieldDecl, cval);

        // Create deep copy of @ffval in state
        SValue ffval; state->getValue(fval, ffval);
        SValue cffval = state->copyIntSubValues(ffval, level, cval, cfval);
        // Put field value for constructed record object
        state->putValue(cfval, cffval);
        // Set level for record fields, no level for record values itself
        state->setValueLevel(cfval, level);
        //cout << "   cval " << cval << " lfval " << lfval << endl;

        // Put field into @codeWriter, does nothing in CPA
        if (fieldDecl->getType()->isArrayType()) {
            std::vector<size_t> arrSizes;
            parseArrayFieldDecl(fieldDecl, cfval, arrSizes);
            
        } else {
            parseFieldDecl(fieldDecl, cfval);
        }
    }
    
    return cval;
}

// Create record value for normal constructor with its base class constructor 
// and member initialization
// \return record value
SValue ScParseExpr::parseRecordCtor(CXXConstructExpr* expr, SValue parent,
                                    SValue currecvar, bool analyzeRecordCtor)
{
    //cout << "parseRecordCtor for var " << currecvar << endl;
    
    // Base classes constructors
    for (auto init : expr->getConstructor()->inits()) {
        if (init->isBaseInitializer()) {
            
            // Parse base constructor
            if (auto baseExpr = dyn_cast<CXXConstructExpr>(init->getInit())) {
                parseRecordCtor(baseExpr, currecvar, parent, false);
            }
        }
    }
    
    // Create value for this record and put it into record variable,
    // required to have variable for record in state, replaced in parseValueDecl
    SValue currec = createRecValue(expr->getBestDynamicClassType(), 
                                   parent, currecvar);
    state->putValue(currecvar, currec);
    //cout << "curmodvar " << curmodvar << ", curmod " << curmod << endl;
    
    // Prepare constructor parameters before initialization list because 
    // they can be used there, parameters declared in previous module
    prepareCallParams(expr, modval, expr->getConstructor());

    // Current module in @recval required for field initialization
    SValue lastRecval(recval); recval = currec;
    
    // Field declarations
    auto modDecl = expr->getType()->getAsCXXRecordDecl();
    //cout << "------- fields " << endl;
    for (auto fieldDecl : modDecl->fields()) {
        //cout << "field " << fieldDecl->getNameAsString() << endl;

        // Check if this field is in initializer list
        auto i = std::find_if(
                    expr->getConstructor()->inits().begin(),
                    expr->getConstructor()->inits().end(),
                    [fdecl = fieldDecl](auto init) {
                        return (init->isMemberInitializer() && 
                                init->getMember() == fdecl);
        });
        bool hasInit = i != expr->getConstructor()->inits().end();
        bool isRecord = isUserDefinedClass(fieldDecl->getType());

        // @init and @stmt can be @nullptr
        auto init = hasInit ? (*i)->getInit() : nullptr;
        // Default initializer processed from @fieldDecl, so clear @init
        init = (init && !isa<CXXDefaultInitExpr>(init)) ? init : nullptr;
        // @init used in initialization list, @getInClassInitializer used
        // in initialization in "{...}" or with assignment
        auto stmt = init ? init : fieldDecl->getInClassInitializer();

        //if (init) init->dumpColor(); else cout << "NULL init" << endl;

        if (isRecord) {
            // Record member
            //cout << "-------- record member" << endl;
            
            if (isScModuleOrInterface(fieldDecl->getType())) {
                ScDiag::reportScDiag(expr->getBeginLoc(), 
                                     ScDiag::SYNTH_LOCAL_MODULE_DECL);
            }
            // Remove @ExprWithCleanups from @CXXConstructExpr
            Expr* ctorExpr = removeExprCleanups(stmt);
                
            if (auto fieldCtor = dyn_cast<CXXConstructExpr>(ctorExpr)) {
                // Used to fill @locrecvar, no initialization used here
                SValue val;
                parseDeclStmt(stmt, fieldDecl, val, init);
            
                // Create member record recursively
                parseRecordCtor(fieldCtor, recval, locrecvar, false);
                //state->print();

            } else {
                SCT_TOOL_ASSERT (false, "No CXXConstructExpr in member initializer");
            }
        } else {
            //cout << "-------- non-record member" << endl;
            
            // Do not remove @stmt as it`s used to store expression in scope graph
            setNoRemoveStmt(true);
            SValue val;
            // Field is initialized with @init
            parseDeclStmt(stmt, fieldDecl, val, init);
            setNoRemoveStmt(false);

            storeStmtStr(stmt);
        }
    }

    // Restore current module before parse constructor call
    recval = lastRecval;
    
    // Activate call constructor body as function, parameters already prepared
    if (analyzeRecordCtor) {
        prepareCallContext(expr, modval, currec, expr->getConstructor(), NO_VALUE);
    }
    
    state->fillDerivedClasses(currec);
    
    return currec;
}

void ScParseExpr::parseExpr(clang::CXXThisExpr* expr, SValue& thisPtrVal)
{
    using namespace std;
    //cout << "parse CXXThisExpr, recval "  << recval << ", modval " << modval << endl;
    
    thisPtrVal = SValue(expr->getType()); // pointer
    SValue recVal = recval ? recval : modval;
    state->putValue(thisPtrVal, recVal, false);
    state->setValueLevel(thisPtrVal, level+1);
    
    //cout << "   level " << level << ", thisPtrVal " << thisPtrVal << endl;
}

void ScParseExpr::parseExpr(clang::MemberExpr *expr, SValue &val)
{
    using namespace clang;
    using namespace std;
    
    // Parse @this expression, put it into @tval
    if (!expr->getBase()) {
        SCT_TOOL_ASSERT (false, "In parseExpr for MemberExpr no base found");
    }
    
    SValue tval = evalSubExpr(expr->getBase());

    //cout << "ScParseExpr::MemberExpr tval = " << tval << ", base->getType() = " 
    //     << expr->getBase()->getType().getAsString() << endl;
    
    // Get record from variable/dynamic object
    SValue ttval = getRecordFromState(tval, ArrayUnkwnMode::amArrayUnknown);
    //cout << "final ttval = " << ttval << endl;
    //state->print();
    
    // Allowed parent kinds
    if (!ttval.isArray() && !ttval.isRecord() && !ttval.isVariable()) {
        ScDiag::reportScDiag(expr->getSourceRange().getBegin(),
                             ScDiag::SYNTH_INCORRECT_RECORD);
        cout << "ScParseExpr::parseExpr, incorrect : tval " << tval << ", ttval " << ttval << endl;
    }
    
    ValueDecl* decl = expr->getMemberDecl();
    val = (ttval.isUnknown()) ? NO_VALUE : SValue(decl, ttval);

    //cout << "ScParseExpr::MemberExpr tval = " << tval
    //     << ", expr->getType() = " << expr->getType().getAsString()
    //     << ", val = "  << val << endl;
}

void ScParseExpr::parseExpr(clang::ImplicitCastExpr *expr, SValue &val)
{
    using namespace clang;
    using namespace std;
    
    // Parse sub-expression
    val = evalSubExpr(expr->getSubExpr());
    auto castKind = expr->getCastKind();

    if (castKind == CK_DerivedToBase || castKind == CK_UncheckedDerivedToBase) {
        // Derived casted to base class
        // Type to cast -- not required up to now
        QualType exprType = expr->getType();
        bool isPtr = isPointer(exprType);
        bool isPortIf = isScPort(exprType);   // sc_port<IF>
        auto castType = exprType;
        // @tval can be NO_VALUE
        SValue tval = val;

        // Get record object from pointer and convert pointer type to the type
        if (isPtr) {
            SValue ttval;
            state->getValue(tval, ttval, false);
            tval = ttval;

            castType = exprType->getPointeeType();
            
        } else 
        if (isPortIf) {
            // Get first argument of @sc_port<IF>
            if (auto tmpArg = getTemplateArgAsType(exprType, 0)) {
                castType = tmpArg.getValue();
            }
        }
        
        // !!! Use getRecordFromState -- need to ensure it really required
        // Get record from variable/dynamic object
        bool isVariable = !tval.isRecord();
        if (isVariable) {
            SValue ttval;
            state->getValue(tval, ttval, false);
            tval = ttval;
        }

        if (tval.isRecord()) {
            // CXX class type cast
            if (!isUserDefinedClass(castType)) {
                SCT_TOOL_ASSERT (false, "Derived cast type is not class");
            }
            
            // Get base class of @tval with @castType type
            SValue bval = getBaseClass(tval, castType);
            //cout << "Base class " << bval.asString() << endl;

            // Add variable for base record object
            if (isVariable) {
                SValue bbval(castType);
                state->putValue(bbval, bval);
                state->setValueLevel(bbval, level+1);

                bval = bbval;
                //cout << "Variable to base class bval = " << bval.asString() << endl;
            }

            // Create pointer to base record object
            if (isPtr || isPortIf) {
                val = SValue(exprType);
                state->putValue(val, bval);
                state->setValueLevel(val, level+1);
                //cout << "Pointer to base class variable val = " << val.asString() << endl;
            } else {
                val = bval;
            }
            //cout << "Return val " << val.asString() << endl;
            
        } else {
            // Do nothing, it can be SC data type cast
        }

    } else 
    if (castKind == CK_DerivedToBaseMemberPointer) {
        // Member pointer in derived class to member pointer in base class
        SCT_INTERNAL_FATAL(expr->getBeginLoc(), "Unsupported cast kind");
        
    } else {
        // Another kind of type cast
        //cout << "ScParseExpr::ImplicitCastExpr val = " << val << endl;
    }
}

void ScParseExpr::parseExpr(clang::ExplicitCastExpr* expr, SValue& val)
{
    val = evalSubExpr(expr->getSubExpr());
}

void ScParseExpr::parseExpr(clang::ParenExpr* expr, SValue& val)
{
    val = evalSubExpr(expr->getSubExpr());
}

void ScParseExpr::parseExpr(clang::CXXDefaultArgExpr* expr, SValue& val)
{
    val = evalSubExpr(expr->getExpr());
}

void ScParseExpr::parseExpr(clang::CXXConstructExpr* expr, SValue& val)
{
    // Parse initialization expressions, it can have arbitrary type
    for (unsigned i = 0; i < expr->getNumArgs(); i++) {
        evalSubExpr(expr->getArg(i));
    }
}

void ScParseExpr::parseExpr(clang::OpaqueValueExpr* expr, SValue& val)
{
    SCT_INTERNAL_FATAL(expr->getBeginLoc(), 
                     "OpaqueValueExpr found, check C++ compiler error message");
    
    if (expr->getSourceExpr()) {
        val = evalSubExpr(expr->getSourceExpr());
    }
}

}