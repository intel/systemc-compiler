/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Author: Mikhail Moiseev
 */

#include "sc_tool/expr/ScParseExprValue.h"
#include "sc_tool/diag/ScToolDiagnostic.h"
#include "sc_tool/utils/CppTypeTraits.h"
#include "sc_tool/utils/ScTypeTraits.h"
#include "sc_tool/utils/CfgFabric.h"
#include "sc_tool/utils/DebugOptions.h"
#include "sc_tool/ScCommandLine.h"
#include "ScParseExprValue.h"

namespace sc {

using namespace std;
using namespace clang;
using namespace llvm;


void ScParseExprValue::declareValue(const SValue& var) 
{
    if (!var.isVariable()) {
        SCT_INTERNAL_ERROR (currStmt->getBeginLoc(), 
                            "Non variable value in declareValue()");
    }

    state->declareValue(var);
    
    //cout << "DL " << var << endl;
    auto i = defVarStmts.emplace(var, unordered_set<Stmt*>({currStmt}));
    if (!i.second) {
        i.first->second.insert(currStmt);
    }
}

void ScParseExprValue::writeToValue(const SValue& val, bool isDefined) 
{
    QualType valType = getDerefType(val.getType());
    bool isRecord = !val.isScChannel() && isUserClass(valType, false);
    //cout << "writeToValue val " << val << " isDefined " << isDefined 
    //     << " isRecord " << isRecord << endl;
    
    // Write all record fields, do not consider record channel
    // Do not consider record reference 
    // Record/record channel value is also written together with its fields
    if (isRecord) {
        auto recDecl = valType->getAsRecordDecl();
        SCT_TOOL_ASSERT (recDecl, "No record declaration found");
        
        SValue parent = val;
        if (val.isVariable()) state->getValue(val, parent);
        //cout << "  parent " << parent << endl;
        
        // Skip temporary record
        if (!parent.isRecord() || !parent.getRecord().isTemp()) {
            for (auto fieldDecl : recDecl->fields()) {
                SValue fval(fieldDecl, parent);
                writeToValue(fval, isDefined);
            }
        }
    } else {

        SValue var = state->writeToValue(val, isDefined);

        if (var.isVariable()) {
        //cout << "WR " << var << endl;
            auto i = defVarStmts.emplace(var, unordered_set<Stmt*>({currStmt}));
            if (!i.second) {
                i.first->second.insert(currStmt);
            }

            // Check is this variable is local in the current function
            // Function parameters not considered, so function with internal calls
            // cannot be evaluated as constant
            auto declContext = var.getVariable().getDecl()->getDeclContext();
            bool localVar = isa<clang::FunctionDecl>(declContext) && 
                            funcDecl == declContext;

            // If variable is non-local set side-effect for current function 
            sideEffectFunc = sideEffectFunc || !localVar;

        } else {
            // Any kind of object except variable and temporary considered as non-local
            sideEffectFunc = sideEffectFunc || !var.isTmpVariable();
        }
    }
}

void ScParseExprValue::readFromValue(const SValue& val) 
{
    QualType valType = getDerefType(val.getType());
    bool isRecord = !val.isScChannel() && isUserClass(valType, false);
    //cout << "readFromValue val " << val << " isRecord " << isRecord << endl;
    
    // Read all record fields, do not consider record channel
    // Do not consider record reference 
    // Record/record channel value is also read together with its fields
    if (isRecord) {
        auto recDecl = valType->getAsRecordDecl();
        SCT_TOOL_ASSERT (recDecl, "No record declaration found");
        
        SValue parent = val;
        if (val.isVariable()) state->getValue(val, parent);
        //cout << "  parent " << parent << endl;

        // Skip temporary record
        if (!parent.isRecord() || !parent.getRecord().isTemp()) {
            for (auto fieldDecl : recDecl->fields()) {
                SValue fval(fieldDecl, parent);
                readFromValue(fval);
            }
        }
    } else {

        SValue var = state->readFromValue(val);

        if (var.isVariable()) {
        //cout << "RD " << var << endl;
            auto i = useVarStmts.emplace(var, unordered_set<Stmt*>({currStmt}));
            if (!i.second) {
                i.first->second.insert(currStmt);
            }
        }
    }
}

/// Enter and exit constant evaluating mode 
struct EvalMode {
    bool isDebug;
    bool lastEvaluateConstMode;
    bool& evaluateConstMode;
    EvalMode(bool& mode) : 
        lastEvaluateConstMode(mode), evaluateConstMode(mode) 
    {
        isDebug = DebugOptions::isDebug();
        if (isDebug) DebugOptions::suspend();
        evaluateConstMode = true;
    }
    ~EvalMode() {
        evaluateConstMode = lastEvaluateConstMode;
        if (isDebug && !lastEvaluateConstMode) {
            DebugOptions::resume();
        }
    }
};

// Parse SCT_ASSERT in module scope to fill read value
void ScParseExprValue::parseSvaDecl(const clang::FieldDecl* fdecl) 
{
    // Assertion expression cannot contain function call
    // Suspend debug to ignore parsing expressions done for constant evaluation
    // and prevent user defined function calls
    EvalMode em(evaluateConstMode);

    Expr* expr = removeExprCleanups(fdecl->getInClassInitializer());
    
    std::vector<Expr*> args;
    if (CXXConstructExpr* ctorExpr = dyn_cast<CXXConstructExpr>(expr)) {
        for (auto arg : ctorExpr->arguments()) {
            args.push_back(arg);
        }
    }
    SCT_TOOL_ASSERT (args.size() == 6 || args.size() == 7, 
                     "Incorrect argument number");
    state->setParseSvaArg(true);
    
    // Parse LHS and RHS to fill read from variables, 
    SValue fval = evalSubExpr(args[0]);
    SValue sval = evalSubExpr(args[1]); 
    // Do not parse time argument(s) as its evaluated to integer

    readFromValue(fval);
    readFromValue(sval);
    state->setParseSvaArg(false);
}

// Parse and evaluate one expression/statement as constant integer
std::pair<SValue, SValue>  
ScParseExprValue::evaluateConstInt(SValue val, bool checkConst, bool checkRecOnly)
{
    // Return integer value
    if (val.isInteger()) {
        return make_pair(val, val);
    }
    
    // Do not check @checkConst here as under @checkConst it is evaluated 
    // from constants in state
    if (val.isTmpVariable()) {
        SValue rval = getValueFromState(val, checkRecOnly ? 
                                        ArrayUnkwnMode::amFirstElementRec :
                                        ArrayUnkwnMode::amNoValue);
        return make_pair(val, rval);
    }

    // Check if it is constant variable, not applied for temporary variables
    bool isConst = false;
    if (val.isVariable() || val.isObject()) {
        isConst = isConstOrConstRef(val.getType());
    }
    
    // Get value from constant variable
    if (isConst || !checkConst) {
        SValue rval = getValueFromState(val, checkRecOnly ? 
                                        ArrayUnkwnMode::amFirstElementRec :
                                        ArrayUnkwnMode::amNoValue);
        return make_pair(val, rval);
    }
    
    return make_pair(val, NO_VALUE);
}

std::pair<SValue, SValue>  
ScParseExprValue::evaluateConstInt(Expr* expr, bool checkConst, bool checkRecOnly)
{
    // Suspend debug to ignore parsing expressions done for constant evaluation
    // and prevent user defined function calls
    EvalMode em(evaluateConstMode);
    
    SValue val = evalSubExpr(expr);
    //cout << "evaluateConstInt val " << val << " rval " << getValueFromState(val) << endl;
    
    return evaluateConstInt(val, checkConst, checkRecOnly);
}

// Try to get integer from state, return NO_VALUE if not
SValue ScParseExprValue::evaluateConstInt(clang::Expr* expr, const SValue& val, 
                                          bool checkConst)
{
    //cout << "evaluateConstInt val " << val << " rval " << getValueFromState(val) << endl;
    
    // Return integer value
    if (val.isInteger()) {
        return val;
    }
    
    // Suspend debug to ignore parsing expressions done for constant evaluation
    // and prevent user defined function calls
    EvalMode em(evaluateConstMode);
    
    // Do not check @checkConst here as under @checkConst it is evaluated 
    // from constants in state
    if (val.isTmpVariable()) {
        return getValueFromState(val);
    }

    // Check if it is constant variable, not applied for temporary variables
    bool isConst = false;
    if (val.isVariable() || val.isObject()) {
        isConst = isConstOrConstRef(val.getType());
    }
    
    // Get value from constant variable
    if (isConst || !checkConst) {
        return getValueFromState(val);
    }
    
    return NO_VALUE;                 
}


// Check if @val is integer or any kind of RValue 
bool ScParseExprValue::checkConstRefValue(SValue val)
{
    if (val.isInteger()) return true;
    
    // Suspend debug to ignore parsing expressions done for constant evaluation
    // and prevent user defined function calls
    EvalMode em(evaluateConstMode);
    
    if (val.isTmpVariable()) {
        val = getValueFromState(val);

    } else 
    if (val.isVariable() || val.isObject()) {
        if (isConstOrConstRef(val.getType())) {
            val = getValueFromState(val);
        }
    }
    
    // Return true for @val which is RValue 
    return !(val.isVariable() || val.isTmpVariable() || 
             val.isArray() || val.isSimpleObject());
}

// Parse function call or constructor call parameters
void ScParseExprValue::prepareCallParams(clang::Expr* expr, 
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
    
    // Increase level to create function parameters at the function level
    level += 1;
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
        bool isRecCopy = false;
        // Check for copy constructor to use record RValue
        auto argCtorExpr = getCXXCtorExprArg(argExpr);
        if (argCtorExpr) {
            isRecCopy = true;
            if (!argCtorExpr->getConstructor()->isCopyOrMoveConstructor()) {
                ScDiag::reportScDiag(expr->getBeginLoc(), 
                         ScDiag::SYNTH_NONTRIVIAL_COPY);
            }
        }
        
        ParmVarDecl* parDecl = const_cast<ParmVarDecl*>(
                               callFuncDecl->getParamDecl(paramIndx++));
        QualType type = parDecl->getType();
        if (isScPort(type)) {
            ScDiag::reportScDiag(expr->getBeginLoc(), 
                    ScDiag::SYNTH_SC_PORT_INCORRECT) << "in function parameter";
        }

        // Skip zero width parameter
        if (isZeroWidthType(type) || isZeroWidthArrayType(type)) continue;
        
        bool isRef = sc::isReference(type);
        bool isConstRef = sc::isConstReference(type);
        bool isPtr = type->isPointerType();
        bool isArray = type->isArrayType();
        bool isRecord = isUserClass(getDerefType(type));
        
        // Fill state with parameter and corresponding argument value @arg
        // Parameter variable, can be array variable
        // Use module @funcModval where function is called
        bool lastAssignLHS = assignLHS;
        if (isRef && !isConstRef) assignLHS = true;
        auto parvals = parseValueDecl(parDecl, funcModval, arg, false);
        assignLHS = lastAssignLHS;
        
        SValue pval = parvals.first;
        SValue ipval = (parvals.second.size() == 1) ? 
                        parvals.second.front() : NO_VALUE;
        bool unkwIndex;
        bool isArr = state->isArray(ipval, unkwIndex);
        bool isArrUnkwn = isArr && unkwIndex;
         
        // Constant reference initialized with constant value
        bool isConstRefInit = false;
        // Constant reference initialized with constant array element at unknown index
        bool isConstRefArrUnkw = false;

        if (!isRecord && isConstRef) {
            // Corresponded logic in ScGenerateExpr
            bool A = checkConstRefValue(ipval);
            isConstRefInit = A && !isArrUnkwn;
            isConstRefArrUnkw = A && isArrUnkwn;
            //cout << " ival " << ival << endl;
        }
        
        // If parameter is not pointer or reference, it passed by value
        // Array passed by pointer, reference on argument is used
        if ((!isRef && !isPtr && !isArray) || isConstRefInit || isConstRefArrUnkw) {
            // Put parameter passed by value to @defined
            writeToValue(pval, true);
            
            // Put argument passed by value to @read, apply for constant array 
            // element at unknown index,
            // do not apply for record copy as done in parsing @CXXConstructExpr
            if (ipval && !isRecCopy) {
                readFromValue(ipval);
            }
        }
        
        //cout << "  pval " << pval << " : " << ipval << " isConstRefInit " << isConstRefInit << endl;
    }

    inFuncParams = lastFuncParams;
    level -= 1;
}

//----------------------------------------------------------------------------

// Declared variable may be initialized with expression inside
// Used for variable declaration with and without initialization
// \param initExpr -- record/record field initializer, not used for normal variables
void ScParseExprValue::parseDeclStmt(Stmt* stmt, ValueDecl* decl, SValue& val,
                                     clang::Expr* initExpr)
{
    VarDecl* varDecl = dyn_cast<VarDecl>(decl);
    FieldDecl* fieldDecl = dyn_cast<FieldDecl>(decl);
    SCT_TOOL_ASSERT (varDecl || fieldDecl, "No declaration");

    bool hasInit = (varDecl && varDecl->hasInit()) || 
                   (fieldDecl && fieldDecl->hasInClassInitializer());
    Expr* iexpr = initExpr ? initExpr : 
                  (hasInit ? (varDecl ? varDecl->getInit() : 
                             fieldDecl->getInClassInitializer()) : nullptr);

    const QualType& type = decl->getType();
    bool isRef = type->isReferenceType();
    bool isPtr = type->isPointerType();
    bool isArr = type->isArrayType();
    bool isRec = isUserClass(getDerefType(type));
    bool isRecArr = !isRec && getUserDefinedClassFromArray(type);
    
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
        if (isRec && !isCopyCtor) {
            iexpr = nullptr;
        }
    }
    
    // Use @recval for local record field declarations
    SValue recVal = recval ? recval : modval;
    // Put constructor call expression for record initialization
    // or record field initialization from initialization list
    auto varvals = parseValueDecl(decl, recVal, iexpr, false);
    val = varvals.first;

    if (isRecArr) {
        // No UseDef for record array supported yet, can lead only to registers
        // declared if read after declaration
    } else {
        if (isArr) {
            // All array element are defined
            SValue aval; state->getValue(val, aval);
            SCT_TOOL_ASSERT (aval.isArray(), "No array value in parseDeclStmt");

            auto elmnum = getArrayElementNumber(type);
            for (unsigned i = 0; i < elmnum; i++) {
                // Get @i-th element in multi-dimensional array @aval
                SValue eval = getArrElmByIndx(type, i, aval);

                // Set defined/declared for elements to avoid registers
                if (iexpr) 
                    state->writeToArrElmValue(eval);
                else 
                    state->declareValue(eval);
            }
        }
        // Only initialized variable is defined, SC data type has zero initializer
        // Declare variable to avoid register creation and remove declaration 
        // Declare variable/array variable to remove declaration if not used 
        // Do not define or declare reference/pointer
        if (!isRef && !isPtr) {
            if (iexpr) {
                writeToValue(val, true); 
            } else {
                declareValue(val);
            }
        }
    }
    
    if (isRec && locrecvar) {
        // Artificial copy constructor required for function which 
        // returns record to avoid RVO in C++17
        if (evaluateConstMode) {
            // Do nothing
        } else {
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
            SValue rrec = rval; 
            if (!rval.isRecord()) state->getValue(rval, rrec);

            // Copy values of record fields and put it to record variable
            copyRecFieldValues(lrec, rrec);
            state->putValue(locrecvar, lrec, false, false);

            // Add defined/read for record fields
            readFromValue(rrec);
            writeToValue(lrec, true); 
        }
        
        locrecvar = NO_VALUE;
        
    } else
    if (iexpr && !isRec && !isRecArr && !isRef && !isPtr) {
        // If declaration has initializer add it as read to @state
        // Do not need to consider constant reference
        for (SValue ival : varvals.second) {
            readFromValue(ival);
        }
    }

    // No value for declaration statement
    val = NO_VALUE;
}

// Used for local variables usage like assignment statement in left/right parts
void ScParseExprValue::parseExpr(clang::DeclRefExpr* expr, SValue& val)
{
    using namespace llvm;
    auto* decl = expr->getDecl();

    ScParseExpr::parseExpr(expr, val);

    // Initialization of global and static variables
    if (!isa<clang::EnumConstantDecl>(decl)) {
        if (isa<clang::RecordDecl>(decl->getDeclContext())) {
            // Class data members
            if (auto* varDecl = dyn_cast<clang::VarDecl>(decl)) {
                if (varDecl->isStaticDataMember()) {
                    // Static data member
                    if (val.getType().isConstQualified()) {
                        parseGlobalConstant(val);
                    }
                }
            }
        } else 
        if (!isa<clang::FunctionDecl>(decl->getDeclContext())) {
            // Global variable
            if (val.getType().isConstQualified()) {
                parseGlobalConstant(val);
            }
        }
    }
}

// Any access of member variable
void ScParseExprValue::parseExpr(clang::MemberExpr* expr, SValue& val)
{
    ValueDecl* decl = expr->getMemberDecl();
    ScParseExpr::parseExpr(expr, val);

    // Initialization of static constant variables
    if (!isa<clang::EnumConstantDecl>(decl)) {
        if (isa<clang::RecordDecl>(decl->getDeclContext())) {
            // Class data members
            if (auto* varDecl = dyn_cast<clang::VarDecl>(decl)) {
                if (varDecl->isStaticDataMember()) {
                    // Static data member
                    if (val.getType().isConstQualified()) {
                        parseGlobalConstant(val);
                    }
                }
            }
        }
    } 
}

// Used for default initializer in constructor or in aggregate initialization
void ScParseExprValue::parseExpr(CXXDefaultInitExpr* expr, SValue& val)
{
    FieldDecl* fdecl = expr->getField();
    QualType type  = expr->getType();
    Expr* iexpr = expr->getExpr();
    // Variable or Array variable (i.e. pointer to array)
    val = SValue(fdecl, modval);
    //state->setValueLevel(val, level); //TODO: check me 

    if (type->isArrayType()) {
        // Create stack array, VarDecl parsed in SValue constructor
        QualType elmType = cast<clang::ArrayType>(type)->getElementType();
        size_t size = getArraySize(fdecl);
        // Create array object
        SValue aval(elmType, size, 0);
        // Both @val and @aval are not references
        state->putValue(val, aval, false); // @val -> @aval

        // Prefill array with zeros
        SValue ival = SValue(APSInt(64, elmType->isUnsignedIntegerType()), 10);
        for (unsigned i = 0; i < size; i++) {
            aval.getArray().setOffset(i);
            // Both @aval and @ival are not references
            state->putValue(aval, ival, false);
            //state->setValueLevel(aval, level); //TODO: check me 
        }

        if (auto init = dyn_cast<InitListExpr>(iexpr)) {
            for (unsigned i = 0; i < init->getNumInits(); i++) {
                SValue ival = evalSubExpr(init->getInit(i));
                aval.getArray().setOffset(i);
                assignValueInState(aval, ival);
            }
        } else {
            string s = iexpr->getStmtClassName();
            ScDiag::reportScDiag(expr->getBeginLoc(), 
                                 ScDiag::SYNTH_UNSUPPORTED_INIT) << s;
        }    
    } else {
        // Single variable
        // Parse initializer expression
        SValue ival = evalSubExpr(iexpr);
        assignValueInState(val, ival);
    }
}

// Used for default initializer in constructor or aggregate initialization, T{}
void ScParseExprValue::parseExpr(InitListExpr* expr, SValue& val)
{
    QualType type = expr->getType();
    
    if (type->isArrayType()) {
        // Array initialization list
        if (expr->getNumInits() > 0) {
            QualType elmType = cast<clang::ArrayType>(type)->getElementType();
            val = SValue(elmType);
            //state->setValueLevel(val, level+1); //TODO: check me, +1 ???  
            
            size_t size = expr->getNumInits();
            SValue aval(elmType, size, 0);    // Array object
            // Both @val and @aval are not references
            state->putValue(val, aval, false);

            // Array initialization list
            for (unsigned i = 0; i < size; i++) {
                SValue ival = evalSubExpr(expr->getInit(i));
                readFromValue(ival);
                
                aval.getArray().setOffset(i);
                assignValueInState(aval, ival);
                //state->setValueLevel(aval, level); //TODO: check me 
            }
        } else 
        if (auto arrayFiller = expr->getArrayFiller()) {
            // Multidimensional array initializer with empty brackets
            val = evalSubExpr(arrayFiller);
            
        } else {
            ScDiag::reportScDiag(expr->getBeginLoc(), 
                                 ScDiag::SYNTH_UNSUPPORTED_INIT) << " no values";
        }    
    } else {
        if (isUserClass(type)) {
            if (auto recDecl = type->getAsCXXRecordDecl()) {
                if (locrecvar) {
                    ScDiag::reportScDiag(expr->getBeginLoc(), 
                                         ScDiag::SYNTH_RECORD_INIT_LIST);
                }
                SValue var(type, NO_VALUE);
                val = createRecValue(recDecl, NO_VALUE, var, true, 0, false);
            } else {
                SCT_INTERNAL_ERROR(expr->getBeginLoc(), "Incorrect class type");
            }
        } else
        if (expr->getNumInits() == 0) {
            val = SValue(SValue::zeroValue(type));
        } else
        if (expr->getNumInits() == 1) {
            val = evalSubExpr(expr->getInit(0));
        } else {
            ScDiag::reportScDiag(expr->getBeginLoc(), 
                ScDiag::SYNTH_UNSUPPORTED_INIT) << " unexpected number of values";
        }
    }
}

// Used for construction temporary record object, T()
void ScParseExprValue::parseExpr(CXXTemporaryObjectExpr* expr, SValue& val)
{
    QualType type = expr->getType();
    
    if (isUserClass(type)) {
        if (auto recDecl = type->getAsCXXRecordDecl()) {
            if (locrecvar) {
                ScDiag::reportScDiag(expr->getBeginLoc(), 
                             ScDiag::SYNTH_RECORD_INIT_LIST);
            }
            SValue var(type, NO_VALUE);
            val = createRecValue(recDecl, NO_VALUE, var, true, 0, false);
        } else {
            SCT_INTERNAL_ERROR(expr->getBeginLoc(), "Incorrect class type");
        }
    } else {
        parseExpr((CXXConstructExpr*)expr, val);
    }
}


// CXX constructor, including @sc_in/@sc_out/@sc_signal/@sc_uint/...
void ScParseExprValue::parseExpr(CXXConstructExpr* expr, SValue& val)
{
    QualType type = expr->getType();
    auto ctorDecl = expr->getConstructor();
    bool isCopyCtor = ctorDecl->isCopyConstructor();
    bool isMoveCtor = ctorDecl->isMoveConstructor();
    bool isZeroWidth = isZeroWidthType(type);
    auto nsname = getNamespaceAsStr(ctorDecl);
    
    if (isZeroWidth) {
        val = ZW_VALUE;
                
    } else
    if (nsname && (*nsname == "sc_dt" || *nsname == "sc_core")) {
        if (isScChannel(type, false)) {
            // Channels considered unknown during expression evaluation
            val = NO_VALUE;
            
        } else 
        if (isAnyScInteger(type)) {
            // SC integer constructor
            SValue initVal;

            if (expr->getNumArgs() == 0) {
                initVal = SValue::zeroValue(expr);
                
            } else 
            if (expr->getNumArgs() == 1) {
                auto argExpr = expr->getArg(0);
                
                if (isAnyIntegerRef(argExpr->getType())) {
                    // Parse constructor argument
                    SValue rval = evalSubExpr(argExpr);
                    readFromValue(rval);

                    // Try to get integer value
                    SValue rrval = getValueFromState(rval);
                    //cout << "   rrval " << rrval << endl;

                    // Adjust SC type width and sign for integer value
                    if (rrval.isInteger()) {
                        if (isa<AutoType>(type)) {
                            initVal = rval;
                            
                        } else {
                            size_t width = 64; 
                            bool isUnsigned = true;

                            if (auto typeInfo = getIntTraits(
                                                getTypeForWidth(expr), true)) {
                                width = typeInfo.getValue().first;
                                isUnsigned = typeInfo.getValue().second;

                            } else {
                                ScDiag::reportScDiag(expr->getBeginLoc(),
                                                     ScDiag::SYNTH_UNKNOWN_TYPE_WIDTH) << 
                                                     type.getAsString();
                            }

                            // This detects sc_uint<0> 
                            if (width != 0) {
                                initVal = SValue(extrOrTrunc(rrval.getInteger(),
                                                 width, isUnsigned), rrval.getRadix());
                            } else {
                                ScDiag::reportScDiag(expr->getBeginLoc(), 
                                    ScDiag::SYNTH_ZERO_TYPE_WIDTH) << "";
                            }
                        }
                    } else {
                        initVal = rval;
                    }
                } else 
                if (isConstCharPtr(argExpr->getType())) {
                    // Parse expression with string literal inside
                    unsigned lastWidth = strLiterWidth;
                    bool lastUnsigned = strLiterUnsigned;
                    if (auto typeInfo = getIntTraits(getTypeForWidth(expr), true)) {
                        strLiterWidth = typeInfo.getValue().first;
                        strLiterUnsigned = typeInfo.getValue().second;
                    } else {
                        strLiterWidth = 0;  // Provides no string literal parsing
                    }
                    
                    SValue rrval = evalSubExpr(argExpr);

                    strLiterWidth = lastWidth;
                    strLiterUnsigned = lastUnsigned;

                    if (rrval.isInteger()) {
                        initVal = rrval;
                    } else {
                        ScDiag::reportScDiag(argExpr->getBeginLoc(), 
                                             ScDiag::CPP_NOT_NUMBER_LITER);
                    }
                } else {
                    ScDiag::reportScDiag(argExpr->getBeginLoc(),
                                         ScDiag::SYNTH_TYPE_NOT_SUPPORTED)
                                         << expr->getType();
                }
            } else {
                SCT_INTERNAL_ERROR (expr->getBeginLoc(), "Unexpected argument number");
            }

            // Create SC type single object value
            val = SValue(type);
            assignValueInState(val, initVal);
            state->setValueLevel(val, level+1);
            
        } else 
        if (isScIntegerArray(type, false)) {
            // Local array of SC type, no in-place initialization supported
            // Do nothing
            val = NO_VALUE;
            
        } else {
            val = NO_VALUE;
            ScDiag::reportScDiag(expr->getBeginLoc(),
                                 ScDiag::SYNTH_TYPE_NOT_SUPPORTED) << 
                                 expr->getType();
        }

    } else 
    if (isUserClass(type)) {
        // User defined class or structure
        if (isScModuleOrInterface(type)) {
            ScDiag::reportScDiag(expr->getBeginLoc(), 
                                 ScDiag::SYNTH_LOCAL_MODULE_DECL);
        }
        if (locrecvar) {
            if (evaluateConstMode) {
                // No user-define function call in constant evaluation mode
                // Prevent construction body analysis in @parseRecordCtor() 
                val = NO_VALUE;
                
            } else 
            if (isCopyCtor || isMoveCtor) {
                // Copy constructor to pass record parameter by value
                if (!expr->getArg(0)) {
                    SCT_INTERNAL_ERROR (expr->getBeginLoc(), 
                                        "No argument in copy constructor");
                }

                // Created record value 
                SValue lrec = createRecValue(type->getAsCXXRecordDecl(), 
                                             modval, locrecvar, true, 0, false);
                
                // Get record from constructor argument
                SValue rval = evalSubExpr(expr->getArg(0));
                SValue rrec = rval; 
                if (!rval.isRecord()) state->getValue(rval, rrec);

                // Copy values of record fields or clear if @rrec is unknown
                if (rrec.isRecord()) {
                    copyRecFieldValues(lrec, rrec);
                } else {
                    // Record channel and others
                    state->removeIntSubValues(lrec, false);
                }

                // Use array element with unknown index instead of record
                bool unkwIndex;
                bool isArr = state->isArray(rval, unkwIndex);
                if (isArr && unkwIndex) {
                    rrec = rval;
                }
                
                // Add defined/read for record fields
                readFromValue(rrec);
                writeToValue(lrec, true); 
                val = lrec;
                
            } else {
                // Normal constructor of local record
                // Return created record to assign to its variable
                val = parseRecordCtor(expr, modval, locrecvar, true);
                // Add defined for record fields
                auto recDecl = val.getType()->getAsCXXRecordDecl();
                for (auto fieldDecl : recDecl->fields()) {
                    SValue fval(fieldDecl, val);
                    writeToValue(fval);
                }
                // No set level as record is returned
            }
            locrecvar = NO_VALUE;
        }
        isUserCallStmt = true;
        isRequiredStmt = true;
        
    } else 
    if (getUserDefinedClassFromArray(type)) {
        // Do nothing, record array filled in ScParseExpr::parseValueDecl()
        isUserCallStmt = true;
        isRequiredStmt = true;
        
    } else {
        SCT_INTERNAL_FATAL(expr->getBeginLoc(), 
                         "Unexpected type in CXX constructor : "+
                         type.getAsString()); 
    }
}

// Operator @new and @new[]
void ScParseExprValue::parseExpr(CXXNewExpr* expr, SValue& val)
{
    val = NO_VALUE;
    ScDiag::reportScDiag(expr->getBeginLoc(), ScDiag::SC_NEW_IN_PROC_FUNC);
}

// Operator delete
void ScParseExprValue::parseExpr(CXXDeleteExpr* expr, SValue& val)
{
    val = NO_VALUE;
    ScDiag::reportScDiag(expr->getBeginLoc(), ScDiag::SC_DELETE_IN_PROC_FUNC);
}

// Common function for operator[] in @ArraySubscriptExpr and @CXXOperatorCall
SValue ScParseExprValue::parseArraySubscript(Expr* expr, const SValue& bval, 
                                             const SValue& ival)
{
    // Result value
    SValue val = NO_VALUE;
    
    // Get referenced variable for array (reference to array: T (&a)[N])
    SValue bbval; // Array variable value
    state->getDerefVariable(bval, bbval);

    if (isScZeroWidth(bbval)) {
        return ZW_VALUE; 
    }

    // Get index value
    SValue iival = getValueFromState(ival);
    //cout << "operator [] : " << bbval << "[" << ival << "(" << iival << ")]" << endl;

    // Check @iival is integer, may be NO_VALUE
    if (bbval.isVariable() || bbval.isTmpVariable() || bbval.isArray()) {
        // Get array from variable
        SValue eval;
        // Keep array unknown element
        eval = getValueFromState(bbval, ArrayUnkwnMode::amArrayUnknown);
        //cout << "   eval " << eval << " var " << eval.isVariable() << endl;

        if (iival.isInteger()) {
            // Integer index, increase offset
            size_t indxOffset = iival.getInteger().getExtValue();

            if (eval.isArray()) {
                // Normal array in state
                if (!eval.getArray().isUnknown()) {
                    size_t arrOffset = eval.getArray().getOffset() + indxOffset;
                    if (arrOffset < eval.getArray().getSize()) {
                        eval.getArray().setOffset(arrOffset);
                    } else {
                        ScDiag::reportScDiag(expr->getBeginLoc(), 
                                             ScDiag::CPP_ARRAY_OUT_OF_BOUND);
                        cout << "setOffset offset " << arrOffset << ", size " 
                             << eval.getArray().getSize() << endl;
                    }
                }
                val = eval;
                //cout << "Offset " << eval.getArray().getOffset() << "+" << indxOffset << endl;
            } else 
            if (eval.isVariable()) {
                // Variable returned for array at unknown index, 
                // it returned to provide correct UseDef
                val = eval;

            } else {
                // Array in template or other constant array not in state
                // Try to get value from AST
                if (!getConstASTArrElement(astCtx, bbval, indxOffset, val)) {
                    if (checkNoValue) {
                        SCT_INTERNAL_FATAL(expr->getBeginLoc(), 
                                           "No constant value for array element");
                    }
                }
            }
        } else 
        if (iival.isUnknown()) {
            // Unknown index, set unknown offset to element
            if (eval.isArray()) {
                eval.getArray().setUnknownOffset();
                val = eval;
                //cout << "Offset UNKWN_OFFSET " << endl;
            } else 
            if (eval.isVariable()) {
                // Variable returned for array at unknown index, 
                // it returned to provide correct UseDef
                val = eval;
            }
        } else {
            cout << "parseArraySubscript, iival " << iival << endl; 
            SCT_INTERNAL_ERROR(expr->getBeginLoc(), 
                               "Incorrect array index value");
        }
    }
    //cout << "   final val " << val << endl;
    return val;
}

// Array index access operator []
void ScParseExprValue::parseExpr(ArraySubscriptExpr* expr, SValue& val)
{
    SValue bval = evalSubExpr(expr->getBase()); // Array variable
    bool lastAssignLHS = assignLHS; assignLHS = false;
    SValue ival = evalSubExpr(expr->getIdx());  // Index value
    assignLHS = lastAssignLHS;
    readFromValue(ival);
    
    val = parseArraySubscript(expr, bval, ival);
}
    
// Used for explicit/implicit type cast and LValue to RValue cast
void ScParseExprValue::parseImplExplCast(CastExpr* expr, SValue& rval, 
                                         SValue& val)
{
    //cout << "parseImplExplCast val " << val << endl;
    auto castKind = expr->getCastKind();
 
    if (castKind == CK_PointerToIntegral) {
        SCT_INTERNAL_FATAL (expr->getBeginLoc(),
                          "PointerToIntegral no supported yet");
    }
    
    // Checking cast pointer to boolean and substitute literal if possible
    if (castKind == CK_MemberPointerToBoolean || castKind == CK_PointerToBoolean) 
    {
        //ScParseExpr::chooseExprMethod(expr->getSubExpr(), rval);
        SValue rrval = getValueFromState(rval, ArrayUnkwnMode::amNoValue);
        //cout << " rval " << rval << "rrval " << rrval << endl;
        
        // Check for null and dangling pointer
        SValue rvalzero = state->getFirstArrayElementForAny(
                                rval, ScState::MIF_CROSS_NUM);
        SValue valzero = getValueFromState(
                                rvalzero, ArrayUnkwnMode::amArrayUnknown);
        //cout << "rvalzero " << rvalzero << " valzero " << valzero << endl;
        
        if (valzero.isUnknown()) {
            SValue rvar = state->getVariableForValue(rval);
            ScDiag::reportScDiag(expr->getBeginLoc(),
                                 ScDiag::CPP_DANGLING_PTR_CAST) << 
                                 rvar.asString(rvar.isObject());
        }
        //cout << "Cast rvalzero " << rvalzero << " valzero " << valzero << endl;

        if (rrval.isObject()) {
            // Pointer to object is @true
            val = SValue(SValue::boolToAPSInt(true), 10);
            
        } else {
            SValue irval = evaluateConstInt(expr->getSubExpr(), rval, false);
            if (irval.isInteger()) {
                // Convert pointer value to @false or @true
                val = SValue(SValue::boolToAPSInt(irval.getBoolValue()), 10);
            }
        }
    } else {
        if (castKind == CK_IntegralToBoolean || castKind == CK_IntegralCast) {
            if (val.isVariable()) {
                // Read value if real type casting done, can be in RHS only
                readFromValue(val);

                // Replace variable with its value
                val = getValueFromState(val);
            }
        }

        if (castKind == CK_BooleanToSignedIntegral)
        {
            SCT_INTERNAL_FATAL (expr->getBeginLoc(),
                              "BooleanToSignedIntegral no supported yet");
        }

        if (castKind == CK_IntegralToFloating || 
            castKind == CK_FloatingToIntegral ||
            castKind == CK_FloatingToBoolean || castKind == CK_FloatingCast)
        {
            SCT_INTERNAL_FATAL (expr->getBeginLoc(),
                              "Float type cast no supported yet");
        }
        
        if (castKind == CK_IntegralToBoolean) {
            if (val.isInteger()) {
                //cout << "IntegralToBoolean implicit cast" << endl;
                //SValue::boolToAPSInt(val.getBoolValue()).dump();
                val = SValue(SValue::boolToAPSInt(val.getBoolValue()), 10);
            }
        } else 
        if (castKind == CK_IntegralCast) {
            if (val.isInteger()) {
                QualType type = expr->getType();
                auto typeInfo = getIntTraits(type, true);
                if (!typeInfo) {
                    SCT_INTERNAL_ERROR (expr->getBeginLoc(), 
                                        "No integral type width extracted");
                }
                size_t width = typeInfo.getValue().first;
                bool isUnsigned = typeInfo.getValue().second;

                //cout << "IntegralCast implicit cast, isUnsigned " << isUnsigned 
                //     << ", width " << width << ", val " << val << endl;
                
                // Align integer value width
                val = SValue(extrOrTrunc(val.getInteger(), width, isUnsigned), 
                             val.getRadix());
                //cout << "Updated val " << val << endl;
            }
        }
    }
}

void ScParseExprValue::parseExpr(ImplicitCastExpr* expr, SValue& rval, SValue& val)
{
    ScParseExpr::parseExpr(expr, rval, val);
    parseImplExplCast(expr, rval, val);
}

// Reduce value width in explicit type cast
void ScParseExprValue::parseExpr(ExplicitCastExpr* expr, SValue& rval, SValue& val)
{
    ScParseExpr::parseExpr(expr, rval, val);
    
    if (isa<CStyleCastExpr>(expr) || isa<CXXFunctionalCastExpr>(expr) || 
        isa<CXXStaticCastExpr>(expr)) {
        // Use underlying ImplicitCast which is part of this cast and 
        // contains the explicit cast kind, type, and others 
        if (auto implCastExpr = dyn_cast<ImplicitCastExpr>(expr->getSubExpr())) {
            parseImplExplCast(implCastExpr, rval, val);
            
        } else {
            // No underlying implicit cast
            auto castKind = expr->getCastKind();
            if (castKind == CK_ConstructorConversion || castKind == CK_ToVoid ||
                castKind == CK_NoOp || castKind == CK_IntegralCast) {
                // No cast required
            } else 
            if (castKind == CK_LValueBitCast) {
                ScDiag::reportScDiag(expr->getBeginLoc(), 
                                     ScDiag::SYNTH_LVALUE_BIT_CAST);
            } else {
                cout << "Cast kind " << expr->getCastKindName(castKind) << endl;
                SCT_INTERNAL_ERROR (expr->getBeginLoc(), 
                                    "Unsupported cast kind in ExplicitCastExpr");
            }
        }
    } else 
    if (isa<CXXConstCastExpr>(expr)) {
        // No @const_cast in assignment LHS
        if (assignLHS) {
            ScDiag::reportScDiag(expr->getBeginLoc(), ScDiag::SYNTH_CONST_CAST);
        }
    }
}

// General binary statements including assignment
// Parse statement and run evalSubExpr for each operand
void ScParseExprValue::parseBinaryStmt(BinaryOperator* stmt, SValue& val)
{
    // Operation code
    BinaryOperatorKind opcode = stmt->getOpcode();
    string opcodeStr = stmt->getOpcodeStr().str();

    Expr* lexpr = stmt->getLHS();
    Expr* rexpr = stmt->getRHS();
    auto linfo = getIntTraits(lexpr->getType());
    auto rinfo = getIntTraits(rexpr->getType());
    // Operations with pointers have no integer traits
    size_t ltypeWidth = linfo ? linfo->first : 0;
    size_t rtypeWidth = rinfo ? rinfo->first : 0;
    
    if (isConstCharPtr(lexpr->getType())) {
        ScDiag::reportScDiag(lexpr->getBeginLoc(), ScDiag::CPP_STRING_BINARY_OPER);
    }
    if (isConstCharPtr(rexpr->getType())) {
        ScDiag::reportScDiag(rexpr->getBeginLoc(), ScDiag::CPP_STRING_BINARY_OPER);
    }
    
    // Parse left part
    SValue lval;
    if (opcode == BO_LOr || opcode == BO_LAnd) {
        auto i = condStoredValue.find(stmt);
        if (i != condStoredValue.end()) {
            lval = i->second;
        } else {
            // This option required for @ScParseExprValue instance usage 
            lval = evalSubExpr(lexpr);
        }
    } else {
        bool lastAssignLHS = assignLHS;
        if (opcode == BO_Assign) assignLHS = true;
        lval = evalSubExpr(lexpr);
        assignLHS = lastAssignLHS;
    }
    
    SValue llval = getValueFromState(lval);
    SValue tmp;
    state->getDerefVariable(llval, tmp); llval = tmp;

    // Right part not parsed in ||/&& which known from left part    
    // excluding parsing of assertion expression arguments
    bool parseRight = true;
    if (!state->getParseSvaArg() && (opcode == BO_LOr || opcode == BO_LAnd)) {
        if (!llval.isUnknown()) {
            if (opcode == BO_LOr && llval.getBoolValue()) { 
                parseRight = false;
            } else 
            if (opcode == BO_LAnd && !llval.getBoolValue()) {
                parseRight = false;
            }
        }
    }
    //cout << "Binary operator lval " << lval << ", llval " << llval << endl;
    //cout << "parseRight " << parseRight << endl;
    
    // Parse right part if required
    SValue rval; SValue rrval;
    if (parseRight) {
        rval = evalSubExpr(rexpr);
        rrval = getValueFromState(rval);
    }
    state->getDerefVariable(rrval, tmp); rrval = tmp;
    
    //cout << "Binary operator lval " << lval << ", rval " << rval << endl;
    
    // Add defined/read to state
    readFromValue(rval);
    if (opcode == BO_Assign) {
        writeToValue(lval);
    } else {
        readFromValue(lval);
    }
    
    SValue obj1 = (llval.isVariable() || llval.isTmpVariable() || 
                    llval.isObject()) ? llval : 
                  ((rrval.isVariable() || rrval.isTmpVariable() || 
                    rrval.isObject()) ? rrval : NO_VALUE);
    SValue obj2 = ((llval.isVariable() || llval.isTmpVariable() || 
                    llval.isObject()) && 
                   (rrval.isVariable() || rrval.isTmpVariable() || 
                    rrval.isObject())) ? rrval : NO_VALUE;
    SValue int1 = (llval.isInteger()) ? llval : 
                   ((rrval.isInteger()) ? rrval : NO_VALUE);
    SValue int2 = (llval.isInteger() && rrval.isInteger()) ? 
                   rrval : NO_VALUE;

    // Calculate result @SValue and return it
    if (opcode == BO_Assign) {
        if (!lval.isUnknown()) {
            // Do not keep array unknown element as pointer cannot be assigned
            assignValueInState(lval, rval);
        }
        val = lval;

        isAssignStmt   = true;
        isRequiredStmt = (sc::isScChannel(lexpr->getType()) || 
                          sc::isScChannelArray(lexpr->getType())) ? 
                          true : isRequiredStmt;
        
    } else 
    if (opcode == BO_Add || opcode == BO_Sub) 
    {
        // Do not apply to array unknown element
        if (obj1.isArray() && !obj1.getArray().isUnknown() && int1.isInteger()) {
            // Pointer arithmetic
            int64_t shiftIndx = (opcode == BO_Add) ?
                (obj1.getArray().getOffset()+int1.getInteger().getExtValue()) :
                (obj1.getArray().getOffset()-int1.getInteger().getExtValue());

            // Get shifted object
            if (shiftIndx >= 0 && (size_t)shiftIndx < obj1.getArray().getSize()) {
                obj1.getArray().setOffset(shiftIndx);
            } else {
                obj1 = NO_VALUE;
                ScDiag::reportScDiag(stmt->getBeginLoc(), 
                                     ScDiag::CPP_ARRAY_OUT_OF_BOUND);
                cout << "setOffset offset " << shiftIndx << ", size " 
                     << obj1.getArray().getSize() << endl;
            }
            // Return value
            QualType type = (obj1.isObject()) ? obj1.getObjectPtr()->getType() : 
                                                QualType();
            val = SValue(type);
            state->putValue(val, obj1);
            state->setValueLevel(val, level+1);

        } else 
        if (int1.isInteger() && int2.isInteger()) {
            // Integer arithmetic
            APSInt val1 = int1.getInteger(); 
            APSInt val2 = int2.getInteger();

            SCT_TOOL_ASSERT (ltypeWidth && rtypeWidth, 
                             "No integer type traits in BinaryOperator");

            // Extend value width before operation to fit result value
            extendBitWidthBO(val1, val2, ltypeWidth, rtypeWidth);
            // Required by APInt operators
            adjustIntegers(val1, val2, val1, val2);
            //cout << "After adjust, val1 " << sc::APSintToString(val1, 16) << " val2 " << sc::APSintToString(val2, 16) << endl;
            //cout << "     itwidth, val1 " << val1.getBitWidth() << " val2 " << val2.getBitWidth() << endl;

            APSInt res;
            if (opcode == BO_Add) {
                res = val1 + val2;
            } else {
                res = val1 - val2;
            }
            char radix = int1.isInteger() ? int1.getRadix() : 10;
            val = SValue(res, radix);
            //cout << "res " << sc::APSintToString(res, 10) << " val " << val << endl;

        } else {
            // No value for one of arguments, do nothing
            val = NO_VALUE;
        }

    } else
    if (opcode == BO_Mul || opcode == BO_Div || opcode == BO_Rem || 
        opcode == BO_Shl || opcode == BO_Shr || opcode == BO_Comma ||
        opcode == BO_And || opcode == BO_Or  || opcode == BO_Xor) 
    {
        if (int1 != NO_VALUE && int2 != NO_VALUE) {
            // Adjust value widths and signs
            APSInt val1 = int1.getInteger(); 
            APSInt val2 = int2.getInteger();
            //cout << "Init int1 " << int1 << ", int2 " << int2 << endl;
            
            SCT_TOOL_ASSERT (ltypeWidth && rtypeWidth, 
                             "No integer type traits in BinaryOperator");
            
            if (opcode == BO_Comma) {
                // No adjustment required
            } else 
            if (opcode == BO_Shl || opcode == BO_Shr) {
                // Extend value width before operation to fit result value
                extendBitWidthBO(val1, val2, ltypeWidth, rtypeWidth);
            } else {
                // Extend value width before operation to fit result value
                extendBitWidthBO(val1, val2, ltypeWidth, rtypeWidth);
                // Width and sign adjustment, required by APSInt operators
                adjustIntegers(val1, val2, val1, val2);
            }
            
            // Extend both values to specified precision @EPRECISION
            APSInt res;
            if (opcode == BO_Mul) {
                res = val1 * val2;
            } else 
            if (opcode == BO_Div) {
                res = val1 / val2;
            } else 
            if (opcode == BO_Rem) {
                res = val1 % val2;
            } else 
            if (opcode == BO_Shl) {
                res = val1 << val2.getExtValue();
            } else 
            if (opcode == BO_Shr) {
                res = val1 >> val2.getExtValue();
            } else 
            if (opcode == BO_Comma) {
                res = val2;
            } else 
            if (opcode == BO_And) {
                res = val1 & val2;
            } else 
            if (opcode == BO_Or) {
                res = val1 | val2;
            } else 
            if (opcode == BO_Xor) {
                res = val1 ^ val2;
            } else {
                SCT_INTERNAL_FATAL(stmt->getBeginLoc(), 
                                 "Unsupported statement "+opcodeStr);
            }
            char radix = int1.isInteger() ? int1.getRadix() : 10;
            val = SValue(res, radix);
            //cout << "res " << sc::APSintToString(res, 16) << endl; 
            
        } else 
        if (obj1 != NO_VALUE) {
            SCT_INTERNAL_FATAL(stmt->getBeginLoc(), 
                             "Unsupported statement for object "+opcodeStr);
        } else {
            // No value for one of arguments, do nothing
            val = NO_VALUE;
        }
        
    } else     
    if (opcode == BO_EQ || opcode == BO_NE) 
    {
        if (int1 != NO_VALUE && int2 != NO_VALUE)
        {
            // Integer arithmetic 
            APSInt val1; APSInt val2;
            // Required by APInt operators
            adjustIntegers(int1.getInteger(), int2.getInteger(), val1, val2);
            
            bool b = false;
            if (opcode == BO_EQ) {
                b = (val1 == val2);
            }  else 
            if (opcode == BO_NE) {
                b = (val1 != val2);
            }
            // Return value
            val = SValue(SValue::boolToAPSInt(b), 10);

        } else
        if (obj1 != NO_VALUE && obj2 != NO_VALUE)
        {
            // Pointer arithmetic
            bool b = false;
            if (opcode == BO_EQ) {
                b = (obj1 == obj2);
            }  else 
            if (opcode == BO_NE) {
                b = (obj1 != obj2);
            }
            // Return value
            val = SValue(SValue::boolToAPSInt(b), 10);

        } else
        if (obj1 != NO_VALUE && int1 != NO_VALUE) {
            // Pointer and integer, for example (p == nullptr)
            // Address stored in integer variable is not supported, so
            // pointer to object never equals to integer literal
            // Pointer to NULL has integer value 0, so integers comparison used 
            
            bool b = false;
            if (opcode == BO_EQ) {
                b = false;
            }  else 
            if (opcode == BO_NE) {
                b = true;
            } else {
                SCT_INTERNAL_FATAL(stmt->getBeginLoc(), 
                                 "Unsupported statement "+opcodeStr);
            }
            // Return value
            val = SValue(SValue::boolToAPSInt(b), 10);

        } else 
        if (int1 != NO_VALUE && int2 == NO_VALUE) {
            // Check variable is not equal to integer constant because of width
            val = NO_VALUE;
            SValue uval = (llval.isInteger()) ? rval : lval;
            
            if (auto typeInfo = getIntTraits(uval.getType())) {
                size_t varWidth = typeInfo->first;
                bool isUnsigned = typeInfo->second;
                APSInt maxVal = APSInt::getMaxValue(varWidth, isUnsigned);
                APSInt minVal = APSInt::getMinValue(varWidth, isUnsigned);
                APSInt literVal = int1.getInteger(); 
                                
                // Required by APInt comparison
                adjustIntegers(literVal, maxVal, literVal, maxVal);
                adjustIntegers(literVal, minVal, literVal, minVal);
                
//                cout << "varWidth " << varWidth << " isUnsigned " << isUnsigned
//                     << " literVal " << sc::APSintToString(literVal, 10) << " maxVal " << sc::APSintToString(maxVal, 10)
//                     << " minVal " << sc::APSintToString(minVal, 10) << endl;
                     
                if (literVal > maxVal || literVal < minVal) {
                    val = SValue(SValue::boolToAPSInt(opcode == BO_NE), 10);
                }
            }
            
        } else {
            val = NO_VALUE;
            if (checkNoValue) {
                SCT_INTERNAL_FATAL(stmt->getBeginLoc(), 
                                 "No values in binary statement "+opcodeStr);
            }
        }
    } else 
    if (opcode == BO_GT || opcode == BO_LT || opcode == BO_GE || opcode == BO_LE) 
    {
        if (int1.isInteger() && int2.isInteger()) {
            // Integer arithmetic
            APSInt val1; APSInt val2;
            // Required by APInt operators
            adjustIntegers(int1.getInteger(), int2.getInteger(), val1, val2);

            bool b = false;
            if (opcode == BO_GT) {
                b = val1 > val2;
            } else 
            if (opcode == BO_GE) {
                b = val1 >= val2;
            } else 
            if (opcode == BO_LT) {
                b = val1 < val2;
            } else 
            if (opcode == BO_LE) {
                b = val1 <= val2;
            } else {
                SCT_INTERNAL_FATAL(stmt->getBeginLoc(), 
                                 "Unsupported statement "+opcodeStr);
            }
            // Return value
            val = SValue(SValue::boolToAPSInt(b), 10);

        } else
        if (obj1.isArray() && !obj1.getArray().isUnknown() &&
            obj2.isArray() && !obj2.getArray().isUnknown()) { 
            // Pointer comparison
            bool b = false;

            if (obj1.isArray() && obj2.isArray()) {
                // Comparison array objects
                if (obj1.getArray().compareObject(obj2.getArray())) {
                    size_t offset1 = obj1.getArray().getOffset();
                    size_t offset2 = obj2.getArray().getOffset();

                    if (opcode == BO_GT) {
                        b = (offset1 > offset2);
                    } else 
                    if (opcode == BO_GE) {
                        b = (offset1 >= offset2);
                    } else 
                    if (opcode == BO_LT) {
                        b = (offset1 < offset2);
                    } else 
                    if (opcode == BO_LE) {
                        b = (offset1 <= offset2);
                    } else {
                        SCT_INTERNAL_FATAL(stmt->getBeginLoc(), 
                                         "Unsupported statement "+opcodeStr);
                    }
                } else {
                    ScDiag::reportScDiag(stmt->getBeginLoc(), 
                                         ScDiag::CPP_DIFF_POINTER_COMP);
                }
            } else {
                // Comparison non-array objects
                if (*obj1.getObjectPtr() == *obj2.getObjectPtr()) {
                    b = (opcode == BO_GE || opcode == BO_LE);
                }
            }
            // Return value
            val = SValue(SValue::boolToAPSInt(b), 10);

        } else 
        if (int1 != NO_VALUE && int2 == NO_VALUE) {
            // Check variable is not equal to integer constant because of width
            val = NO_VALUE;
            SValue uval = (llval.isInteger()) ? rval : lval;
            // Variable GE or GT than literal
            bool varGreatLiter = rrval.isInteger() ? 
                                 opcode == BO_GE || opcode == BO_GT :
                                 opcode == BO_LE || opcode == BO_LT;
            
            if (auto typeInfo = getIntTraits(uval.getType())) {
                size_t varWidth = typeInfo->first;
                bool isUnsigned = typeInfo->second;
                APSInt maxVal = APSInt::getMaxValue(varWidth, isUnsigned);
                APSInt minVal = APSInt::getMinValue(varWidth, isUnsigned);
                APSInt literVal = int1.getInteger(); 
                                
                // Required by APInt comparison
                adjustIntegers(literVal, maxVal, literVal, maxVal);
                adjustIntegers(literVal, minVal, literVal, minVal);
                
//                cout << "BO varWidth " << varWidth << " isUnsigned " << isUnsigned
//                     << " literVal " << sc::APSintToString(literVal, 10) << " maxVal " << sc::APSintToString(maxVal, 10)
//                     << " minVal " << sc::APSintToString(minVal,10) << endl;
                
                if (literVal > maxVal) {
                    val = SValue(SValue::boolToAPSInt(!varGreatLiter), 10);
                } else
                if (literVal < minVal) {
                    val = SValue(SValue::boolToAPSInt(varGreatLiter), 10);
                }
            }
            
        } else {
            val = NO_VALUE;
            if (checkNoValue) {
                SCT_INTERNAL_FATAL(stmt->getBeginLoc(), 
                                 "No values in binary statement "+opcodeStr);
            }
        }

    } else 
    if (opcode == BO_LOr || opcode == BO_LAnd) {
        bool b; bool hasVal = false;

        // Try to get value from left argument
        if (!llval.isUnknown()) {
            if (opcode == BO_LOr && llval.getBoolValue()) { 
                b = true; hasVal = true;
            } else 
            if (opcode == BO_LAnd && !llval.getBoolValue()) {
                b = false; hasVal = true;
            }
        }
        // Cannot get result from one left argument, check right argument
        if (!hasVal) {
            if (!rrval.isUnknown()) {
                if (opcode == BO_LOr) { 
                    b = rrval.getBoolValue(); 
                    hasVal = !llval.isUnknown() || b;
                } else 
                if (opcode == BO_LAnd) {
                    b = rrval.getBoolValue(); 
                    hasVal = !llval.isUnknown() || !b;
                }
            }
        }

        // Return value
        if (hasVal) {
            val = SValue(SValue::boolToAPSInt(b), 10);
        } else {
            val = NO_VALUE;
            if (checkNoValue) {
                SCT_INTERNAL_FATAL(stmt->getBeginLoc(), 
                                 "No values in binary statement "+opcodeStr);
            }
        }
    } else {
        SCT_INTERNAL_FATAL(stmt->getBeginLoc(), 
                           "Unknown binary operator "+opcodeStr);
        val = NO_VALUE;
    }
}

/// Parse statement and run evalSubExpr for each operand
void ScParseExprValue::parseCompoundAssignStmt(CompoundAssignOperator* stmt,
                                               SValue& val) 
{
    Expr* lexpr = stmt->getLHS();
    Expr* rexpr = stmt->getRHS();
    bool lastAssignLHS = assignLHS; assignLHS = true;
    SValue lval = evalSubExpr(lexpr);
    assignLHS = lastAssignLHS; 
    SValue rval = evalSubExpr(rexpr);

    // Keep array unknown element
    SValue llval = getValueFromState(lval);
    SValue rrval = getValueFromState(rval);
    // Get referenced variable for @llval and @rrval
    SValue tmp;
    state->getDerefVariable(rrval, tmp); rrval = tmp;
    state->getDerefVariable(llval, tmp); llval = tmp;
    
    // Add @lval to @read first to produce @readndef if required
    readFromValue(rval);
    readFromValue(lval);
    writeToValue(lval);

    BinaryOperatorKind opcode = stmt->getOpcode();
    
    if (llval.isInteger() && rrval.isInteger()) {
        // Integer arithmetic
        APSInt val1 = llval.getInteger(); 
        APSInt val2 = rrval.getInteger();

        auto linfo = getIntTraits(lexpr->getType());
        auto rinfo = getIntTraits(rexpr->getType());
        size_t ltypeWidth = linfo ? linfo->first : 0;
        size_t rtypeWidth = rinfo ? rinfo->first : 0;
        
        SCT_TOOL_ASSERT (ltypeWidth && rtypeWidth, 
                         "No integer type traits in CompoundAssignOperator");
        
        if (opcode == BO_ShlAssign || opcode == BO_ShrAssign) {
            // Extend value width before operation to fit result value
            extendBitWidthBO(val1, val2, ltypeWidth, rtypeWidth);
            
        } else {
            // Extend value width before operation to fit result value
            extendBitWidthBO(val1, val2, ltypeWidth, rtypeWidth);
            // Width and sign adjustment, required by APSInt operators
            adjustIntegers(val1, val2, val1, val2);
        } 
        
        APSInt res;
        if (opcode == BO_AddAssign) {
            res = val1 + val2;
        } else 
        if (opcode == BO_SubAssign) {
            res = val1 - val2;
        } else
        if (opcode == BO_MulAssign) {
            res = val1 * val2;
        } else 
        if (opcode == BO_DivAssign) {
            res = val1 / val2;
        } else 
        if (opcode == BO_RemAssign) {
            res = val1 % val2;
        } else 
        if (opcode == BO_ShlAssign) {
            res = val1 << val2.getExtValue();
        } else 
        if (opcode == BO_ShrAssign) {
            res = val1 >> val2.getExtValue();
        } else 
        if (opcode == BO_OrAssign) {
            res = val1 | val2;
        } else 
        if (opcode == BO_AndAssign) {
            res = val1 & val2;
        } else 
        if (opcode == BO_XorAssign) {
            res = val1 ^ val2;
        } else {
            cout << "Opcode " << opcode << endl;
            SCT_TOOL_ASSERT (false, "parseCompAssign : Unknown integer opcode");
        }

        // Update the @lval and return it
        state->putValue(lval, SValue(res, llval.getRadix())); 

    } else {
        // Remove LHS variable value (de-reference inside)
        // Remove values for all array elements by unknown element access
        state->putValue(lval, NO_VALUE);
    }
    
    // Return @lval anyway
    val = lval;
    isSideEffStmt  = true;
    isAssignStmt   = true;
    isRequiredStmt = true;
}

void ScParseExprValue::parseUnaryStmt(UnaryOperator* stmt, SValue& val)
{
    // there is no value for many unary statements
    val = NO_VALUE;
    
    UnaryOperatorKind opcode = stmt->getOpcode();
    string opcodeStr = stmt->getOpcodeStr(stmt->getOpcode()).str();
    bool isPrefix = stmt->isPrefix(stmt->getOpcode());
    bool isIncrDecr = opcode == UO_PostInc || opcode == UO_PreInc || 
                      opcode == UO_PostDec || opcode == UO_PreDec;

    Expr* expr = stmt->getSubExpr();
    bool lastAssignLHS = assignLHS;
    if (isIncrDecr) assignLHS = true;
    SValue rval = evalSubExpr(stmt->getSubExpr());
    assignLHS = lastAssignLHS;
    
    // Get referenced variable for @rval
    SValue tmp;
    state->getDerefVariable(rval, tmp); rval = tmp;

    //cout << "parseUnaryStmt #" << hex <<stmt << dec << " rval " << rval << endl;
    
    if (opcode == UO_AddrOf) 
    {
        if (rval.isVariable() || rval.isTmpVariable() || rval.isArray() || 
            rval.isSimpleObject()) {
            // Create temporary object pointer to @rval
            QualType type = (rval.isVariable()) ? 
                                rval.getVariable().getType() :
                                ((rval.isTmpVariable()) ? 
                                        rval.getTmpVariable().getType() : 
                                        rval.getObjectPtr()->getType());
            val = SValue(type);
            state->putValue(val, rval);
            state->setValueLevel(val, level+1);

        } else {
            if (checkNoValue) {
                SCT_INTERNAL_FATAL(expr->getBeginLoc(), 
                                 "Unknown or integer type for unary &");
            }
        }
        
    } else 
    if (opcode == UO_Deref)
    {
        if (rval.isVariable() || rval.isTmpVariable() || rval.isObject()) {
            // Pointer dereference preserving array unknown index
            val = derefPointer(rval, stmt, true);
            
        } else {
            if (checkNoValue) {
                SCT_INTERNAL_FATAL(expr->getBeginLoc(), 
                                 "Unknown or integer type for unary dereference");
            }
        }
    } else 
    if (opcode == UO_Plus) {
        // Plus operation
        val = rval;
        readFromValue(rval);
        
    } else 
    if (opcode == UO_Minus)  
    {
        if (rval.isInteger()) {
            // Minus operation, result is signed
            APSInt res(rval.getInteger(), false);
            val = SValue(res.operator -(), rval.getRadix());

        } else 
        if (rval.isVariable() || rval.isTmpVariable() || rval.isObject()) {
            SValue rrval = getValueFromState(rval);
            
            if (rrval.isInteger()) {
                // Minus operation, result is signed
                APSInt res(rrval.getInteger(), false);
                val = SValue(res.operator -(), rrval.getRadix());
            }
        } else {
            if (checkNoValue) {
                SCT_INTERNAL_FATAL(expr->getBeginLoc(), 
                                   "Unknown or integer type for unary minus");
            }
        }
        //cout << "Minus rval " << rval << " val " << val << endl;
        readFromValue(rval);
          
    } else 
    if (opcode == UO_LNot)
    {
        val = evalUnaryLNot(rval);
        readFromValue(rval);
        
    } else 
    if (opcode == UO_Not)
    {
        // Get traits from expression (not argument) type
        if (auto i = getIntTraits(stmt->getType())) {
            // Bit width required to extend result of "~"
            val = evalUnaryBNot(rval, i->first);
        } else {
            // Do not use @evalUnaryBNot as it does not propagate MSB 
            val = NO_VALUE;
        }
        readFromValue(rval);
        
    } else 
    if (isIncrDecr)
    {
        val = parseIncDecStmt(rval, isPrefix, opcode == UO_PostInc || 
                              opcode == UO_PreInc);

        readFromValue(rval);
        writeToValue(rval);
        
        if (assignLHS) {
            // Not supported by vendor simulator tool
            ScDiag::reportScDiag(expr->getBeginLoc(), 
                                 ScDiag::SYNTH_INCRDECR_LHS);
        }
        
        isSideEffStmt  = true;
        isRequiredStmt = true;
        
    } else {
        ScDiag::reportScDiag(expr->getBeginLoc(), 
                             ScDiag::SC_WARN_EVAL_UNSUPPORTED_EXPR) << 
                             "Unsupported UnaryOperator, opcode " << opcodeStr;
    }
}


SValue ScParseExprValue::parseIncDecStmt(const SValue& rval, bool isPrefix, 
                                         bool isIncrement)
{
    SCT_TOOL_ASSERT(rval.isVariable() || rval.isTmpVariable() || rval.isObject(),
                    "No variable or object");

    // Get referenced variable for @rrval
    // Keep array unknown element
    SValue rrval = getValueFromState(rval, ArrayUnkwnMode::amArrayUnknown);
    SValue tmp;
    state->getDerefVariable(rrval, tmp); rrval = tmp;

    // Old (not-updated) value, use temporary object to support pointers
    QualType type = rval.getType();
    SValue orval = SValue(type);
    assignValueInState(orval, rval);
    state->setValueLevel(orval, level+1);

    if (rrval.isArray() && !rrval.getArray().isUnknown()) {
        // Pointer arithmetic
        int64_t shiftIndx = (isIncrement) ? (rrval.getArray().getOffset()+1) : 
                                            (rrval.getArray().getOffset()-1);
        if (shiftIndx >= 0 && (size_t)shiftIndx < rrval.getArray().getSize()) {
            rrval.getArray().setOffset(shiftIndx);

        } else {
            rrval = NO_VALUE;
            if (checkNoValue) {
               //cout << "parseUnaryStmt : Array/object shift out-of-bound " << shiftIndx << endl;
            }
        }
    } else
    if (rrval.isInteger()) {
        // Integer arithmetic
        if (isIncrement) {
            rrval.getInteger().operator ++();
        } else {
            rrval.getInteger().operator --();
        }

    } else {
        // Remove array values by unknown element access
        if (rrval.isArray() && rrval.getArray().isUnknown()) {
            state->putValue(rrval, NO_VALUE);
        }
        
        rrval = NO_VALUE; 
        if (checkNoValue) {
            // cout << "parseUnaryStmt : No value for pointer " << rval.asString() << endl;
        }
    }
    // Update the @rval which is @lval at the same time
    state->putValue(rval, rrval);
    
    return (isPrefix) ? rval : orval;
}

// Ternary operator (...) ? ... : ...
void ScParseExprValue::parseConditionalStmt(ConditionalOperator* stmt, SValue& val) 
{
    //cout << "<<<< parseConditionalStmt " << endl;
    SValue rval;
    auto i = condStoredValue.find(stmt);
    if (i != condStoredValue.end()) {
        rval = i->second;
    } else {
        // This option required for @ScParseExprValue instance usage 
        rval = evalSubExpr(stmt->getCond());
    }
   
    // Use @checkRecOnly to have pointer null/not null value for record/MIF
    // array element accessed at unknown index, required for condition of
    // pointer initialized: p ? p->f() : a;
    // Do check const for thread scope assert to consider local variables
    bool checkConst = state->getParseSvaArg() && !moduleSctAssert;
    auto condvals = evaluateConstInt(rval, checkConst, true);
    SValue cval = condvals.second;
    //cout << "cval " << cval << endl;

    if (cval) {
        if (cval.getBoolValue()) {
            val = evalSubExpr(stmt->getTrueExpr());
            readFromValue(val);
            
        } else {
            val = evalSubExpr(stmt->getFalseExpr());
            readFromValue(val);
        }
    } else {
        val = NO_VALUE;
        SValue trueval  = evalSubExpr(stmt->getTrueExpr());
        SValue falseval = evalSubExpr(stmt->getFalseExpr());
        readFromValue(trueval);
        readFromValue(falseval);
    }
    // ReadDefined properties for condition added in @TraverseConst::evaluateTermCond

    // Store condition to use in ScTraverseProc, different results joined to NO_VALUE
    // For non-SVA condition that is put in ScTraverseConst::run()
    if (state->getParseSvaArg()) {
        putSvaCondTerm(stmt, cval);
    }
    //cout << ">>>> parseConditionalStmt " << endl;
}

//---------------------------------------------------------------------------

// Get value for and_reduce, or_reduce and other reduce functions
SValue ScParseExprValue::getReducedVal(const string& fname, const SValue& rval) 
{
    SValue val = NO_VALUE;
    
    if (rval.isInteger()) {
        APSInt intVal = rval.getInteger();

        if (fname == "or_reduce") {
            val = SValue(SValue::boolToAPSInt(!intVal.isNullValue()), 10);
        } else 
        if (fname == "nor_reduce") {
            val = SValue(SValue::boolToAPSInt(intVal.isNullValue()), 10);
        } else 
        if (fname == "and_reduce") {
            val = SValue(SValue::boolToAPSInt(intVal.isAllOnesValue()), 10);
        } else 
        if (fname == "nand_reduce") {
            val = SValue(SValue::boolToAPSInt(!intVal.isAllOnesValue()), 10);
        } else 
        if (fname == "xor_reduce") {
            if (intVal.isNullValue()) {
                val = SValue(SValue::boolToAPSInt(false), 10);
            } else 
            if (intVal.isOneValue()) {
                val = SValue(SValue::boolToAPSInt(true), 10);
            } else {
                // May be implemented
            }
        } else 
        if (fname == "xnor_reduce") {
            if (intVal.isNullValue()) {
                val = SValue(SValue::boolToAPSInt(true), 10);
            } else 
            if (intVal.isOneValue()) {
                val = SValue(SValue::boolToAPSInt(false), 10);
            } else {
                // May be implemented
            }
        }
    }
    return val;
}

// Get type width, sign and APSInt value for arbitrary @val, that is 
// possible if @val is Integer or Variable/Object has Integer value
// \return true if that is possible
bool ScParseExprValue::getIntValueInfo(const SValue& val, 
                                       std::pair<size_t, bool>& info,
                                       APSInt& intVal) 
{    
    if (isScZeroWidth(val)) {
        intVal = llvm::APSInt(llvm::APInt(1, 0), true);
        info = std::pair<size_t, bool>(0, true);
        return true;
        
    } else
    if (val.isVariable() || val.isObject()) {
        auto optInfo = getIntTraits(val.getType(), true);
        SValue ival = getValueFromState(val);
        //cout << "ival(1) " << ival << " type " << val.getType().getAsString() << endl;
        
        if (optInfo && ival.isInteger()) {
            intVal = ival.getInteger();
            info = optInfo.getValue();
            return true;
        }
    } else 
    if (val.isInteger()) {
        intVal = val.getInteger();
        info = std::pair<size_t, bool>(intVal.getBitWidth(), intVal.isUnsigned());
        //cout << "ival(2) " << val << " type width " << intVal.getBitWidth() 
        //     << " sign " << !intVal.isSigned() << endl;
        return true;
    }
    return false;
}

// Get value for concat()
SValue ScParseExprValue::getConcatVal(const SValue& fval, const SValue& sval) 
{
    //cout << "getConcatVal fval " << fval << " sval " << sval << endl;
    
    SValue val = NO_VALUE;
    std::pair<size_t, bool> fInfo; APSInt fInt;
    std::pair<size_t, bool> sInfo; APSInt sInt;
    
    if (getIntValueInfo(fval, fInfo, fInt) && 
        getIntValueInfo(sval, sInfo, sInt)) 
    {
        unsigned resWidth = fInfo.first + sInfo.first;
        bool resUnsign = fInfo.second && sInfo.second; 
        //cout << "resWidth " << resWidth << ", resSign " << !resUnsign << endl;
        //cout << "fInt " << fInt.toString(10) << " sInt " << sInt.toString(10) << endl;
        //cout << "fInfo.first " << fInfo.first << " sInfo.first " << sInfo.first << endl;
        
        // Ensure value has the same width as its type
        if (fInfo.first) fInt = fInt.extOrTrunc(fInfo.first);
        if (sInfo.first) sInt = sInt.extOrTrunc(sInfo.first);
        SCT_TOOL_ASSERT((fInt.getBitWidth() == fInfo.first || !fInfo.first) &&
                        (sInt.getBitWidth() == sInfo.first || !sInfo.first), 
                        "Bit width mismatched");
        
        if (resWidth == 0) {
            // Return unsigned 32bit zero
            val = SValue(APSInt(32, true), 10);
        } else {
            APSInt resInt(resWidth, resUnsign);
            if (sInfo.first) resInt.insertBits(sInt, 0);
            if (fInfo.first) resInt.insertBits(fInt, sInfo.first);

            char radix = fInfo.first ? (fval.isInteger() ? fval.getRadix() : 10) :
                         sInfo.first ? (sval.isInteger() ? sval.getRadix() : 10) : 10;
            val = SValue(resInt, radix);
        }
    }
    //cout << "getConcatVal sval " << sval << ", fval " << fval << ", val " << val << endl;
    return val;
}

// Function call expression
void ScParseExprValue::parseCall(CallExpr* expr, SValue& val)
{
    // There is no value for many of member calls
    val = NO_VALUE;
    
    // Get arguments
    Expr** args = expr->getArgs();
    unsigned argNum = expr->getNumArgs();

    FunctionDecl* funcDecl = expr->getDirectCallee();
    if (funcDecl == nullptr) {
        SCT_INTERNAL_FATAL(expr->getBeginLoc(), 
                         "No function found for call expression");
    }
    
    string fname = funcDecl->getNameAsString();
    QualType retType = funcDecl->getReturnType();
    auto nsname = getNamespaceAsStr(funcDecl);

    if (DebugOptions::isEnabled(DebugComponent::doConstFuncCall)) {
        cout << "parseCall nsname : " << (nsname ? *nsname : "-- ")
             << ", fname : " << fname  << ", type : " << retType.getAsString() << endl;
    }
    
    // Check SC data type functions
    bool isScTypeFunc = isAnyScIntegerRef(retType);
    if (argNum > 0) {
        isScTypeFunc = isScTypeFunc || isAnyScIntegerRef(args[0]->getType());
    }     

    if (fname == "sct_assert") {
        // Parsing argument to put it in UseDef
        SCT_TOOL_ASSERT (argNum == 1 || argNum == 2, "Incorrect argument number");

        SValue fval = evalSubExpr(args[0]);
        readFromValue(fval);
        
        isRequiredStmt = noSvaGenerate ? isRequiredStmt : true;
        
    } else 
    if (fname == "sct_assert_const" || fname == "__assert" || 
        fname == "sct_assert_unknown") {
        // Checking assertion in regression tests
        SCT_TOOL_ASSERT (argNum == 1, "Incorrect argument number");
        SValue cval = evalSubExpr(args[0]);
        SValue ccval = getValueFromState(cval);

        if (fname == "sct_assert_unknown") {
            if (!ccval.isUnknown()) {
                ScDiag::reportScDiag(expr->getBeginLoc(),
                                     ScDiag::CPP_ASSERT_FAILED);
            }
        } else {
            if (!ccval.isInteger() || !ccval.getBoolValue()) {
                ScDiag::reportScDiag(expr->getBeginLoc(),
                                     ScDiag::CPP_ASSERT_FAILED);
                cout << "--------------------------" << endl;
                state->print();
            }
        }
        
    } else
    if (fname == "sct_assert_defined" || fname == "sct_assert_register" ||
        fname == "sct_assert_read" || fname == "sct_assert_latch" ||
        fname == "sct_assert_array_defined") {
        // Checking @defined, @readndef and others
        Expr** args = expr->getArgs();
        unsigned argNum = expr->getNumArgs();
        SCT_TOOL_ASSERT (argNum == 2, "Incorrect argument number");
        
        SValue lval = evalSubExpr(args[0]);
        SValue flag = evalSubExpr(args[1]);
        SCT_TOOL_ASSERT (flag.isInteger(), "Flag has no value in sct_assert_*");
        
        // Extract value to check UseDef
        // @extractValue check and return variable value for given @lval
        SValue llval; 
        state->getDerefVariable(lval, llval, true); lval = llval;
        if (lval.isObject() || lval.isScChannel()) {
            llval = state->getVariableForValue(lval);
        }

        if (llval.isVariable()) {
            // Get first element as it is done in @filterUseDefValues()
            llval = state->getFirstArrayElementForAny(llval);

            bool b = (fname == "sct_assert_defined") ?
                            state->getDefAllPathValues().count(llval) :
                     (fname == "sct_assert_register") ?       
                            state->getReadNotDefinedValues().count(llval) :
                     (fname == "sct_assert_read") ?
                            state->getReadValues().count(llval) :
                     (fname == "sct_assert_latch") ?
                            state->getDefSomePathValues().count(llval) :
                            state->getDefArrayValues().count(llval);

            // Register latch
            if (fname == "sct_assert_latch" && flag.getBoolValue()) {
                assertLatches.insert(llval);
            }

            if (flag.getBoolValue() != b) {
                cout << "Incorrect assertion for " << llval << endl;
                ScDiag::reportScDiag(expr->getBeginLoc(),
                                     ScDiag::CPP_ASSERT_FAILED);
            }
        } else {
            ScDiag::reportScDiag(expr->getBeginLoc(),
                        ScDiag::CPP_INCORRECT_ASSERT) << llval.asString();
        }
        
    } else 
    if (fname == "sct_assert_level") {
        // Do nothing, implemented in ScTraverseConst
        
    } else 
    if (fname == "sct_alive_loop") {
        // Set the following loop has at least one iteration
        aliveLoop = true;
        
    } else 
    if (fname == "sct_assert_in_proc_start") {
        // Set parsing SVA argument mode to make them registers
        if (!noSvaGenerate) state->setParseSvaArg(true);
        
        isRequiredStmt = noSvaGenerate ? isRequiredStmt : true;
        
    } else 
    if (fname == "sct_assert_in_proc_func") {
        // Called in @SCT_ASSERT macro
        SCT_TOOL_ASSERT (argNum == 4 || argNum == 5, "Incorrect argument number");

        // Parse LHS and RHS to fill read from variables, 
        if (!noSvaGenerate) {
            SValue fval = evalSubExpr(args[0]);
            SValue sval = evalSubExpr(args[1]);
            // Do not parse time argument(s) as its evaluated to integer
            
            readFromValue(fval);
            readFromValue(sval);
            state->setParseSvaArg(false);
        }
        isRequiredStmt = noSvaGenerate ? isRequiredStmt : true;
        
    } else 
    if (fname == "sct_is_method_proc") {
        // Get process kind functions in @sct_fifo_if.h
        bool isMethod = isCombProcess && !state->getParseSvaArg(); 
        val = SValue(SValue::boolToAPSInt(isMethod), 10);
        
    } else
    if (fname == "sct_is_thread_proc") {
        // Get process kind functions in @sct_fifo_if.h
        bool isThread = !isCombProcess || state->getParseSvaArg(); 
        val = SValue(SValue::boolToAPSInt(isThread), 10);
        
    } else    
    if (nsname && *nsname == "sc_dt" && isScTypeFunc) {
        // SC data type functions
        if (fname == "concat") {
            SCT_TOOL_ASSERT (argNum == 2, "Incorrect argument number");

            Expr* fexpr = args[0];
            Expr* sexpr = args[1];
            
            SValue fval = evalSubExpr(fexpr);
            SValue sval = evalSubExpr(sexpr);
            
            // Read value for any usage, that can lead to extra register for LHS
            readFromValue(fval);
            readFromValue(sval);
            
            // Put concatenation expression
            val = getConcatVal(fval, sval);
            
            // Clear variables if they are in LHS
            if (assignLHS) {
                state->putValue(fval, NO_VALUE);
                state->putValue(sval, NO_VALUE);
                
                writeToValue(fval); writeToValue(sval);
            }
            
        } else
        if (fname == "and_reduce" || fname == "or_reduce" ||
            fname == "xor_reduce" || fname == "nand_reduce" || 
            fname == "nor_reduce" || fname == "xnor_reduce") {
            SCT_TOOL_ASSERT (argNum == 1, "Incorrect argument number");
            Expr* fexpr = args[0];
            
            // Argument and its value
            SValue rval = evalSubExpr(fexpr);
            SValue rrval = getValueFromState(rval);
            
            // This cannot be in left part
            readFromValue(rval);

            // Put reduction unary expression
            val = getReducedVal(fname, rrval);
            
        }

    } else 
    if (nsname && *nsname == "sc_core") {
        
    } else 
    if (((nsname && *nsname == "std") || isLinkageDecl(funcDecl)) &&
        (fname == "printf" || fname == "fprintf" || 
         fname == "sprintf" || fname == "snprintf" ||
         fname == "fopen" || fname == "fclose"))
    {
        
    } else {
        // General functions, most logic is in ScTraverseConst
        // Declare temporary return variable in current module
        if (!isVoidType(retType)) {
            val = SValue(retType, modval);
        }
        isUserCallStmt = true;
        isRequiredStmt = true;
    }
}

// Member function call expression, used for general function call
// This method is overridden for module/port/signal/clock special methods
// \param val -- evaluated value/return value for user method
// \param thisVal -- this value for user method
void ScParseExprValue::parseMemberCall(CXXMemberCallExpr* callExpr, SValue& tval,
                                       SValue& val)
{   
    // There is no value for many of member calls
    val = NO_VALUE;
    
    // Get arguments
    Expr** args = callExpr->getArgs();
    unsigned argNum = callExpr->getNumArgs();

    // Get method
    CXXMethodDecl* methodDecl = callExpr->getMethodDecl();
    string fname = methodDecl->getNameAsString();
    QualType retType = methodDecl->getReturnType();
    auto nsname = getNamespaceAsStr(methodDecl);
    
    // Get @this expression and its type
    Expr* thisExpr = callExpr->getImplicitObjectArgument();
    QualType thisType = thisExpr->getType();
    // Method called for this pointer "->"
    bool isPointer = thisType->isAnyPointerType();
    
    // Get value for @this 
    tval = evalSubExpr(thisExpr);
    
    if (DebugOptions::isEnabled(DebugComponent::doConstFuncCall)) {
        cout << "CXXMemberCallExpr this = " << thisType.getAsString() 
             << ",\n fname = " << fname  << ", return type = " 
             << retType.getAsString() << ", tval " << tval << ", module is " 
             << modval << endl;
    }
    
    bool isScInteger = isAnyScIntegerRef(thisType, true);
    bool lhsChannel = isScChannel(thisType);

    SValue ttval = tval;
    if (isPointer) {
        // Pointer dereference preserving array unknown index
        ttval = derefPointer(tval, callExpr, isScInteger || lhsChannel);
    }
    
    bool lhsZeroWidth = isScZeroWidth(ttval);
    //cout << "isScZeroWidth tval " << tval << " ttval " << ttval << " lhsZeroWidth " << lhsZeroWidth <<endl;
    
    if (isScInteger || (!lhsChannel && lhsZeroWidth)) {
        // SC integer type object
        if (auto convDecl = dyn_cast<CXXConversionDecl>(methodDecl)) {
            // Type conversion
            if (lhsZeroWidth) {
                val = SValue(APSInt(32, true), 10); 
                
            } else {
                QualType convType = convDecl->getConversionType().getCanonicalType();
                if ( isa<BuiltinType>(convType) ) {
                    // Return this value, no get value from state here 
                    val = ttval;
                }
            }
        } else 
        if (fname == "bit" || fname == "operator[]") {
            SCT_TOOL_ASSERT (argNum == 1, "Incorrect argument number");
            Expr* indxExpr = args[0];
            
            if (lhsZeroWidth) {
                val = ZW_VALUE;
                
            } else {
                SValue rval = evalSubExpr(indxExpr);

                // Read index value
                readFromValue(rval);
                // Read value for any usage, that can lead to extra register for LHS
                readFromValue(ttval);
                // Get bit value
                val = evalRangeSelect(callExpr, ttval, rval, rval);

                // Clear variable value if it in LHS
                if (assignLHS) {
                    state->putValue(ttval, NO_VALUE);
                    writeToValue(ttval);
                }
            }
        } else
        if (fname == "range" || fname == "operator()") {
            SCT_TOOL_ASSERT (argNum == 2, "Incorrect argument number");
            Expr* hiExpr = args[0];
            Expr* loExpr = args[1];
            
            if (lhsZeroWidth) {
                val = ZW_VALUE;
                
            } else {
                SValue hval = evalSubExpr(hiExpr);
                SValue lval = evalSubExpr(loExpr);

                // Read index values
                readFromValue(hval);
                readFromValue(lval);
                // Read value for any usage, that can lead to extra register for LHS
                readFromValue(ttval);
                // Get range value
                val = evalRangeSelect(callExpr, ttval, hval, lval);

                // Clear variable value if it in LHS
                if (assignLHS) {
                    state->putValue(ttval, NO_VALUE);
                    writeToValue(ttval);
                }
            }
        } else 
        if (fname == "and_reduce" || fname == "or_reduce" || 
            fname == "xor_reduce" || fname == "nand_reduce" || 
            fname == "nor_reduce" || fname == "xnor_reduce") {
            
            if (lhsZeroWidth) {
                bool result = fname == "nand_reduce" || fname == "nor_reduce" || 
                              fname == "xnor_reduce";
                val = SValue(SValue::boolToAPSInt(result), 10); 
                
            } else {
                SValue tttval = getValueFromState(ttval);
                // This cannot be in left part, use @ttval to extract variable
                readFromValue(ttval);
                // Put reduction unary expression
                val = getReducedVal(fname, tttval);
            }
        } else 
        if (fname.find("to_i") != string::npos ||
            fname.find("to_u") != string::npos ||
            fname.find("to_long") != string::npos) {
            
            if (lhsZeroWidth) {
                val = SValue(APSInt(32, true), 10); 
                
            } else {
                // This cannot be in left part, use @ttval to extract variable
                readFromValue(ttval);
                // Get value of variable @ttval from state
                val = getValueFromState(ttval);

                if (val.isInteger()) {
                    QualType type = callExpr->getType();
                    auto typeInfo = getIntTraits(type, true);
                    if (!typeInfo) {
                        SCT_INTERNAL_ERROR (callExpr->getBeginLoc(),
                                            "No integral type width extracted");
                    }
                    size_t width = typeInfo.getValue().first;
                    bool isUnsigned = typeInfo.getValue().second;

                    // Align integer value width
                    val = SValue(extrOrTrunc(val.getInteger(), width, isUnsigned), 
                                 val.getRadix());
                }
            }
        } else 
        if (fname.find("to_bool") != string::npos) {
            if (lhsZeroWidth) {
                val = SValue(SValue::boolToAPSInt(false), 10); 
                
            } else {
                // This cannot be in left part, use @ttval to extract variable
                readFromValue(ttval);
                // Get value of variable @ttval from state
                val = getValueFromState(ttval);
                // Convert to boolean
                if (val.isInteger()) {
                    val = SValue(SValue::boolToAPSInt(val.getBoolValue()), 10);
                }
            }
        } else     
        if (fname.find("length") != string::npos) {
            // Get length form template parameter for @sc_bv/sc_(big)(u)int
            // not work for channel of these types
            if (lhsZeroWidth) {
                val = SValue(APSInt(32, true), 10); 
                
            } else {
                if (auto length = getTemplateArgAsInt(tval.getType(), 0)) {
                    val = SValue(*length, 10);
                } else {
                    SCT_INTERNAL_ERROR(callExpr->getBeginLoc(), 
                                       "Cannot get type width for length()");
                }
            }
        } else     
        if (fname.find("is_01") != string::npos) {
            // For @sc_bv type, always return true
            val = SValue(SValue::boolToAPSInt(1), 10);
            
        } else     
        if (fname.find("operator") != string::npos) {
            // ->operator=, ++, --, +=, -=, ... not supported for now 
            // as it is not widely used form
            ScDiag::reportScDiag(callExpr->getSourceRange().getBegin(), 
                                 ScDiag::SYNTH_UNSUPPORTED_OPER) << fname 
                                 << "ScParseExprValue::parseMemberCall";

        } else {
            SCT_INTERNAL_FATAL(callExpr->getBeginLoc(), 
                             "Unsupported SC object method "+fname);
        }
        
    } else 
    if (lhsChannel) {
        // Channel object, use @ttval here
        SValue cval = getChannelFromState(ttval);
        //cout << "Channel ttval " << ttval << " cval " << cval << endl;
        
        // Channel write and read access
        if ((cval.isScOutPort() || cval.isScSignal() || lhsZeroWidth) &&
            (fname == "write"))
        {
            SCT_TOOL_ASSERT (argNum == 1, "Incorrect argument number");
            if (lhsZeroWidth) {
                val = ZW_VALUE;
            
            } else {
                // Written value
                Expr* writeExpr = args[0];
                SValue wval = evalSubExpr(writeExpr);
                // Mark channel written variable as read
                readFromValue(wval);
                // Mark channel variable as defined, use @ttval to extract variable
                writeToValue(ttval);

                isAssignStmt   = true;
                isRequiredStmt = true;
            }
        } else
        if ((cval.isScChannel() || lhsZeroWidth) && (fname == "read" || 
            (fname.find("operator") != string::npos && (
             fname.find("const") != string::npos ||  // for operator const T&
             fname.find("int") != string::npos ||    // covers sc_int, sc_uint, ...
             fname.find("char") != string::npos ||
             fname.find("long") != string::npos ||
             fname.find("bool") != string::npos ||
             fname.find("sc_bv") != string::npos))))
        {
            if (lhsZeroWidth) {
                val = fname == "read" ? ZW_VALUE : SValue(APSInt(32, true), 10); 
            } else {
                // For record channel return channel value to access its fields
                if (isUserClass(cval.getType(), false)) val = cval;
                // Mark channel variable as read, use @ttval to extract variable
                readFromValue(ttval);
            }
        }
    } else 
    if (isAnyScCoreObject(thisType)) {
        // Do nothing 
        
    } else 
    if (isConstCharPtr(callExpr->getType())) {
        if (fname == "c_str") {
            val = ttval;
        }
        
    } else 
    if (state->getParseSvaArg()) {
        // For function call in assert replace it with returned expression
        // Read the used variables to ensure variable generated for them
        if (argNum == 0) {
            SValue curModval = modval;
            modval = ttval;
    
            Stmt* funcBody = methodDecl->getBody();
            //callExpr->dumpColor(); funcBody->dumpColor();
            
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
                if (retType->isIntegerType()) {
                    SValue val = evalSubExpr(retExpr);
                    readFromValue(val);
                }
            }
            
            modval = curModval;
        }
        
    } else {
        // General methods, most logic implemented in ScTraverseConst
        // Declare temporary return variable in current module
        if (!isVoidType(retType)) {
            val = SValue(retType, modval);
        }
        isUserCallStmt = true;
        isRequiredStmt = true;
    }
}

// Operator call expression
void ScParseExprValue::parseOperatorCall(CXXOperatorCallExpr* expr, SValue& tval,
                                         SValue& val)
{
    // There is generally no value for operator call
    val = NO_VALUE;
    
    Expr** args = expr->getArgs();
    unsigned argNum = expr->getNumArgs();
    SCT_TOOL_ASSERT (argNum != 0, "Operator without arguments");
    
    OverloadedOperatorKind opcode = expr->getOperator();
    string opcodeStr = getOperatorSpelling(opcode);

    // Get first argument type, it can be this object
    Expr* thisExpr = args[0];
    QualType thisType = thisExpr->getType();
    FunctionDecl* methodDecl = expr->getCalleeDecl()->getAsFunction();
    if (methodDecl == nullptr) {
        SCT_INTERNAL_FATAL(expr->getBeginLoc(),
                           "No function found for call expression");
    }
    QualType retType = methodDecl->getReturnType();
    auto nsname = getNamespaceAsStr(methodDecl);
    
    // Method called for this pointer "->"
    bool isPointer = thisType->isAnyPointerType();

    bool isAssignOperator = expr->isAssignmentOp() && opcode == OO_Equal;
    bool isIncrDecr = opcode == OO_PlusPlus || opcode == OO_MinusMinus;
    bool isCompoundAssign = opcode == OO_PlusEqual || opcode == OO_MinusEqual || 
            opcode == OO_StarEqual || opcode == OO_SlashEqual ||
            opcode == OO_PercentEqual || 
            opcode == OO_GreaterGreaterEqual || opcode == OO_LessLessEqual ||
            opcode == OO_AmpEqual || opcode == OO_PipeEqual || 
            opcode == OO_CaretEqual;
    bool isAccessAtIndex = (isStdArray(thisType) || isStdVector(thisType) || 
                            isScVector(thisType)) && opcode == OO_Subscript;
    
    // Get value for @this 
    bool lastAssignLHS = assignLHS;
    if (isAssignOperator || isIncrDecr || isCompoundAssign) assignLHS = true;
    tval = evalSubExpr(thisExpr);
    assignLHS = lastAssignLHS;
    
    if (isPointer) {
        // There are no real examples for operator for pointer argument,
        // If found, use pointer dereference like in parseMemberCall():
        //SValue ttval = derefPointer(tval, callExpr, isScInteger || isChannel);
        ScDiag::reportScDiag(expr->getSourceRange().getBegin(), 
                     ScDiag::SYNTH_UNSUPPORTED_OPER) << "operator for pointer"
                     << opcodeStr;
    }
    
    bool lhsZeroWidth = isScZeroWidth(tval);

    if (isAssignOperator) {
        // Assignment "operator=" for all types including SC data types
        // Assignments with add, subtract, ... processed below
        SCT_TOOL_ASSERT (argNum == 2, "Incorrect argument number");

        if (lhsZeroWidth) {
            val = ZW_VALUE;
            
        } else 
        if (isAssignOperatorSupported(thisType)) {
            // Declare temporary return variable in current module
            val = NO_VALUE;
            if (!isVoidType(retType)) {
                val = SValue(retType, modval);
            }
            isUserCallStmt = true;
            isRequiredStmt = true;
            
        } else {
            bool isInt = isAnyScIntegerRef(thisType, true);
            bool isRef = !thisType.isNull() && thisType->isReferenceType();
            thisType = getDerefType(thisType);
            bool isRecord = !isInt && isUserClass(thisType, true);
            
            // @strLiterWidth/@strLiterUnsigned work for integer argument only
            unsigned lastWidth = strLiterWidth;
            bool lastUnsigned = strLiterUnsigned;
            if (auto typeInfo = getIntTraits(getTypeForWidth(expr), true)) {
                strLiterWidth = typeInfo.getValue().first;
                strLiterUnsigned = typeInfo.getValue().second;
            } else {
                strLiterWidth = 0;  // Provides no string literal parsing
            }
            
            // Set @locrecvar to provide owner to record value created for @rval
            if (isRecord && !isRef) locrecvar = tval;
            SValue rval = evalSubExpr(args[1]);
            if (isRecord && !isRef) locrecvar = NO_VALUE;
            
            strLiterWidth = lastWidth; 
            strLiterUnsigned = lastUnsigned;
        
            if (isInt) {
                assignValueInState(tval, rval);
                val = tval;
            } else 
            if (isRecord) {
                // Record pointer not supported yet
                assignRecValueInState(tval, rval);
                val = tval;
            }

            // Mark variable/object/channel as written/read
            readFromValue(rval);
            writeToValue(tval);

            isAssignStmt   = true;
            isRequiredStmt = (sc::isScChannel(thisType) || 
                              sc::isScChannelArray(thisType)) ? 
                              true : isRequiredStmt;
        }
    } else 
    if (isScPort(thisType) && opcode == OO_Arrow) {
        // Operator "->" for @sc_port<IF>
        SCT_TOOL_ASSERT (argNum == 1, "Incorrect argument number");

        if (lhsZeroWidth) {
            val = ZW_VALUE;
            
        } else {
            // Get value for @this which points-to module/MIF object
            val = tval;
        }
    } else
    if (isAccessAtIndex) {
        // std::array, std::vector, @sc_vector access at index
        SCT_TOOL_ASSERT (argNum == 2, "Incorrect argument number");
        
        if (lhsZeroWidth) {
            val = ZW_VALUE;
            
        } else {
            bool lastAssignLHS = assignLHS;
            assignLHS = false;
            SValue rval = evalSubExpr(args[1]);
            assignLHS = lastAssignLHS;
            readFromValue(rval);

            val = parseArraySubscript(expr, tval, rval);
        }
    } else
    if (isIoStream(thisType)) {
        // Do nothing for @cout << and @cin >>, do not parse other arguments
        
    } else 
    if (nsname && *nsname == "sc_dt") {
        // SC data types
        if (opcode == OO_Subscript) { // "[]"
            SCT_TOOL_ASSERT (argNum == 2, "Incorrect argument number");
            
            if (lhsZeroWidth) {
                val = ZW_VALUE;

            } else {
                SValue rval = evalSubExpr(args[1]);
                readFromValue(rval);
                // Read value for any usage, that can lead to extra register for LHS
                readFromValue(tval);

                // Get bit value
                val = evalRangeSelect(expr, tval, rval, rval);

                // Clear variable value if it in LHS
                if (assignLHS) {
                    state->putValue(tval, NO_VALUE);
                    writeToValue(tval);
                }
            }
        } else 
        if (opcode == OO_Call) {  // "()"
            SCT_TOOL_ASSERT (argNum == 3, "Incorrect argument number");
            
            if (lhsZeroWidth) {
               val = ZW_VALUE;
            
            } else {
                SValue rval = evalSubExpr(args[1]);
                SValue rrval = evalSubExpr(args[2]);
                readFromValue(rval);
                readFromValue(rrval);
                // Read value for any usage, that can lead to extra register for LHS
                readFromValue(tval);

                // Get range value
                val = evalRangeSelect(expr, tval, rval, rrval);

                // Clear variable value if it in LHS
                if (assignLHS) {
                    state->putValue(tval, NO_VALUE);
                    writeToValue(tval);
                }
            }
        } else 
        if (opcode == OO_Exclaim) { // "!"
            SCT_TOOL_ASSERT (argNum == 1, "Incorrect argument number");
            
            if (lhsZeroWidth) {
                val = SValue(SValue::boolToAPSInt(true), 10);
                
            } else {
                val = evalUnaryLNot(tval);
                readFromValue(tval);
            }
        } else 
        if (opcode == OO_Tilde) { // "~"
            SCT_TOOL_ASSERT (argNum == 1, "Incorrect argument number");

            if (lhsZeroWidth) {
                val = SValue(SValue::boolToAPSInt(true), 10);
                
            } else {
                // Get traits from expression (not argument) type
                if (auto i = getIntTraits(expr->getType())) {
                    // Bit width required to extend result of "~"
                    val = evalUnaryBNot(tval, i->first);
                } else {
                    // Do not use @evalUnaryBNot as it does not propagate MSB 
                }
                readFromValue(tval);
            }
        } else 
        if (opcode == OO_Comma) { // ","
            SCT_TOOL_ASSERT (argNum == 2, "Incorrect argument number");

            SValue rval = evalSubExpr(args[1]);
            // Read value for any usage, that can lead to extra register for LHS
            readFromValue(tval); 
            readFromValue(rval);
            
            // Put concatenation expression
            val = getConcatVal(tval, rval);
            
            // Clear variables if they are in LHS
            if (assignLHS) {
                state->putValue(tval, NO_VALUE);
                state->putValue(rval, NO_VALUE);
                
                writeToValue(tval); writeToValue(rval);
            }
            
        } else 
        // "++" "--"
        if (isIncrDecr) {
            // Postfix ++/-- has artifical argument
            SCT_TOOL_ASSERT (argNum == 1 || argNum == 2, "Incorrect argument number");
            
            if (lhsZeroWidth) {
                val = ZW_VALUE; 
                
            } else {
                readFromValue(tval);
                writeToValue(tval);

                bool isPrefix = (argNum == 1);
                val = parseIncDecStmt(tval, isPrefix, opcode == OO_PlusPlus);

                // Not required to clear value if @assignLHS as it is normally assigned
                if (assignLHS) {
                    // Not supported by vendor simulator tool
                    ScDiag::reportScDiag(expr->getBeginLoc(), 
                                         ScDiag::SYNTH_INCRDECR_LHS);
                }

                isSideEffStmt  = true;
                isRequiredStmt = true;
            }
        } else    
        // Unary "-" and "+"
        if (argNum == 1 && (opcode == OO_Plus || opcode == OO_Minus)) {
            if (lhsZeroWidth) {
                val = ZW_VALUE; 
                
            } else {
                readFromValue(tval);
            
                if (opcode == OO_Minus) {
                    SValue rrval = getValueFromState(tval);

                    if (rrval.isInteger()) {
                        // Minus operation, result is signed
                        APSInt res(rrval.getInteger(), false);
                        val = SValue(res.operator -(), rrval.getRadix());
                        //cout << "Minus rval " << rrval << " val " << val << endl;
                    }
                } else {
                    val = tval;
                }
            }
        } else 
        // "+" "-" "*"  "/" "==" "!=" "<" "<=" ">" ">=" "<<" ">>" "%" "^" "&" "|" 
        // There is no operators "&&" "||" for SC data types
        if (argNum == 2 && (opcode == OO_Plus || opcode == OO_Minus || 
            opcode == OO_Star || opcode == OO_Slash || opcode == OO_EqualEqual || 
            opcode == OO_ExclaimEqual || opcode == OO_Less ||
            opcode == OO_LessEqual || opcode == OO_Greater || 
            opcode == OO_GreaterEqual || opcode == OO_LessLess || 
            opcode == OO_GreaterGreater || 
            opcode == OO_Percent || opcode == OO_Caret || opcode == OO_Amp || 
            opcode == OO_Pipe || opcode == OO_AmpAmp || opcode == OO_PipePipe))
        {
            // As no &&/||, so no LHS based evaluation required
            SCT_TOOL_ASSERT (opcode != OO_AmpAmp && opcode != OO_PipePipe,
                             "No &&/|| for SC data types supported");
                    
            SValue rval = evalSubExpr(args[1]);
            readFromValue(tval);
            readFromValue(rval);

            // Prepare values for @lval and @rval for arithmetic operations
            SValue llval = getValueFromState(tval);
            SValue rrval = getValueFromState(rval);
            // Get referenced variable for @llval and @rrval
            SValue tmp;
            state->getDerefVariable(rrval, tmp); rrval = tmp;
            state->getDerefVariable(llval, tmp); llval = tmp;

            if (llval.isInteger() && rrval.isInteger()) {
                APSInt val1 = llval.getInteger(); 
                APSInt val2 = rrval.getInteger();
                
                auto linfo = getIntTraits(args[0]->getType());
                auto rinfo = getIntTraits(args[1]->getType());
                // Type width can be determined for C++ types only
                size_t ltypeWidth = linfo ? linfo->first : 0;
                size_t rtypeWidth = rinfo ? rinfo->first : 0;

                // Comparison operators require the same width and sign of arguments
                if (opcode == OO_LessLess || opcode == OO_GreaterGreater) {
                    // Width and sign adjustment, required by APSInt operators
                    extendBitWidthOO(val1, val2, ltypeWidth, rtypeWidth, opcode);
                } else {
                    // Width and sign adjustment, required by APSInt operators
                    extendBitWidthOO(val1, val2, ltypeWidth, rtypeWidth, opcode);
                    // Adjust APSInt to the same sign and maximal bit width
                    adjustIntegers(val1, val2, val1, val2, true);
                    //cout << "OO llval " << llval << " rrval " << rrval << endl;
                    //cout << "val1 " << sc::APSintToString(val1, 16) << " val2 " << sc::APSintToString(val2, 16) << endl;
                }
            
                llvm::Optional<APSInt> res;
                if (opcode == OO_Plus) {
                    res = val1 + val2;
                } else 
                if (opcode == OO_Minus) {
                    res = val1 - val2;
                } else 
                if (opcode == OO_Star) {
                    res = val1 * val2;
                } else 
                if (opcode == OO_Slash) {
                    res = val1 / val2;
                } else 
                if (opcode == OO_EqualEqual) {
                    bool b = (val1 == val2);
                    res = SValue::boolToAPSInt(b);
                } else 
                if (opcode == OO_ExclaimEqual) {
                    bool b = (val1 != val2);
                    res = SValue::boolToAPSInt(b);
                } else 
                if (opcode == OO_Less) {
                    bool b = val1 < val2;
                    res = SValue::boolToAPSInt(b);
                } else 
                if (opcode == OO_LessEqual) {
                    bool b = val1 <= val2;
                    res = SValue::boolToAPSInt(b);
                } else 
                if (opcode == OO_Greater) {
                    bool b = val1 > val2;
                    res = SValue::boolToAPSInt(b);
                } else 
                if (opcode == OO_GreaterEqual) {
                    bool b = val1 >= val2;
                    res = SValue::boolToAPSInt(b);
                } else 
                if (opcode == OO_LessLess) {
                    res = val1 << val2.getExtValue();
                } else 
                if (opcode == OO_GreaterGreater) {
                    res = val1 >> val2.getExtValue();
                } else 
                if (opcode == OO_Percent) {
                    res = val1 % val2;
                } else 
                if (opcode == OO_AmpAmp) {
                    res = SValue::boolToAPSInt(val1.getBoolValue() && 
                                               val2.getBoolValue());
                } else 
                if (opcode == OO_PipePipe) {
                    res = SValue::boolToAPSInt(val1.getBoolValue() || 
                                               val2.getBoolValue());
                } else 
                if (opcode == OO_Caret) {
                    res = val1 ^ val2;
                } else 
                if (opcode == OO_Amp) {
                    res = val1 & val2;
                } else 
                if (opcode == OO_Pipe) {
                    res = val1 | val2;
                } else {
                    cout << "Opcode " << opcode << endl;
                    SCT_TOOL_ASSERT (false, "Unknown opcode for SC type operator");
                }
                
                char radix = llval.isInteger() ? llval.getRadix() : 10;
                val = res ? SValue(res.getValue(), radix) : NO_VALUE;
                //cout << "res " << (res ? res->toString(16) : "--") << " val " << val << endl;
                
            } else 
            if (llval.isInteger() || rrval.isInteger()) {
                val = NO_VALUE;
                SValue int1 = (llval.isInteger()) ? llval : rrval;
                SValue uval = (llval.isInteger()) ? rval : tval;
                // Variable GE or GT than literal
                bool equalNotEqual = opcode == OO_EqualEqual || 
                                     opcode == OO_ExclaimEqual;
                bool varGreatLiter = rrval.isInteger() ? 
                            opcode == OO_GreaterEqual || opcode == OO_Greater :
                            opcode == OO_LessEqual || opcode == OO_Less;

                if (auto typeInfo = getIntTraits(uval.getType())) {
                    size_t varWidth = typeInfo->first;
                    bool isUnsigned = typeInfo->second;
                    APSInt maxVal = APSInt::getMaxValue(varWidth, isUnsigned);
                    APSInt minVal = APSInt::getMinValue(varWidth, isUnsigned);
                    APSInt literVal = int1.getInteger(); 

                    // Required by APInt comparison
                    adjustIntegers(literVal, maxVal, literVal, maxVal);
                    adjustIntegers(literVal, minVal, literVal, minVal);

//                    cout << "OO varWidth " << varWidth << " isUnsigned " << isUnsigned
//                         << " literVal " << sc::APSintToString(literVal, 10) << " maxVal " << sc::APSintToString(maxVal, 10)
//                         << " minVal " << sc::APSintToString(minVal, 10) << endl;

                    if (equalNotEqual) {
                        if (literVal > maxVal || literVal < minVal) {
                            val = SValue(SValue::boolToAPSInt(opcode == OO_ExclaimEqual), 10);
                        }
                    } else {
                        if (literVal > maxVal) {
                            val = SValue(SValue::boolToAPSInt(!varGreatLiter), 10);
                        } else
                        if (literVal < minVal) {
                            val = SValue(SValue::boolToAPSInt(varGreatLiter), 10);
                        }
                    }
                }
            }
            
        } else
        // "+=" "-=" "*="  "/=" "%=" ">>=" "<<=" "&=" "|=" "^="
        if (isCompoundAssign)
        {
            SCT_TOOL_ASSERT (argNum == 2, "Incorrect argument number");
            
            if (lhsZeroWidth) {
                val = ZW_VALUE; 
                
            } else {
                SValue rval = evalSubExpr(args[1]);
                // Add @lval to @read first to produce @readndef if required
                readFromValue(rval);
                readFromValue(tval);
                writeToValue(tval);

                // Prepare values for @lval and @rval for arithmetic operations
                SValue llval = getValueFromState(tval);
                SValue rrval = getValueFromState(rval);
                // Get referenced variable for @llval and @rrval
                SValue tmp;
                state->getDerefVariable(rrval, tmp); rrval = tmp;
                state->getDerefVariable(llval, tmp); llval = tmp;

                if (llval.isInteger() && rrval.isInteger()) {
                    APSInt val1 = llval.getInteger(); 
                    APSInt val2 = rrval.getInteger();

                    auto linfo = getIntTraits(args[0]->getType());
                    auto rinfo = getIntTraits(args[1]->getType());
                    // Type width can be determined for C++ types only
                    size_t ltypeWidth = linfo ? linfo->first : 0;
                    size_t rtypeWidth = rinfo ? rinfo->first : 0;

                    if (opcode == OO_LessLessEqual || opcode == OO_GreaterGreaterEqual) {
                        // Extend value width before operation to fit result value
                        extendBitWidthOO(val1, val2, ltypeWidth, rtypeWidth, opcode);
                    } else {
                        // Extend value width before operation to fit result value
                        extendBitWidthOO(val1, val2, ltypeWidth, rtypeWidth, opcode);
                        // Width and sign adjustment, required by APSInt operators
                        adjustIntegers(val1, val2, val1, val2, true);
                    }      

                    APSInt res;
                    if (opcode == OO_PlusEqual) {
                        res = val1 + val2;
                    } else 
                    if (opcode == OO_MinusEqual) {
                        res = val1 - val2;
                    } else
                    if (opcode == OO_StarEqual) {
                        res = val1 * val2;
                    } else 
                    if (opcode == OO_SlashEqual) {
                        res = val1 / val2;
                    } else 
                    if (opcode == OO_PercentEqual) {
                        res = val1 % val2;
                    } else 
                    if (opcode == OO_LessLessEqual) {
                        res = val1 << val2.getExtValue();
                    } else 
                    if (opcode == OO_GreaterGreaterEqual) {
                        res = val1 >> val2.getExtValue();
                    } else 
                    if (opcode == OO_PipeEqual) {
                        res = val1 | val2;
                    } else 
                    if (opcode == OO_AmpEqual) {
                        res = val1 & val2;
                    } else 
                    if (opcode == OO_CaretEqual) {
                        res = val1 ^ val2;
                    } else {
                        cout << "Opcode " << opcode << endl;
                        SCT_TOOL_ASSERT (false, "Unknown opcode for SC type operator");
                    }

                    // Update the @lval and return it
                    state->putValue(tval, SValue(res, llval.getRadix())); 

                } else {
                    // Remove LHS variable value (de-reference inside)
                    // Remove values for all array elements by unknown element access
                    state->putValue(tval, NO_VALUE);
                }

                // Return @lval anyway
                val = tval;
                isSideEffStmt  = true;
                isAssignStmt   = true;
                isRequiredStmt = true;
            }
        } else {
            expr->dumpColor();
            string opStr = getOperatorSpelling(opcode);
            ScDiag::reportScDiag(expr->getSourceRange().getBegin(), 
                                 ScDiag::SYNTH_UNSUPPORTED_OPER) << opcodeStr
                                 << "ScParseExprValue::parseOperatorCall";
        }
        
    } else 
    if (nsname && *nsname == "std") {
        
    } else 
    if (nsname && *nsname == "sc_core") {
        
    } else {
        // User-defined operators not supported yet
        SCT_INTERNAL_FATAL(expr->getBeginLoc(), 
                         "User-defined operator not supported yet");
    }
}

// Return statement
void ScParseExprValue::parseReturnStmt(ReturnStmt* stmt, SValue& val)
{
    Expr* expr = stmt->getRetValue();
    
    if (expr != nullptr) {
        // Remove @ExprWithCleanups from @CXXConstructExpr
        auto retExpr = removeExprCleanups(expr);
        // Check for copy constructor to use record RValue
        if (auto ctorExpr = getCXXCtorExprArg(retExpr)) {
            if (!ctorExpr->getConstructor()->isCopyOrMoveConstructor()) {
                ScDiag::reportScDiag(expr->getBeginLoc(), 
                         ScDiag::SYNTH_NONTRIVIAL_COPY);
            }
        }
        QualType retType = expr->getType();
        
        // Skip zero width parameter
        if (isZeroWidthType(retType) || isZeroWidthArrayType(retType)) {
            val = ZW_VALUE; return;
        }
        
        // Set return temporal variable value to analyze @CXXConstructExpr and
        // use as record copy local variable
        locrecvar = returnValue; 
        
        // Parse return expression, 
        // decrease level to have result outside of the function
        level -= 1;
        val = evalSubExpr(expr);
        level += 1;
        
        // Clear after
        locrecvar = NO_VALUE;
        
    } else {
        val = NO_VALUE;
    }
    
    // Store return value to use in @calledFuncs, @returnValue can be NO_VALUE
    // for void function and if it used not in ScTraverseConst
    if (returnValue.isTmpVariable()) {
        assignValueInState(returnValue, val);
        readFromValue(val);
        
    } else 
    if (returnValue.isUnknown()) {
        // Do nothing
    } else {
        SCT_INTERNAL_ERROR (stmt->getBeginLoc(), 
                            "Unexpected kind of return variable value");
    }
    
    if (returnStmtFunc) {
        simpleReturnFunc = false;
    } else {
        returnStmtFunc = stmt;
    }

    isRequiredStmt = true;
}

void ScParseExprValue::parseExpr(ExpressionTraitExpr* expr, SValue& val)
{
    val = NO_VALUE;
}

//----------------------------------------------------------------------------

// Pointer dereference preserving unknown array index, does null/dangling 
// pointer error reporting
// \return pointe value
SValue ScParseExprValue::derefPointer(const SValue& rval, Stmt* stmt, 
                                      bool checkPtr)
{
    // Keep array unknown element
    SValue val = getValueFromState(rval, ArrayUnkwnMode::amArrayUnknown);

    if (checkPtr) {
        // Check for null and dangling pointer
        SValue rvalzero = state->getFirstArrayElementForAny(
                                    rval, ScState::MIF_CROSS_NUM);
        SValue valzero = getValueFromState(
                                    rvalzero, ArrayUnkwnMode::amArrayUnknown);
    //    cout << "derefPointer rval " << rval << " val " << val << " rvalzero " 
    //         << rvalzero << " valzero " << valzero << endl;

        if (valzero.isUnknown()) {
            SValue rvar = state->getVariableForValue(rval);
            ScDiag::reportScDiag(stmt->getBeginLoc(), 
                                 ScDiag::CPP_DANGLING_PTR_DEREF) << 
                                 rvar.asString(rvar.isObject());
        } else 
        if (valzero.isInteger() && valzero.getInteger().isNullValue()) {
            SValue rvar = state->getVariableForValue(rval);
            ScDiag::reportScDiag(stmt->getBeginLoc(), 
                                 ScDiag::CPP_NULL_PTR_DEREF) << 
                                 rvar.asString(rvar.isObject());
        }

        // Check for non-constant pointer to constant variable
        if (val.isVariable() && val.getType().isConstQualified()) {
            if (!isPointerToConst(rval.getType())) {
                ScDiag::reportScDiag(stmt->getBeginLoc(), 
                    ScDiag::SYNTH_NONCOST_PTR_CONST) << val.asString(false);
            }
        }
    }
    
    return val;
}

// Try to calculate range or bit selection 
SValue ScParseExprValue::evalRangeSelect(const Expr* expr, SValue val, 
                                         SValue hi, SValue lo)
{
    if ( (val = getIntOrUnkwn(val)) && (hi = getIntOrUnkwn(hi)) && 
         (lo = getIntOrUnkwn(lo)) ) 
    {
        auto hiIndx = hi.getInteger().getExtValue();
        auto loIndx = lo.getInteger().getExtValue();
        APSInt valInt = val.getInteger();
        
        if (hiIndx < loIndx || hiIndx < 0 || loIndx < 0) 
        {
            ScDiag::reportScDiag(expr->getBeginLoc(), 
                                 ScDiag::SC_RANGE_WRONG_INDEX);
            // Required to prevent more errors
            SCT_INTERNAL_FATAL_NOLOC ("Incorrect range error");
        }
        if (valInt.getBitWidth() <= hiIndx) 
        {
            ScDiag::reportScDiag(expr->getBeginLoc(), 
                                 ScDiag::SC_RANGE_WRONG_WIDTH);
            // Required to prevent more errors
            SCT_INTERNAL_FATAL_NOLOC ("Incorrect range error");
        }
        
        char radix = val.isInteger() ? val.getRadix() : 10;
        return SValue(APSInt(valInt.extractBits(hiIndx-loIndx+1, loIndx)), radix);
    }
    return NO_VALUE;
}

SValue ScParseExprValue::getIntOrUnkwn(SValue val)
{
    if (val.isInteger())
        return val;

    SValue resVal = getValueFromState(val);
    if (resVal.isInteger()) {
        return resVal;
    } else {
        return NO_VALUE;
    }
}

SValue ScParseExprValue::evalUnaryLNot(const SValue& val)
{
    // Get referenced variable for @rrval
    SValue rval = getValueFromState(val);
    SValue rrval;
    state->getDerefVariable(rval, rrval);

    if (rrval != NO_VALUE) {
        return SValue(SValue::boolToAPSInt(!rrval.getBoolValue()), 10);
    } else {
        return NO_VALUE;
    }
}

// Bitwise not "~" with extension to @bitWidth bits
SValue ScParseExprValue::evalUnaryBNot(const SValue& val, unsigned bitWidth)
{
    // Get referenced variable for @rrval
    SValue rval = getValueFromState(val);
    SValue rrval;
    state->getDerefVariable(rval, rrval);

    if (rrval.isInteger()) {
        APSInt extInt = rrval.getInteger().extOrTrunc(bitWidth);
        return SValue(extInt.operator ~(), rrval.getRadix());
    } else {
        return NO_VALUE;
    }
}

}
