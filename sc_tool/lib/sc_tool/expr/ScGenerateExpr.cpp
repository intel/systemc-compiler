/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Author: Mikhail Moiseev
 */

#include "sc_tool/expr/ScParseRangeExpr.h"
#include "sc_tool/expr/ScGenerateExpr.h"
#include "sc_tool/diag/ScToolDiagnostic.h"
#include "sc_tool/utils/StringFormat.h"
#include "sc_tool/utils/ScTypeTraits.h"
#include "sc_tool/utils/CheckCppInheritance.h"
#include "sc_tool/utils/CppTypeTraits.h"
#include "sc_tool/utils/DebugOptions.h"
#include "sc_tool/ScCommandLine.h"

#include "clang/AST/Decl.h"
#include <sstream>

namespace sc {

using namespace std;
using namespace clang;
using namespace llvm;


// ---------------------------------------------------------------------------

ScGenerateExpr::ScGenerateExpr(
                    const clang::ASTContext& context_, 
                    std::shared_ptr<ScState> state_,
                    bool isCombProcess_,
                    const SValue& modval_,
                    ScVerilogWriter* codeWriter_) : 
    ScParseExpr(context_, state_, isCombProcess_, modval_),
    codeWriter(codeWriter_)
{
    using namespace std;
    using namespace clang;

    {
        auto typeInfo = getIntTraits(astCtx.CharTy);  
        SCT_TOOL_ASSERT(typeInfo, "No type info found");
        if (typeInfo->first != 8) {
            ScDiag::reportScDiag(ScDiag::CPP_NONSTD_TYPE_WIDTH) << "char";
        }
    }
    {
        auto typeInfo = getIntTraits(astCtx.UnsignedCharTy);  
        SCT_TOOL_ASSERT(typeInfo, "No type info found");
        if (typeInfo->first != 8) {
            ScDiag::reportScDiag(ScDiag::CPP_NONSTD_TYPE_WIDTH) << "char";
        }
    }
    {
        auto typeInfo = getIntTraits(astCtx.ShortTy);  
        SCT_TOOL_ASSERT(typeInfo, "No type info found");
        if (typeInfo->first != 16) {
            ScDiag::reportScDiag(ScDiag::CPP_NONSTD_TYPE_WIDTH) << "char";
        }
    }
    {
        auto typeInfo = getIntTraits(astCtx.UnsignedShortTy);  
        SCT_TOOL_ASSERT(typeInfo, "No type info found");
        if (typeInfo->first != 16) {
            ScDiag::reportScDiag(ScDiag::CPP_NONSTD_TYPE_WIDTH) << "char";
        }
    }
    {
        auto typeInfo = getIntTraits(astCtx.IntTy);  
        SCT_TOOL_ASSERT(typeInfo, "No type info found");
        if (typeInfo->first != 32) {
            ScDiag::reportScDiag(ScDiag::CPP_NONSTD_TYPE_WIDTH) << "int";
        }
    }
    {
        auto typeInfo = getIntTraits(astCtx.UnsignedIntTy);  
        SCT_TOOL_ASSERT(typeInfo, "No type info found");
        if (typeInfo->first != 32) {
            ScDiag::reportScDiag(ScDiag::CPP_NONSTD_TYPE_WIDTH) << "unsigned";
        }
    }
    {
        auto typeInfo = getIntTraits(astCtx.LongTy);  
        SCT_TOOL_ASSERT(typeInfo, "No type info found");
        if (typeInfo->first != 32 && typeInfo->first != 64) {
            ScDiag::reportScDiag(ScDiag::CPP_NONSTD_TYPE_WIDTH) << "long";
        }
    }
    {
        auto typeInfo = getIntTraits(astCtx.UnsignedLongTy);  
        SCT_TOOL_ASSERT(typeInfo, "No type info found");
        if (typeInfo->first != 32 && typeInfo->first != 64) {
            ScDiag::reportScDiag(ScDiag::CPP_NONSTD_TYPE_WIDTH) << "unsigned long";
        }
    }
    {
        auto typeInfo = getIntTraits(astCtx.LongLongTy);  
        SCT_TOOL_ASSERT(typeInfo, "No type info found");
        if (typeInfo->first != 64) {
            ScDiag::reportScDiag(ScDiag::CPP_NONSTD_TYPE_WIDTH) << "long long";
        }
    }
    {
        auto typeInfo = getIntTraits(astCtx.UnsignedLongLongTy);  
        SCT_TOOL_ASSERT(typeInfo, "No type info found");
        if (typeInfo->first != 64) {
            ScDiag::reportScDiag(ScDiag::CPP_NONSTD_TYPE_WIDTH) 
                    << "unsigned long long";
        }
    }     
}

// ---------------------------------------------------------------------------
// Utility functions

// Report warning if @val is register or pointer/reference to register
void ScGenerateExpr::reportReadRegister(const SValue& val, const Expr* expr) 
{
    SValue lval;
    if (val.isPointer()) {
        // Get dereferenced variable
        SValue rval; state->getValue(val, rval);
        lval = state->getVariableForValue(rval);   
        
    } else 
    if (val.isReference()) {
        state->getDerefVariable(val, lval);
        
    } else {
        lval = val;
    }
    //cout << "!!! val " << val << " lval " << lval << endl;
    if (codeWriter->isRegister(lval)) {
        ScDiag::reportScDiag(expr->getBeginLoc(), 
                             ScDiag::SYNTH_READ_REG_IN_RESET);
    }
}

// Check if argument is narrowing integral to boolean cast
bool ScGenerateExpr::isIntToBoolCast(const Expr* expr) 
{
    if (auto castExpr = dyn_cast<ImplicitCastExpr>(expr)) {
        auto castKind = castExpr->getCastKind();
        
        if (castKind == CK_IntegralToBoolean) {
            auto argType = castExpr->getSubExpr()->getType();
            
            if (auto typeInfo = getIntTraits(argType, false)) {
                size_t width = typeInfo.getValue().first;
                return (width > 1);
            }
        }
    }
    return false;
}

// Check if argument is SC channel
bool ScGenerateExpr::isArgChannel(const Expr* argExpr) {
    if (const MemberExpr* expr = dyn_cast<MemberExpr>(argExpr)) {
        if (expr->getMemberDecl()) {
            return isScChannel(expr->getMemberDecl()->getType());
        }
    } else 
    if (const UnaryOperator* oper = dyn_cast<UnaryOperator>(argExpr)) {
        if (oper->getOpcode() == UO_Deref) {
            return isScChannel(oper->getType());
        }
    } else 
    if (const CastExpr* expr = dyn_cast<CastExpr>(argExpr)) {
        return isArgChannel(expr->getSubExpr());
    }
    // Another expression type support may be required
    return false;
}

// Check if statement is operator of IO stream cin/cout
bool ScGenerateExpr::isIoStreamStmt(const Stmt* stmt) 
{
    if (auto callExpr = dyn_cast<CXXOperatorCallExpr>(stmt)) {
        if (callExpr->getNumArgs() > 0) {
            QualType thisType = callExpr->getArg(0)->getType();
            return isIoStream(thisType);
        }
    }
    return false;
}


// Check if stmt is wait(n) 
bool ScGenerateExpr::isWaitNStmt(const Stmt* stmt)
{
    if (auto callExpr = dyn_cast<const CallExpr>(stmt)) 
    {
        const FunctionDecl* funcDecl = callExpr->getDirectCallee();
        SCT_TOOL_ASSERT (funcDecl, "No function found for call expression");
        string fname = funcDecl->getNameAsString();
        
        string nsname = "unknown";
        if (auto nsDecl = dyn_cast<NamespaceDecl>(funcDecl->
                          getEnclosingNamespaceContext())) {
            nsname = nsDecl->getNameAsString();
        }
    
        if (nsname == "sc_core" && fname == "wait" && 
            callExpr->getNumArgs() == 1) 
        {
            return isAnyInteger(callExpr->getArg(0)->getType());
        }
    }
    return false;
}

// Returns true if statement is call of @sct_assert_in_proc_func()
bool ScGenerateExpr::isTemporalAssert(const Stmt* stmt) const
{
    if (auto expr = dyn_cast<CallExpr>(stmt)) {
        
        if (auto funcDecl = expr->getDirectCallee()) {
            std::string fname = funcDecl->getNameAsString();
            return (fname == "sct_assert_in_proc_func");
        }
    }
    return false;
}

// Returns true if statement is call of @sct_assert()
bool ScGenerateExpr::isSctAssert(const Stmt* stmt) const
{
    if (auto expr = dyn_cast<CallExpr>(stmt)) {
        
        if (auto funcDecl = expr->getDirectCallee()) {
            std::string fname = funcDecl->getNameAsString();
            return (fname == "sct_assert");
        }
    }
    return false;
}

// Get string from char* argument
llvm::Optional<std::string> 
ScGenerateExpr::getStringFromArg(Expr* argExpr) {
    
    Expr* expr = argExpr;
    if (auto* castExpr = dyn_cast<ImplicitCastExpr>(expr)) {
        expr = castExpr->getSubExpr();
    } 
    if (auto* strExpr = dyn_cast<clang::StringLiteral>(expr)) {
        std::string res = strExpr->getString().str();
        return res;
    }
    
    return llvm::None;
}

// Parse and evaluate pointer/time expression as constant integer, operates
// with constants only, no @keepConstVariables/@calcPartSelectArg checked
// \param val  -- integer if returned true or some other evaluated value else
// \return <result, integer value of result>
std::pair<SValue, SValue>  
ScGenerateExpr::evaluateConstIntNoCheck(Expr* expr)
{
    // Clone state to avoid its modification
    // Use current @recval as it can be called from record constructor
    //cout << "evaluateConstIntPtr expr# " << hex << expr << dec << endl;
    ScParseExprValue parseExprValue(astCtx, shared_ptr<ScState>(state->clone()), 
                                    true, modval, false, recval);
    return parseExprValue.evaluateConstInt(expr, true);
}

// Parse and evaluate one expression/statement as constant integer
// returns NO_VALUE for expression with references
// \return <result, integer value of result>
std::pair<SValue, SValue>  
ScGenerateExpr::evaluateConstInt(Expr* expr, bool checkConst,
                                 bool checkRecOnly)
{
    return evaluateConstIntNoCheck(expr);
}

// The same as previous one, just return integer value only
// \return <integer value of result>
SValue ScGenerateExpr::evaluateConstInt(clang::Expr* expr, const SValue& val, 
                                        bool checkConst)
{
    if (val.isInteger()) return val;
    
    // Do not try to get value from @state as it does not have values for 
    // all array elements, only for first one
    
    return evaluateConstIntNoCheck(expr).second;
}

// Evaluate variable dependent range (@hexpr, @lexpr) expression
// Returns @true if non-constant variable is in the range
bool ScGenerateExpr::evaluateRangeExpr(Expr* hexpr, Expr* lexpr)
{
    // Evaluating the range expression
    ScParseRangeExpr parseRange(astCtx, state, true, modval);
    
    SValue hval;
    parseRange.evaluate(hexpr, hval);
    auto hvars = parseRange.getVariables();
    size_t hvarNum = hvars.size();
    SValue lval;
    parseRange.evaluate(lexpr, lval);
    auto lvars = parseRange.getVariables();
    size_t lvarNum = lvars.size();
    
    /*cout << "simplifyRangeExpr hval = " << hval << endl;
    for (auto i : hvars) {
        cout << "   " << i << endl;
    }

    cout << "simplifyRangeExpr lval = " << lval << endl;
    for (auto i : lvars) {
        cout << "   " << i << endl;
    }*/
    
    // Check high and low index variables are the same
    bool equalVars = (lvarNum == hvarNum);
    if (equalVars) {
        for (auto h = hvars.begin(); h != hvars.end(); ++h) {
            bool found = false;
            for (auto l = lvars.begin(); l != lvars.end(); ++l) {
                if (*h == *l) {
                    found = true; 
                    lvars.erase(l);
                    break;
                }
            }
            equalVars = equalVars && found;
        }
    }
    equalVars = equalVars && lvars.empty();
    
    if (equalVars) {
        if (!hval.isInteger()) {
            ScDiag::reportScDiag(hexpr->getBeginLoc(),
                                 ScDiag::SC_RANGE_NO_INTEGER);
        }
        if (!lval.isInteger()) {
            ScDiag::reportScDiag(lexpr->getBeginLoc(), 
                                 ScDiag::SC_RANGE_NO_INTEGER);
        }
        //cout << "varNum " << hvarNum << endl;
        
        if (hvarNum == 0) {
            // Store calculated values to indices
            codeWriter->putLiteral(hexpr, hval);
            codeWriter->putLiteral(lexpr, lval);
            return false;
            
        } else {
            // Parse lo index expression and put it to hi index expression
            codeWriter->copyTerm(lexpr, hexpr);
            // Put delta to lo index expression
            APSInt hint; APSInt lint;
            adjustIntegers(hval.getInteger(), lval.getInteger(), hint, lint);
            APSInt delta = hint - lint;
            delta.operator ++();    // plus 1
            SValue rval(delta, 10);
            codeWriter->putLiteral(lexpr, rval);
            return true;
        }
    } else {
        ScDiag::reportScDiag(hexpr->getSourceRange().getBegin(), 
                             ScDiag::SC_RANGE_DIFF_VARS);
        // Required to prevent more errors
        SCT_INTERNAL_FATAL_NOLOC ("Incorrect range error");
    }
    return false;
}


// Parse SCT_ASSERT in module scope and put assertion string into codeWriter
// \return statement for which assertion string is stored
const clang::Stmt* ScGenerateExpr::parseSvaDecl(const clang::FieldDecl* fdecl) 
{
    Expr* expr = removeExprCleanups(fdecl->getInClassInitializer());
    
    std::vector<Expr*> args;
    if (CXXConstructExpr* ctorExpr = dyn_cast<CXXConstructExpr>(expr)) {
        for (auto arg : ctorExpr->arguments()) {
            args.push_back(arg);
        }
    }
    SCT_TOOL_ASSERT (args.size() == 6 || args.size() == 7, 
                     "Incorrect argument number");

    // Set @parseSvaArg to check function call there
    codeWriter->setParseSvaArg(true);
    
    SValue fval; SValue sval; SValue stval;
    chooseExprMethod(args[0], fval);
    chooseExprMethod(args[1], fval);
    chooseExprMethod(args[2], fval);
    assertName = getStringFromArg(args[3]);
    
    fval = evaluateConstIntNoCheck(args[4]).second;
    if (args.size() == 7) {
        sval = evaluateConstIntNoCheck(args[5]).second;
        stval = evaluateConstIntNoCheck(args[6]).second;
    } else {
        sval = fval;
        stval = evaluateConstIntNoCheck(args[5]).second;
    }

    if (fval.isInteger() && sval.isInteger() && stval.isInteger()) {
        unsigned stable = stval.getInteger().getExtValue();
        unsigned timeInt = abs(sval.getInteger().getExtValue() -
                               fval.getInteger().getExtValue());
        
        std::string timeStr = parseSvaTime(fval.getInteger().getExtValue(), 
                                           sval.getInteger().getExtValue(), 
                                           stable);
        codeWriter->putTemporalAssert(expr, args[0], args[1], timeStr, args[2], 
                                      stable, timeInt);
    } else {
        ScDiag::reportScDiag(expr->getBeginLoc(), 
                             ScDiag::SYNTH_SVA_INCORRECT_TIME);
    }
    codeWriter->setParseSvaArg(false);
    
    return expr;
}

// Generate function parameter assignments
void ScGenerateExpr::prepareCallParams(clang::Expr* expr, 
                                       const SValue& funcModval, 
                                       const clang::FunctionDecl* callFuncDecl) 
{
    bool isCall = isa<CallExpr>(expr);
    bool isCtor = isa<CXXConstructExpr>(expr);
    SCT_TOOL_ASSERT (isCall || isCtor, "No function call or constructor");

    auto callExpr = isCall ? dyn_cast<CallExpr>(expr) : nullptr;
    auto ctorExpr = isCtor ? dyn_cast<CXXConstructExpr>(expr) : nullptr;

    bool isOperator = callFuncDecl->isOverloadedOperator();
    unsigned paramIndx = 0;
    unsigned paramNum = callFuncDecl->getNumParams();
    auto arguments = isCall ? callExpr->arguments() : ctorExpr->arguments();

    // Set function parameters flag to avoid another function/constructor call 
    bool lastFuncParams = inFuncParams; inFuncParams = true;

    for (auto arg : arguments) {
        // Skip first argument for operator which is this
        if (isOperator) {
            isOperator = false; continue;
        }
        SCT_TOOL_ASSERT (paramIndx < paramNum, "Incorrect parameter index");
        
        // Remove @ExprWithCleanups from @CXXConstructExpr
        auto argExpr = removeExprCleanups(arg);
        // Check for copy constructor to use record RValue
        auto argCtorExpr = getCXXCtorExprArg(argExpr);
        if (argCtorExpr) {
            if (!argCtorExpr->getConstructor()->isCopyOrMoveConstructor()) {
                ScDiag::reportScDiag(expr->getBeginLoc(), 
                         ScDiag::SYNTH_NONTRIVIAL_COPY);
            }
        }
        
        ParmVarDecl* parDecl = const_cast<ParmVarDecl*>(
                               callFuncDecl->getParamDecl(paramIndx++));
        // Get original type, required as array passed to function as pointer
        QualType type = parDecl->getType();
        if (isScPort(type)) {
            ScDiag::reportScDiag(expr->getBeginLoc(), 
                        ScDiag::SYNTH_SC_PORT_INCORRECT) << "in function parameter";
        }

        // Skip zero width parameter
        if (isZeroWidthType(type) || isZeroWidthArrayType(type)) continue;
        
        bool isRef = isReference(type);
        bool isConstRef = sc::isConstReference(type);
        bool isPtr = isPointer(type);
        bool isArray = type->isArrayType();
        bool isConst = (isRef) ? type.getNonReferenceType().
                        isConstQualified() : type.isConstQualified();
        bool isRecord = isUserClass(getDerefType(type));

        if (auto dtype = dyn_cast<DecayedType>(type)) {
            type = dtype->getOriginalType();
        }
        
        // Fill state with array elements and initialization values if 
        // variable is constant type, use @arg as initialization expression
        // Parameter variable, can be array variable
        // Use module @funcModval where function is called
        bool lastAssignLHS = assignLHS;
        if (isRef && !isConstRef) assignLHS = true;
        
        auto parvals = parseValueDecl(parDecl, funcModval, arg, true);

        assignLHS = lastAssignLHS;
        SValue pval = parvals.first;
        SValue ival = (parvals.second.size() == 1) ? 
                       parvals.second.front() : NO_VALUE;
        
        // For non-reference and constant reference try to evaluate integer,
        // there is temporary variable for constant reference, so bit/range supported
        SValue iival = ival;
        if (!isRecord && (!isRef || isConst)) {
            if (ival.isInteger()) {
                // Put constant integer for the initializer expression
                codeWriter->putLiteral(arg, ival);
            }
        }
        
        if (isRef) {
            //cout << "Ref pval " << pval << " ival " << ival << " iival " << iival << endl;
            const QualType& refType = getDerefType(type);
            // Check channel has the same type as constant reference parameter
            bool isChanRef = ival.isScChannel() && ival.getScChannel()->
                             getType()->getCanonicalTypeInternal() == 
                             refType->getCanonicalTypeInternal();
            
            // Reference can be to array element or dynamic object
            if ( !(iival.isVariable() || iival.isTmpVariable() || 
                   iival.isArray() || iival.isSimpleObject() || isChanRef) )
            {
                // Constant reference refers to RValue, special variable created
                if (isRecord)
                    SCT_INTERNAL_ERROR (argExpr->getBeginLoc(), 
                                        "Unexpected record initialization");
                SCT_TOOL_ASSERT (isConst, "Incorrect reference initializer");
                
                // Put constant reference to itself to use variable name in code
                codeWriter->storeRefVarDecl(pval, pval);
                // Declare constant reference variable w/o initialization,
                // always declare parameter as range/bit selection can be used
                codeWriter->putVarDecl(nullptr, pval, refType, nullptr, true, 
                                       level);
                // Add initialization with the argument corresponds to declaration
                codeWriter->putFCallParam(expr, pval, arg);
                //cout << "prepareCallParams ConstRef pval " << pval << " iival " << iival << endl;
                
            } else {
                // Non-constant or constant reference refers to LValue
                // It is replaced with @arg string in generated code
                // Do not check if there is no terms
                codeWriter->storeRefVarDecl(pval, arg, false);
                
                // Store indices string for array record passed by reference
                bool isRecordRef = isUserClass(refType);
                if (isRecordRef) {
                    string s;
                    if (ival.isReference()) {
                        s = recarrRefIndices[ival];
                    } else {
                        s = codeWriter->getIndexString(ival);
                    }
                    recarrRefIndices[pval] = s;
                    //cout << "prepareCallParams for isRecordRef pval " << pval 
                    //     << " ival " << ival << " indxStr " << s << endl;
                }
            }

        } else 
        if (isPtr || isArray) {
            // For pointers and arrays (really array always converted to pointer)
            // Check if argument is pointer array at unknown index
            bool unkwIndex;
            bool isArr = state->isArray(ival, unkwIndex);
            
            if (isArr) {
                // It is replaced with @arg string in generated code
                codeWriter->storePointerVarDecl(pval, arg);
                
            } else {
                // Try to get channel value from pointe
                SValue cval = getChannelFromState(ival);
                codeWriter->storePointerVarDecl(pval, ival, cval);
            }

        } else {
            // Variable declaration w/o initialization
            codeWriter->putVarDecl(nullptr, pval, type, nullptr, true, level);

            // Add parameter initialization with the argument
            if (isRecord) {
                // Get field initialization string from @CXXConstructExpr
                if (argCtorExpr) {
                    codeWriter->addTerm(argCtorExpr, expr);
                }
                
            } else {
                codeWriter->putFCallParam(expr, pval, arg);
            }
        }
    }
    
    inFuncParams = lastFuncParams;
    
    // Put empty string to ensure there is term for @expr, required for call
    // in right part of &&\||
    if (!codeWriter->getStmtString(expr)) {
        codeWriter->putEmptyFCallParam(expr);
    }
    
    // Print constructor parameters before its body to insert initialization list 
    // between them, for normal function body graph tied to parameters expression
    if (isCtor) {
        storeStmtStrNull(expr);
    }
}

// Get vector with parent record values for given field variable or record value
// For record array it get record value from its most inner zero element
// \param val -- record field variable or record value
// \return -- vector with record values in hierarchy
std::vector<SValue> ScGenerateExpr::getRecVector(const SValue& val) 
{
    //cout << "\ngetRecVector for val " << val << " : " << endl;
    std::vector<SValue> recarrs;
    
    // Get parent record value
    SValue parval;
    if (val.isVariable()) {
        // @val is record field variable
        parval = val.getVariable().getParent();
    } else 
    if (val.isRecord()) {
        // @val is parent record 
        parval = val;
    } else 
    if (val.isScChannel()) {
        auto chanType = val.getScChannel()->getType();
        if (isUserClass(chanType)) {
            // @val is record channel 
            parval = val;
        }
    }
        
    if (parval.isRecord() || parval.isScChannel()) {
        // Fill vector of parent record variables, check @synmodval to avoid
        // hangs up for MIF process accessing parent module fields
        //cout << " parval " << parval << ", modval " << modval 
        //     << " synmodval " << synmodval << endl;
        while (parval && parval != modval && parval != synmodval) {
            SValue recvar = state->getVariableForValue(parval);
            //cout << " parval " << parval << ", recvar " << recvar << endl;
            if (!recvar.isVariable()) break;
            
            //cout << "  recvar " << recvar << endl;
            SValue recarr = state->getValue(recvar);
            // Find most inner array object as it is used in @arraySubIndices
            SValue aval = state->getValue(recarr);
            while (aval.isArray()) {
                recarr = aval;
                aval = state->getValue(recarr);
            }
            recarrs.push_back(recarr);
            
            // Go upper along hierarchy 
            parval = recvar.getVariable().getParent();
            //cout << "  parval " << parval << endl;
        }
        //cout << endl;
    }
    //state->print();
    return recarrs;
}

// Put any member expression specified in @val for @stmt 
void ScGenerateExpr::putMemberExpr(const Expr* expr, const SValue& val, 
                                   const string& refRecarrIndxStr) 
{
    // Get parent record value
    SValue tval; 
    if (val.isVariable()) {
        tval = val.getVariable().getParent();
        
        // Check for array of @sc_port -- not supported 
        QualType type = val.getType();
        if (isArray(type) || isScVector(type)) {
            if ( isScPort(getArrayElementType(type)) ) {
                ScDiag::reportScDiag(expr->getBeginLoc(), 
                                     ScDiag::SYNTH_SC_PORT_ARRAY);
            }
        }
    }
    tval = getRecordFromState(tval, ArrayUnkwnMode::amNoValue);
    // Get record values vector to create name suffix for record member
    auto recvecs = getRecVector(val);

//    cout << "\nputMemberExpr(1) #" << hex << expr << dec << " val " << val 
//         << " , recval " << recval << ", modval " << modval 
//         << ", recvars size " << recvecs.size() << ", tval " << tval << endl;
//    cout << "  getMIFName " << codeWriter->getMIFName().first << " " << codeWriter->getMIFName().second << endl;
//    cout << "  getRecordName " << codeWriter->getRecordName().first << " " << codeWriter->getRecordName().second << endl;
    //state->print();
    
    // Check current MIF array element as a parent of the member
    bool elemOfMifArr = val.isVariable() && 
                        state->checkRecord(val, codeWriter->getMIFName().first,
                                           ScState::MIF_CROSS_NUM);
    // Check access to member of record/MIF from this record/MIF called method,
    // check if @tval is same or base class of current record (@this)
    SValue currecval = codeWriter->getRecordName().first;
    bool elemOfRecArr = tval.isRecord() && currecval.isRecord() &&
                        checkBaseClass(tval, currecval);
    // check if @tval is member or base class of member of current record (@this)
    if (SValue tvar = state->getVariableForValue(tval)) {
        SValue tpar = tvar.getVariable().getParent();
        elemOfRecArr = elemOfRecArr || (tval.isRecord() && currecval.isRecord() && 
                       checkBaseClass(tpar, currecval));
        //cout << "\nputMemberExpr(2) val " << val << ", tval " << tval << ", tvar " << tvar 
        //     << ", currecval " << currecval << endl;
    }
    
    //cout << "elemOfMifArr " << elemOfMifArr << " elemOfRecArr " << elemOfRecArr << endl;
    
    // Return NO_VALUE for non-channels including record channel fields
    SValue cval = getChannelFromState(val);
    //cout << "  cval " << cval << endl;
    
    if (cval) {
        QualType ctype = cval.getScChannel()->getType();
        if ( isUserClass(ctype) ) {
            // Put nothing for record channel, channels for individual fields 
            // are put in assignment and read
        } else {
            // Put channels to provide different read/write names
            codeWriter->putChannelExpr(expr, cval, recvecs, elemOfMifArr, 
                                       elemOfRecArr, "");
        }
    } else {
        codeWriter->putValueExpr(expr, val, recvecs, elemOfMifArr, elemOfRecArr, 
                                 refRecarrIndxStr, "");
    }
}


// Put any pointer variable de-referenced string, including pointer parameter
// Used everywhere pointer de-referenced or pointer method called
void ScGenerateExpr::putPtrDerefExpr(const Expr* expr, 
                                     const SValue& rvar,
                                     const SValue& rval, 
                                     const SValue& val)
{
    if (!codeWriter->putLocalPtrValueExpr(expr, rval)) {
        //cout << "putPtrDerefExpr rvar " << rvar << ", rval " << rval << ", val " << val << endl;

        SValue rrval;
        if (val.isVariable() || val.isTmpVariable()) {
            // Use pointed variable/object name
            rrval = val;
            
        } else 
        if (rval.isVariable() || rval.isTmpVariable()) {
            // Use pointer variable name for dynamic object
            SCT_TOOL_ASSERT (false, "Apply putPtrDerefExpr for @lval is variable");
            rrval = rval;
            
        } else 
        if (rval.isObject() && rvar.isVariable()) {
            // Use variable to pointer variable, required for array of pointers
            rrval = rvar;
            
        } else {
            // Do nothing, that could be @this object at method call
        }

        if (!rrval.isUnknown()) {
            // Put expression into code writer
            putMemberExpr(expr, rrval, "");
        }
    }
}

// ---------------------------------------------------------------------------
// Parse statement functions

// Integer literal
void ScGenerateExpr::parseExpr(IntegerLiteral* expr, SValue& val) 
{
    ScParseExpr::parseExpr(expr, val);
    codeWriter->putLiteral(expr, val);
}

void ScGenerateExpr::parseExpr(ConstantExpr* expr, SValue& val)
{
    ScParseExpr::parseExpr(expr, val);
    codeWriter->putLiteral(expr, val);
}

// Boolean literal
void ScGenerateExpr::parseExpr(CXXBoolLiteralExpr* expr, SValue& val) 
{
    ScParseExpr::parseExpr(expr, val);
    codeWriter->putLiteral(expr, val);
}

// @nullptr literal
void ScGenerateExpr::parseExpr(CXXNullPtrLiteralExpr* expr, SValue& val) 
{
    ScParseExpr::parseExpr(expr, val);
    codeWriter->putLiteral(expr, val);
}

void ScGenerateExpr::parseExpr(clang::StringLiteral* expr, SValue& val) 
{
    ScParseExpr::parseExpr(expr, val);
    codeWriter->putLiteral(expr, val);
}

// Used for local variables access in left/right parts
void ScGenerateExpr::parseExpr(DeclRefExpr* expr, SValue& val)
{
    ScParseExpr::parseExpr(expr, val);
    
    refRecarrIndx = "";
    
    // Report warning for read register in reset section
    if (codeWriter->isResetSection() && !assignLHS) {
        reportReadRegister(val, expr);
    }
    
    // Do de-reference for reference and pointer type 
    if (val.isReference()) {
        // Try to put local reference
        if (codeWriter->putLocalRefValueExpr(expr, val)) {
            
            auto i = recarrRefIndices.find(val);
            if (i != recarrRefIndices.end()) {
                refRecarrIndx = i->second;
            }
            //cout << "DeclRefExpr val " << val << " " << refRecarrIndx << endl;
            //state->print();
             
            return; 
        }
        // Module/global reference
        SValue lval(val);
        // Put referenced variable into @val
        state->getValue(lval, val, false);  
        
    } else 
    if (val.isPointer()) {
        // Try to put local pointer value
        if (codeWriter->putLocalPtrValueExpr(expr, val)) {
            return;
        }
        // Module/global pointer 
    }
    
    // Do not put channel, it could be used in reference to channel
    if (!val.isScChannel()) {
        // Local variable declared w/o record prefix, no @recvecs and no @tval used
        codeWriter->putValueExpr(expr, val);
    }
}

// Any access of member variable or method 
void ScGenerateExpr::parseExpr(MemberExpr* expr, SValue& val) 
{
    //cout << "parseExpr " << hex << expr << dec << endl;
    
    // Set @keepArrayIndices to do not erase them from @arraySubIndices
    // required for array of channel/record/MIF with inner record
    QualType baseType = getDerefType(expr->getBase()->getType());
    bool setSaveArrayIndices = false;
    bool isRecord = isUserClass(baseType, true);
    bool isRecordChan = isUserClassChannel(baseType, true).hasValue();

    if ((isRecord || isRecordChan) && !codeWriter->isKeepArrayIndices()) {
        codeWriter->setKeepArrayIndices();
        setSaveArrayIndices = true;
    }
    
    ScParseExpr::parseExpr(expr, val);
    
    if (setSaveArrayIndices) {
        codeWriter->resetKeepArrayIndices();
    }
    
    // Report warning for read register in reset section
    if (codeWriter->isResetSection() && !assignLHS) {
        reportReadRegister(val, expr);
    }
    
    // Do de-reference for reference type 
    if (val.isReference()) {
        // Try to put local reference
        if (codeWriter->putLocalRefValueExpr(expr, val)) {
            return;
        }
        // Module/global reference
        SValue lval(val);
        // Put referenced variable into @val
        state->getValue(lval, val, false);
        
    } else 
    if (val.isPointer()) {
        // Do not try to put pointer value as it is not local one
        
    }
    
    // Put expression into code writer
    putMemberExpr(expr, val, refRecarrIndx);
    // Clear indices suffix after use
    refRecarrIndx = "";
    
    //cout << "ScGenerateExpr::parseExpr(MemberExpr*) >>> #" << hex << expr << dec << endl;
}

// Set root to determine module hierarchy
void ScGenerateExpr::parseExpr(CXXThisExpr* expr, SValue& val)
{
    ScParseExpr::parseExpr(expr, val);
    // No terms should be added here
}

// Used for explicit/implicit type cast and LValue to RValue cast
void ScGenerateExpr::parseExpr(ImplicitCastExpr* expr, SValue& rval, SValue& val)
{
    // Parse sub-expression is inside
    ScParseExpr::parseExpr(expr, rval, val);
    auto castKind = expr->getCastKind();

    auto i = constReplacedFunc.find(expr->getSubExpr());
    if (i != constReplacedFunc.end()) {
        constReplacedFunc.emplace(expr, i->second);
        //cout << "Add ImplicitCast stmt " << hex << expr << " for " << i->second << dec << endl;
    }
    
    // Checking cast pointer to boolean and substitute literal if possible
    if (castKind == CK_MemberPointerToBoolean || castKind == CK_PointerToBoolean) {
        // Convert pointer to boolean, true if pointer value is object
        // Do not check pointer is constant as pointer arithmetic is prohibited
        SValue rval = getValueFromState(val);
        codeWriter->putLiteral(expr, SValue(SValue::boolToAPSInt(
                                            rval.isObject()), 10));
        return;
        
    } else 
    if (castKind == CK_IntegralToBoolean) {
        // Convert integer to boolean for literal and variable
        if (val.isInteger()) {
            val = SValue(SValue::boolToAPSInt(val.getBoolValue()), 10);
            codeWriter->putLiteral(expr, val);
            return;
            
        } else {
            // OR reduction required to preserve C++ semantic of boolean type
            string opcodeStr = "|";
            // Add brackets for complex expression
            if (!val.isVariable()) {
                codeWriter->copyTermInBrackets(expr->getSubExpr(), 
                                               expr->getSubExpr());
            }
            codeWriter->putUnary(expr, opcodeStr, expr->getSubExpr(), true);
            return;
        }
    } else
    // May be some other cast types should be added
    if (castKind == CK_IntegralCast) {
        
        QualType type = expr->getType();
        QualType origType = expr->getSubExpr()->getType();
        bool isBool = origType->isBooleanType();
        bool isSigned = isSignedType(type);
        bool isOrigSigned = isSignedType(origType);

        if (val.isInteger()) {
            // Reduce literal for narrowing cast (required for CK_IntegralCast)
            auto typeInfo = getIntTraits(type, true);
            SCT_TOOL_ASSERT (typeInfo, "No integral type width extracted");
            size_t width = typeInfo.getValue().first;
            bool isUnsigned = typeInfo.getValue().second;
        
            val = SValue(extrOrTrunc(val.getInteger(), width, isUnsigned), 
                         val.getRadix());
            
            codeWriter->putLiteral(expr, val);
            
        } else {
            // No type cast required, in Verilog RHS implicitly narrowed to LHS
            codeWriter->copyTerm(expr->getSubExpr(), expr);
        }
        
        // Set/reset sign cast for literals and expressions
        // Skip boolean type conversion as it considered as unsigned
        // No special rule for @enum required, unsigned @enum casted to @int
        if (!isBool && isOrigSigned != isSigned) { 
            codeWriter->putSignCast(expr, isSigned ? CastSign::SCAST : 
                                                     CastSign::UCAST);
        } else 
        if (isBool && isOrigSigned != isSigned) {
            codeWriter->putBoolCast(expr);
        }  
        return;
    }
    
    codeWriter->copyTerm(expr->getSubExpr(), expr);
}

void ScGenerateExpr::parseExpr(ExplicitCastExpr* expr, SValue& rval, SValue& val)
{
    ScParseExpr::parseExpr(expr, rval, val);
    QualType type = expr->getType();
    QualType origType = expr->getSubExpr()->getType();
    
    // Partial select applied only for C++ or SC integral type
    if (isVoidType(type) && (isa<CStyleCastExpr>(expr) || 
        isa<CXXFunctionalCastExpr>(expr) || isa<CXXStaticCastExpr>(expr))) {
        // Explicit cast to void, remove it from code
        // Used to replace assert in NDEBUG mode, see assert.h
        // Do not put anything for @expr
            
    } else {
        //cout << hex << (size_t)expr << " " << (size_t)expr->getSubExpr()<< dec << " rval " << rval << " val " << val << endl;
        if (getIntTraits(type, true) && !isBoolType(type) &&
            (isa<CStyleCastExpr>(expr) || isa<CXXFunctionalCastExpr>(expr) || 
             isa<CXXStaticCastExpr>(expr))) {
            // Explicit cast
            // Not required to support boolean converting as it is done 
            // in underlying implicit cast
            // Type cast with partial select possible
            codeWriter->putTypeCast(expr->getSubExpr(), expr, type);

        } else {
            // No partial select here
            codeWriter->copyTerm(expr->getSubExpr(), expr);
        }

        bool isBool = origType->isBooleanType();
        bool isSigned = isSignedType(type);
        bool isOrigSigned = isSignedType(origType);

        // Set/reset sign cast for literals and expressions
        // Skip boolean type conversion as it considered as unsigned
        if (!isBool && isOrigSigned != isSigned) { 
            codeWriter->putSignCast(expr, isSigned ? CastSign::SCAST : 
                                                     CastSign::UCAST);
        }
    }
}

// Parenthesized expression, i.e. expression in "()"
void ScGenerateExpr::parseExpr(ParenExpr* expr, SValue& val)
{
    ScParseExpr::parseExpr(expr, val);
    
    // Do not add brackets for concatenation like {a, b}
    bool isConcatSubExpr = false;
    if (auto operExpr = dyn_cast<CXXOperatorCallExpr>(expr->getSubExpr())) {
        OverloadedOperatorKind op = operExpr->getOperator();
        isConcatSubExpr = op == OO_Comma;
    }
    
    if (isConcatSubExpr) {
        codeWriter->copyTerm(expr->getSubExpr(), expr); 
    } else {
        codeWriter->copyTermInBrackets(expr->getSubExpr(), expr); 
    }
}

// Used for default argument value in function calls
void ScGenerateExpr::parseExpr(CXXDefaultArgExpr* expr, SValue& val)
{
    ScParseExpr::parseExpr(expr, val);
    codeWriter->copyTerm(expr->getExpr(), expr);
}

// Used for default initializer in constructor or in aggregate initialization
void ScGenerateExpr::parseExpr(CXXDefaultInitExpr* expr, SValue& val)
{
    // Cannot be called from process
    ScDiag::reportScDiag(expr->getSourceRange().getBegin(), 
                         ScDiag::SC_CXX_INIT_IN_PROC_FUNC);
}

// Used for default initializer in constructor or aggregate initialization, T{}
void ScGenerateExpr::parseExpr(InitListExpr* expr, SValue& val)
{
    QualType type = expr->getType();
    
    if (type->isArrayType()) {
        // Implemented in @parseDeclStmt()
            
    } else {
        if (isUserClass(type)) {
            if (auto recDecl = type->getAsCXXRecordDecl()) {
                SValue var = locrecvar ? locrecvar : SValue(type, NO_VALUE);
                val = createRecValue(recDecl, NO_VALUE, var, true, 0, false);
                temprec = val;
                //cout << "val " << val << endl;
                //state->print();

            } else {
                SCT_INTERNAL_ERROR(expr->getBeginLoc(), "Incorrect class type");
            }
        } else {
            if (expr->getNumInits() == 0) {
                val = SValue(SValue::zeroValue(type));
                codeWriter->putLiteral(expr, val);
            } else
            if (expr->getNumInits() == 1) {
                val = evalSubExpr(expr->getInit(0));
                codeWriter->putLiteral(expr, val);
            } else {
                ScDiag::reportScDiag(expr->getBeginLoc(), 
                    ScDiag::SYNTH_UNSUPPORTED_INIT) << " unexpected number of values";
            }
        }
    }
}

// Used for construction temporary record object, T()
void ScGenerateExpr::parseExpr(CXXTemporaryObjectExpr* expr, SValue& val)
{
    QualType type = expr->getType();
    
    if (isUserClass(type)) {
        if (auto recDecl = type->getAsCXXRecordDecl()) {
            SValue var = locrecvar ? locrecvar : SValue(type, NO_VALUE);
            val = createRecValue(recDecl, NO_VALUE, var, true, 0, false);
            temprec = val;
            //cout << "val " << val << endl;
            //state->print();

        } else {
            SCT_INTERNAL_ERROR(expr->getBeginLoc(), "Incorrect class type");
        }
    } else {
        parseExpr((CXXConstructExpr*)expr, val);
    }
}


// CXX constructor, including @sc_in/@sc_out/@sc_signal/@sc_uint/...
void ScGenerateExpr::parseExpr(CXXConstructExpr* expr, SValue& val)
{
    val = NO_VALUE;
    QualType type = expr->getType();
    auto ctorDecl = expr->getConstructor();
    bool isCopyCtor = ctorDecl->isCopyConstructor();
    bool isMoveCtor = ctorDecl->isMoveConstructor();
    bool isZeroWidth = isZeroWidthType(type);
    
    if (isZeroWidth) {
        val = ZW_VALUE; codeWriter->putLiteral(expr, val);
        
    } else 
    if (isAnyScInteger(type)) {
        // @sc_uint, @sc_biguint, @sc_int, @sc_bigint
        if (expr->getNumArgs() == 0) {
            // No parameters, constructor initialize object with 0
            val = SValue(APSInt(64, isScUInt(type) || isScBigUInt(type)), 10);
            codeWriter->putLiteral(expr, val);
            
        } else 
        if (expr->getNumArgs() == 1) {
            // Parse initialization expression, it can have arbitrary type
            Expr* iexpr = expr->getArg(0);
            QualType origType = iexpr->getType();
            
            if (isAnyIntegerRef(origType)) {
                SValue rval; 
                chooseExprMethod(iexpr, rval);

                bool isBool = origType->isBooleanType();
                bool isSigned = isSignedType(type);
                bool isOrigSigned = isSignedType(origType);

                // No type cast required, in Verilog RHS implicitly narrowed to LHS
                codeWriter->copyTerm(iexpr, expr); 

                // Skip boolean type conversion as it considered as unsigned
                if (!isBool && isOrigSigned != isSigned) { 
                    codeWriter->putSignCast(expr, isSigned ? CastSign::SCAST : 
                                                             CastSign::UCAST);
                }
            } else 
            if (isConstCharPtr(origType)) {
                // Parse expression with string literal inside
                unsigned lastWidth = strLiterWidth;
                bool lastUnsigned = strLiterUnsigned;
                if (auto typeInfo = getIntTraits(getTypeForWidth(expr), true)) {
                    strLiterWidth = typeInfo.getValue().first;
                    strLiterUnsigned = typeInfo.getValue().second;
                } else {
                    strLiterWidth = 0;  // Provides no string literal parsing
                }
                
                SValue rval; 
                chooseExprMethod(iexpr, rval);

                strLiterWidth = lastWidth;
                strLiterUnsigned = lastUnsigned;
                
                codeWriter->putLiteral(expr, rval); 
            }
        } else {
            SCT_TOOL_ASSERT (false, "Unexpected argument number");
        }
    } else 
    if (isScIntegerArray(type, false)) {
        // Local array of SC type, no in-place initialization supported
        // Do nothing
        
    } else 
    if (isUserClass(type)) {
        // User defined class or structure
        if (codeWriter->isEmptySensitivity()) {
            ScDiag::reportScDiag(expr->getBeginLoc(), 
                                 ScDiag::SYNTH_NOSENSITIV_METH);
        }
        if (isScModuleOrInterface(type)) {
            ScDiag::reportScDiag(expr->getBeginLoc(), 
                                 ScDiag::SYNTH_LOCAL_MODULE_DECL);
        }
        
        // Check @locarecvar to skip sub-statement with @CXXConstructExpr
        if (locrecvar) {
            if (isCopyCtor || isMoveCtor) {
                // Copy constructor to pass record parameter by value
                //cout << "CXXConstructExpr copy " << hex << expr << dec << endl;
                SCT_TOOL_ASSERT (expr->getArg(0), "No argument in copy constructor");

                // Created record value 
                SValue lrec = createRecValue(type->getAsCXXRecordDecl(), 
                                             modval, locrecvar, true, 0, false);

                // Get record from constructor argument
                SValue rval = evalSubExpr(expr->getArg(0));
                
                // Check for record/channel of record
                QualType rtype = getDerefType(rval.getType());
                bool rhsRecord = isUserClass(rtype, false);
                auto rrecType = isUserClassChannel(rtype, false);
                bool rhsRecordChan = (rhsRecord && rval.isScChannel()) || 
                                      rrecType.hasValue();
                bool rhsTempRecord = rhsRecord && rval.isRecord() && 
                                     rval == temprec;
                bool rhsRefRecord = rhsRecord && isReference(rval.getType());
                // Clear used temporary record
                temprec = NO_VALUE;

                //cout << "rval " << rval << " " << rhsRecord << rhsRecordChan 
                //     << " rhsTempRecord " << rhsTempRecord 
                //     << " locrecvarCtor " << locrecvarCtor << endl;
                
                SValue rrec;
                if (rhsTempRecord || rhsRecordChan && rval.isScChannel()) {
                    // Record channel is used instead of parent record
                    rrec = rval;
                } else {
                    // Get record or record channel value
                    state->getValue(rval, rrec);
                }

                // Copy values of record fields or clear if @rrec is unknown
                // Do not copy for reference as it could be array unknown element
                if (rrec.isRecord() && !rhsRefRecord) {
                    copyRecFieldValues(lrec, rrec);
                } else {
                    // Record channel and others
                    // Declare record fields and clear values in state
                    declareRecFields(lrec);
                    state->removeIntSubValues(lrec, false);
                }
                
                // Get RHS record indices
                string rrecSuffix;
                if (rhsRefRecord) {
                    rrecSuffix = refRecarrIndx;
                    // Clear indices suffix after use
                    refRecarrIndx = "";
                } else 
                if (rrec.isRecord() || rhsRecordChan) {
                    auto rrecvecs = getRecVector(rrec);
                    rrecSuffix = codeWriter->getRecordIndxs(rrecvecs);
                }
                
                if (locrecvarCtor) {
                    // Do nothing as soon as record already constructed
                    // Avoid double assignment to record fields
                } else
                if (rhsTempRecord) {
                    codeWriter->putRecordAssignTemp(expr, locrecvar, lrec, rrec, 
                                                    "", llvm::None, state.get());
                } else
                if (rhsRecord || rhsRecordChan) {
                    // Put assign for records and record channels
                    codeWriter->putRecordAssign(expr, locrecvar, lrec, rrec, 
                                                "", rrecSuffix, llvm::None);
                } else {
                    SCT_INTERNAL_FATAL (expr->getBeginLoc(), 
                                "User defined operator= () not supported yet"); 
                }
                val = lrec;
               
            } else {
                // Normal constructor of local record
                // Return created record to assign to its variable
                //cout << "CXXConstructExpr normal " << hex << expr << dec << endl;
                val = parseRecordCtor(expr, modval, locrecvar, true);
                locrecvarCtor = true;
            }
            locrecvar = NO_VALUE;
        }
    } else 
    if (getUserDefinedClassFromArray(type)) {
        // Do nothing, record array filled in ScParseExpr::parseValueDecl()
        
    } else {
         ScDiag::reportScDiag(expr->getSourceRange().getBegin(), 
                              ScDiag::SC_CXX_CTOR_IN_PROC_FUNC);
    }
}

// Operator @new and @new[]
void ScGenerateExpr::parseExpr(CXXNewExpr* expr, SValue& val)
{
    // Cannot be called from process
    ScDiag::reportScDiag(expr->getSourceRange().getBegin(), 
                         ScDiag::SC_NEW_IN_PROC_FUNC);
}

// Operator @delete
void ScGenerateExpr::parseExpr(CXXDeleteExpr* expr, SValue& val)
{
    // Cannot be called from process
    ScDiag::reportScDiag(expr->getSourceRange().getBegin(), 
                         ScDiag::SC_DELETE_IN_PROC_FUNC);
}

// Transform temporary that is written to memory so that a reference can bind it
void ScGenerateExpr::parseExpr(MaterializeTemporaryExpr* expr, SValue& val)
{
    ScParseExpr::parseExpr(expr, val);
    codeWriter->copyTerm(expr->getSubExpr(), expr); 
}

// Expression with cleanup
void ScGenerateExpr::parseExpr(ExprWithCleanups* expr, SValue& val)
{
    ScParseExpr::parseExpr(expr, val);
    codeWriter->copyTerm(expr->getSubExpr(), expr); 
}

// Represents binding an expression to a temporary.
// Used to call destructor for temporary object
void ScGenerateExpr::parseExpr(CXXBindTemporaryExpr* expr, SValue& val)
{
    ScParseExpr::parseExpr(expr, val);
    codeWriter->copyTerm(expr->getSubExpr(), expr); 
}

// Template parameter use as integer value
void ScGenerateExpr::parseExpr(SubstNonTypeTemplateParmExpr* expr, SValue& val) 
{
    ScParseExpr::parseExpr(expr, val);
    codeWriter->copyTerm(expr->getReplacement(), expr); 
}

// Used for local variable declaration
void ScGenerateExpr::parseDeclStmt(Stmt* stmt, ValueDecl* decl, SValue& val, 
                                   clang::Expr* initExpr)
{
    VarDecl* varDecl = dyn_cast<VarDecl>(decl);
    FieldDecl* fieldDecl = dyn_cast<FieldDecl>(decl);
    SCT_TOOL_ASSERT (varDecl || fieldDecl, "No declaration");
    
    // Use @initExpr if it is given
    bool hasInit = (varDecl && varDecl->hasInit()) || 
                   (fieldDecl && fieldDecl->hasInClassInitializer());
    Expr* iexpr = initExpr ? initExpr : 
                  (hasInit ? (varDecl ? varDecl->getInit() : 
                   fieldDecl->getInClassInitializer()) : nullptr);

    const QualType& type = decl->getType();
    bool isPtr = type->isPointerType();
    bool isRef = type->isReferenceType();
    bool isConst = (isRef) ? type.getNonReferenceType().isConstQualified() : 
                             type.isConstQualified();
    bool isRecord = isUserClass(getDerefType(type));
    
    if (iexpr) {
        // Remove ExprWithCleanups wrapper
        iexpr = removeExprCleanups(iexpr);
        
        // Do not analyze copy constructor, just assign RValue
        bool isCopyCtor = false;
        if (auto ctorExpr = getCXXCtorExprArg(iexpr)) {
            if (ctorExpr->getConstructor()->isCopyOrMoveConstructor()) {
                isCopyCtor = true;
            }
        }
        // Clear @iexpr to do not assign it in normal record constructor call
        if (isRecord && !isRef && !isCopyCtor) {
            iexpr = nullptr;
        }
    }
    
    // Use @recval for local record field declarations
    SValue recVal = recval ? recval : modval;
    
    // Variable or Array variable (i.e. pointer to array)
    // Remove sub-value for non-reference, required to clear state of tuples
    // added at previous analysis of this statement, to get variable for value 
    auto varvals = parseValueDecl(decl, recVal, iexpr, true, !isRef);
    val = varvals.first;

    // Fill state with array elements and initialization values if 
    // variable is constant type
    if (type->isArrayType()) {
        // Array initialization list
        SCT_TOOL_ASSERT (type->isConstantArrayType(), "Unsupported array type");
        vector<size_t> arrSizes = getArraySizes(type);
        size_t elmnum = getArrayElementNumber(type);
        auto recType = getUserDefinedClassFromArray(type);
        
        // Array declaration w/o initialization
        codeWriter->putArrayDecl(stmt, val, type, arrSizes, iexpr, level);
        
        if (iexpr && !recType) {
            
            if (auto init = dyn_cast<InitListExpr>(iexpr)) {
                // Normal initializer list, can be combined with array filler
                // if not all element values provided in initializer list
                // Get all sub-expressions
                std::vector<Expr*> subexpr = getAllInitExpr(iexpr);
                // Put initialized array elements
                for (size_t i = 0; i < subexpr.size(); ++i) {
                    auto arrInds = getArrayIndices(type, i);
                    codeWriter->putArrayElemInit(stmt, val, arrInds, subexpr[i]);
                }

                // Array filler, used for rest of array element 
                if (init->getArrayFiller()) {
                    for (size_t i = subexpr.size(); i < elmnum; i++) {
                        auto arrInds = getArrayIndices(type, i);
                        codeWriter->putArrayElemInitZero(stmt, val, arrInds);
                    }
                }
            } else 
            if (isScIntegerArray(type, false)) {
                // Default initialization SC data type array, fill with zeros
                for (size_t i = 0; i < elmnum; i++) {
                    auto arrInds = getArrayIndices(type, i);
                    codeWriter->putArrayElemInitZero(stmt, val, arrInds);
                }
            } else {
                string s = iexpr->getStmtClassName();
                ScDiag::reportScDiag(stmt->getSourceRange().getBegin(), 
                                     ScDiag::SYNTH_UNSUPPORTED_INIT) << s;
            }    
        }
    } else {
        // Single variable 
        
        // SC data type variable has initialization
        SValue ival = (varvals.second.size() == 1) ? 
                       varvals.second.front() : NO_VALUE;

        // For non-reference and constant reference try to evaluate 
        // expression as constexpr
        SValue iival = ival;
        if (!isRecord && (!isRef || isConst)) {
            if (ival.isInteger()) {
                // Put constant integer for the initializer expression
                codeWriter->putLiteral(iexpr, ival);
            }
        }
        
        if (isRef) {
            //cout << "Ref val " << val << " ival " << ival << " isArgArrayElm " << isArgArrayElm << endl;
            const QualType& refType = type.getNonReferenceType();
            
            // Check channel has the same type as constant reference parameter
            bool isChanRef = ival.isScChannel() && ival.getScChannel()->
                             getType()->getCanonicalTypeInternal() == 
                             refType->getCanonicalTypeInternal();
            
            // Reference can be to array element or dynamic object
            if  ( !isRecord && !(iival.isVariable() || iival.isTmpVariable() || 
                  iival.isArray() || iival.isSimpleObject() || isChanRef) )
            {
                // Constant reference refers to RValue, special variable created
                SCT_TOOL_ASSERT (isConst, "Incorrect reference initializer");

                // Put constant reference to itself to use variable name in code
                codeWriter->storeRefVarDecl(val, val);
                // Declare constant reference variable w/o initialization,
                // always declare parameter as range/bit selection can be used
                codeWriter->putVarDecl(nullptr, val, refType, nullptr, false, 
                                       level);
                // Add initialization with the initializer
                codeWriter->putAssign(stmt, val, iexpr);
                
            } else {
                // Non-constant or constant reference refers to LValue
                // It is replaced with @iexpr string in generated code
                // Do not check if there is no terms
                codeWriter->storeRefVarDecl(val, iexpr, false);
                
                if (isRecord) {
                    string s;
                    if (ival.isReference()) {
                        s = recarrRefIndices[ival];
                    } else {
                        s = codeWriter->getIndexString(ival);
                    }
                    recarrRefIndices[val] = s;
                    //cout << "parseDeclStmt for isRecordRef val " << val 
                    //     << " ival " << ival << " indxStr " << s << endl;
                }
            }
        } else 
        if (isPtr) {
            // Local pointer variable declaration and initialization

            // Get pointed object for temporal variable returned from function
            if (ival.isTmpVariable()) {
                ival = getValueFromState(ival);
            }
            //cout << "parseDecl ival " << ival << " iexpr " << hex << iexpr << dec << endl;

            bool unkwIndex;
            bool isArr = state->isArray(ival, unkwIndex);
            
            if (isArr) {
                // Put term string for array to support pointer to array element
                if (iexpr) {
                    codeWriter->storePointerVarDecl(val, iexpr);
                }
            } else {
                // No local variable pointer to channel supported
                codeWriter->storePointerVarDecl(val, ival, NO_VALUE);
            }
        } else {
            // Single variable declaration with/without initialization,
            // No record declaration/initialization, that is done for its fields            
            //cout << "  parseDecl isRecord " << isRecord << " iexpr " 
            //     << hex << iexpr << " val " << val << " locrecvar " << locrecvar << endl;
            if (isRecord) {
                if (locrecvar) {
                    // Artificial copy constructor required for function which 
                    // returns record to avoid RVO in C++17
                    // Create record value 
                    SValue lrec = createRecValue(type->getAsCXXRecordDecl(), 
                                                 modval, locrecvar, true, 0, false);

                    // Restore initialization expression
                    iexpr = initExpr ? initExpr : 
                            (hasInit ? (varDecl ? varDecl->getInit() : 
                            fieldDecl->getInClassInitializer()) : nullptr);
                    SCT_TOOL_ASSERT (iexpr, "No initialization expression for record");
                    
                    // Get record temporary variable from initialization expression
                    SValue rval = evalSubExpr(iexpr);

                    SCT_TOOL_ASSERT (rval != temprec, 
                                     "No temporary record supported here");
                    SCT_TOOL_ASSERT (!isReference(rval.getType()), 
                                     "No reference supported here");

                    // Get record or record channel value
                    SValue rrec = rval; 
                    if (!rval.isRecord()) state->getValue(rval, rrec);
                    //cout << " locrecvar " << locrecvar << " lrec " << lrec 
                    //     << " rval " << rval << " rrec " << rrec <<  endl;

                    // Copy values of record fields and put it to record variable
                    copyRecFieldValues(lrec, rrec);
                    state->putValue(locrecvar, lrec, false, false);

                    // Put assign for records and record channels
                    codeWriter->putRecordAssign(stmt, locrecvar, lrec, rrec, 
                                                "", "", llvm::None);
                    locrecvar = NO_VALUE;
                    
                } else 
                if (iexpr) {
                    // Get field initialization string from @CXXConstructExpr
                    codeWriter->copyTerm(iexpr, stmt);
                }
            } else {
                codeWriter->putVarDecl(stmt, val, type, iexpr, false, level, 
                                       iival.isInteger());

                // Register this statement to add comment into scope graph
                auto i = constReplacedFunc.find(iexpr);
                if (i != constReplacedFunc.end()) {
                    constReplacedFunc.emplace(stmt, i->second);
                    //cout << "Add DeclStmt stmt " << hex << stmt << " for " << i->second << dec << endl;
                }
            }
        }
    }
}

// Parse field declaration without initialization to put into @codeWriter, 
void ScGenerateExpr::parseFieldDecl(ValueDecl* decl, const SValue& lfvar)
{
    //cout << "parseFieldDecl lfvar " << lfvar << " locrecvar " << locrecvar << endl;
    const QualType& type = decl->getType();
    
    // Put field declaration to @localDeclVerilog, @nullptr no initialization
    codeWriter->putVarDecl(nullptr, lfvar, type, nullptr, false, level);
}

// Parse array field declaration without initialization to put into @codeWriter, 
// used in declaration array of records
void ScGenerateExpr::parseArrayFieldDecl(ValueDecl* decl, const SValue& lfvar, 
                                         const vector<size_t>& arrSizes)
{
    //cout << "parseArrayFieldDecl lfvar " << lfvar << " locrecvar " << locrecvar << endl;
    vector<size_t> allSizes(arrSizes); 
    const QualType& type = decl->getType();

    // Merge record array dimensions and field array dimensions
    if (type->isArrayType()) {
        auto fieldSizes = getArraySizes(type);
        allSizes.insert(allSizes.end(), fieldSizes.begin(), fieldSizes.end());
    }
    
    // Put field declaration to @localDeclVerilog, @nullptr no initialization
    codeWriter->putArrayDecl(nullptr, lfvar, type, allSizes, nullptr, level);
}

// Parse statement and run @chooseExprMethod for each operand
void ScGenerateExpr::parseBinaryStmt(BinaryOperator* stmt, SValue& val) 
{
    // There is no value for general binary statement
    val = NO_VALUE;
    Expr* lexpr = stmt->getLHS();
    Expr* rexpr = stmt->getRHS();

    // Operation code
    BinaryOperatorKind opcode = stmt->getOpcode();
    string opcodeStr = stmt->getOpcodeStr(opcode).str();

    // Parse LHS 
    bool lastAssignLHS = assignLHS;
    if (opcode == BO_Assign) assignLHS = true;
    SValue lval; 
    chooseExprMethod(lexpr, lval);
    // Required for lvalue function call in SVA which replaced with integer,
    // but indices not used, so need to avoid using them in rvalue
    assignLHS = lastAssignLHS;
    
    // Get record indices from @arraySubIndices, it is erased after use
    SValue lrec; state->getValue(lval, lrec);
    string lrecSuffix;
    if (lrec.isRecord()) {
        auto lrecvecs = getRecVector(lrec);
        lrecSuffix = codeWriter->getRecordIndxs(lrecvecs);
    }

    if (opcode == BO_LOr || opcode == BO_LAnd) {
        // Do not consider pointer in condition as it casted to @bool
        // Get condition for term, use @stmt instead of @lexpr as it is term
        SValue flval;
        getTermCondValue(stmt, lval, flval);
        //cout << "BO lexpr " << hex << stmt << dec << " lval " << lval << endl;
        
        if (lval.isInteger() && 
            ((opcode == BO_LOr && !lval.getInteger().isNullValue()) ||
             (opcode == BO_LAnd && lval.getInteger().isNullValue()))) {
            
            val = lval;
            
            // Constant condition evaluated
            codeWriter->putValueExpr(stmt, lval);
            
        } else {
            // Parse RValue only if required, 
            // dead condition blocks not analyzed in ScTraverseProc
            SValue rval; 
            chooseExprMethod(rexpr, rval);
            codeWriter->putBinary(stmt, opcodeStr, lexpr, rexpr);
        }
        
    } else {
        SValue rval; 
        chooseExprMethod(rexpr, rval);
        
        // Get record indices from @arraySubIndices, it is erased after use
        SValue rrec; state->getValue(rval, rrec);
        string rrecSuffix;
        if (rrec.isRecord()) {
            auto rrecvecs = getRecVector(rrec);
            rrecSuffix = codeWriter->getRecordIndxs(rrecvecs);
        }
        
        auto ltype = lval.getTypePtr();
        auto rtype = rval.getTypePtr();
        bool lhsPointer = (ltype && ltype->isPointerType());
        bool rhsPointer = (rtype && rtype->isPointerType());

        if (opcode == BO_Assign) {
            // Left value is result of assignment statement
            val = lval;

            // Check pointer as LHS only, pointer as RHS can be assigned to boolean
            if (lhsPointer) {
                // Check pointer type is FILE*
                bool filePtr = false;
                if (auto ptrType = lval.getTypePtr()) {
                    auto objType = ptrType->getPointeeOrArrayElementType();
                    QualType type(objType, 0);
                    filePtr = type.getAsString() == "FILE";
                }
                // No error message for file pointer assignment
                if (!filePtr) {
                    ScDiag::reportScDiag(stmt->getBeginLoc(),
                                         ScDiag::SYNTH_POINTER_OPER) << opcodeStr;
                }
            } else {
                // Assignment with variable/expression in right part
                // Get owner variable, required for arrays
                SValue lvar = state->getVariableForValue(lval);
                // Try to get channel value, used as key in @varTraits 
                SValue cval = getChannelFromState(lvar);
                if (cval) {
                    lvar = cval;
                }
                
                // Put assign for record and others
                bool isRecord = isUserClass(getDerefType(lval.getType()), true);

                if (isRecord) {
                    SCT_INTERNAL_FATAL(stmt->getBeginLoc(), 
                                       "No record anticipated in binary operator");
                } else {
                    codeWriter->putAssign(stmt, lvar, lexpr, rexpr);
                    // Consider multiple assignments
                    if (auto multipleAssign = codeWriter->getAssignStmt(rexpr)) {
                        codeWriter->addTerm(stmt, multipleAssign);
                        codeWriter->copyTerm(multipleAssign, stmt);
                    }
                    // Register this statement to add comment into scope graph
                    auto i = constReplacedFunc.find(rexpr);
                    if (i != constReplacedFunc.end()) {
                        constReplacedFunc.emplace(stmt, i->second);
                        //cout << "Add BinaryStmt stmt " << hex << stmt << " for " << i->second << dec << endl;
                    }
                }
            }
        } else 
        if (opcode == BO_Comma) {
            // C++ operator comma
            val = rval;
            if (lhsPointer || rhsPointer) {
                ScDiag::reportScDiag(stmt->getBeginLoc(), 
                                     ScDiag::SYNTH_POINTER_OPER) << opcodeStr;
            } else {
                // Store LHS comma statement into scope graph
                storeStmtStr(lexpr);
                
                codeWriter->copyTerm(rexpr, stmt);
            }
            // Comma is seldom used, may be concatenation was intended
            ScDiag::reportScDiag(stmt->getBeginLoc(), ScDiag::SYNTH_CPP_COMMA);
            
        } else
        if (opcode == BO_Add || opcode == BO_Sub || opcode == BO_Mul || 
            opcode == BO_Div || opcode == BO_Rem || 
            opcode == BO_Shl || opcode == BO_Shr || 
            opcode == BO_And || opcode == BO_Or  || opcode == BO_Xor) 
        {
            if (lhsPointer || rhsPointer) {
                ScDiag::reportScDiag(stmt->getBeginLoc(), 
                                     ScDiag::SYNTH_POINTER_OPER) << opcodeStr;
            } else {
                if (opcode == BO_Shr) {
                    opcodeStr = ">>>";
                } else 
                if (opcode == BO_Shl) {
                    opcodeStr = "<<<";
                }
                
                // RHS operand is self-determined in Verilog (see LRM 11.6)
                if (codeWriter->isIncrWidth(rexpr) && 
                    (opcode == BO_Shr || opcode == BO_Shl)) {
                    // 32bit is always enough for shift value
                    unsigned width = codeWriter->getExprTypeWidth(rexpr, 32);
                    width = (width > 32) ? 32 : width;
                    codeWriter->extendTypeWidth(rexpr, width);
                }
                
                // LHS operand is sized to result width in Verilog, 
                // so it needs to be updated for SHR/DIV/REM only
                if (codeWriter->isIncrWidth(lexpr) && 
                    (opcode == BO_Shr || opcode == BO_Div || opcode == BO_Rem)) {
                    // Try to get expression/type width
                    unsigned width = codeWriter->getExprTypeWidth(lexpr);
                    codeWriter->extendTypeWidth(lexpr, width);
                }
                codeWriter->putBinary(stmt, opcodeStr, lexpr, rexpr);
                
                if (codeWriter->getAssignStmt(lexpr) || 
                    codeWriter->getAssignStmt(rexpr)) {
                    ScDiag::reportScDiag(stmt->getBeginLoc(),
                                         ScDiag::SYNTH_INTERNAL_ASSIGN_OPER);
                }
            }

        } else 
        if (opcode == BO_GT || opcode == BO_LT || opcode == BO_GE || 
            opcode == BO_LE || opcode == BO_EQ || opcode == BO_NE) 
        {
            // Comparison allowed for pointers, but evaluated into constant
            if (lhsPointer || rhsPointer) {
                val = evaluateConstIntNoCheck(stmt).second;
                if (val.isInteger()) {
                    codeWriter->putLiteral(stmt, SValue(SValue::boolToAPSInt(
                                           val.getBoolValue()), 10));
                } else {
                    ScDiag::reportScDiag(stmt->getBeginLoc(), 
                                         ScDiag::SYNTH_UNSUPPORTED_OPER) << 
                                         (opcodeStr+" for pointer(s)") << 
                                         "ScGenerateExpr::parseBinaryStmt";
                }
            } else {
                codeWriter->putBinary(stmt, opcodeStr, lexpr, rexpr);

                if (codeWriter->getAssignStmt(lexpr) || 
                    codeWriter->getAssignStmt(rexpr)) {
                    ScDiag::reportScDiag(stmt->getBeginLoc(),
                                         ScDiag::SYNTH_INTERNAL_ASSIGN_OPER);
                }
            }
        } else {
            ScDiag::reportScDiag(stmt->getSourceRange().getBegin(), 
                                 ScDiag::SYNTH_UNSUPPORTED_OPER) 
                                 << opcodeStr << "ScGenerateExpr::parseBinaryStmt";
        }
    }
}

// Parse statement and run @chooseExprMethod for each operand
void ScGenerateExpr::parseCompoundAssignStmt(CompoundAssignOperator* stmt, 
                                             SValue& val) 
{
    Expr* lexpr = stmt->getLHS();
    Expr* rexpr = stmt->getRHS();

    // Parse left and right parts to fill @terms
    bool lastAssignLHS = assignLHS; assignLHS = true;
    SValue lval; 
    chooseExprMethod(lexpr, lval);
    assignLHS = lastAssignLHS; 
    SValue rval;
    chooseExprMethod(rexpr, rval);
    
    QualType ltype = lexpr->getType();
    QualType rtype = rexpr->getType();
    bool isPtr = isPointer(ltype) || isPointer(rtype);
    
    // Left value is result of assignment statement
    val = lval;

    // Operation code
    BinaryOperatorKind opcode = stmt->getOpcode();
    BinaryOperatorKind compOpcode = stmt->getOpForCompoundAssignment(opcode);
    string opcodeStr = stmt->getOpcodeStr(compOpcode).str();
    
    if (opcode == BO_AddAssign || opcode == BO_SubAssign || 
        opcode == BO_MulAssign || opcode == BO_DivAssign ||
        opcode == BO_RemAssign || opcode == BO_ShlAssign ||
        opcode == BO_ShrAssign || opcode == BO_AndAssign ||
        opcode == BO_OrAssign || opcode == BO_XorAssign) 
    {
        if (codeWriter->isEmptySensitivity()) {
            ScDiag::reportScDiag(stmt->getBeginLoc(), 
                                 ScDiag::SYNTH_NOSENSITIV_METH);
        }
        if (codeWriter->isResetSection()) {
            SValue varval = state->getVariableForValue(lval);
            if (codeWriter->isRegister(varval)) {
                ScDiag::reportScDiag(lexpr->getBeginLoc(), 
                                     ScDiag::SYNTH_MODIFY_REG_IN_RESET);
            }
        }
        
        if (isPtr) {
            ScDiag::reportScDiag(stmt->getSourceRange().getBegin(), 
                                 ScDiag::SYNTH_POINTER_OPER) << (opcodeStr+"=");
        } else {
            
            // RHS operand is self-determined in Verilog (see LRM 11.6)
            if (codeWriter->isIncrWidth(rexpr) && 
                (opcode == BO_ShrAssign || opcode == BO_ShlAssign)) {
                // 32bit is always enough for shift value
                unsigned width = codeWriter->getExprTypeWidth(rexpr, 32);
                width = (width > 32) ? 32 : width;
                codeWriter->extendTypeWidth(rexpr, width);
            }
            
            // Get owner variable, required for arrays
            SValue lvar = state->getVariableForValue(lval);

            codeWriter->putCompAssign(stmt, opcodeStr, lvar, lexpr, rexpr);
            
            // Consider multiple assignments
            if (auto multipleAssign = codeWriter->getAssignStmt(rexpr)) {
                codeWriter->addTerm(stmt, multipleAssign);
                codeWriter->copyTerm(multipleAssign, stmt);
            }
        }    
    } else {
        ScDiag::reportScDiag(stmt->getSourceRange().getBegin(), 
                             ScDiag::SYNTH_UNSUPPORTED_OPER) << opcodeStr
                             << "ScGenerateExpr::parseCompoundAssignStmt";
    }
}

// Parse statement and run @chooseExprMethod for the operand
void ScGenerateExpr::parseUnaryStmt(UnaryOperator* stmt, SValue& val) 
{
    // There is no value for general unary statement
    val = NO_VALUE;
    
    UnaryOperatorKind opcode = stmt->getOpcode();
    string opcodeStr = stmt->getOpcodeStr(stmt->getOpcode()).str();
    bool isPrefix = stmt->isPrefix(stmt->getOpcode());
    bool isIncrDecr = opcode == UO_PostInc || opcode == UO_PreInc || 
                      opcode == UO_PostDec || opcode == UO_PreDec;
    
    Expr* rexpr = stmt->getSubExpr();
    bool lastAssignLHS = assignLHS; 
    if (isIncrDecr) assignLHS = true;
    SValue rval;
    chooseExprMethod(rexpr, rval);
    assignLHS = lastAssignLHS;
    
    auto rtype = rval.getTypePtr();
    bool isPointer = rtype && rtype->isPointerType();
    
    if (codeWriter->getAssignStmt(rexpr)) {
        ScDiag::reportScDiag(stmt->getBeginLoc(),
                             ScDiag::SYNTH_INTERNAL_ASSIGN_OPER);
    }

    if (opcode == UO_Plus) {
        val = rval;
        if (isPointer) {
            // Pointer argument variable name, skip non-variable as array element
            if (val.isVariable() || val.isTmpVariable()) {
                // Try to get channel value from pointe
                SValue cval = getChannelFromState(rval);
                codeWriter->storePointerVarDecl(val, rval, cval);
                
            } else {
                codeWriter->copyTerm(rexpr, stmt);
            }
        } else {
            codeWriter->copyTerm(rexpr, stmt);
        }
    } else 
    if (opcode == UO_Minus)
    {
        if (isPointer) {
            ScDiag::reportScDiag(stmt->getSourceRange().getBegin(), 
                                 ScDiag::SYNTH_POINTER_OPER) << opcodeStr;
        } else {
            // Literal considered inside of @putUnary
            codeWriter->putUnary(stmt, opcodeStr, rexpr, true);
        }

    } else
    if (opcode == UO_Not)
    {
        if (isBoolArgument(rexpr)) {
            ScDiag::reportScDiag(stmt->getSourceRange().getBegin(), 
                                 ScDiag::CPP_BOOL_BITWISE_NOT);
        }
        
        if (isPointer) {
            ScDiag::reportScDiag(stmt->getSourceRange().getBegin(), 
                                 ScDiag::SYNTH_POINTER_OPER) << opcodeStr;
        } else {
            codeWriter->putUnary(stmt, opcodeStr, rexpr, true);
        }

    } else
    if (opcode == UO_LNot) 
    {
        if (isPointer) {
            SValue ival = evaluateConstIntNoCheck(stmt).second;
            if (ival.isInteger()) {
                codeWriter->putLiteral(stmt, SValue(SValue::boolToAPSInt(
                                       ival.getBoolValue()), 10));
                val = val;
                
            } else {
                ScDiag::reportScDiag(stmt->getSourceRange().getBegin(), 
                                     ScDiag::SYNTH_UNSUPPORTED_OPER) << opcodeStr 
                                     << "ScGenerateExpr::parseUnaryStmt for pointer";
            }
        } else {
            codeWriter->putUnary(stmt, opcodeStr, rexpr, true);
        }

    } else    
    if (isIncrDecr)
    {
        if (codeWriter->isEmptySensitivity()) {
            ScDiag::reportScDiag(stmt->getBeginLoc(), 
                                 ScDiag::SYNTH_NOSENSITIV_METH);
        }
        if (codeWriter->isResetSection()) {
            SValue varval = state->getVariableForValue(rval);
            if (codeWriter->isRegister(varval)) {
                ScDiag::reportScDiag(rexpr->getBeginLoc(), 
                                     ScDiag::SYNTH_MODIFY_REG_IN_RESET);
            }
        }

        // Do not set @val to @rval to avoid reference to ++/-- as it can be
        // zero/multiple used that lead to incorrect code
        
        if (isPointer) {
            ScDiag::reportScDiag(stmt->getBeginLoc(), 
                                 ScDiag::SYNTH_POINTER_OPER) << opcodeStr;
        } else {
            codeWriter->putUnary(stmt, opcodeStr, rexpr, isPrefix);
        }

    } else
    if (opcode == UO_Deref) {
        // If @rval is reference it de-referenced in getValue()
        // Dereferenced value provided in @val
        state->getValue(rval, val);
        
        // Get variable for @rval
        SValue rvar = state->getVariableForValue(rval);

        if (rval.isArray()) {
            // Array cannot contains pointers to variables, so it is variable itself
            codeWriter->copyTerm(rexpr, stmt);
            
        } else 
        if (rval.isVariable() || rval.isTmpVariable()) {
            //cout << "Deref : rval " << rval << " val " << val << endl;
            if (val.isVariable()) {
                // Put de-referenced pointer variable 
                putPtrDerefExpr(stmt, rvar, rval, val);
                
            } else 
            if (val.isInteger()) {
                // Put pointer integer value, used for null pointer de-reference
                codeWriter->putLiteral(stmt, val);
                
            } else 
            if (val.isSimpleObject()) {
                // Put pointer variable, used for pointer to channel and dynamic object
                codeWriter->copyTerm(rexpr, stmt);
                
            } else
            if (val.isArray()) {
                // Array to pointer cast, put pointer variable as best option
                ScDiag::reportScDiag(stmt->getBeginLoc(), 
                                     ScDiag::SYNTH_ARRAY_TO_POINTER);
                codeWriter->copyTerm(rexpr, stmt);
                
            } else {
                // Dangling pointer de-references, put pointer variable as best option
                ScDiag::reportScDiag(stmt->getBeginLoc(), 
                                     ScDiag::CPP_DANGLING_PTR_DEREF) << 
                                     rvar.asString(rvar.isObject());
                codeWriter->copyTerm(rexpr, stmt);
            }
            
        } else {
            // Put de-referenced pointer variable 
            putPtrDerefExpr(stmt, rvar, rval, val);
        }
            
    } else 
    if (opcode == UO_AddrOf) 
    {
        // "&" in process function not supported 
        ScDiag::reportScDiag(stmt->getSourceRange().getBegin(), 
                             ScDiag::SYNTH_UNSUPPORTED_OPER) << "&" 
                             << "ScGenerateExpr::parseUnaryStmt";
        
        // Get referenced variable for @rval
        SValue rrval;
        state->getDerefVariable(rval, rrval); rval = rrval;
        
        if (rval.isVariable() || rval.isTmpVariable() || rval.isArray() || 
            rval.isSimpleObject()) {
            // Create temporary object pointer to @rval
            QualType type = (rval.isVariable()) ? 
                                rval.getVariable().getType() :
                                ((rval.isTmpVariable()) ? 
                                        rval.getTmpVariable().getType() : 
                                        rval.getObjectPtr()->getType());
            val = SValue(type);
            // Put address of value to state, no code generated here
            state->putValue(val, rval);
            state->setValueLevel(val, level+1);
            
        } else {
            ScDiag::reportScDiag(stmt->getSourceRange().getBegin(), 
                                 ScDiag::CPP_INCORRECT_REFER) << rval.asString();
        }
    } else {
        ScDiag::reportScDiag(stmt->getSourceRange().getBegin(), 
                             ScDiag::SYNTH_UNSUPPORTED_OPER) << opcodeStr << 
                             "ScGenerateExpr::parseUnaryStmt";
    }  
}

// Common function for operator[] in @ArraySubscriptExpr and @CXXOperatorCall
SValue ScGenerateExpr::parseArraySubscript(Expr* expr, 
                                           Expr* baseExpr, 
                                           Expr* indxExpr)
{
    // Result value
    SValue val = NO_VALUE;

    SValue bval;  // Array variable 
    chooseExprMethod(baseExpr, bval);
    
    // Skip sign cast for array index
    bool skipSignCastOrig = codeWriter->isSkipSignCast();
    codeWriter->setSkipSignCast(true);
    
    // Index variable/value
    bool lastAssignLHS = assignLHS; assignLHS = false;
    SValue ival;    
    chooseExprMethod(indxExpr, ival);
    assignLHS = lastAssignLHS;
    
    codeWriter->setSkipSignCast(skipSignCastOrig);
    
    // Get referenced variable for array (reference to array: T (&a)[N])
    SValue bbval;
    state->getDerefVariable(bval, bbval);
    //cout << "parseExpr(ArraySubscriptExpr) " << hex << expr << dec 
    //     << ", bval " << bval << ", bbval " << bbval << ", ival " << ival << endl;
    
    if (isScZeroWidth(bbval)) {
        val = ZW_VALUE; codeWriter->putLiteral(expr, val);
        return val;
    }
    
    // Return value of array element, it can be sub-array
    if (bbval.isVariable() || bbval.isTmpVariable() || bbval.isArray()) {
        // Provide array element, no offset as only zero elements used
        state->getValue(bbval, val, false);
        
    } else {
        ScDiag::reportScDiag(expr->getBeginLoc(), ScDiag::SYNTH_NO_ARRAY) 
                << bbval.asString();
        // Required to prevent more errors
        SCT_INTERNAL_FATAL_NOLOC ("No array found");
    }
    
    // Get most inner array object and put it into @aval
    SValue aval(val); SValue rval(val);
    while (rval.isArray()) {
        aval = rval;
        state->getValue(aval, rval);
    }
    
    // Store subscript index to for record array
    codeWriter->addSubscriptIndex(aval, indxExpr);
    
    // Check if this operator is for port/signal
    QualType type = expr->getType();
    // Use @isPointer as sc_port<IF> cannot point-to channel
    bool isChannelPointer = isPointer(type) ? 
                            isScChannel(type->getPointeeType()) : false;
    
    // Put statement string to code writer  
    if (isScChannel(type) || isScChannelArray(type) || isChannelPointer) {
        // Get channel value
        SValue cval = getChannelFromState(val);

        if (cval.isScChannel()) {
            QualType ctype = cval.getScChannel()->getType();
            if ( isUserClass(ctype) ) {
                // Put nothing for record channel, channels for individual fields 
                // are put in assignment and read
            } else {
                codeWriter->putArrayIndexExpr(expr, baseExpr, indxExpr);
            }

        } else {
            ScDiag::reportScDiag(expr->getBeginLoc(), ScDiag::SYNTH_NO_CHANNEL);
        }
    } else {
        codeWriter->putArrayIndexExpr(expr, baseExpr, indxExpr);
    }
    return val;
}

// Array index access operator []
void ScGenerateExpr::parseExpr(ArraySubscriptExpr* expr, SValue& val)
{
    val = parseArraySubscript(expr, expr->getBase(), expr->getIdx());
}

// Function call expression
void ScGenerateExpr::parseCall(CallExpr* expr, SValue& val) 
{
    // There is no value for general function call
    val = NO_VALUE;
    
    // Get arguments
    Expr** args = expr->getArgs();
    unsigned argNum = expr->getNumArgs();

    // Get function name and type
    FunctionDecl* funcDecl = expr->getDirectCallee();
    if (funcDecl == nullptr) {
        SCT_INTERNAL_FATAL(expr->getBeginLoc(), 
                         "No function found for call expression");
    }
    
    string fname = funcDecl->getNameAsString();
    QualType retType = funcDecl->getReturnType();
    auto nsname = getNamespaceAsStr(funcDecl);
    
    if (DebugOptions::isEnabled(DebugComponent::doGenFuncCall)) {
        cout << "parseCall nsname : " << (nsname ? *nsname : "-- ")
             << ", fname : " << fname  << ", type : " << retType.getAsString() << endl;
    }
    
    // Check SC data type functions
    bool isScTypeFunc = isAnyScIntegerRef(retType);
    if (argNum > 0) {
        isScTypeFunc = isScTypeFunc || isAnyScIntegerRef(args[0]->getType());
    }            

    if (fname == "sct_assert") {
        // Checking assertion in regression tests
        SCT_TOOL_ASSERT (argNum == 1 || argNum == 2, "Incorrect argument number");
        
        std::string msgStr; 
        if (argNum == 2) {
            if (auto str = getStringFromArg(args[1])) {
                msgStr = str.getValue();
            }
        }

        SValue rval;
        chooseExprMethod(args[0], rval);
        
        if (!noSvaGenerate) {
            codeWriter->putAssert(expr, args[0], msgStr);
        }
        
    } else 
    if (fname == "sct_alive_loop") {
        // Do nothing, implemented in ScTraverseConst
        
    } else
    if (fname == "sct_assert_const" || fname == "__assert" || 
        fname == "sct_assert_unknown") {
        // Do nothing
        
    } else 
    if (fname == "sct_assert_defined" || fname == "sct_assert_register" ||
        fname == "sct_assert_read" || fname == "sct_assert_latch") {
        // Do nothing
        
    } else 
    if (fname == "sct_assert_level") {
        // Do nothing
        
    } else 
    if (fname == "sct_assert_in_proc_start") {
        if (!noSvaGenerate) codeWriter->setParseSvaArg(true);
        
    } else 
    if (fname == "sct_assert_in_proc_func") {
        // Called in @SCT_ASSERT macro
        SCT_TOOL_ASSERT (argNum == 4 || argNum == 5, 
                         "Incorrect argument number");

        SValue fval; SValue sval;
        chooseExprMethod(args[0], fval);
        chooseExprMethod(args[1], fval);
        assertName = getStringFromArg(args[2]);
        fval = evaluateConstIntNoCheck(args[3]).second;
        if (argNum == 5) {
            sval = evaluateConstIntNoCheck(args[4]).second;
        } else {
            sval = fval;
        }

        if (!noSvaGenerate) {
            if (fval.isInteger() && sval.isInteger()) {
                std::string timeStr = parseSvaTime(
                                            fval.getInteger().getExtValue(), 
                                            sval.getInteger().getExtValue());
                codeWriter->putTemporalAssert(expr, args[0], args[1], timeStr);

            } else {
                ScDiag::reportScDiag(expr->getBeginLoc(), 
                                     ScDiag::SYNTH_SVA_INCORRECT_TIME);
            }
            codeWriter->setParseSvaArg(false);
        }
        
    } else 
    if (fname == "sct_is_method_proc") {
        // Get process kind functions in @sct_fifo_if.h
        // SVA runs at clock edge, so considered as thread
        bool isMethod = isCombProcess && !codeWriter->isParseSvaArg(); 
        codeWriter->putLiteral(expr, SValue(SValue::boolToAPSInt(isMethod), 10));
        
    } else
    if (fname == "sct_is_thread_proc") {
        // Get process kind functions in @sct_fifo_if.h
        bool isThread = !isCombProcess || codeWriter->isParseSvaArg(); 
        codeWriter->putLiteral(expr, SValue(SValue::boolToAPSInt(isThread), 10));
        
    } else
    if (fname == "wait") {
        SCT_TOOL_ASSERT (nsname && *nsname == "sc_core",
                         "wait() in non-sc_core namespace");
                
        // Add nothing for wait() and counter assignment for wait(n)
        // Other logic implemented in ScTraverseProc
        if (argNum == 1) {
            // Parse @wait(n) argument
            //cout << "TYPE " << args[0]->getType().getAsString() << endl;
            
            // Check argument is integer type
            if (isAnyInteger(args[0]->getType())) {
                SValue ival;
                chooseExprMethod(args[0], ival);

                // Put wait(n) counter assignment
                codeWriter->putWaitNAssign(expr, args[0]);
            }
        }
    } else 
    if (nsname && *nsname == "sc_dt" && isScTypeFunc) {
        // SC data type functions
        if (fname == "concat") {
            SCT_TOOL_ASSERT (argNum == 2, "Incorrect argument number");
            Expr* lexpr = args[0];
            Expr* rexpr = args[1];
            
            // Parse argument expression to fill @terms
            SValue lval; SValue rval;
            chooseExprMethod(lexpr, lval);
            chooseExprMethod(rexpr, rval);
            
            // Report implicit integer to boolean cast, can be non-intended
            if (isIntToBoolCast(lexpr) || isIntToBoolCast(rexpr)) {
                ScDiag::reportScDiag(expr->getBeginLoc(), 
                                     ScDiag::SC_CONCAT_INT_TO_BOOL);
            }
            bool lhsZeroWidth = isScZeroWidth(lval);
            bool rhsZeroWidth = isScZeroWidth(rval);
            
            if (lhsZeroWidth && rhsZeroWidth) {
                // Put 32bit unsigned 0
                codeWriter->putLiteral(expr, ZW_VALUE);
            } else 
            if (lhsZeroWidth) {
                codeWriter->copyTerm(rexpr, expr);
            } else 
            if (rhsZeroWidth) {
                codeWriter->copyTerm(lexpr, expr);
            } else {
                codeWriter->putConcat(expr, lexpr, rexpr);
            }
        } else     
        if (fname == "and_reduce" || fname == "or_reduce" || 
            fname == "xor_reduce" || fname == "nand_reduce" || 
            fname == "nor_reduce" || fname == "xnor_reduce") {
            SCT_TOOL_ASSERT (argNum == 1, "Incorrect argument number");
            Expr* fexpr = args[0];
            
            string opcode = (fname == "and_reduce") ? "&" :
                            (fname == "or_reduce") ? "|" : 
                            (fname == "xor_reduce") ? "^" :
                            (fname == "nand_reduce") ? "~&" :
                            (fname == "nor_reduce") ? "~|" : "~^";
            
            // Parse argument expression to fill @terms
            SValue rval;
            chooseExprMethod(fexpr, rval);
            
            // Put reduction unary expression
            codeWriter->putUnary(expr, opcode, fexpr);
            
        } else {
            SCT_INTERNAL_FATAL(expr->getBeginLoc(), 
                               string("Unsupported function for SC type") + fname);
        }
    } else 
    if (nsname && *nsname == "sc_core" && 
        (fname == "report" || fname == "sc_time_stamp" || 
         fname == "sc_delta_count")) {
        
        // Do nothing

    } else 
    if (((nsname && *nsname == "std") || isLinkageDecl(funcDecl)) &&
        (fname == "printf" || fname == "fprintf" || 
         fname == "sprintf" || fname == "snprintf" ||
         fname == "fopen" || fname == "fclose"))
    {
        // Do nothing
        
    } else 
    if (fname == "__assert_fail") {
        // This function has NORETURN attribute and cannot be supported
        //cout << "__assert_fail at " << expr->getSourceRange().getBegin().printToString(sm) << endl;
        ScDiag::reportScDiag(expr->getBeginLoc(), ScDiag::SC_ERROR_NORETURN);
        
    } else {
        // General functions, most logic is in ScTraverseProc 
        if (codeWriter->isEmptySensitivity()) {
            ScDiag::reportScDiag(expr->getBeginLoc(), 
                                 ScDiag::SYNTH_NOSENSITIV_METH);
        }

        // Declare temporary return variable in current module
        if (!isVoidType(retType)) {
            val = SValue(retType, modval);
            // cout << "Return value " << val << endl;
        }
    }
}

// Member function call expression
void ScGenerateExpr::parseMemberCall(CXXMemberCallExpr* expr, SValue& tval, 
                                     SValue& val) 
{
    // There is no value for method call
    val = NO_VALUE; 
    
    // Get arguments
    Expr** args = expr->getArgs();
    unsigned argNum = expr->getNumArgs();

    // Get method
    FunctionDecl* methodDecl = expr->getMethodDecl()->getAsFunction();
    string fname = methodDecl->getNameAsString();
    QualType retType = methodDecl->getReturnType();
    
    // Get @this expression and its type
    Expr* thisExpr = expr->getImplicitObjectArgument();
    QualType thisType = thisExpr->getType();
    // Method called for this pointer "->"
    bool isPointer = thisType->isAnyPointerType();
    bool isScIntegerType = isAnyScIntegerRef(thisType, true);
    bool isCharPtrFunc = isConstCharPtr(expr->getType());
        
    // Get value for @this if it is not string function converted to integer
    if (!(isCharPtrFunc && fname == "c_str") || strLiterWidth != 0) {
        chooseExprMethod(thisExpr, tval);
    }
    
    if (DebugOptions::isEnabled(DebugComponent::doGenFuncCall)) {
        cout << "CXXMemberCallExpr this = " << thisType.getAsString() << ", fname = " 
             << fname  << ", return type = " << retType.getAsString() 
             << ", tval " << tval << ", module is " << modval << endl;
    }

    // Constant pointer to dynamically allocated memory, need to replace by integer
    bool replacePtrObj = false; 
    SValue pival;

    // Pointer de-reference
    SValue ttval = tval;
    if (isPointer) {
        state->getValue(tval, ttval);
        //cout << "Ptr tval " << tval << " ttval " << ttval << endl;

        // Array to pointer cast
        if (ttval.isArray()) {
            ScDiag::reportScDiag(expr->getBeginLoc(), 
                                 ScDiag::SYNTH_ARRAY_TO_POINTER);
        }
        
        // Check pointer and pointe both are constant type, 
        // for dynamic object constantness specified by pointer only
        bool constPtr = isPointerToConst(tval.getType());
        replacePtrObj = constPtr && ttval.isSimpleObject();
        // Replace pointer with integer value
        if (replacePtrObj) {
            pival = state->getValue(ttval);
            //cout << "   ival " << ival << endl;
        }
    }
    
    bool lhsChannel = isScChannel(thisType);
    bool lhsZeroWidth = isScZeroWidth(ttval);
    //cout << "#" << hex << expr << " ttval " << ttval << " " << lhsZeroWidth << endl;

    // Parse method function call
    if (isScIntegerType || (!lhsChannel && lhsZeroWidth)) {
        // SC integer type object
        if (fname.find("operator") != string::npos && (
            fname.find("sc_unsigned") != string::npos ||
            fname.find("int") != string::npos ||
            fname.find("long") != string::npos ||  
            fname.find("bool") != string::npos)) {
            
            // Implicit SC type conversion to some integer, 
            // not correspondent to any operator in the code
            
            if (lhsZeroWidth) {
                val = SValue(APSInt(32, true), 10); 
                codeWriter->putLiteral(expr, val);
                
            } else {
                // Replace constant pointer with its integer value
                if (pival.isInteger()) {
                    codeWriter->putLiteral(thisExpr, pival);
                }
                codeWriter->copyTerm(thisExpr, expr);
            }
        } else 
        if (fname == "bit" || fname == "operator[]") {
            
            SCT_TOOL_ASSERT (argNum == 1, "Incorrect argument number");
            Expr* indxExpr = args[0];

            if (replacePtrObj) {
                ScDiag::reportScDiag(thisExpr->getBeginLoc(), 
                                     ScDiag::SC_BIT_CONSTPTR_BASE);
            }

            if (lhsZeroWidth) {
                val = ZW_VALUE; codeWriter->putLiteral(expr, val);
                
            } else {
                // Skip sign cast for part select index
                bool skipSignCastOrig = codeWriter->isSkipSignCast();
                codeWriter->setSkipSignCast(true);
                // Parse argument expression to fill @terms
                SValue rval;
                chooseExprMethod(indxExpr, rval);
                codeWriter->setSkipSignCast(skipSignCastOrig);

                codeWriter->putBitSelectExpr(expr, ttval, thisExpr, indxExpr);            
            }
        } else 
        if (fname == "range" || fname == "operator()") {
            
            SCT_TOOL_ASSERT (argNum == 2, "Incorrect argument number");
            Expr* hiExpr = args[0];
            Expr* loExpr = args[1];
            
            if (replacePtrObj) {
                ScDiag::reportScDiag(thisExpr->getBeginLoc(), 
                                     ScDiag::SC_RANGE_CONSTPTR_BASE);
            }

            if (lhsZeroWidth) {
                val = ZW_VALUE; codeWriter->putLiteral(expr, val);
                
            } else {
                // Skip sign cast for part select index
                bool skipSignCastOrig = codeWriter->isSkipSignCast();
                codeWriter->setSkipSignCast(true);
                // Parse to remove sub-statements
                SValue rval;
                chooseExprMethod(hiExpr, rval);
                chooseExprMethod(loExpr, rval);

                // Evaluate range par-select which is possible variable dependent
                bool useDelta = evaluateRangeExpr(hiExpr, loExpr);
                codeWriter->setSkipSignCast(skipSignCastOrig);

                codeWriter->putPartSelectExpr(expr, ttval, thisExpr, hiExpr, 
                                              loExpr, useDelta);
            }
        } else 
        if (fname == "and_reduce" || fname == "or_reduce" || 
            fname == "xor_reduce" || fname == "nand_reduce" || 
            fname == "nor_reduce" || fname == "xnor_reduce") {
            
            if (lhsZeroWidth) {
                bool result = fname == "nand_reduce" || fname == "nor_reduce" || 
                              fname == "xnor_reduce";
                val = SValue(SValue::boolToAPSInt(result), 10); 
                codeWriter->putLiteral(expr, val);
                
            } else {
                string opcode = (fname == "and_reduce") ? "&" :
                                (fname == "or_reduce") ? "|" : 
                                (fname == "xor_reduce") ? "^" :
                                (fname == "nand_reduce") ? "~&" :
                                (fname == "nor_reduce") ? "~|" : "~^";

                // Replace constant pointer with its integer value
                if (pival.isInteger()) {
                    codeWriter->putLiteral(thisExpr, pival);
                }

                // Put reduction unary expression for @this
                codeWriter->putUnary(expr, opcode, thisExpr);    
            }
        } else 
        if (fname.find("to_i") != string::npos ||
            fname.find("to_u") != string::npos ||
            fname.find("to_long") != string::npos) {
            
            if (lhsZeroWidth) {
                val = SValue(APSInt(32, true), 10); 
                codeWriter->putLiteral(expr, val);
                
            } else {
                // Replace constant pointer with its integer value
                if (pival.isInteger()) {
                    codeWriter->putLiteral(thisExpr, pival);
                }

                QualType type = expr->getType();
                codeWriter->putTypeCast(thisExpr, expr, type);

                bool isSigned = isSignedType(type);
                bool isOrigSigned = isSignedType(thisType);

                // Set/reset sign cast for literals and expressions
                if (isOrigSigned != isSigned) { 
                    codeWriter->putSignCast(expr, isSigned ? CastSign::SCAST : 
                                                             CastSign::UCAST);
                }
            }
        } else
        if (fname.find("to_bool") != string::npos) {
            // No type cast required for @to_bool as it applied for @bit_ref only
            if (lhsZeroWidth) {
                val = SValue(SValue::boolToAPSInt(false), 10); 
                codeWriter->putLiteral(expr, val);
                
            } else {
                codeWriter->copyTerm(thisExpr, expr);
            }
        } else
        if (fname.find("length") != string::npos) {
            // Get length form template parameter for @sc_bv/sc_(big)(u)int
            // not work for channel of these types
            if (lhsZeroWidth) {
                val = SValue(APSInt(32, true), 10); 
                codeWriter->putLiteral(expr, val);
                
            } else {
                if (auto length = getTemplateArgAsInt(tval.getType(), 0)) {
                    codeWriter->putLiteral(expr, SValue(*length, 10));
                } else {
                    SCT_INTERNAL_ERROR(expr->getBeginLoc(), 
                                       "Cannot get type width for length()");
                }
            }
        } else
        if (fname.find("is_01") != string::npos) {
            // For @sc_bv type, always return true
            codeWriter->putLiteral(expr, SValue(SValue::boolToAPSInt(1), 10));
            
        } else
        if (fname.find("operator") != string::npos) {
            // ->operator=, ++, --, +=, -=, ... not supported for now 
            // as it is not widely used form, it difficult to implement for arrays
            ScDiag::reportScDiag(expr->getBeginLoc(), 
                                 ScDiag::SYNTH_UNSUPPORTED_OPER) << fname 
                                 << "ScGenerateExpr::parseMemberCall";
        } else {
            ScDiag::reportScDiag(expr->getBeginLoc(), 
                                 ScDiag::SYNTH_UNSUPPORTED_OPER) << fname 
                                 << "ScGenerateExpr::parseMemberCall";
        }
    } else 
    if (lhsChannel) {
        // Channel object
        SValue cval = getChannelFromState(ttval);
        
        if ((cval.isScOutPort() || cval.isScSignal() || lhsZeroWidth) && 
            (fname == "write")) 
        {
            // Output port write method
            SCT_TOOL_ASSERT (argNum == 1, "Incorrect argument number");
            
            if (lhsZeroWidth) {
                val = ZW_VALUE; codeWriter->putLiteral(expr, val);
            
            } else {
                Expr* argExpr = args[0];
                QualType ltype = getDerefType(cval.getType());
                bool lhsRecord = isUserClass(ltype, true);
                auto lrecType = isUserClassChannel(ltype, true);
                bool lhsRecordChan = (lhsRecord && cval.isScChannel()) || 
                                     lrecType.hasValue();

                //cout << "cval " << cval << " " << lhsRecord << lhsRecordChan << endl;
                //ltype.dump();

                // Get LHS record indices 
                string lrecSuffix;
                if (cval.isRecord() || lhsRecordChan) {
                    auto lrecvecs = getRecVector(cval);
                    lrecSuffix = codeWriter->getRecordIndxs(lrecvecs);
                }

                // Parse argument expression to fill @terms
                SValue rval;
                chooseExprMethod(argExpr, rval);

                // Check for record/channel of record
                QualType rtype = getDerefType(rval.getType());
                bool rhsRecord = isUserClass(rtype, false);
                auto rrecType = isUserClassChannel(rtype, false);
                bool rhsRecordChan = (rhsRecord && rval.isScChannel()) || 
                                      rrecType.hasValue();            
                bool rhsTempRecord = rhsRecord && rval.isRecord() && 
                                     rval == temprec;
                bool rhsRefRecord = rhsRecord && isReference(rval.getType());
                // Clear used temporary record
                temprec = NO_VALUE;

                //cout << "rval " << rval << " " << rhsRecord << rhsRecordChan 
                //     << " rhsTempRecord " << rhsTempRecord << " locrecvarCtor " << locrecvarCtor << endl;
                
                SValue rrec;
                if (rhsTempRecord || rhsRecordChan && rval.isScChannel()) {
                    // Record channel is used instead of parent record
                    rrec = rval;
                } else {
                    // Get record or record channel value 
                    state->getValue(rval, rrec);
                }

                // Get RHS record indices 
                string rrecSuffix;
                if (rhsRefRecord) {
                    rrecSuffix = refRecarrIndx;
                    // Clear indices suffix after use
                    refRecarrIndx = "";
                } else 
                if (rrec.isRecord() || rhsRecordChan) {
                    auto rrecvecs = getRecVector(rrec);
                    rrecSuffix = codeWriter->getRecordIndxs(rrecvecs);
                }

                if (lhsRecord || lhsRecordChan) {
                    if (!rval) {
                        // No record for RValue, probably ctor with parameters
                        ScDiag::reportScDiag(argExpr->getBeginLoc(), 
                                             ScDiag::SYNTH_CHAN_RECORD_TEMP);
                    } else 
                    if (rhsTempRecord) {
                        codeWriter->putRecordAssignTemp(expr, cval, cval, rrec, 
                                                lrecSuffix, lrecType, state.get());
                    } else {
                        // Put assign for record channel
                        codeWriter->putRecordAssign(expr, cval, cval, rrec, 
                                                    lrecSuffix, rrecSuffix, lrecType);
                    }
                } else {
                    // Put assignment instead of method call, use @cval as it is channel
                    codeWriter->putAssign(expr, cval, thisExpr, argExpr);
                }
            }
        } else 
        if ((cval.isScChannel() || lhsZeroWidth) && (fname == "read" || 
            (fname.find("operator") != string::npos && (
             fname.find("const") != string::npos ||  // for operator const T&
             fname.find("int") != string::npos ||    // covers sc_int, sc_uint, ...   
             fname.find("char") != string::npos ||
             fname.find("long") != string::npos ||
             fname.find("bool") != string::npos ||
             fname.find("sc_bv") != string::npos)))) {

            // Channel read access method
            SCT_TOOL_ASSERT (argNum == 0, "Incorrect argument number");

            if (lhsZeroWidth) {
                val = fname == "read" ? ZW_VALUE : SValue(APSInt(32, true), 10); 
                codeWriter->putLiteral(expr, val);
            } else {
                codeWriter->copyTermRemoveBrackets(thisExpr, expr);
                // Return channel for read access, required to get channel for 
                // constant reference parameter to use channel variable
                val = cval;
            }
        } else 
        if (fname.find("operator") != string::npos && (
            fname.find("int") != string::npos ||    // covers sc_int, sc_uint, ...
            fname.find("char") != string::npos ||
            fname.find("long") != string::npos ||
            fname.find("bool") != string::npos ||
            fname.find("sc_bv") != string::npos)) {
            
            // Channel read/access operator, used when channel object is unknown
            // like result of ternary operator of channels
            
            // Copy terms for this expression
            codeWriter->copyTerm(thisExpr, expr);
            // Return channel for read access, required to get channel for 
            // constant reference parameter to use channel variable
            val = cval;
        } else 
        if (fname.find("pos") != string::npos || 
            fname.find("neg") != string::npos) {
            
            bool posEdge = fname.find("pos") != string::npos;
            bool negEdge = fname.find("neg") != string::npos;
            
            // Used for @SCT_PROPERTY
            codeWriter->putClockEdge(expr, thisExpr, posEdge, negEdge);
            
        } else 
        if (fname.find("kind") != string::npos || 
            fname.find("name") != string::npos || 
            fname.find("trace") != string::npos || 
            fname.find("dump") != string::npos || 
            fname.find("print") != string::npos) {
            
        } else {
            ScDiag::reportScDiag(expr->getSourceRange().getBegin(), 
                                 ScDiag::SYNTH_UNSUPP_CHANNEL_METH) << fname;
        }
    } else 
    // Non-channel/non-SCType method call
    if ( isAnyScCoreObject(thisType) ) {
        if (fname == "wait") {
            // Add nothing for wait() and counter assignment for wait(n)
            // Other logic implemented in ScTraverseProc
            if (argNum == 1) {
                //cout << "TYPE " << args[0]->getType().getAsString() << endl;
                // Check argument is integer type
                if (isAnyInteger(args[0]->getType())) {
                    // Parse @wait(n) argument
                    SValue ival;
                    chooseExprMethod(args[0], ival);

                    // Put wait(n) counter assignment
                    codeWriter->putWaitNAssign(expr, args[0]);
                }
            }
        } else {
            // Do nothing for other @sc_core functions
        }

    } else 
    if (isCharPtrFunc) {
        if (fname == "c_str") {
            codeWriter->copyTerm(thisExpr, expr);
            val = ttval;
        }
        
    } else    
    if (codeWriter->isParseSvaArg()) {
        // For function call in assert replace it with returned expression
        if (argNum == 0) {
            if (ttval.isInteger() && ttval.getInteger().isNullValue()) {
                // No record, @fifo in target is @nullptr, use zero value
                codeWriter->putLiteral(expr, SValue(APSInt(64, true), 10));
                
            } else {
                // Normal record for channel
                Stmt* funcBody = methodDecl->getBody();
                //expr->dumpColor(); funcBody->dumpColor();

                // Get return statement from function body
                ReturnStmt* retStmt = nullptr;
                if (auto compStmt = dyn_cast<ReturnStmt>(funcBody)) {
                    retStmt = compStmt;
                } else 
                if (auto compStmt = dyn_cast<CompoundStmt>(funcBody)) {
                    retStmt = dyn_cast<ReturnStmt>(compStmt->body_front());
                }

                Expr* retExpr = retStmt ? retStmt->getRetValue() : nullptr;

                // Parse return expression
                if (retExpr) {
                    QualType retType = retExpr->getType();
                    SCT_TOOL_ASSERT(!isUserClass(retType, false), 
                        "No record type return for function call in assertion supported");

                    SValue funcModval = getRecordFromState(tval, 
                                            ArrayUnkwnMode::amNoValue);
                    // Set @recordValueName to support array of record/MIF
                    if (auto thisStr = codeWriter->getStmtString(thisExpr)) {
                        codeWriter->setRecordName(funcModval, thisStr.getValue());
                        //cout << "   thisStr : " << funcModval << " " << thisStr.getValue() << endl;
                    }
                    SValue curModval = modval;
                    modval = funcModval;

                    chooseExprMethod(retExpr, val);
                    codeWriter->copyTerm(retExpr, expr);
                    modval = curModval;

                } else {
                    ScDiag::reportScDiag(expr->getBeginLoc(), 
                                     ScDiag::SYNTH_FUNC_IN_ASSERT);
                }
            }
        } else {
            ScDiag::reportScDiag(expr->getBeginLoc(), 
                                 ScDiag::SYNTH_FUNC_IN_ASSERT);
        }
    } else {
        // General methods, most logic is in ScTraverseProc 
        if (codeWriter->isEmptySensitivity()) {
            ScDiag::reportScDiag(expr->getBeginLoc(), 
                                 ScDiag::SYNTH_NOSENSITIV_METH);
        }

        // Declare temporary return variable in current module
        if (!isVoidType(retType)) {
            val = SValue(retType, modval);
            //cout << "Return value " << val << endl;
        }
    }
}

// Operator call expression
void ScGenerateExpr::parseOperatorCall(CXXOperatorCallExpr* expr, SValue& tval,
                                       SValue& val) 
{
    // There is generally no value for operator call
    val = NO_VALUE;
    
    // Get arguments
    Expr** args = expr->getArgs();
    unsigned argNum = expr->getNumArgs();
    // Get first argument type, it can be this object
    SCT_TOOL_ASSERT (argNum > 0, "Operator without arguments");
    Expr* lexpr = args[0];
    QualType thisType = lexpr->getType();

    // Get operator name
    FunctionDecl* methodDecl = expr->getCalleeDecl()->getAsFunction();
    string fname = methodDecl->getNameAsString();
    QualType retType = methodDecl->getReturnType();
    auto nsname = getNamespaceAsStr(methodDecl);
    
    // Get opcode and opcode string
    OverloadedOperatorKind opcode = expr->getOperator();
    string opStr = getOperatorSpelling(opcode);
    
    bool isAssignOperator = expr->isAssignmentOp() && opcode == OO_Equal;
    bool isIncrDecr = opcode == OO_PlusPlus || opcode == OO_MinusMinus;
    bool isCompoundAssign = opcode == OO_PlusEqual || opcode == OO_MinusEqual || 
            opcode == OO_StarEqual || opcode == OO_SlashEqual ||
            opcode == OO_PercentEqual || 
            opcode == OO_GreaterGreaterEqual || opcode == OO_LessLessEqual ||
            opcode == OO_AmpEqual || opcode == OO_PipeEqual || 
            opcode == OO_CaretEqual;
    
    if (DebugOptions::isEnabled(DebugComponent::doGenFuncCall)) {
        cout << "ScGeneratExpr::parseOperatorCall fname : " << fname 
             << ", type : " << thisType.getAsString() << endl;
    }
    
    // Check LHS is temporary expression materialized into in memory value
    // Required for @sc_biguint/@sc_bigint
    bool lhsIsTempExpr = isa<MaterializeTemporaryExpr>(lexpr) && 
                         (isScBigInt(thisType) || isScBigUInt(thisType));
    
    if (isAssignOperator) {
        // Assignment "operator=" for all types including SC data types
        // Assignments with add, subtract, ... processed below
        // Output port/signal write method
        SCT_TOOL_ASSERT (argNum == 2, "Incorrect argument number");

        if (lhsIsTempExpr) {
            ScDiag::reportScDiag(expr->getBeginLoc(), 
                                 ScDiag::SYNTH_TEMP_EXPR_ARG);
        }
        
        // Parse argument expression to fill @terms
        bool lastAssignLHS = assignLHS; assignLHS = true;
        SValue lval;
        chooseExprMethod(lexpr, lval);
        assignLHS = lastAssignLHS;
        
        val = lval;
        tval = lval;
        bool lhsZeroWidth = isScZeroWidth(lval);
        
        if (lhsZeroWidth) {
            val = ZW_VALUE;
            
        } else 
        if (isAssignOperatorSupported(lval.getType())) {
            // SS channel operators, used for operator =() of register and ff_sync
            // General methods, most logic is in ScTraverseProc 
            if (codeWriter->isEmptySensitivity()) {
                ScDiag::reportScDiag(expr->getBeginLoc(),
                                     ScDiag::SYNTH_NOSENSITIV_METH);
            }

            // Declare temporary return variable in current module
            val = NO_VALUE;
            if (!isVoidType(retType)) {
                val = SValue(retType, modval);
                //cout << "Return value " << val << endl;
            }
            
        } else {
            // Check for record/channel of record
            QualType ltype = getDerefType(lval.getType());
            bool lhsRecord = isUserClass(ltype, true);
            auto lrecType = isUserClassChannel(ltype, true);
            bool lhsRecordChan = (lhsRecord && lval.isScChannel()) || 
                                 lrecType.hasValue();
            bool lhsRefRecord = lhsRecord && isReference(lval.getType());

            SValue lrec;
            state->getValue(lval, lrec);
            //cout << "lval " << lval << " lrec " << lrec << " " << lhsRecord << lhsRecordChan << endl;
            //ltype.dump();

            // Get LHS record indices
            string lrecSuffix;
            if (lhsRefRecord) {
                lrecSuffix = refRecarrIndx;
                // Clear indices suffix after use
                refRecarrIndx = "";
            } else 
            if (lrec.isRecord() || lhsRecordChan) {
                auto lrecvecs = getRecVector(lrec);
                lrecSuffix = codeWriter->getRecordIndxs(lrecvecs);
            }

            // @strLiterWidth/@strLiterUnsigned work for integer argument only
            unsigned lastWidth = strLiterWidth;
            bool lastUnsigned = strLiterUnsigned;
            if (auto typeInfo = getIntTraits(getTypeForWidth(expr), true)) {
                strLiterWidth = typeInfo.getValue().first;
                strLiterUnsigned = typeInfo.getValue().second;
            } else {
                strLiterWidth = 0;  // Provides no string literal parsing
            }

            SValue rval;
            Expr* rexpr = args[1];
            // Set @locrecvar to provide owner to record value created for @rval
            if (lrec.isRecord()) {locrecvar = lval; locrecvarCtor = false;}
            chooseExprMethod(rexpr, rval);
            if (lrec.isRecord()) locrecvar = NO_VALUE;

            strLiterWidth = lastWidth;
            strLiterUnsigned = lastUnsigned;

            // Check for record/channel of record
            QualType rtype = getDerefType(rval.getType());
            bool rhsRecord = isUserClass(rtype, false);
            auto rrecType = isUserClassChannel(rtype, false);
            bool rhsRecordChan = (rhsRecord && rval.isScChannel()) || 
                                  rrecType.hasValue();
            bool rhsTempRecord = rhsRecord && rval.isRecord() && 
                                 rval == temprec;
            bool rhsRefRecord = rhsRecord && isReference(rval.getType());
            // Clear used temporary record
            temprec = NO_VALUE;
            
            //cout << "rval " << rval << " " << rhsRecord << rhsRecordChan 
            //     << " rhsTempRecord " << rhsTempRecord << " locrecvarCtor " << locrecvarCtor << endl;
            //rtype.dump();
            //state->print();
            
            SValue rrec;
            if (rhsTempRecord || rhsRecordChan && rval.isScChannel()) {
                // Record channel is used instead of parent record
                rrec = rval;
            } else {
                // Get record or record channel value 
                state->getValue(rval, rrec);
            }

            // Get RHS record indices
            string rrecSuffix;
            if (rhsRefRecord) {
                rrecSuffix = refRecarrIndx;
                // Clear indices suffix after use
                refRecarrIndx = "";
            } else 
            if (rrec.isRecord() || rhsRecordChan) {
                auto rrecvecs = getRecVector(rrec);
                rrecSuffix = codeWriter->getRecordIndxs(rrecvecs);
            }

            // Get owner variable, required for arrays
            SValue lvar = state->getVariableForValue(lval);
            // Try to get channel value, used as key in @varTraits 
            SValue cval = getChannelFromState(lvar);
            if (cval) {
                lvar = cval;
            }

            if (lhsRecord || lhsRecordChan) {
                // Put assign for records and record channels
                if (!rval) {
                    // No record for RValue, probably ctor with parameters
                    ScDiag::reportScDiag(rexpr->getBeginLoc(), 
                                         ScDiag::SYNTH_CHAN_RECORD_TEMP);
                } else 
                if (locrecvarCtor && !lhsRecordChan) {
                    // Do nothing as soon as record already constructed
                    // Avoid double assignment to record fields
                } else
                if (rhsTempRecord) {
                    codeWriter->putRecordAssignTemp(expr, lvar, lrec, rrec, 
                                                    lrecSuffix, lrecType, state.get());
                } else
                if (rhsRecord || rhsRecordChan) {
                    if (codeWriter->getAssignStmt(rexpr)) {
                        ScDiag::reportScDiag(expr->getBeginLoc(), 
                                            ScDiag::SYNTH_MULT_ASSIGN_REC);
                    }
                    codeWriter->putRecordAssign(expr, lvar, lrec, rrec, 
                               lrecSuffix, rrecSuffix, lrecType);
                } else {
                    SCT_INTERNAL_FATAL (expr->getBeginLoc(), 
                                        "User defined operator= () not supported yet"); 
                }
            } else {
                // Put assign for channels and others
                codeWriter->putAssign(expr, lvar, lexpr, rexpr);
                if (auto multipleAssign = codeWriter->getAssignStmt(rexpr)) {
                    codeWriter->addTerm(expr, multipleAssign);
                    codeWriter->copyTerm(multipleAssign, expr);
                }

                // Register this statement to add comment into scope graph
                auto i = constReplacedFunc.find(rexpr);
                if (i != constReplacedFunc.end()) {
                    constReplacedFunc.emplace(expr, i->second);
                    //cout << "Add Operator expr " << hex << expr << " for " << i->second << dec << endl;
                }
            }
        }
    } else 
    if (isScPort(thisType) && opcode == OO_Arrow) {
        // Operator "->" for @sc_port<IF>
        
        // Get value for @this which points-to module/MIF object
        SValue lval;
        chooseExprMethod(lexpr, lval);
        bool lhsZeroWidth = isScZeroWidth(lval);

        if (lhsZeroWidth) {
            val = ZW_VALUE; codeWriter->putLiteral(expr, val);
            
        } else {
            // Dereferenced value provided in @val
            state->getValue(lval, val);

            // Get MIF record value from port
            SValue recval; state->getValue(val, recval);
            SValue dyntval;
            state->getMostDerivedClass(recval, dyntval);
            //cout << "Operator -> dyntval " << dyntval << " recval " << recval 
            //         << " val " << val << " lval " << lval  << endl;
            //state->print();
            
            if (dyntval.isRecord()) {
                // Provide MIF variable and array indices to be used in 
                // TraverseProc to set @recordValueName
                // Check port bound to an element of record/MIF array 
                vector<SValue> mifarrs = state->getAllMifArrays(
                                            dyntval, ScState::MIF_CROSS_NUM);
                std::string portMifIndicesStr;
                for (const SValue& mval : mifarrs) {
                    portMifIndicesStr += "["+std::to_string(mval.getArray().getOffset())+"]";
                }

                vector<SValue> recarrs;
                SValue dvar = state->getVariableForValue(dyntval);
                codeWriter->putValueExpr(expr, dvar, recarrs, false, false, 
                                         "", portMifIndicesStr);
            } else
            if (lval.isVariable() || lval.isTmpVariable()) {
                // Use variable to pointer variable, required for array of pointers
                codeWriter->copyTerm(lexpr, expr);
            }
        }
    } else 
    if ((isStdArray(thisType) || isStdVector(thisType) || isScVector(thisType)) && 
        opcode == OO_Subscript) {
        // sc_vector access at index
        SCT_TOOL_ASSERT (argNum == 2, "Incorrect argument number");
        Expr* rexpr = args[1];
        val = parseArraySubscript(expr, lexpr, rexpr);

    } else
    if (isIoStream(thisType)) {
        // Do nothing for @cout << and @cin >>
        
    } else 
    if (nsname && *nsname == "sc_dt") {
        // SC data type operators
        bool lastAssignLHS = assignLHS; 
        if (isIncrDecr || isCompoundAssign) assignLHS = true;
        SValue lval;
        chooseExprMethod(lexpr, lval);
        assignLHS = lastAssignLHS;
        bool lhsZeroWidth = isScZeroWidth(lval);
        
        if (opcode == OO_Subscript) { // "[]"
            SCT_TOOL_ASSERT (argNum == 2, "Incorrect argument number");
            Expr* rexpr = args[1];

            if (lhsIsTempExpr) {
                ScDiag::reportScDiag(expr->getBeginLoc(), 
                                     ScDiag::SYNTH_TEMP_EXPR_ARG);
            }
            
            if (lhsZeroWidth) {
                val = ZW_VALUE; codeWriter->putLiteral(expr, val);
                
            } else {
                // Skip sign cast for part select index
                bool skipSignCastOrig = codeWriter->isSkipSignCast();
                codeWriter->setSkipSignCast(true);
                SValue rval; 
                chooseExprMethod(rexpr, rval);
                codeWriter->setSkipSignCast(skipSignCastOrig);

                // Put bit access expression as array index access
                codeWriter->putBitSelectExpr(expr, lval, lexpr, rexpr);
            }
        } else 
        if (opcode == OO_Call) {  // "()"
            SCT_TOOL_ASSERT (argNum == 3, "Incorrect argument number");
            if (lhsIsTempExpr) {
                ScDiag::reportScDiag(expr->getBeginLoc(), 
                                     ScDiag::SYNTH_TEMP_EXPR_ARG);
            }

            if (lhsZeroWidth) {
                val = ZW_VALUE; codeWriter->putLiteral(expr, val);
                
            } else {
                // Skip sign cast for part select index
                bool skipSignCastOrig = codeWriter->isSkipSignCast();
                codeWriter->setSkipSignCast(true);
                SValue rval; SValue rrval;
                chooseExprMethod(args[1], rval);
                chooseExprMethod(args[2], rrval);

                // Evaluate range par-select which is possible variable dependent
                bool useDelta = evaluateRangeExpr(args[1], args[2]);
                codeWriter->setSkipSignCast(skipSignCastOrig);

                codeWriter->putPartSelectExpr(expr, lval, lexpr, args[1], args[2], 
                                              useDelta);
            }
        } else 
        // "," which is @concat() here    
        if (opcode == OO_Comma) { 
            SCT_TOOL_ASSERT (argNum == 2, "Incorrect argument number");
            Expr* rexpr = args[1];
            SValue rval; 
            chooseExprMethod(rexpr, rval);
            
            // Check incorrect type conversion to @bool for operand with 
            // not specified length
            auto checkCastToBool = [expr](const Expr* arg) {
                if (auto castExpr = dyn_cast<ImplicitCastExpr>(arg)) {
                    auto castKind = castExpr->getCastKind();
                    if (castKind == CK_IntegralToBoolean) {
                        if (isa<BinaryOperator>(castExpr->getSubExpr())) {
                            ScDiag::reportScDiag(expr->getBeginLoc(), 
                                                ScDiag::SYNTH_CONCAT_CAST_BOOL);
                        }
                    }
                }
            };
            checkCastToBool(lexpr);
            checkCastToBool(rexpr);
            
            // Report implicit integer to boolean cast, can be non-intended
            if (isIntToBoolCast(lexpr) || isIntToBoolCast(rexpr)) {
                ScDiag::reportScDiag(expr->getBeginLoc(), 
                                     ScDiag::SC_CONCAT_INT_TO_BOOL);
            }
            
            bool rhsZeroWidth = isScZeroWidth(rval);
            if (lhsZeroWidth && rhsZeroWidth) {
                // Put 32bit unsigned 0
                val = ZW_VALUE; codeWriter->putLiteral(expr, val);
            } else 
            if (lhsZeroWidth) {
                codeWriter->copyTerm(rexpr, expr);
            } else 
            if (rhsZeroWidth) {
                codeWriter->copyTerm(lexpr, expr);
            } else {
                codeWriter->putConcat(expr, lexpr, rexpr);
            }
            
        } else 
        // "!" "~" 
        if (opcode == OO_Exclaim || opcode == OO_Tilde) {
            // Postfix ==/-- has artifical argument
            SCT_TOOL_ASSERT (argNum == 1, "Incorrect argument number");
            
            if (lhsZeroWidth) {
                val = SValue(SValue::boolToAPSInt(true), 10);
                codeWriter->putLiteral(expr, val);
            } else {
                codeWriter->putUnary(expr, opStr, lexpr);
            }
            
            if (codeWriter->getAssignStmt(lexpr)) {
                ScDiag::reportScDiag(expr->getBeginLoc(),
                                     ScDiag::SYNTH_INTERNAL_ASSIGN_OPER);
            }
        } else 
        // "++" "--"
        if (isIncrDecr) {
            // Postfix ++/-- has artifical argument
            SCT_TOOL_ASSERT (argNum == 1 || argNum == 2, "Incorrect argument number");
            
            if (codeWriter->isEmptySensitivity()) {
                ScDiag::reportScDiag(expr->getBeginLoc(), 
                                     ScDiag::SYNTH_NOSENSITIV_METH);
            }
            if (codeWriter->isResetSection()) {
                SValue varval = state->getVariableForValue(lval);
                if (codeWriter->isRegister(varval)) {
                    ScDiag::reportScDiag(lexpr->getBeginLoc(), 
                                         ScDiag::SYNTH_MODIFY_REG_IN_RESET);
                }
            }
            if (lhsIsTempExpr) {
                ScDiag::reportScDiag(expr->getBeginLoc(), 
                                     ScDiag::SYNTH_TEMP_EXPR_ARG);
            }
            
            SValue rval; 
            if (argNum == 2) chooseExprMethod(args[1], rval);
            
            if (lhsZeroWidth) {
                val = ZW_VALUE; codeWriter->putLiteral(expr, val);
            } else {
                codeWriter->putUnary(expr, opStr, lexpr, argNum == 1);
            }
            
            if (codeWriter->getAssignStmt(lexpr)) {
                ScDiag::reportScDiag(expr->getBeginLoc(),
                                     ScDiag::SYNTH_INTERNAL_ASSIGN_OPER);
            }
        } else    
        // Unary "-" and "+"
        if (argNum == 1 && (opcode == OO_Plus || opcode == OO_Minus)) 
        {
            if (lhsZeroWidth) {
                val = ZW_VALUE; codeWriter->putLiteral(expr, val);
            } else {
                if (opcode == OO_Minus) {
                    codeWriter->putUnary(expr, opStr, lexpr, true);
                } else {
                    val = lval;
                    codeWriter->copyTerm(lexpr, expr);
                }
            }
            
            if (codeWriter->getAssignStmt(lexpr)) {
                ScDiag::reportScDiag(expr->getBeginLoc(),
                                     ScDiag::SYNTH_INTERNAL_ASSIGN_OPER);
            }
        } else   
        // "+" "-" "*"  "/" "==" "!=" "<" "<=" ">" ">=" "<<" ">>" "%" "^" "&" "|" 
        // There is no operators "&&" "||" for SC data types
        if (argNum == 2 && (opcode == OO_Plus || opcode == OO_Minus || 
            opcode == OO_Star || opcode == OO_Slash || opcode == OO_EqualEqual || 
            opcode == OO_ExclaimEqual || opcode == OO_Less ||
            opcode == OO_LessEqual || opcode == OO_Greater || 
            opcode == OO_GreaterEqual || opcode == OO_LessLess || 
            opcode == OO_GreaterGreater || opcode == OO_Percent || 
            opcode == OO_Caret || opcode == OO_Amp || opcode == OO_Pipe || 
            opcode == OO_AmpAmp || opcode == OO_PipePipe))
        {
            // As no &&/||, so no LHS based evaluation required
            SCT_TOOL_ASSERT (opcode != OO_AmpAmp && opcode != OO_PipePipe,
                             "No &&/|| for SC data types supported");
                    
            Expr* rexpr = args[1];
            SValue rval; 
            chooseExprMethod(rexpr, rval);
            
            if (opcode == OO_GreaterGreater) {
                opStr = ">>>";
            } else 
            if (opcode == OO_LessLess) {
                opStr = "<<<";        
            }

            // RHS operand is self-determined in Verilog (see LRM 11.6)
            // 32bit is always enough for shift value
            if (codeWriter->isIncrWidth(rexpr) && 
                (opcode == OO_GreaterGreater || opcode == OO_LessLess)) {
                // 32bit is always enough for shift value
                unsigned width = codeWriter->getExprTypeWidth(rexpr, 32);
                width = (width > 32) ? 32 : width;
                codeWriter->extendTypeWidth(rexpr, width);
            }

            // LHS operand is sized to result width in Verilog, 
            // so it needs to be updated for SHR/DIV/REM only
            if (codeWriter->isIncrWidth(lexpr) && 
                (opcode == OO_Slash || opcode == OO_GreaterGreater || 
                 opcode == OO_Percent)) {
                // Try to get expression/type width
                unsigned width = codeWriter->getExprTypeWidth(lexpr);
                codeWriter->extendTypeWidth(lexpr, width);
            }
            codeWriter->putBinary(expr, opStr, lexpr, rexpr);
            
            if (codeWriter->getAssignStmt(lexpr) || 
                codeWriter->getAssignStmt(rexpr)) {
                ScDiag::reportScDiag(expr->getBeginLoc(),
                                     ScDiag::SYNTH_INTERNAL_ASSIGN_OPER);
            }
            
        } else     
        // "+=" "-=" "*="  "/=" "%=" ">>=" "<<=" "&=" "|=" "^="
        if (isCompoundAssign)
        {
            SCT_TOOL_ASSERT (argNum == 2, "Incorrect argument number");
            
            if (codeWriter->isEmptySensitivity()) {
                ScDiag::reportScDiag(expr->getBeginLoc(), 
                                     ScDiag::SYNTH_NOSENSITIV_METH);
            }
            if (codeWriter->isResetSection()) {
                SValue varval = state->getVariableForValue(lval);
                if (codeWriter->isRegister(varval)) {
                    ScDiag::reportScDiag(lexpr->getBeginLoc(), 
                                         ScDiag::SYNTH_MODIFY_REG_IN_RESET);
                }
            }
           
            if (lhsZeroWidth) {
                val = ZW_VALUE; codeWriter->putLiteral(expr, val);
                
            } else {
                // Left value is result of assignment statement
                val = lval;

                if (lhsIsTempExpr) {
                    ScDiag::reportScDiag(expr->getBeginLoc(), 
                                         ScDiag::SYNTH_TEMP_EXPR_ARG);
                }

                Expr* rexpr = args[1];
                SValue rval; 
                chooseExprMethod(rexpr, rval);

                if (opcode == OO_GreaterGreaterEqual) {
                    opStr = ">>>";
                } else
                if (opcode == OO_LessLessEqual) {
                    opStr = "<<<";
                } else {
                    // Remove last symbol "=" from @opcode
                    opStr = opStr.substr(0, opStr.length()-1);
                }

                // RHS operand is self-determined in Verilog (see LRM 11.6)
                // 32bit is always enough for shift value
                if (codeWriter->isIncrWidth(rexpr) && 
                    (opcode == OO_GreaterGreaterEqual || opcode == OO_LessLessEqual)) {
                    // 32bit is always enough for shift value
                    unsigned width = codeWriter->getExprTypeWidth(rexpr, 32);
                    width = (width > 32) ? 32 : width;
                    codeWriter->extendTypeWidth(rexpr, width);
                }

                // Get owner variable, required for arrays
                SValue lvar = state->getVariableForValue(lval);

                codeWriter->putCompAssign(expr, opStr, lvar, lexpr, rexpr);

                // Consider multiple assignments
                if (auto multipleAssign = codeWriter->getAssignStmt(rexpr)) {
                    codeWriter->addTerm(expr, multipleAssign);
                    codeWriter->copyTerm(multipleAssign, expr);
                }
            }
        } else { 
            ScDiag::reportScDiag(expr->getSourceRange().getBegin(), 
                                 ScDiag::SYNTH_UNSUPPORTED_OPER) << opStr << 
                                 "ScGenerateExpr::parseOperatorCall";
        }
    } else {
        SCT_INTERNAL_FATAL(expr->getBeginLoc(), 
                           string("Unsupported operator ") + fname);
    }
}

// Return statement
void ScGenerateExpr::parseReturnStmt(ReturnStmt* stmt, SValue& val) 
{
    // There is generally no value for operator call
    val = NO_VALUE;

    Expr* expr = stmt->getRetValue();
    
    if (expr != nullptr) {
        // Remove @ExprWithCleanups from @CXXConstructExpr
        auto retExpr = removeExprCleanups(expr);
        // Check for copy constructor to use record RValue
        auto ctorExpr = getCXXCtorExprArg(retExpr);
        if (ctorExpr) {
            if (!ctorExpr->getConstructor()->isCopyOrMoveConstructor()) {
                ScDiag::reportScDiag(expr->getBeginLoc(), 
                         ScDiag::SYNTH_NONTRIVIAL_COPY);
            }
        }
        QualType retType = expr->getType();
        bool isPointer = sc::isPointer(retType);

        // Skip zero width parameter
        if (isZeroWidthType(retType) || isZeroWidthArrayType(retType)) {
            val = ZW_VALUE; return;
        }
        
        // Set return temporal variable value to analyze @CXXConstructExpr and
        // use as record copy local variable
        locrecvar = returnValue; locrecvarCtor = false;

        // Parse return expression
        chooseExprMethod(expr, val);
        
        // Clear after
        locrecvar = NO_VALUE;
        
        // Store record value in return variable
        if (returnValue.isTmpVariable()) {
            assignValueInState(returnValue, val);
        } else
        if (returnValue.isUnknown()) {
            // Do nothing
        } else {
            SCT_TOOL_ASSERT (false, "Unexpected kind of return variable value");
        }
        
        // Assign to specified temporary variable
        // Put assign for record and others
        bool isRecord = isUserClass(getDerefType(retType), true);

        if (isRecord) {
            // Get field initialization string from @CXXConstructExpr
            if (ctorExpr) {
                codeWriter->copyTerm(ctorExpr, stmt);
            }
            
        } else 
        if (isPointer) {
            // For pointer type pointed object/array registered to use in
            // ScTraverseProc::chooseExprMethod()
            SValue vval = getValueFromState(val, ArrayUnkwnMode::amArrayUnknown);
            returnPtrVal = state->getVariableForValue(vval);

        } else {
            codeWriter->putAssign(stmt, returnValue, expr);
            
            // Register this statement to add comment into scope graph
            auto i = constReplacedFunc.find(retExpr);
            if (i != constReplacedFunc.end()) {
                constReplacedFunc.emplace(stmt, i->second);
                //cout << "Add Return stmt " << hex << stmt << " for " << i->second << dec << endl;
            }
        }
        
        //cout << codeWriter->getStmtString(stmt).getValue() << endl;
    } else {
        // Nothing to write
    }
}

// Ternary operator (...) ? ... : ...
void ScGenerateExpr::parseConditionalStmt(ConditionalOperator* stmt, SValue& val) 
{
    // There is no value for unary statement yet
    val = NO_VALUE;
    
    Expr* cexpr = stmt->getCond();
    Expr* lexpr = stmt->getLHS();
    Expr* rexpr = stmt->getRHS();
    SValue cval; SValue fcval;

    // Parse condition
    chooseExprMethod(cexpr, cval);
    // Get condition for term
    getTermCondValue(stmt, cval, fcval);
    //cout << "? lexpr " << hex << stmt << dec << " cval " << cval << endl;
    
    QualType ltype = getDerefType(lexpr->getType());
    bool lhsRecord = isUserClass(ltype, true);
    auto lrecType = isUserClassChannel(ltype, true);
    bool lhsRecordChan = (lhsRecord && cval.isScChannel()) || 
                         lrecType.hasValue();    
    
    if (lhsRecord || lhsRecordChan) {
        // Record type
        SCT_INTERNAL_FATAL(stmt->getBeginLoc(), 
                        "Conditional operator for records is not supported yet");
        
    } else
    if (cval.isInteger()) {
        // Try to calculate condition into integer constant
        if (cval.getInteger().isNullValue()) {
            SValue rval;
            chooseExprMethod(rexpr, rval);
            val = rval;
            codeWriter->copyTerm(rexpr, stmt);
            
        } else {
            SValue lval;
            chooseExprMethod(lexpr, lval);
            val = lval;
            codeWriter->copyTerm(lexpr, stmt);
        }
    } else {
        SValue lval; SValue rval;
        chooseExprMethod(lexpr, lval);
        chooseExprMethod(rexpr, rval);
        
        codeWriter->putCondStmt(stmt, cexpr, lexpr, rexpr);
    }
}

// Choose and run DFS step in accordance with expression type.
void ScGenerateExpr::chooseExprMethod(Stmt *stmt, SValue &val)
{
    ScParseExpr::chooseExprMethod(stmt, val);
}

// ----------------------------------------------------------------------------
// Parse general statement 
llvm::Optional<string> ScGenerateExpr::parse(const Stmt* stmt)
{
    codeWriter->startStmt();
    // Clear indices after previous statement
    refRecarrIndx = "";

    SValue val;
    auto ncstmt = const_cast<Stmt*>(stmt);
    chooseExprMethod(ncstmt, val);

    if (val.isScZeroWidth()) return llvm::None;
    
    return codeWriter->getStmtString(ncstmt);
}

// Parse block terminator statement 
llvm::Optional<string> ScGenerateExpr::parseTerm(const Stmt* stmt, 
                                                 const SValue& termCond, 
                                                 bool artifIf)
{
    codeWriter->startStmt();
    
    // It is prohibited to use any statement except IF in method w/o sensitivity
    if (codeWriter->isEmptySensitivity() && !isa<const IfStmt>(stmt) && 
        !isa<const ConditionalOperator>(stmt)) {
        ScDiag::reportScDiag(stmt->getBeginLoc(), 
                             ScDiag::SYNTH_NOSENSITIV_METH);
    }

    if (auto ifstmt = dyn_cast<const IfStmt>(stmt)) {
        auto cexpr = const_cast<Expr*>(ifstmt->getCond());

        if (termCond.isInteger()) {
            codeWriter->putLiteral(cexpr, termCond);
        } else {
            SValue val;
            chooseExprMethod(cexpr, val);
            // No IF statement with unknown condition in method w/o sensitivity
            if (codeWriter->isEmptySensitivity()) {
                ScDiag::reportScDiag(stmt->getBeginLoc(), 
                                     ScDiag::SYNTH_NOSENSITIV_METH);
            }
        }
        
        return codeWriter->getIfString(cexpr);
        
    } else 
    if (auto swstmt = dyn_cast<const SwitchStmt>(stmt)) {
        auto cexpr = const_cast<Expr*>(swstmt->getCond());

        if (termCond.isInteger()) {
            codeWriter->putLiteral(cexpr, termCond);
        } else {
            SValue val;
            chooseExprMethod(cexpr, val);
        }
        return codeWriter->getSwitchString(cexpr);

    } else  
    if (auto binstmt = dyn_cast<const BinaryOperator>(stmt)) {
        auto cexpr = binstmt->getLHS();

        if (termCond.isInteger()) {
            codeWriter->putLiteral(cexpr, termCond);
        } else {
            SValue val;
            chooseExprMethod(cexpr, val);
        }
        return codeWriter->getStmtString(cexpr);    // Not used

    } else 
    if (auto cndstmt = dyn_cast<const ConditionalOperator>(stmt)) {
        auto cexpr = cndstmt->getCond();

        if (termCond.isInteger()) {
            codeWriter->putLiteral(cexpr, termCond);
        } else {
            SValue val;
            chooseExprMethod(cexpr, val);
        }
        return codeWriter->getStmtString(cexpr);    // Not used

    } else 
    if (auto forstmt = dyn_cast<const ForStmt>(stmt)) {
        auto init  = const_cast<Stmt*>(forstmt->getInit());
        auto cexpr = const_cast<Expr*>(forstmt->getCond());
        auto incr  = const_cast<Expr*>(forstmt->getInc());
        SValue val;
        
        if (init) {
            DeclStmt* declStmt = dyn_cast<DeclStmt>(init);
            if (declStmt && !declStmt->isSingleDecl()) {
                ScDiag::reportScDiag(init->getSourceRange().getBegin(), 
                                     ScDiag::CPP_LOOP_COMPLEX_INIT);
            }
            // Set FOR loop mode to put declaration into @init statement string
            // If there is not declaration it means normal local variable
            // is used as loop counter
            if (!artifIf) codeWriter->setForLoopInit(declStmt);
            chooseExprMethod(init, val);
            if (!artifIf) codeWriter->setForLoopInit(false);
        }
        if (cexpr) {
            if (!artifIf && termCond.isInteger()) {
                codeWriter->putLiteral(cexpr, termCond);
            } else {
                chooseExprMethod(cexpr, val);
            }
        }
        if (incr) {
            BinaryOperator* binStmt = dyn_cast<BinaryOperator>(incr);
            if (binStmt && 
                binStmt->getOpcodeStr(binStmt->getOpcode()) == ",") {
                ScDiag::reportScDiag(incr->getSourceRange().getBegin(), 
                                     ScDiag::CPP_LOOP_COMPLEX_INCR);
            }
            chooseExprMethod(incr, val);
        }

        if (artifIf) {
            return codeWriter->getIfString(cexpr);
        } else {
            return codeWriter->getForString(init, cexpr, incr);
        }
        
    } else 
    if (auto whiStmt = dyn_cast<const WhileStmt>(stmt)) {
        auto cexpr = const_cast<Expr*>(whiStmt->getCond());

        if (termCond.isInteger()) {
            codeWriter->putLiteral(cexpr, termCond);
        } else {
            SValue val;
            chooseExprMethod(cexpr, val);
        }
        
        if (artifIf) {
            return codeWriter->getIfString(cexpr);
        } else {
            return codeWriter->getWhileString(cexpr);
        }

    } else 
    if (auto doStmt = dyn_cast<const DoStmt>(stmt)) {
        auto cexpr = const_cast<Expr*>(doStmt->getCond());

        if (termCond.isInteger()) {
            codeWriter->putLiteral(cexpr, termCond);
        } else {
            SValue val;
            chooseExprMethod(cexpr, val);
        }
        
        if (artifIf) {
            return codeWriter->getIfString(cexpr);
        } else {
            return codeWriter->getWhileString(cexpr);
        }

    } else 
    if (isa<const BreakStmt>(stmt)) {
        return codeWriter->getBreakString();

    } else 
    if (isa<const ContinueStmt>(stmt)) {
        return codeWriter->getContinueString();

    } else {
        SCT_INTERNAL_FATAL(stmt->getBeginLoc(), string("Unsupported terminator ")+
                         stmt->getStmtClassName());
        
        return llvm::Optional<string>();
    }    
}

}