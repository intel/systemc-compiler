/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Author: Mikhail Moiseev
 */

#include "sc_tool/scope/ScVerilogWriter.h"
#include "sc_tool/utils/ScTypeTraits.h"
#include "sc_tool/utils/CppTypeTraits.h"
#include "sc_tool/utils/StringFormat.h"
#include "sc_tool/diag/ScToolDiagnostic.h"
#include "sc_tool/utils/DebugOptions.h"
#include "sc_tool/utils/VerilogKeywords.h"
#include "sc_tool/utils/CheckCppInheritance.h"
#include "sc_tool/ScCommandLine.h"

#include "clang/AST/ExprCXX.h"
#include "llvm/Support/ScopedPrinter.h"
#include "sc_tool/cfg/ScState.h"
#include <limits.h>
#include <iostream>

using namespace std;
using namespace clang;
using namespace llvm;


namespace std {
    
std::size_t hash< std::pair<sc::SValue, bool> >::operator () (
                        const std::pair<sc::SValue, bool>& obj) const 
{
    using std::hash; 
    return (std::hash<sc::SValue>()(obj.first) ^ 
            std::hash<bool>()(obj.second));
}

std::size_t hash< std::pair<std::pair<sc::SValue, bool>, sc::SValue> >::operator () (
            const std::pair<std::pair<sc::SValue, bool>, sc::SValue>& obj) const 
{
    using std::hash;
    return (std::hash<sc::SValue>()(obj.first.first) ^ 
            std::hash<bool>()(obj.first.second) ^ 
            std::hash<sc::SValue>()(obj.second));
}


std::size_t hash< std::pair<std::string, sc::SValue> >::operator () (
            const std::pair<std::string, sc::SValue>& obj) const 
{
    using std::hash;
    return (std::hash<std::string>()(obj.first) ^ 
            std::hash<sc::SValue>()(obj.second));
}

} // namespace std

//============================================================================

using namespace sc;

// Get LHS for any kind of assignment statement/operator
// \return LHS statement of the assignment or @nullptr
const Expr* ScVerilogWriter::getAssignLhs(const Stmt* stmt) const
{
    if (auto constExpr = dyn_cast<CXXConstructExpr>(stmt)) {
        if (constExpr->getNumArgs() != 0) 
            return getAssignLhs(constExpr->getArg(0));
        else 
            return nullptr;
    } else 
    if (auto bindExpr = dyn_cast<CXXBindTemporaryExpr>(stmt)) {
        return getAssignLhs(bindExpr->getSubExpr());
    } else 
    if (auto tempExpr = dyn_cast<MaterializeTemporaryExpr>(stmt)) {
        return getAssignLhs(tempExpr->getSubExpr());
    } else 
    if (auto parenExpr = dyn_cast<ParenExpr>(stmt)) {
        return getAssignLhs(parenExpr->getSubExpr());
    } else 
    if (auto castExpr = dyn_cast<CastExpr>(stmt)) {
        return getAssignLhs(castExpr->getSubExpr());
    } else
    if (auto callExpr = dyn_cast<CXXMemberCallExpr>(stmt)) {
        Expr* thisExpr = callExpr->getImplicitObjectArgument();
        bool isScIntegerType = isAnyScIntegerRef(thisExpr->getType(), true);
        FunctionDecl* methodDecl = callExpr->getMethodDecl()->getAsFunction();
        string fname = methodDecl->getNameAsString();
        
        if (isScIntegerType && fname.find("operator") != string::npos && (
            fname.find("sc_unsigned") != string::npos ||
            fname.find("int") != string::npos ||
            fname.find("long") != string::npos ||  
            fname.find("bool") != string::npos)) 
        {
            return getAssignLhs(thisExpr);
        }
    } else    
    if (auto compundOper = dyn_cast<CompoundAssignOperator>(stmt)) {
        return compundOper->getLHS();
    } else 
    if (auto binaryOper = dyn_cast<BinaryOperator>(stmt)) {
        auto opcode = binaryOper->getOpcode();
        if (opcode == BO_Assign) {
            return binaryOper->getLHS();
        }
    } else 
    if (auto callExpr= dyn_cast<CXXOperatorCallExpr>(stmt)) {    
        auto opcode = callExpr->getOperator();
        if ((callExpr->isAssignmentOp() && opcode == OO_Equal) ||
            (opcode == OO_PlusEqual || opcode == OO_MinusEqual || 
             opcode == OO_StarEqual || opcode == OO_SlashEqual ||
             opcode == OO_PercentEqual || 
             opcode == OO_GreaterGreaterEqual || opcode == OO_LessLessEqual ||
             opcode == OO_AmpEqual || opcode == OO_PipeEqual || 
             opcode == OO_CaretEqual)) 
        {
            if (callExpr->getNumArgs() != 0) 
                return callExpr->getArg(0);
            else 
                return nullptr;
        }
    }
    return nullptr;
}

// Get variable declaration string in Verilog
string ScVerilogWriter::getVarDeclVerilog(const QualType& type, 
                                          const std::string& varName, 
                                          const Stmt* init) 
{
    // Get the most inner array element type, it needs to run even for 
    // non pointer type if it is @auto
    QualType ctype = type;
    do {
        ctype = ctype->getPointeeOrArrayElementType()->
                getCanonicalTypeInternal();
    } while (ctype->isPointerType() || ctype->isArrayType());
    // Remove reference
    ctype = getDerefType(ctype);
    
    if (isUserClass(ctype)) {
        string s;
        auto recDecl = ctype->getAsRecordDecl();
        bool first = true;

        for (auto fieldDecl : recDecl->fields()) {
            string fieldName = varName + "_" + fieldDecl->getNameAsString();
            s = s + (first ? "" : "\n") + 
                getVarDeclVerilog(fieldDecl->getType(), fieldName);
            first = false;
        }
        return s;
        
    } else {
        string s;
        
        if (auto typeInfo = getIntTraits(ctype)) {
            unsigned width = typeInfo->first;
            bool isUnsigned = typeInfo->second;
            
            // SV integer and unsigned are exactly 32bit
            bool isInt = false;
            bool isUInt = false;
            if (auto btype = dyn_cast<BuiltinType>(ctype.getTypePtr())) {
                auto kind = btype->getKind();
                isInt = width == 32 && kind == BuiltinType::Kind::Int;
                isUInt = width == 32 && kind == BuiltinType::Kind::UInt;
            }

            if (isInt) {
                s = "integer";
            } else 
            if (isUInt) {
                s = "integer unsigned";
            } else 
            if (isUnsigned) {
                if (width > 1) {
                    s = "logic [" + to_string(width-1) + ":0]";
                } else 
                if (width == 1) {
                    s = "logic";
                }
            } else {
                if (width > 1) {
                    s = "logic signed [" + to_string(width-1) + ":0]";
                } else 
                if (width == 1) {
                    s = "logic signed";
                }
            }
            
        } else 
        if (init && isScSigned(ctype)) {
            unsigned width = getExprWidth(init);
            
            if (width > 1) {
                s = "logic signed [" + to_string(width-1) + ":0]";

            } else 
            if (width == 1) {
                s = "logic signed";
 
            } else {
                if (init) {
                    ScDiag::reportScDiag(init->getBeginLoc(), 
                                         ScDiag::SYNTH_ZERO_TYPE_WIDTH) << varName;
                } else {
                    ScDiag::reportScDiag(ScDiag::SYNTH_ZERO_TYPE_WIDTH) << varName;
                }
            }
        } else 
        if (init && isScUnsigned(ctype)) {
            unsigned width = getExprWidth(init);
            
            if (width > 1) {
                s = "logic [" + to_string(width-1) + ":0]";

            } else 
            if (width == 1) {
                s = "logic";

            } else {
                if (init) {
                    ScDiag::reportScDiag(init->getBeginLoc(), 
                                         ScDiag::SYNTH_ZERO_TYPE_WIDTH) << varName;
                } else {
                    ScDiag::reportScDiag(ScDiag::SYNTH_ZERO_TYPE_WIDTH) << varName;
                }
            }
        }    
        
        s = s + " " + varName;
        return s;
    }
}
    
// Get index to provide unique name for local variable
std::string ScVerilogWriter::getUniqueName(const std::string& origVarName) 
{
    //cout << "getUniqueNameIndex for " << origVarName << " val " << val 
    //     << " isNext " << isNext << " " << varNameIndex.size() << endl;
    
    // Get next index for the variable original name
    unsigned index;
    auto ni = varNameIndex.find(origVarName);
    if (ni == varNameIndex.end()) {
        index = 0;
        // Check there is any constructed name equals to this original name,
        // and suffix is numerical
        size_t i = origVarName.length()-1;
        bool numSuffix = true;
        for (; i != 0; i--) {
            const char c = origVarName.at(i);
            if (c == NAME_SUFF_SYM) break;
            numSuffix = numSuffix && isdigit(c);
        }
        // Add index to exclude coincidence
        if (i != 0 && i != origVarName.length()-1 && numSuffix) {
            // Get original name without numerical suffix
            string origVarNameBase = origVarName.substr(0, i);

            if (varNameIndex.find(origVarNameBase) != varNameIndex.end()) {
                index = 1;
            }
        }
    } else {
        index = ni->second + 1;
    }

    // Check name is unique with already constructed and external names
    string nameWithIndex = origVarName + ((index == 0) ? "" : 
                           NAME_SUFF_SYM + to_string(index));
    // Check conflict for previous processes local names if empty sensitivity 
    while (varNameIndex.count(nameWithIndex) || 
           namesGenerator.isTaken(nameWithIndex, emptySensitivity)) {
        index += 1;
        nameWithIndex = origVarName + NAME_SUFF_SYM + to_string(index);
    }

    // Store the index
    auto i = varNameIndex.emplace(origVarName, index);
    if (!i.second) {
        i.first->second = index;
    }
    
    return nameWithIndex;
}

// Get unique read and write names for variable in scope
// \param recvar -- used to provide unique name inside of record instance
// \return <readName, writeName>
std::pair<string, string> ScVerilogWriter::getVarName(const SValue& val)
{
    SCT_TOOL_ASSERT (val.isVariable() || val.isTmpVariable(),
                     "No variable found");
    if (DebugOptions::isEnabled(DebugComponent::doGenName)) {
        cout << endl << "getVarName for val " << val << hex << " #"
             << (val.isVariable() ? val.getVariable().getDecl() : 0) << dec << endl;
    }

    // Do not manage external name, it should be unique by itself
    if (!isCombProcess) {
        //cout << "Get thread name for " << val << endl;
        auto i = varTraits.find(val);
        
//        cout << "varTraits: " << endl;
//        for (auto& e : varTraits) {
//            cout << "   " << e.first << " : " << (e.second.currName ? 
//                  e.second.currName.getValue() : "---") << e.second << endl;
//        }
        
        if (i != varTraits.end()) {
            // There is no name for local variable, 
            // local constant defined in reset has module scope and declaration
            if (i->second.isModuleScope || i->second.isRegister()) {
                SCT_TOOL_ASSERT(i->second.currName, "No current name");
                SCT_TOOL_ASSERT(!i->second.isRegister() || i->second.nextName,
                                "No next name for register");
                // In reset section current name is used, it placed in always_ff
                // Always use next name for @SCT_ASSERT
                string vname = (i->second.isRegister() && 
                               (!isClockThreadReset || parseSvaArg)) ?
                                *i->second.nextName : *i->second.currName;

                // For combinational member variable it needs to create
                // local declaration in reset section to avoid multiple drivers
                // Combinational variable defined at reset section only 
                // not considered here
                if (isClockThreadReset && i->second.isComb()) {
                    extrCombVarUsedRst.insert(val);
                }
                
                if (DebugOptions::isEnabled(DebugComponent::doVerWriter)) {
                    cout << (i->second.isRegister() ? "register " : "") 
                         << (i->second.isModuleScope ? "external " : "") 
                         << "name " << vname << endl;
                }

                if (DebugOptions::isEnabled(DebugComponent::doGenName)) {
                    cout << "   varTraits name " << vname << endl;
                }
                return pair<string, string>(vname, vname);
            }
        }
    }
    
    // CTHREAD used/defined variable can be not found in @varTraits, 
    // for example declared pointer initialized with pointer 
    //cout << "Get external name for " << val << endl;
    auto i = extrValNames.find(val);
    if ( i != extrValNames.end() ) {
//            cout << "extrValNames: " << endl;
//            for (auto& e : extrValNames) {
//                cout << "   " << e.first << " : " << e.second << endl;
//            }

        if (DebugOptions::isEnabled(DebugComponent::doGenName)) {
            cout << "   External name " << i->second << endl;
        }

        return pair<string, string>(i->second, i->second);
    }
    
    // Original name of variable
    string origVarName = val.asString(false);
    // If given name is Verilog keyword add some suffix
    if (VerilogKeywords::check(origVarName)) {
        origVarName += VERILOG_KEYWORD_SUFFIX;
    }

    //cout << "Get local name for " << val << endl;
    
    string finalName;
    if (forLoopInit) {
        // Loop variable which is combinatorial variable considered here, 
        // so next suffix is false
        std::pair<SValue, bool> viKey(val, false);
        
        auto i = varIndex.find(viKey);
        if (i == varIndex.end()) {
            // Get unique name with index 
            finalName = getUniqueName(origVarName);
            // Store name with index 
            varIndex.emplace(viKey, finalName);
        } else {
            finalName = i->second;
        }

        if (DebugOptions::isEnabled(DebugComponent::doGenName)) {
            cout << "   Local loop initializer name " << finalName << endl;
        }
        
    } else {
        // Normal local variable
        // In reset section current name is used, it placed in @always_ff
        bool isNext = isRegister(val) && !isClockThreadReset;
        // @val considers difference between variable in base classes
        pair<SValue, bool> viKey(val, isNext);

        auto i = varIndex.find(viKey);
        if (i == varIndex.end()) {
            // Local record field prefix
            string recPrefix = ScState::getLocalRecName(val);
            if (!recPrefix.empty()) recPrefix = recPrefix + NAME_SUFF_SYM;
            origVarName = (isNext) ? (origVarName + NEXT_VAR_SUFFIX) :
                                     (recPrefix + origVarName);
            // Get unique name with index 
            finalName = getUniqueName(origVarName);
            // Store name with index 
            varIndex.emplace(viKey, finalName);
            
        } else {
            finalName = i->second;
        }
        
        // Add local variable name from empty sensitive METHOD as it is declared
        // in module scope, that provides variable name uniqueness
        if (emptySensitivity) {
            namesGenerator.addTakenName(finalName);
        }
        
        if (DebugOptions::isEnabled(DebugComponent::doGenName)) {
            cout << "   Local name " << finalName << endl;
        }
    }
    // Register local name to avoid name conflict with registers
    namesGenerator.addLocalName(finalName);
    
    return pair<string, string>(finalName, finalName);
}

// Get name for ScChannel, used for port/signal
// \param cval -- channel value
// \return <readName, writeName>
pair<string, string> ScVerilogWriter::getChannelName(const SValue& cval)
{
    if (DebugOptions::isEnabled(DebugComponent::doGenName)) {
        cout << "getScChannelName for val " << cval << endl;
    }

    if (!isCombProcess) {
        // Clocked thread process
        auto i = varTraits.find(cval);
        
//        cout << "getScChannelName for val " << cval << endl;
//        cout << "varTraits: " << endl;
//        for (auto& e : varTraits) {
//            cout << "   " << e.first << " : " << (e.second.currName ? e.second.currName.getValue() : "---") 
//                 << " " << (e.second.nextName ? e.second.nextName.getValue() : "---") << endl;
//        }
        
        // Channel cannot be local variable
        if (i == varTraits.end()) {
            // Check in external names, required for in-process SVA for channels 
            // not used in thread processes, so not @varTraits for them
            auto j = extrValNames.find(cval);
            if (j != extrValNames.end()) {
                //cout << "  external name " << j->second << endl;
                return pair<string, string>(j->second, j->second); 
            }
            
            cout << "getScChannelName for val " << cval << endl;
            cout << "varTraits: " << endl;
            for (auto& e : varTraits) {
                cout << "   " << e.first << " : " << (e.second.currName ?
                        *e.second.currName : "---") << endl;
            }
            SCT_INTERNAL_FATAL_NOLOC ("No channel in VarTraits");
        }

        SCT_TOOL_ASSERT (i->second.currName && 
                         (!i->second.isRegister() || i->second.nextName),
                         "No channel name in VarTraits");
        
        // In reset section current name is used, it placed in @always_ff
        string rdName = (i->second.isCombSig() && isClockThreadReset) ?
                         *i->second.nextName : *i->second.currName;

        // For @sct_comb_signal assign to current value
        string wrName = (((i->second.isRegister() || i->second.isClearSig()) && 
                          !isClockThreadReset) || 
                         (i->second.isCombSig() && isClockThreadReset)) ?
                         *i->second.nextName : *i->second.currName;

        if (DebugOptions::isEnabled(DebugComponent::doGenName)) {
            cout << "   varTraits channel names " << rdName << " " << wrName << endl;
        }
        return pair<string, string>(rdName, wrName);
        
    } else {
        // Method process
        auto i = extrValNames.find(cval);
        
//        for (const auto& i : extrValNames) {
//            cout << "  " << i.first << " : " << i.second << endl;
//        }
        if (i == extrValNames.end()) {
            SCT_INTERNAL_FATAL_NOLOC ("No channel name in state");
        }

        if (DebugOptions::isEnabled(DebugComponent::doGenName)) {
            cout << "   External channel name " << i->second << endl;
        }

        return pair<string, string>(i->second, i->second);
    }
}

// Get indices suffix from record name or empty string
std::string ScVerilogWriter::getIndexFromRecordName(std::string recName) const
{
    auto i = recName.find('[');
    auto j = recName.rfind(']');
    if (i == string::npos) {
        return "";
    } else {
        return recName.substr(i, j-i+1);
    }
}

// Check if variable value is not registered in varTraits and extrValNames 
bool ScVerilogWriter::isLocalVariable(const SValue& val) 
{
    // Empty sensitivity method variables are module members
    if (emptySensitivity) return false;
    
    auto i = varTraits.find(val);
    auto j = extrValNames.find(val);
    bool local = (i == varTraits.end() || !i->second.isModuleScope) && 
                 (j == extrValNames.end());
    return local;
}

// Check is value corresponds to member constant which translated to @localparam
bool ScVerilogWriter::isConstVerVar(const SValue& val) 
{
    if (val.isUnknown()) {
        return false;
    }
    
    auto i = varTraits.find(val);
    return ((i != varTraits.end()) && i->second.isConstVerVar());
} 

// Return true if @val is register in @varTraits, only in splitting thread mode
bool ScVerilogWriter::isRegister(const SValue& val) 
{
    if (val.isUnknown()) {
        return false;
    }
    
    if (!isCombProcess) {
        auto i = varTraits.find(val);
        return ((i != varTraits.end()) && i->second.isRegister());
    }
    return false;
} 

// Check @sct_comb_sig wit CLEAR flag false
bool ScVerilogWriter::isCombSig(const SValue& val) 
{
    if (val.isUnknown()) {
        return false;
    }
    
    if (!isCombProcess) {
        auto i = varTraits.find(val);
        return ((i != varTraits.end()) && i->second.isCombSig());
    }
    return false;
}

// Check @sct_comb_sig with CLEAR flag true
bool ScVerilogWriter::isCombSigClear(const SValue& val) 
{
    if (val.isUnknown()) {
        return false;
    }
    
    if (!isCombProcess) {
        auto i = varTraits.find(val);
        return ((i != varTraits.end()) && i->second.isCombSigClear());
    }
    return false;
}

// Check @sct_clear_sig 
bool ScVerilogWriter::isClearSig(const SValue& val) 
{
    if (val.isUnknown()) {
        return false;
    }
    
    if (!isCombProcess) {
        auto i = varTraits.find(val);
        return ((i != varTraits.end()) && i->second.isClearSig());
    }
    return false;
}

 unsigned ScVerilogWriter::getBracketNum(const string& s) 
{
    // Get number of leading open brackets
    size_t slen = s.length();
    if (slen < 2) return 0;
    
    unsigned leadNum = 0;
    for (; s[leadNum] == '(' && leadNum < slen; leadNum++) {}
    if (leadNum == 0) return 0;
    
    // Get number of trailing close brackets
    unsigned trailNum = 0;
    for (; s[slen-1-trailNum] == ')' && trailNum < slen; trailNum++) {}
    if (trailNum == 0) return 0;
    
    unsigned minNum = leadNum;
    for (size_t i = leadNum; i < slen-trailNum; i++) {
        if (s[i] == '(') {
            leadNum++;
        } else 
        if (s[i] == ')') {
            leadNum--;
        }
        minNum = (minNum > leadNum) ? leadNum : minNum;
    }
    SCT_TOOL_ASSERT (minNum <= trailNum, 
                     "Incorrect bracket number in getBracketNum()");
  
    return minNum;
}

// Check if string is in brackets, return true if it is
bool ScVerilogWriter::isTermInBrackets(const string& s) 
{
    return (getBracketNum(s) != 0);
}

// Remove all leading "(" and tailing ")" brackets in the given string 
string ScVerilogWriter::removeBrackets(const string& s)
{
    unsigned bracketNum = getBracketNum(s);
    return s.substr(bracketNum, s.length()-2*bracketNum);
}

// Remove one leading "{" and one tailing "}" brackets if exist
string ScVerilogWriter::removeCurlyBrackets(const string& s)
{
    size_t slen = s.length();
    
    if (slen < 2 || s[0] != '{' || s[slen-1] != '}') return s;
    
    // Check no other curly brackets inside the expression
    for (size_t i = 1; i < slen-1; i++) {
        if (s[i] == '{' || s[i] == '}') return s;
    }

    return s.substr(1, slen-2);
}

// Remove minus at first position
string ScVerilogWriter::removeLeadMinus(const string& s) 
{
    string res = s;
    if (res.front() == '-') {
        res.erase(0, 1);
    }
    return res;
}

string ScVerilogWriter::addLeadMinus(const string& s) 
{
    // Remove double minus
    if (s.front() == '-') {
        return removeLeadMinus(s);
    }
    string res = string("-") + s;
    return res;
}
 
// Extract signed or unsigned value from given literal string 
std::pair<std::optional<uint64_t>, std::optional<int64_t>> 
ScVerilogWriter::getLiteralVal(const std::string& literStr)
{
    std::pair<std::optional<uint64_t>, 
              std::optional<int64_t>> res(std::nullopt, std::nullopt);
    
    APSInt val(literStr);
    // Do not call @getZExtValue/getExtValue() for value more than 64bits
    SCT_TOOL_ASSERT (val.getActiveBits() <= 64, 
                     "Literal wider 64bit not supported in getLiteralVal()");

    if (val.isSigned()) {
        res.second = val.getExtValue();
    } else {
        res.first = val.getZExtValue();
    }
    return res;
}

// Get absolute value of given literal
uint64_t ScVerilogWriter::getLiteralAbs(const Stmt* stmt) 
{
    auto info = terms.at(stmt);
    auto vals = getLiteralVal(info.str.first);
    uint64_t val = vals.first ? *vals.first : std::abs(*vals.second);
    return val;
}

// Make literal term string in sized form if required
std::string ScVerilogWriter::makeLiteralStr(const std::string& literStr,
                                            char radix, unsigned minCastWidth, 
                                            unsigned lastCastWidth,
                                            CastSign castSign,
                                            bool addNegBrackets)
{
    APSInt val(literStr);
    string s = makeLiteralStr(val, radix, minCastWidth, lastCastWidth, 
                              castSign, addNegBrackets);
    
    return s;
}

std::string ScVerilogWriter::makeLiteralStr(APSInt val,
                                            char radix, unsigned minCastWidth, 
                                            unsigned lastCastWidth,
                                            CastSign castSign, 
                                            bool addNegBrackets)
{
    bool isZero = val.isZero();
    bool isOne = val == 1;
    bool isNegative = val < 0;
    unsigned bitNeeded = getBitsNeeded(val);
//    cout << "  getBitsNeeded " << bitNeeded 
//         << " val.abs().getActiveBits() " << val.abs().getActiveBits() 
//         << " val.getActiveBits() " << val.getActiveBits() << endl;
    
    // It is possible to have no cast for non-negative literal in integer range
    bool valueCast = minCastWidth;
    // Width >32bit required size
    bool isBigWidth = false;
    // Maximal/minimal decimal value is 2147483647/-2147483647, 
    // greater values are represented as hex (required by Lintra)
    if (bitNeeded < (isNegative ? 33 : 32)) {
        if (isZero || isOne) {
            radix = (radix == 2) ? 2 : 10;
        }
    } else {
        radix = 16; valueCast = true; isBigWidth = true;
    }
   
    // Zero value not casted, negative value always considered as signed
    bool isUnsignCast = !isZero && (
                        castSign == CastSign::UCAST && valueCast && !isNegative);
    bool isSignCast = !isZero && (
                      (castSign == CastSign::SCAST && valueCast) || isNegative);
    bool isCast = minCastWidth || lastCastWidth || isSignCast || isUnsignCast;
    
    string baseStr;
    if (isCast || radix != 10) {
        baseStr.append("'");
    }
    if (isSignCast) {
        baseStr.append("s");
    }
    if (isCast || radix != 10) {
        baseStr.append(radix == 10 ? "d" : radix == 16 ? "h" : 
                       radix == 8 ? "o" : "b");
    }
    
//    cout << "  isCast " << isCast << " minCastWidth "
//         << minCastWidth << " isSignCast " << isSignCast << " isUnsignCast " 
//         << isUnsignCast << endl;

    // Negative literal is always based, set base as value width
    if ((isSignCast || isUnsignCast) && !minCastWidth) {
        // +1 bit for sign for positive casted to signed
        minCastWidth = bitNeeded + (isSignCast && !isNegative ? 1 : 0);
    }

    string absVal = isNegative ? sc::APSintToString(APSInt(val.abs()), radix) : 
                                 sc::APSintToString(val, radix);
    // Add size for >32bit literals as some tools required that
    string s = (minCastWidth ? to_string(minCastWidth) : 
                (isBigWidth ? to_string(bitNeeded) : "")) + baseStr + absVal;
    
    if (lastCastWidth && lastCastWidth != minCastWidth) {
        s = to_string(lastCastWidth) + "'(" + s + ')';
    }
    s = string(isNegative ? "-" : "") + s;
    
    if (isNegative && addNegBrackets) {
        s = '(' + s + ')';
    }
//    cout << "  result " << s << endl;

    return s;
}

// Make non-literal term string with sign cast if required
std::string ScVerilogWriter::makeTermStr(const std::string& termStr,
                                         unsigned minCastWidth, 
                                         unsigned lastCastWidth,
                                         CastSign castSign)
{
    string s = termStr;
    
    // Add minimal width cast to cut value
    if (minCastWidth) {
        // Any non-literal term required brackets for cast
        if (!isTermInBrackets(s)) {
            s = '(' + s + ')';
        }
        s = to_string(minCastWidth) + "'" + s;
    }

    // Add last width cast to extend width, used for concatenation
    if (lastCastWidth && lastCastWidth != minCastWidth) {
        // Any non-literal term required brackets for cast
        if (!isTermInBrackets(s)) {
            s = '(' + s + ')';
        }
        s = to_string(lastCastWidth) + "'" + s;
    }

    if (castSign == CastSign::SCAST) {
        if (minCastWidth || lastCastWidth) {
            // Explicit casts w/o sign change implicit cast after and 
            // data type width extended by @extendTypeWidth() cases 
            s = "signed'(" + s + ")";
        } else {
            // Original signed cast by ImplicitCast
            s = "signed'({1'b0, " + s + "})";
        }
    } else 
    if (castSign == CastSign::SACAST) {
        // Artificial signed cast added in @putBinary/@putCompound/@putUnary
        // and explicit cast with sign change implicit cast after
        s = "signed'({1'b0, " + s + "})";
    }
        
    //cout << "makeTermStr " << s << endl;

    return s;
}

// Get @stmt string as RValue, cast optionally applied
// \param skipCast       -- do not add cast for non-literal, 
//                          required for bit/range select argument
// \param addNegBrackets -- add brackets for negative literal, 
//                          used for binary, unary 
pair<string, string> ScVerilogWriter::getTermAsRValue(const Stmt* stmt, 
                                                      bool skipCast, 
                                                      bool addNegBrackets,
                                                      bool doSignCast,
                                                      bool doConcat) 
{
    auto info = terms.at(stmt);

    std::pair<std::string, std::string> names;
    
    if (auto lhsAssign = getAssignLhs(stmt)) {
        // Replace multiple assignment with internal assignment LHS
        // Casts are taken from the original statement 
        names = terms.at(lhsAssign).str;
    } else {
        names = info.str;
    }
        
    string rdName = names.first;
    string wrName = names.second;
    
    SCT_TOOL_ASSERT ((info.minCastWidth == 0) == 
                     (info.lastCastWidth == 0 || !info.explCast), 
                     "Only one of minCastWidth/lastCastWidth is non-zero");
    
    if (info.literRadix) {
        // Apply lastCastWidth only for explicit cast or concatenation
        // For concatenation minimal width taken as minCastWidth if exists, 
        // lastCastWidth if exists (required for replaced constants) or exprWidth
        unsigned lastCastWidth = info.explCast ? info.lastCastWidth  : 0; 
        unsigned minCastWidth = (info.minCastWidth || !doConcat) ? 
                    info.minCastWidth :
                    info.lastCastWidth ? info.lastCastWidth : info.exprWidth; 
        
        rdName = makeLiteralStr(rdName, info.literRadix, minCastWidth, 
                                lastCastWidth, info.castSign, addNegBrackets);
        wrName = rdName;

    } else {
        // Last cast cannot be w/o explicit cast flag
        SCT_TOOL_ASSERT (info.lastCastWidth == 0 || info.explCast, 
                         "Last cast with no explicit cast flag");
        if (!skipCast) {
            CastSign castSign = doSignCast ? info.castSign : CastSign::NOCAST;
            
            rdName = makeTermStr(rdName, info.minCastWidth, info.lastCastWidth, 
                                 castSign);
            wrName = makeTermStr(wrName, info.minCastWidth, info.lastCastWidth, 
                                 castSign);
        }
    }
    //cout << "getTermAsRValue #" << hex << stmt << dec << " rdName " << rdName << endl;
    
    return pair<string,string>(rdName, wrName);
}

// Put/replace string into @terms
void ScVerilogWriter::putString(const Stmt* stmt, 
                                const TermInfo& info)
{
    //cout << "putString #" << hex << stmt << dec << " " << info.str.first
    //     << " exprWidth " << info.exprWidth << endl;
    SCT_TOOL_ASSERT (stmt, "putString stmt is NULL");
    auto i = terms.find(stmt);
    
    if (i != terms.end()) {
        i->second = info;
    } else {
        terms.emplace(stmt, info);
    }
}

// Put/replace string into @terms with given flags
void ScVerilogWriter::putString(const Stmt* stmt, 
                                const pair<string, string>& s, 
                                unsigned exprWidth, bool isChannel)
{
    putString(stmt, TermInfo(s, exprWidth, isChannel));
}

// Put/replace the same string into @terms with empty flags and no range
void ScVerilogWriter::putString(const Stmt* stmt, const string& s,
                                unsigned exprWidth, bool isChannel)
{
    putString(stmt, TermInfo(pair<string, string>(s, s), exprWidth, isChannel));
}

// Add string into @terms string with empty flags, no range and no channel
void ScVerilogWriter::addString(const Stmt* stmt, const string& s)
{
    auto i = terms.find(stmt);
    if (i != terms.end()) {
        auto& info = i->second;
        info.str.first = info.str.first + "; "+ s;
        info.str.second = info.str.second + "; "+ s;
        info.simplTerm = false;
        info.literRadix = 0;
        
    } else {
        //cout << "addString #" << hex << stmt << dec << " " << s << endl;
        terms.emplace(stmt, TermInfo(pair<string, string>(s, s), 0, false));
    }
}

void ScVerilogWriter::clearLiteralTerm(const Stmt* stmt) 
{
    auto i = terms.find(stmt);
    if (i != terms.end()) {
        auto& info = i->second;
        info.literRadix = 0;
    }
}

void ScVerilogWriter::clearSimpleTerm(const Stmt* stmt) 
{
    //cout << "clearSimpleTerm #" << hex << stmt << dec << endl;

    auto i = terms.find(stmt);
    if (i != terms.end()) {
        auto& info = i->second;
        info.simplTerm = false;
    }
}

void ScVerilogWriter::setExprSign(const Stmt* stmt, bool sign) 
{
//    cout << "setExprSign #" << hex << stmt << dec << " sign " << sign << endl;

    auto i = terms.find(stmt);
    if (i != terms.end()) {
        auto& info = i->second;
        info.exprSign = sign ? ExprSign::SEXPR : ExprSign::UEXPR;
    }
}

// Set @incrWidth for given stmt, 
// that means result width is bigger than operand data width 
void ScVerilogWriter::setIncrWidth(const Stmt* stmt) 
{
    //cout << "setIncrWidth #" << hex << stmt << dec << endl;

    auto i = terms.find(stmt);
    if (i != terms.end()) {
        auto& info = i->second;
        info.incrWidth = true;
    }
}

//============================================================================

// Get @incrWidth for given stmt
bool ScVerilogWriter::isIncrWidth(const Stmt* stmt) const
{
    auto i = terms.find(stmt);
    if (i != terms.end()) {
        auto& info = i->second;
        return info.incrWidth;
        
    }
    return false;
}

// Get expression data width from @lastCast or @exprWidth after that or 
// type information at the end
// \param doConcat -- get expression width for concatenation
// \return @exprWidth for given statement or 0 if width unknown
unsigned ScVerilogWriter::getExprWidth(const Stmt* stmt, bool doConcat) {
    
    auto i = terms.find(stmt);
    if (i == terms.end()) {
        return 0;
    }
    auto& info = i->second;
    
    // Get last cast width if exists, required for concatenation
    unsigned width = ((info.explCast || doConcat) && info.lastCastWidth) ? 
                     info.lastCastWidth : info.exprWidth;
    return width;
}

// Get minimal expression data width as minimal of @minCast and @exprWidth
// \return @exprWidth for given statement or 0 if width unknown
unsigned ScVerilogWriter::getMinExprWidth(const Stmt* stmt) {
    
    auto i = terms.find(stmt);
    if (i == terms.end()) {
        return 0;
    }
    auto& info = i->second;
    
    // Get minimal width to avoid part/bit selection outside of variable width
    unsigned width = info.minCastWidth == 0 ? info.exprWidth :
                        (info.exprWidth == 0 ? info.minCastWidth : 
                         min(info.minCastWidth, info.exprWidth));
    return width;
}

// Get expression data width from @lastCast or @exprWidth after that or 
// type information at the end
// \return expression/type width or 64 with error reporting
unsigned ScVerilogWriter::getExprTypeWidth(const Expr* expr, unsigned defWidth) 
{
    unsigned width = getExprWidth(expr);

    if (width == 0) {
        if (auto typeInfo = getIntTraits(getTypeForWidth(expr))) {
            width = typeInfo->first;
            
        } else {
            width = defWidth;
            ScDiag::reportScDiag(expr->getBeginLoc(),
                    ScDiag::SYNTH_UNKNOWN_TYPE_WIDTH) << expr->getType();
        }
    }
    return width;
}

// Get minimal expression data width as minimal of @minCast and @exprWidth
// \return expression/type width or 64 with error reporting
unsigned ScVerilogWriter::getMinExprTypeWidth(const Expr* expr, unsigned defWidth) 
{
    unsigned width = getMinExprWidth(expr);

    if (width == 0) {
        if (auto typeInfo = getIntTraits(getTypeForWidth(expr))) {
            width = typeInfo->first;
            
        } else {
            width = defWidth;
            ScDiag::reportScDiag(expr->getBeginLoc(),
                    ScDiag::SYNTH_UNKNOWN_TYPE_WIDTH) << expr->getType();
        }
    }
    return width;
}

// Get record array indices string
std::string ScVerilogWriter::getRecordIndxs(const vector<SValue>& recarrs) 
{
    string name;
    
    for (int i = recarrs.size()-1; i >= 0; i--) {
        SValue recarr = recarrs.at(i);
        SCT_TOOL_ASSERT (recarr, "Unknown record variable");
        
        name = name + getIndexString(recarr);
    }
    return name;
}

// Put assignment string, record field supported
void ScVerilogWriter::putAssignBase(const Stmt* stmt, const SValue& lval, 
                                   string lhsName, string rhsName, 
                                   unsigned width) 
{
    bool isReg = isRegister(lval) || isCombSig(lval) || isCombSigClear(lval) || 
                 isClearSig(lval);
    bool isRecord = isUserClass(getDerefType(lval.getType()), true);
    // Do not use non-blocking assignment for channel in METHOD
    bool nbAssign = isClockThreadReset && isReg;
    SCT_TOOL_ASSERT (!isRecord, "Record not expected in putAssignBase");
    
    string s;
    
    // No type cast in left part
    if (emptySensitivity) {
        if (emptySensLhsNames.count(lhsName)) {
            ScDiag::reportScDiag(stmt->getSourceRange().getBegin(), 
                                 ScDiag::SYNTH_DUPLICATE_ASSIGN);
        }
        emptySensLhsNames.insert(lhsName);
        
        s = ASSIGN_STMT_SYM + lhsName + ASSIGN_SYM + rhsName;

    } else {
        // No assign COMBSIG with CLEAR in clocked thread reset
        if (isCombSigClear(lval) && isClockThreadReset) {
            clearStmt(stmt);
            return;
        }

        s = lhsName + (nbAssign ? NB_ASSIGN_SYM : ASSIGN_SYM) + rhsName;
    }
    
    putString(stmt, s, width);
    clearSimpleTerm(stmt);
}

//=========================================================================

// Used for statements which produces nothing, like @ImplicitCastExpr
void ScVerilogWriter::copyTerm(const clang::Stmt* srcStmt, 
                               const clang::Stmt* stmt) 
{
    if (skipTerm) return;
    
    // Copy terms from @srcStmt and store for @stmt
    if (terms.count(srcStmt) != 0) {
        // Do not finalize string, it can remain ArrayFCall w/o read/write
        // That is required for implicit cast and other for channel array 
        // access in left part
        putString(stmt, terms.at(srcStmt));
        
        //cout << "+++ " << hex << (unsigned long)stmt << dec << endl;
    } else {
        // Do not check terms as @this/dereference of @this has no term
        /*SCT_INTERNAL_WARNING(stmt->getBeginLoc(), 
                               "copyTerm : No term for sub-statement ");*/
    }
}

// Append term string of scrStmt to existing string of stmt, 
// used for record parameter which has multiple fields
void ScVerilogWriter::addTerm(const clang::Stmt* srcStmt, 
                              const clang::Stmt* stmt) 
{
    if (skipTerm) return;

    // Copy terms from @srcStmt and store for @stmt
    auto i = terms.find(srcStmt);
    if (i != terms.end()) {
        // Function parameter initialization string
        string s = i->second.str.first;
        // @false -- do not add semicolon after previous string
        addString(stmt, s);

    } else {
        // Do not check terms as @this/dereference of @this has no term
        /*SCT_INTERNAL_WARNING(stmt->getBeginLoc(), 
                               "addTerm : No term for sub-statement ");*/
    }
}

// Used for statements in brackets
void ScVerilogWriter::copyTermInBrackets(const clang::Stmt* srcStmt, 
                                         const clang::Stmt* stmt) 
{
    if (skipTerm) return;

    // Copy term from @srcStmt and store to @stmt
    if (terms.count(srcStmt) != 0) {
        auto info = terms.at(srcStmt);
        
        // Add brackets if required only
        bool literSimpleTerm = info.literRadix || info.simplTerm;

        if (!literSimpleTerm && !isTermInBrackets(info.str.first)) {
            info.str.first = '(' + info.str.first + ')';
        }
        if (!literSimpleTerm && !isTermInBrackets(info.str.second)) {
            info.str.second = '(' + info.str.second + ')';
        }
        putString(stmt, info);

    } else {
        // Do not check terms as @this/dereference of @this has no term
        /*SCT_INTERNAL_WARNING(stmt->getBeginLoc(), 
                             "copyTermInBrackets : No term for sub-statement ");*/
    }
}

void ScVerilogWriter::copyTermRemoveBrackets(const clang::Stmt* srcStmt, 
                                             const clang::Stmt* stmt) 
{
    if (skipTerm) return;
    
    // Copy terms from @srcStmt and store for @stmt
    if (terms.count(srcStmt) != 0) {
        // Do not finalize string, it can remain ArrayFCall w/o read/write
        // That is required for implicit cast and other for channel array 
        // access in left part
        auto info = terms.at(srcStmt);
        info.str.first = removeBrackets(info.str.first);
        info.str.second = removeBrackets(info.str.second);
        putString(stmt, info);

    } else {
        // Do not check terms as @this/dereference of @this has no term
        /*SCT_INTERNAL_WARNING(stmt->getBeginLoc(), 
                             "copyTermRemoveBrackets : No term for sub-statement ");*/
    }
}

// ----------------------------------------------------------------------------

// Used for integral types only in explicit type cast statements, which are 
// inheritors of @ExplicitCastExpr
void ScVerilogWriter::putTypeCast(const clang::Stmt* srcStmt, 
                                  const clang::Stmt* stmt,
                                  const clang::QualType& type) 
{
    if (skipTerm) return;

    // Copy in range terms from @srcStmt and store for @stmt
    if (terms.count(srcStmt) != 0) {

        unsigned width = 64; 
        if (auto typeInfo = getIntTraits(type, true)) {
            width = typeInfo->first;
            
        } else {
            ScDiag::reportScDiag(srcStmt->getBeginLoc(),
                                 ScDiag::SYNTH_UNKNOWN_TYPE_WIDTH) << 
                                 type.getAsString();
        }
        SCT_TOOL_ASSERT(width > 0, "Type width is zero");

        auto info = terms.at(srcStmt);
        info.minCastWidth = (info.minCastWidth) ? min(info.minCastWidth, width) : 
                                                  width;
        info.lastCastWidth = width;
        info.explCast = true;
        
//        cout << "putTypeCast stmt #" << hex << stmt << dec  
//             << " minCastWidth " << info.minCastWidth 
//             << " lastCastWidth " << info.lastCastWidth << endl;
        
        putString(stmt, info);
        
    } else {
        cout << "putTypeCast : arg " << hex << (size_t)srcStmt << dec << endl;
        SCT_INTERNAL_FATAL(stmt->getBeginLoc(),
                           "putTypeCast : No term for sub-statement ");
    }
}

// Put sign cast for literals and expressions
void ScVerilogWriter::putSignCast(const clang::Stmt* stmt, CastSign castSign)
{
    if (skipTerm || skipSignCast) return;

    if (terms.count(stmt)) {
        auto& info = terms.at(stmt);
        // No signed cast for signed expression
        if (info.exprSign != ExprSign::SEXPR) {
            // Apply SACAST after explicit cast to get @signed'({1'b0, ...});
            info.castSign = info.explCast && castSign == CastSign::SCAST ? 
                            CastSign::SACAST : castSign;
//            cout << "putSignCast #" << hex << stmt << dec << " castSign " 
//                 << (int)castSign << endl;
        }
    } else {
        cout << "putSignCast : arg " << hex << (size_t)stmt << dec << endl;
        SCT_INTERNAL_FATAL(stmt->getBeginLoc(), 
                           "putSignCast : No term for statement ");
    }
}

void ScVerilogWriter::putBoolCast(const clang::Stmt* stmt)
{
    if (skipTerm || skipSignCast) return;

    if (terms.count(stmt)) {
        auto& info = terms.at(stmt);
        info.castSign = CastSign::BCAST;

    } else {
        cout << "putBoolCast : arg " << hex << (size_t)stmt << dec << endl;
        SCT_INTERNAL_FATAL(stmt->getBeginLoc(), 
                           "putBoolCast : No term for statement ");
    }
}

// Set cast width for variables/expressions replaced by value,
// used in concatenation
void ScVerilogWriter::setReplacedCastWidth(const clang::Stmt* stmt,
                                           const clang::QualType& type) 
{
    if (skipTerm) return;

    // Copy in range terms from @srcStmt and store for @stmt
    if (terms.count(stmt) != 0) {

        unsigned width = 64;
        if (auto typeInfo = getIntTraits(type, true)) {
            width = typeInfo->first;
            
        } else {
            ScDiag::reportScDiag(stmt->getBeginLoc(),
                                 ScDiag::SYNTH_UNKNOWN_TYPE_WIDTH) << 
                                 type.getAsString();
        }
        SCT_TOOL_ASSERT(width > 0, "Type width is zero");

        auto& info = terms.at(stmt);
        info.lastCastWidth = width;
        info.explCast = false;
        
//        cout << "setLastCastWidth stmt #" << hex << stmt << dec  
//             << " lastCastWidth " << info.lastCastWidth << endl;
        
    } else {
        cout << "setLastCastWidth : arg " << hex << (size_t)stmt << dec << endl;
        SCT_INTERNAL_FATAL(stmt->getBeginLoc(),
                           "setLastCastWidth : No term for statement ");
    }
}

// Extend type width for arithmetic operation  argument self-determined in SV,
// this is type cast to given @width
void ScVerilogWriter::extendTypeWidth(const clang::Stmt* stmt,
                                      unsigned width)
{
    if (skipTerm) return;

    //cout << "extendTypeWidth #" << hex << stmt << dec << " width " << width << endl;

    // Do not extend width if it has explicit type cast
    if (terms.count(stmt) != 0) {
        auto info = terms.at(stmt);
        info.minCastWidth = (info.minCastWidth) ? info.minCastWidth : width;
        info.lastCastWidth = (info.lastCastWidth) ? info.lastCastWidth : width;
        // Extending type considered as type cast
        info.explCast = true;
        //cout << "exprSign " << (int)info.exprSign << endl;
        putString(stmt, info);
            
    } else {
        cout << "addTypeCast : arg " << hex << (size_t)stmt << dec << endl;
        SCT_INTERNAL_FATAL(stmt->getBeginLoc(),
                           "addTypeCast : No term for sub-statement ");
    }
}

// Used for literals
void ScVerilogWriter::putLiteral(const Stmt* stmt, const SValue& val) 
{
    if (skipTerm) return;

    if (val.isInteger()) {
        string s = sc::APSintToString(val.getInteger(), 10);
        unsigned width = getBitsNeeded(val.getInteger());
        char radix = val.getRadix() == 100 ? 10 : val.getRadix();
        
//        cout << "putLiteral stmt #" << hex << stmt << dec << " val " << val 
//             << " s " << s << " width " << width << endl;
        
        putString(stmt, s, width);
        auto& sinfo = terms.at(stmt);
        sinfo.literRadix = radix;
        
        if (DebugOptions::isEnabled(DebugComponent::doVerWriter)) {
            cout << "putLiteral for stmt " << hex << stmt << dec << ", " << s << endl;
        }
    } else {
        SCT_INTERNAL_FATAL(stmt->getBeginLoc(),
                         "putLiteral : Unsupported literal "+ val.asString());
    }
}

// Put local variable (non-array) declaration with possible initialization
void ScVerilogWriter::putVarDecl(const Stmt* stmt, const SValue& val, 
                                 const QualType& type, const Expr* init,
                                 bool funcCall, unsigned level, 
                                 bool replaceConstEnable) 
{
    if (skipTerm) return;
    SCT_TOOL_ASSERT (val.isVariable() || val.isTmpVariable(), 
                     "No variable found to declare");
    SCT_TOOL_ASSERT (!val.isReference() || val.isConstReference(), 
                     "No non-constant reference expected to be declared");
    
    if (checkUnsigned && isSignedType(type) && stmt) {
        ScDiag::reportScDiag(stmt->getBeginLoc(), 
                             ScDiag::SYNTH_UNSIGNED_MODE_DECL);
    }
    
    if (forLoopInit) {
        // For loop counter variable declaration in initialization section
        if (init) {
            if (terms.count(init) == 0) {
                SCT_INTERNAL_FATAL(stmt->getBeginLoc(),
                                   "No term for loop variable initialization");
            }
            //  Loop iterator is combinational variable, so has the same names
            auto names = getVarName(val);
            string s = getVarDeclVerilog(type, names.first) + 
                       ASSIGN_SYM + getTermAsRValue(init).first;
            putString(stmt, s, 0);
            clearSimpleTerm(stmt);
            
            // Register assignment statement, for declared variables only
            putVarAssignStmt(val, stmt);
            
            // Remove the variable declaration
            auto j = localDeclVerilog.begin();
            for (; j != localDeclVerilog.end(); ++j) {
                if (j->first == val) {
                    break;
                }
            }
            if (j != localDeclVerilog.end()) {
                localDeclVerilog.erase(j);
            }
            
        } else {
            putString(stmt, string(""), 0);
            clearSimpleTerm(stmt);
            ScDiag::reportScDiag(ScDiag::CPP_FOR_WITHOUT_INIT);
        }
        
    } else {
        // Normal variable declaration
        bool isReg = isRegister(val);
        bool isRecord = isUserClass(getDerefType(val.getType()), true);
        // Initialization of local variable required for variables w/o initializer
        // and variables at no-zero level (funcCall variable is initialized)
        bool initZero = (isCombProcess && !emptySensitivity && 
                         (level > 0 || (!init && !funcCall))) ||
                        (!isCombProcess && !isClockThreadReset && 
                         (level > 1 || (!init && !funcCall)));
        // Local non-reset accessed register variables
        bool initRegZero = !isCombProcess && !isClockThreadReset && isReg;
        // Initialization of non-initialized reset local variables
        // Ignore temporary variables here as they must be initialized anyway
        bool initResetZero = !isCombProcess && isClockThreadReset && 
                              (!init && !funcCall) && val.isVariable();
        
        SCT_TOOL_ASSERT (!isRecord || !init,
                         "Unexpected record variable with initialization");
        
        //cout << "putVarDecl val " << val << " isRecord " << isRecord
        //     << " initRegZero " << initRegZero << " initLocalRegs " << initLocalRegs << endl;
        
        // Get variable name
        pair<string, string> names = getVarName(val);
        string varName;
        if (isReg) {
            // Name for local register initialization in reset is current name 
            auto i = varTraits.find(val);
            SCT_TOOL_ASSERT (i != varTraits.end(), "No varTraits for register variable");
            varName = *(i->second.currName);
        } else {
            varName = names.first;
        }
        
        // Do not declare/initialize record as individual fields are declared 
        if (!isRecord) {
            // Register assignment statement, for declared variables only
            putVarAssignStmt(val, stmt);

            // Combinatorial variables and combinational process variables
            auto i = localDeclVerilog.begin();
            for (; i != localDeclVerilog.end(); ++i) {
                if (i->first == val) {
                    break;
                }
            }
            if (i == localDeclVerilog.end()) {
                string s = getVarDeclVerilog(type, varName, init);
                //cout << "put to localDeclVerilog val " << val << endl;
                localDeclVerilog.emplace_back(val, s);

                // Add variable initialization with zero
                if ((initZero && initLocalVars) || 
                    (initResetZero && initResetLocalVars) ||
                    (initRegZero && initLocalRegs)) {

                    string s = varName + (isReg ? NB_ASSIGN_SYM : ASSIGN_SYM) + "0";
                    localDeclInitVerilog.emplace_back(val, s);
                    //cout << "put to localDeclInitVerilog val " << val << " " << s << endl;
                }
            }

            // If variable declaration is removed, no initialization required
            bool removeUnusedInit = false;
            if (!isCombProcess) {
                auto i = varTraits.find(val);
                if (i != varTraits.end()) {
                    removeUnusedInit = isClockThreadReset ? 
                        (!i->second.isAccessAfterReset() && 
                         !i->second.isAccessInReset()) :
                        (i->second.isReadOnlyCDR() || 
                         !i->second.isAccessAfterReset());
                }
            }

            // Write variable initialization, already done for isConst && !isReg
            if (stmt && init && !removeUnusedInit) {
                if (terms.count(init) == 0) {
                    SCT_INTERNAL_FATAL (init->getBeginLoc(),
                                        "No term for variable initialization");
                }
                
                // Use read name in @assign (emptySensitivity)
                bool secName = !isClockThreadReset && isReg && !emptySensitivity;
                string lhsName = secName ? names.second : names.first;
                string rhsName = getTermAsRValue(init).first;

                putAssignBase(stmt, val, lhsName, rhsName, 0);
            }
        }
    }
}

// Array declaration statement, array initialization added as separate
// assignments for @stmt  
void ScVerilogWriter::putArrayDecl(const Stmt* stmt, const SValue& val, 
                                   const QualType& type, 
                                   const vector<size_t>& arrSizes,
                                   const Expr* init,
                                   unsigned level) 
{
    if (skipTerm) return;
    SCT_TOOL_ASSERT (val.isVariable() || val.isTmpVariable(), 
                     "No variable found");
    
    bool isReg = isRegister(val);
    bool isRecord = isUserDefinedClassArray(val.getType(), true);
    //cout << "putArrayDecl val " << val << " " << isReg << isRecord << endl;
    
    // Initialization of local variable required for variables w/o initializer
    // and variables at no-zero level
    bool initZero = (isCombProcess && !emptySensitivity && 
                     (level > 0 || !init)) ||
                    (!isCombProcess && !isClockThreadReset && 
                     (level > 1 || !init));
    // Local non-reset accessed register variables
    bool initRegZero = !isCombProcess && !isClockThreadReset && isReg;
    // Initialization of non-initialized reset local variables
    bool initResetZero = !isCombProcess && isClockThreadReset && !init;
    
    //cout << "putArrayDecl val " << val << " isRecord " << isRecord
    //     << " initRegZero " << initRegZero << " initResetZero " << initResetZero << endl;
    
    // Do not declare record as individual fields are declared
    if (!isRecord) {
        // Register assignment statement, for declared variables only
        if (!isReg) putVarAssignStmt(val, stmt);
        
        // Combinatorial variables and combinational process variables
        auto i = localDeclVerilog.begin();
        for (; i != localDeclVerilog.end(); ++i) {
            if (i->first == val) {
                break;
            }
        }
        
        if (i == localDeclVerilog.end()) {
            // Get variable name
            pair<string, string> names = getVarName(val);
            string varName;
            if (isReg) {
                // Name for local register initialization in reset is current name 
                auto i = varTraits.find(val);
                SCT_TOOL_ASSERT (i != varTraits.end(), "No varTraits for register variable");
                varName = *(i->second.currName);
            } else {
                varName = names.first;
            }
            
            // Get array element indices
            string indx;
            for (auto j : arrSizes) {
                indx = indx + '['+to_string(j)+']';
            }
            
            // Array variable declaration, no declaration for registers
            string s = getVarDeclVerilog(type, varName) + indx;
            localDeclVerilog.emplace_back(val, s);
            //cout << "   " << s << endl;
            
            // Add array initialization with zero
            if ((initZero && initLocalVars) || 
                (initResetZero && initResetLocalVars) ||
                (initRegZero && initLocalRegs)) {
                
                string s;
                size_t elmnum = getArrayElementNumber(arrSizes);
                
                for (size_t i = 0; i < elmnum; i++) {
                    string indStr;
                    auto indices = getArrayIndices(arrSizes, i);
                    
                    for (size_t indx : indices) {
                        indStr += "[" + to_string(indx) + "]";
                    }
                    
                    s += varName + indStr + (isReg ? NB_ASSIGN_SYM : ASSIGN_SYM) + 
                         "0" + ((i == elmnum-1) ? "" : "; ");
                }

                localDeclInitVerilog.emplace_back(val, s);
            }
        } 
    }
}

// Put string of @init statement to use instead of the reference variable
// Used for any non-constant reference 
void ScVerilogWriter::storeRefVarDecl(const SValue& val, const Expr* init, 
                                      bool checkNoTerms) 
{
    if (skipTerm) return;
    SCT_TOOL_ASSERT (val.isVariable() || val.isTmpVariable(), "No variable found");

    if (terms.count(init) != 0) {
        // Replace reference with new string, required for second call of the function
        refValueDecl[val] = getTermAsRValue(init);
        //cout << "storeRefVarDecl val " << val << " init " << refValueDecl[val].first << endl;
        
    } else {
        if (checkNoTerms) {
            cout << "putRefVarDecl : arg " << hex << (size_t)init << dec << endl;
            SCT_INTERNAL_FATAL(init->getBeginLoc(),
                               "putRefVarDecl : no term for right part ");
        } else {
            // Do nothing, that is for reference to channel record
        }
    }
}

// Put referenced variable name to use instead of the reference variable
// Used for @rval temporary variable with constant reference value only
void ScVerilogWriter::storeRefVarDecl(const SValue& val, const SValue& rval) 
{
    if (skipTerm) return;
    SCT_TOOL_ASSERT (val.isVariable() || val.isTmpVariable(), 
                     "No left variable found");
    SCT_TOOL_ASSERT (rval.isVariable() || rval.isTmpVariable(), 
                     "No right variable found");
    //cout << "putRefVarDecl val " << val << " rval " << rval << endl;

    // Replace reference with new string, required for second call of the function
    refValueDecl[val] = getVarName(rval); 
}

// Put string of @init statement to use instead of the pointer variable
// Used for array of pointers at unknown index, it cannot be de-referenced
void ScVerilogWriter::storePointerVarDecl(const SValue& val, const Expr* init) 
{
    if (skipTerm) return;
    SCT_TOOL_ASSERT (val.isVariable() || val.isTmpVariable(), "No variable found");
    
    if (terms.count(init) != 0) {
        // Replace pointer with new string, required for second call of the function
        ptrValueDecl[val] = getTermAsRValue(init);
        
    } else {
        cout << "storePointerVarDecl : arg " << hex << (size_t)init << dec << endl;
        SCT_INTERNAL_FATAL(init->getBeginLoc(),
                           "storePointerVarDecl : no term for right part ");
    }
}

// Put string for pointer variable into @ptrValueDecl, 
// @rval can be pointee variable or another pointer as well
// \param cval -- channel value if @rval is channel variable/pointer or NO_VALUE
void ScVerilogWriter::storePointerVarDecl(const SValue& val, const SValue& rval, 
                                        const SValue& cval) 
{
    if (skipTerm) return;
    SCT_TOOL_ASSERT (val.isVariable() || val.isTmpVariable(), "No variable found");
    //cout << "storePointerVarDecl val " << val << " rval " << rval << " cval " << cval << endl;
    
    if (rval.isVariable() || rval.isTmpVariable()) {
        // Replace pointer with new string, required for second call of the function
        if (ptrValueDecl.count(rval)) {
            ptrValueDecl[val] = ptrValueDecl.at(rval);
            
        } else {
            if (cval) {
                ptrValueDecl[val] = getChannelName(cval);
            } else {
                ptrValueDecl[val] = getVarName(rval);
            }
        }
        
    } else {
        // Put empty string, in this case pointer variable/object name used
        ptrValueDecl[val] = pair<string, string>("", "");
    }
}

// Any access to member/local variable/local record field variable 
// and any other expression
// \param recarr -- record array, used to get indices string
// \param elemOfMifArr -- put member of current element of MIF array 
// \param elemOfRecArr -- put member of a element of a record array 
void ScVerilogWriter::putValueExpr(const Stmt* stmt, const SValue& val,
                                   const SValue& recarr,
                                   bool elemOfMifArr, bool elemOfRecArr,
                                   const string& refRecarrIndxStr)
{
    std::vector<SValue> recarrs;
    if (recarr) {
        recarrs.push_back(recarr);
    }
    putValueExpr(stmt, val, recarrs, elemOfMifArr, elemOfRecArr, 
                 refRecarrIndxStr);
}

// Any access to member/local variable/local record field variable 
// and any other expression
// \param recarrs -- vector of record arrays, used to get indices string
// \param elemOfMifArr -- put member of current element of MIF array 
// \param elemOfRecArr -- put member of a element of a record/MIF array 
// \param refRecarrIndxStr -- record array parameter passed by reference indices
// \param portMifarrIndxStr -- MIF array parent accessed through sc_port
void ScVerilogWriter::putValueExpr(const Stmt* stmt, const SValue& val,
                                   const vector<SValue>& recarrs, 
                                   bool elemOfMifArr, bool elemOfRecArr,
                                   const string& refRecarrIndxStr, 
                                   const string& portMifarrIndxStr)
{
    if (skipTerm) return;
//    cout << "putValueExpr for stmt " << hex << stmt << dec << ", val " << val 
//         << ", elemMifArr " << elemOfMifArr << ", elemRecArr " << elemOfRecArr 
//         << ", recarrs size " << recarrs.size()
//         << " refRecarrIndxStr " << refRecarrIndxStr << endl;

    if (val.isInteger()) {
        // Integer put for evaluated expressions
        putLiteral(stmt, val);
        
    } else 
    if (val.isVariable() || val.isTmpVariable()) {
        pair<string, string> names = getVarName(val);
        //cout << "val " << val << " name " << names.first << endl;
        
        // Do not use MIF indices in reset section as variable is declared locally
        bool isExtrCombVarNoIndx = isClockThreadReset && 
                                   extrCombVarUsedRst.count(val) != 0;
        
        // Add MIF variable prefix for its member access
        if (elemOfMifArr && !isExtrCombVarNoIndx) {
            // Access to member of MIF from its process body 
            // No index for local variables 
            if (!isLocalVariable(val)) {
                string indxSuff = MIFValueName.second;
                names.first += indxSuff;
                names.second += indxSuff;
                //cout << "  MIF array add suffix " << indxSuff << endl;
            }
        }
        
        // Add record/MIF variable prefix for its member access
        if (elemOfRecArr) {
            // Access to member of record/MIF from this record method called
            // possible from parent module process
            string indxSuff = recordValueName.second;
            names.first += indxSuff;
            names.second += indxSuff;
            //cout << "  REC from its method suffix " << indxSuff << endl;
        } 
        
        // Add record/MIF variable prefix for its member access
        if (!recarrs.empty()) {
            // Access record/MIF member from module function or access 
            // record array element
            // Add record array parameter passed by reference indices
            // Commented to support array record with array member func parameter
            //if (isLocalVariable(val)) {
                names.first += refRecarrIndxStr;
                names.second += refRecarrIndxStr;
            //}
            //cout << "  isLocalVariable " << val << " " << isLocalVariable(val) << endl;
            
            // Record array index inside, optionally erase indices in @arraySubIndices
            string indxSuff = getRecordIndxs(recarrs);
            names.first += indxSuff;
            names.second += indxSuff;
            //cout << "  REC from module suffix " << refRecarrIndxStr << indxSuff
            //     << " names.first " << names.first << endl;
        }
        
        if (!portMifarrIndxStr.empty()) {
            names.first  += portMifarrIndxStr;
            names.second += portMifarrIndxStr;
        }
        
        // Clear record array indices for static member, required to avoid
        // indices applied to the next term 
        if (val.isVariable()) {
            if (auto varDecl = dyn_cast<VarDecl>(val.getVariable().getDecl())) {
                if (varDecl->isStaticDataMember() && !arraySubIndices.empty()) {
                    arraySubIndices.clear();
                }
            }
        }
        
        // Type width can be not determinable for example for
        // MIF/record array or vector of vectors
        unsigned width = 0;
        if (auto typeInfo = getIntTraits(val.getType(), true)) {
            width = typeInfo->first;
        }
        
        // Get unique variable name
        putString(stmt, names, width);

        //cout << "putValueExpr val " << val << " width " << width << endl;
        //cout << "+++ " << hex << (unsigned long)stmt << dec << endl;        
    } else 
    if (val.isObject()) {
        // Object, may be pointer to some object, do nothing as this object is 
        // intermediate one, will be de-referenced before use
    
    } else 
    if (val.isScChannel()) {
        // Port/signal 
        SCT_TOOL_ASSERT (false, "Channel object not expected");
        
    } else {
        cout << stmt->getBeginLoc().printToString(sm) << endl;
        SCT_TOOL_ASSERT (false, "No value found");
    }
}

// Any access to channel
void ScVerilogWriter::putChannelExpr(const Stmt* stmt, const SValue& val,
                                     const SValue& recarr,
                                     bool elemOfMifArr, bool elemOfRecArr)
{
    std::vector<SValue> recarrs;
    if (recarr) {
        recarrs.push_back(recarr);
    }
    putChannelExpr(stmt, val, recarrs, elemOfMifArr, elemOfRecArr);
}

// Any access to channel
// \param elemOfMifArr -- put member of current element of MIF array 
void ScVerilogWriter::putChannelExpr(const Stmt* stmt, const SValue& cval,
                                     const vector<SValue>& recarrs,
                                     bool elemOfMifArr, bool elemOfRecArr,
                                     const std::string& portMifarrIndxStr)
{
    if (skipTerm) return;
//    cout << "putChannelExpr for stmt " << hex << stmt << dec << ", cval " << cval 
//         << ", elemMifArr " << elemOfMifArr << ", elemRecArr " << elemOfRecArr 
//         << ", recvars size " << recarrs.size() << " arraySubIndices.size "
//         << arraySubIndices.size() << endl;
    
    SCT_TOOL_ASSERT (cval.isScChannel(), "No channel found");

    pair<string, string> names = getChannelName(cval);
    //cout << "  getScChannelName " << names.first << " " << names.second << endl;
    
    // Add MIF variable prefix for its member access
    if (elemOfMifArr) {
        // Access to member of MIF from its process body
        string indxSuff = MIFValueName.second;
        names.first += indxSuff;
        names.second += indxSuff;
        //cout << "  elem of MIF array add suffix " << indxSuff << endl;
    }
    
    // Add record/MIF variable prefix for its member access
    if (elemOfRecArr) {
        // Access to member of record/MIF from this record/MIF called method
        string indxSuff = recordValueName.second;
        names.first += indxSuff;
        names.second += indxSuff;
        //cout << "  REC array add suffix " << indxSuff << endl;
    }  
    
    // Add record/MIF variable prefix for its member access
    if ( !recarrs.empty() ) {
        // Access record/MIF member from module function
        // Get indices for most all records
        string indxSuff = getRecordIndxs(recarrs);
        names.first += indxSuff;
        names.second += indxSuff;
        //cout << "  recvar array add suffix " << indxSuff << endl;
    }
    
    if (!portMifarrIndxStr.empty()) {
        names.first  += portMifarrIndxStr;
        names.second += portMifarrIndxStr;
    }

    // Get variable width, channel must always have determinable width 
    unsigned width = 0;
    QualType ctype = cval.getScChannel()->getType();
    if (auto typeInfo = getIntTraits(ctype)) {
        width = typeInfo->first;
    } else {
        ScDiag::reportScDiag(stmt->getBeginLoc(),
                             ScDiag::SYNTH_UNKNOWN_TYPE_WIDTH) << 
                             ctype.getAsString();
    }
    
    // @true as it is a channel 
    putString(stmt, names, width, true);
    
    //cout << "+++ " << hex << (unsigned long)stmt << dec << endl;
}


// Try to put local reference variable, 
// \return true if @val is local reference
bool ScVerilogWriter::putLocalRefValueExpr(const Stmt* stmt, const SValue& val)
{
    if (skipTerm) return true;

    if (refValueDecl.count(val)) {
        auto names = refValueDecl.at(val);

        // Get variable width 
        unsigned width = 0;
        if (auto typeInfo = getIntTraits(val.getType().getNonReferenceType())) {
            width = typeInfo->first;
        }
        
        putString(stmt, names, width);

        //cout << "putLocalPtrValueExpr #" << hex << stmt << dec << " val " << val 
        //     << " rdName " << names.first << endl;
        return true;
    }
    return false;
}

// Try to put local pointer parameter variable, 
// \return true if @val is local pointer parameter
bool ScVerilogWriter::putLocalPtrValueExpr(const Stmt* stmt, const SValue& val)
{
    if (skipTerm) return true;

    if (ptrValueDecl.count(val)) {
        auto names = ptrValueDecl.at(val);
        if (names.first.empty()) {
            return false;
        }

        // Get variable width 
        unsigned width = 0;
        if (auto typeInfo = getIntTraits(val.getType(), true)) {
            width = typeInfo->first;
        }

        putString(stmt, names, width);
        
        //cout << "putLocalPtrValueExpr #" << hex << stmt << dec << " val " << val << " width " << width << endl;
        return true;
    }
    return false;
}

// Assignment statement, used for general purpose
void ScVerilogWriter::putAssign(const Stmt* stmt, const SValue& lval, 
                                const Expr* lhs, const Expr* rhs) 
{
    //cout << "putAssign for " << hex << (unsigned long)stmt << " lhs " << (unsigned long)lhs  << " rhs " << (unsigned long)rhs << dec << endl;
    if (skipTerm) return;

    if (terms.count(rhs) != 0) {
        if (terms.count(lhs) != 0) {
            auto info = terms.at(lhs);
                    
            // Get LHS names
            auto names = info.str;

            bool isReg = isRegister(lval) || isCombSig(lval) || 
                         isCombSigClear(lval) || isClearSig(lval);
            bool isRecord = isUserClass(getDerefType(lval.getType()), true);
            SCT_TOOL_ASSERT (!isRecord, "Unexpected record variable");
            
            // Use read name in @assign (emptySensitivity)
            bool secName = !isClockThreadReset && isReg && !emptySensitivity;
            string lhsName = removeBrackets(secName ? names.second:names.first);
            string rhsName = removeBrackets(getTermAsRValue(rhs).first);

            unsigned width = getExprWidth(lhs); 

            putAssignBase(stmt, lval, lhsName, rhsName, width);
            //cout << "+++ " << hex << (unsigned long)stmt << dec << endl;
            
        } else {
            cout << "putAssign : arg " << hex << (size_t)lhs << dec << endl;
            SCT_INTERNAL_FATAL(stmt->getBeginLoc(),
                               "putAssign : no term for left part ");
        }
    } else {
        cout << "putAssign : arg " << hex << (size_t)rhs << dec << endl;
        SCT_INTERNAL_FATAL(stmt->getBeginLoc(),
                           "putAssign : no term for right part ");
    }
}

// Assignment statement, for non-channel @lval only 
// Use when there is no expression for @lval, i.e. if @lval is artificial
void ScVerilogWriter::putAssign(const Stmt* stmt, const SValue& lval, 
                                const Expr* rhs)
{
    if (skipTerm) return;

    if (terms.count(rhs) != 0) {
        // For non-channel variables only
        SCT_TOOL_ASSERT (lval.isVariable() || lval.isTmpVariable(),
                         "No variable found");

        // Get LHS names
        auto names = getVarName(lval); 

        bool isReg = isRegister(lval) || isCombSig(lval) || 
                     isCombSigClear(lval) || isClearSig(lval);
        bool isRecord = isUserClass(getDerefType(lval.getType()), true);
        SCT_TOOL_ASSERT (!isRecord, "Unexpected record variable");

        // Use read name in @assign (emptySensitivity)
        bool secName = !isClockThreadReset && isReg && !emptySensitivity;
        string lhsName = removeBrackets(secName ? names.second : names.first);
        string rhsName = removeBrackets(getTermAsRValue(rhs).first);
        
        // Get LHS variable width 
        unsigned width = 0;
        if (auto typeInfo = getIntTraits(lval.getType())) {
            width = typeInfo->first;
        }
                
        putAssignBase(stmt, lval, lhsName, rhsName, width);
        
    } else {
        cout << "putAssign : arg " << hex << (size_t)rhs << dec << endl;
        SCT_INTERNAL_FATAL(stmt->getBeginLoc(),
                         "putAssign : no term for right part ");
    }
}

// Assignment for record variable (record copy)
void ScVerilogWriter::putRecordAssign(const Stmt* stmt, 
                                      const SValue& lvar,
                                      const SValue& lrec, const SValue& rrec,
                                      bool lelemOfMifRecArr, bool relemOfMifRecArr,
                                      string lrecSuffix,
                                      string rrecSuffix) 
{
    //cout << "putRecordAssign lvar " << lvar << " lrec " << lrec << ", rrec " << rrec << endl;
    //cout << "recSuffix " << lrecSuffix << " " << rrecSuffix << endl;
    
    std::vector<SValue> lrecFields;
    if (lrec.isScChannel()) {
        auto recType = dyn_cast<const RecordType>(lrec.getScChannel()->getType().
                                                  getTypePtr());
        getFieldsForRecordChan(recType->getAsRecordDecl(),lrec, lrecFields);
    } else {
        getFieldsForRecord(lrec, lrecFields);
    }
    std::vector<SValue> rrecFields;
    if (rrec.isScChannel()) {
        auto recType = dyn_cast<const RecordType>(rrec.getScChannel()->getType().
                                                  getTypePtr());
        getFieldsForRecordChan(recType->getAsRecordDecl(), rrec, rrecFields);
    } else {
        getFieldsForRecord(rrec, rrecFields);
    }
    //for (const SValue& f : lrecFields) cout << "  " << f << endl;
    //for (const SValue& f : rrecFields) cout << "  " << f << endl;
    
    // Check one of the fields
    // No assign COMBSIG with CLEAR in clocked thread reset
    SValue lfval = lrecFields.back();
    if (isCombSigClear(lfval) && isClockThreadReset) {
        clearStmt(stmt);
        return;
    }
    
    // Record can be inside MIF but not another record, so MIF indices are
    // in @recordValueName or/and @MIFValueName, add one of them
    if (lelemOfMifRecArr) {
        if (recordValueName.first) {
            lrecSuffix = recordValueName.second + lrecSuffix;
        } else 
        if (MIFValueName.first) {
            lrecSuffix = MIFValueName.second + lrecSuffix;
        }
    }
    if (relemOfMifRecArr) {
        if (recordValueName.first) {
            rrecSuffix = recordValueName.second + rrecSuffix;
        } else 
        if (MIFValueName.first) {
            rrecSuffix = MIFValueName.second + rrecSuffix;
        }
    }

//    cout << "varTraits: " << endl;
//    for (auto& e : varTraits) {
//        cout << "   " << e.first << " : " << (e.second.currName ? e.second.currName.getValue() : "---") 
//             << " " << (e.second.nextName ? e.second.nextName.getValue() : "---") 
//             << " isReg " << e.second.isRegister() << endl;
//    }
 
    // Check for register variable and first field (required for record channel)
    bool isReg = isRegister(lvar) || isCombSig(lvar) || 
                 isCombSigClear(lvar) || isClearSig(lvar); 
    //cout << "  lvar " << lvar << " isReg " << isReg << endl;
    
    // Record assignment, assign all the fields
    string s;
    bool first = true;
    for (unsigned i = 0; i != lrecFields.size(); ++i) {
        const SValue& lfval = lrecFields[i];
        const SValue& rfval = rrecFields[i];
        //cout << "  lfval " << lfval << "  rfval " << rfval << endl;
        //cout << "  lfval " << lfval << " isReg " << (isReg || isRegister(lfval)) << endl;

        // Skip zero width type
        auto ftype = lfval.getType();
        if (isZeroWidthType(ftype) || isZeroWidthArrayType(ftype)) continue;

        bool nbAssign = isClockThreadReset && (isReg || isRegister(lfval));
        bool secName  = !isClockThreadReset && (isReg || isRegister(lfval)) && 
                        !emptySensitivity;
       
        // Get name for LHS
        const auto& lnames = lrec.isScChannel() ? 
                             getChannelName(lfval) : getVarName(lfval);
        string lhsName = (secName ? lnames.second : lnames.first) + lrecSuffix;
        
        // Get name for RHS, use read name here
        const auto& rnames = rrec.isScChannel() ? 
                             getChannelName(rfval) : getVarName(rfval);
        string rhsName = rnames.first + rrecSuffix;

        string f = lhsName + (nbAssign ? NB_ASSIGN_SYM : ASSIGN_SYM) + rhsName;
        s = s + (first ? "" : "; ") + f;
        first = false;
    }

    putString(stmt, s, 0);
    clearSimpleTerm(stmt);
}

// Assignment record variable with temporary record object (T{}, T())
void ScVerilogWriter::putRecordAssignTemp(const Stmt* stmt, 
                                      const SValue& lvar, const SValue& lrec, 
                                      const SValue& rrec,
                                      bool lelemOfMifRecArr, string lrecSuffix,
                                      const ScState* state) 
{
    //cout << "putRecordAssignTemp lrec " << lrec << ", rrec " << rrec 
    //     << ", recSuffix " << lrecSuffix << endl;
    
    std::vector<SValue> lrecFields;
    if (lrec.isScChannel()) {
        auto recType = dyn_cast<const RecordType>(lrec.getScChannel()->getType().
                                                  getTypePtr());
        getFieldsForRecordChan(recType->getAsRecordDecl(),lrec, lrecFields);
    } else {
        getFieldsForRecord(lrec, lrecFields);
    }
    std::vector<SValue> rrecFields;
    if (rrec.isScChannel()) {
        auto recType = dyn_cast<const RecordType>(rrec.getScChannel()->getType().
                                                  getTypePtr());
        getFieldsForRecordChan(recType->getAsRecordDecl(), rrec, rrecFields);
    } else {
        getFieldsForRecord(rrec, rrecFields);
    }
    //for (const SValue& f : lrecFields) cout << "  " << f << endl;
    //for (const SValue& f : rrecFields) cout << "  " << f << endl;
    
    // Check one of the fields
    // No assign COMBSIG with CLEAR in clocked thread reset
    SValue lfval = lrecFields.back();
    if (isCombSigClear(lfval) && isClockThreadReset) {
        clearStmt(stmt);
        return;
    }
    
    // Record can be inside MIF but not another record, so MIF indices are
    // in @recordValueName or/and @MIFValueName, add one of them
    if (lelemOfMifRecArr) {
        if (recordValueName.first) {
            lrecSuffix = recordValueName.second + lrecSuffix;
        } else 
        if (MIFValueName.first) {
            lrecSuffix = MIFValueName.second + lrecSuffix;
        }
    }

    // Check for register variable and first field (required for record channel)
    bool isReg = isRegister(lvar) || isCombSig(lvar) || 
                 isCombSigClear(lvar) || isClearSig(lvar); 
    
    // Record assignment, assign all the fields
    string s;
    bool first = true;
    for (unsigned i = 0; i != lrecFields.size(); ++i) {
        const SValue& lfval = lrecFields[i];
        const SValue& rfval = rrecFields[i];
        //cout << "  lfval " << lfval << "  rfval " << rfval << endl;
        //cout << "  lfval " << lfval << " isReg " << (isReg || isRegister(lfval)) << endl;

        // Skip zero width type
        auto ftype = lfval.getType();
        if (isZeroWidthType(ftype) || isZeroWidthArrayType(ftype)) continue;
        
        bool nbAssign = isClockThreadReset && (isReg || isRegister(lfval));
        bool secName  = !isClockThreadReset && (isReg || isRegister(lfval)) && 
                        !emptySensitivity;
        
        // Get name for LHS
        const auto& lnames = lrec.isScChannel() ? 
                             getChannelName(lfval) : getVarName(lfval);
        string lhsName = (secName ? lnames.second : lnames.first) + lrecSuffix;
        
        // Get RHS integer value
        SValue rrval;
        state->getValue(rfval, rrval);

        string rhsName;
        if (rrval.isInteger()) {
            char radix = rrval.getRadix() == 100 ? 10 : rrval.getRadix();
            rhsName = makeLiteralStr(rrval.getInteger(), radix, 
                                     0, 0, CastSign::NOCAST, false);
        } else {
            rhsName = "0";
        }

        string f = lhsName + (nbAssign ? NB_ASSIGN_SYM : ASSIGN_SYM) + rhsName;
        s = s + (first ? "" : "; ") + f;
        first = false;
    }

    putString(stmt, s, 0);
    clearSimpleTerm(stmt);
}

// Put array element initialization, used for array initializer list 
// \param bval -- array variable
// \param ival -- array index integer
void ScVerilogWriter::putArrayElemInit(const Stmt* stmt, const SValue& bval, 
                                       const std::vector<std::size_t>& indices, 
                                       const Expr* iexpr) 
{
    if (skipTerm) return;
    SCT_TOOL_ASSERT ((bval.isVariable() || bval.isTmpVariable()), 
                     "No variable or integer found");
    SCT_TOOL_ASSERT (stmt != iexpr, "Incorrect array inializer");

    if (terms.count(iexpr)) {
        // Get unique variable name for write
        auto names = getVarName(bval);

        bool isReg = isRegister(bval);
        bool nbAssign = isClockThreadReset && isReg;
        bool secName = !isClockThreadReset && isReg;

        string s;
        for (auto indx : indices) {
            s += "[" + to_string(indx) + "]";
        }
        s = (secName ? names.second : names.first) + s +
            (nbAssign ? NB_ASSIGN_SYM : ASSIGN_SYM) + 
            getTermAsRValue(iexpr).first;

        auto i = terms.find(stmt);
        if (i != terms.end()) {
            // Put several elements initialization in one string
            s = i->second.str.first + "; " + s;
        }
        putString(stmt, s, 0);
        clearSimpleTerm(stmt);
    }
}

// Put array element initialization with zero
// \param bval -- array variable
// \param ival -- array index integer
void ScVerilogWriter::putArrayElemInitZero(const Stmt* stmt, const SValue& bval, 
                                           const std::vector<std::size_t>& indices) 
{
    if (skipTerm) return;
    SCT_TOOL_ASSERT ((bval.isVariable() || bval.isTmpVariable()), 
                      "No variable found");

    // Get unique variable name for write
    auto names = getVarName(bval);

    bool isReg = isRegister(bval);
    bool nbAssign = isClockThreadReset && isReg;
    bool secName = !isClockThreadReset && isReg;

    string s;
    for (auto indx : indices) {
        s += "[" + to_string(indx) + "]";
    }
    s = (secName ? names.second : names.first) + s +
        (nbAssign ? NB_ASSIGN_SYM : ASSIGN_SYM) + "0";

    addString(stmt, s);
    clearSimpleTerm(stmt);
}

// Add array subscript index
// \param aval -- most inner array object with all zero indices to distinguish
//                indices of different objects, used for a[j][b[i]]
void ScVerilogWriter::addSubscriptIndex(const SValue& aval, 
                                        const clang::Expr* indx)
{
    if (skipTerm) return;

    // Skip duplicates
    for (const auto& i : arraySubIndices) {
        if (i.first == aval && i.second == indx) return;
    }
    
    arraySubIndices.push_back(pair<SValue, const clang::Stmt*>(aval, indx));
    //cout << "---- addSubscriptIndex for " << aval << hex << " indx " << indx << dec << endl;
}

void ScVerilogWriter::clearSubscriptIndex() {
    arraySubIndices.clear();
}

// Get string from indices "[index1][index2]..." stored in @arraySubIndices
// and erase them in @arraySubIndices if no @keepArrayIndices
std::string ScVerilogWriter::getIndexString(const SValue& val) 
{
    //cout << "---- getIndexString for val " << val << endl;
    string res;
    for (auto i = arraySubIndices.begin(); i != arraySubIndices.end(); ) {
        // Check most inner array object with all zero indices to distinguish
        // indices of different objects, used for a[j][b[i]]
        if (i->first == val) {
            res = res + "[" + getTermAsRValue(i->second).first + "]";
            if (keepArrayIndices) {
                ++i;   
            } else {
                i = arraySubIndices.erase(i);
            }
            //cout << "   " << hex << (unsigned long)(i->second) << dec << " " << res << endl;
        } else {
            ++i;
        }
    }

    //cout << "   " << res << endl;
    return res;
}

// Array index access operator [] for non-channel objects
// @param base  -- base expression 
// @param index -- index expression 
void ScVerilogWriter::putArrayIndexExpr(const Stmt* stmt, const Expr* base,
                                        const Expr* index)
{
    if (skipTerm) return;

    if (terms.count(base) && terms.count(index)) {

        string indx = "[" + getTermAsRValue(index).first + "]";
        auto names = getTermAsRValue(base);
        string rdName = removeBrackets(names.first) + indx;
        string wrName = removeBrackets(names.second) + indx;

        // Get array element type width including array of channels 
        QualType type;
        if (auto subsExpr = dyn_cast<const ArraySubscriptExpr>(stmt)) {
            type = subsExpr->getType();

        } else
        if (auto operExpr = dyn_cast<const CXXOperatorCallExpr>(stmt)) {
            type = operExpr->getType();

        } else {
            SCT_INTERNAL_FATAL(stmt->getBeginLoc(),
                               "putArrayIndexExpr : incorrect expression type");
        }
        
        // Type width can be not determinable for example for
        // MIF/record array or vector of vectors
        unsigned width = 0;
        if (auto typeWidth = getAnyTypeWidth(type, true, true)) {
            width = *typeWidth;
        }

        putString(stmt, pair<string,string>(rdName, wrName), width);
        
    } else {
        cout << "putArrayIndexExpr : arg " << hex 
             << (size_t(terms.count(base) ? index : base)) << dec << endl;
        SCT_INTERNAL_FATAL(stmt->getBeginLoc(),
                         "putArrayIndexExpr : no term for base/index part ");
    }
}

// Constant and variable based range part-select
void ScVerilogWriter::putPartSelectExpr(const Stmt* stmt,  const SValue& val,
                                        const Expr* base,
                                        const Expr* hindx, const Expr* lindx,
                                        bool useDelta)
{
    if (skipTerm) return;
    
    if (terms.count(base) && terms.count(hindx) && terms.count(lindx)) {
        auto baseInfo = terms.at(base);
        if (!baseInfo.simplTerm || baseInfo.literRadix) {
            ScDiag::reportScDiag(base->getBeginLoc(), 
                                 ScDiag::SC_RANGE_WRONG_BASE);
        }
        
        // Use only decimal for range index literals
        auto& hindxInfo = terms.at(hindx);
        if (hindxInfo.literRadix && hindxInfo.literRadix != 10) {
            hindxInfo.literRadix = 10;
        }
        auto& lindxInfo = terms.at(lindx);
        if (lindxInfo.literRadix && lindxInfo.literRadix != 10) {
            lindxInfo.literRadix = 10;
        }

        // Type width after cast
        unsigned castWidth = getExprTypeWidth(base);
        // Internal (minimal) type width, this number of bits can be accessed 
        unsigned intrWidth = getMinExprTypeWidth(base);

        // Check incorrect range
        APSInt lval(APInt(64, 0), true);
        if (lindxInfo.literRadix) {
            lval = APSInt(lindxInfo.str.first);
            if (lval < 0) {
                ScDiag::reportScDiag(lindx->getBeginLoc(), 
                                     ScDiag::SC_RANGE_WRONG_INDEX);
            }
        }
        if (hindxInfo.literRadix) {
            APSInt hval(hindxInfo.str.first);
            APSInt llval; APSInt hhval;
            adjustIntegers(lval, hval, llval, hhval);

            if (hval < 0 || hhval < llval) {
                ScDiag::reportScDiag(hindx->getBeginLoc(), 
                                     ScDiag::SC_RANGE_WRONG_INDEX);
            }
            if (intrWidth == 1 && (!lval.isZero() || !hval.isZero())) {
                ScDiag::reportScDiag(base->getBeginLoc(), 
                                     ScDiag::SC_RANGE_WRONG_INDEX);
            }
        }        
        
        // Add bit suffix if both type widths more than one
        string range = "";
        if (castWidth > 1 && intrWidth > 1) {
            range = "[" + getTermAsRValue(hindx).first + 
                    (useDelta ? " +: " : " : ") + 
                    getTermAsRValue(lindx).first + "]";
        }
        
        // Remove cast prefix/brackets for bit suffix and single bit variable
        auto names = getTermAsRValue(base, castWidth > 1);
        string rdName = names.first + range;
        string wrName = names.second + range;
        
        unsigned width = useDelta ? getLiteralAbs(lindx) : 
                                    getLiteralAbs(hindx)-getLiteralAbs(lindx)+1;
        
        putString(stmt, pair<string,string>(rdName, wrName), width);

    } else {
        SCT_INTERNAL_FATAL(stmt->getBeginLoc(),
                         "putPartSelectExpr : no term for base/index part");
    }
}

// Constant and variable based bit part-select
void ScVerilogWriter::putBitSelectExpr(const Stmt* stmt, const SValue& val,
                                       const Expr* base,
                                       const Expr* index)
{
    if (skipTerm) return;
    
    if (terms.count(base) && terms.count(index)) {
        auto baseInfo = terms.at(base);
        if (!baseInfo.simplTerm || baseInfo.literRadix) {
            ScDiag::reportScDiag(base->getBeginLoc(), 
                                 ScDiag::SC_BIT_WRONG_BASE);
        }
        
        // Use only decimal for bit index literal
        auto& indexInfo = terms.at(index);
        if (indexInfo.literRadix && indexInfo.literRadix != 10) {
            indexInfo.literRadix = 10;
        }
        
        // Type width after cast
        unsigned castWidth = getExprTypeWidth(base);
        // Internal (minimal) type width, this number of bits can be accessed 
        unsigned intrWidth = getMinExprTypeWidth(base);
        
        // Check incorrect bit index
        if (indexInfo.literRadix) {
            APSInt lval(indexInfo.str.first);
            if (lval < 0) {
                ScDiag::reportScDiag(index->getBeginLoc(), 
                                     ScDiag::SC_BIT_WRONG_INDEX);
            }
            if (intrWidth == 1 && !lval.isZero()) {
                ScDiag::reportScDiag(base->getBeginLoc(), 
                                     ScDiag::SC_BIT_WRONG_INDEX);
            }
        }
        
        // Add bit suffix if both type widths more than one
        string bit = "";
        if (castWidth > 1 && intrWidth > 1) {
            bit = "[" + getTermAsRValue(index).first + "]"; 
        }
        
        // Remove cast prefix/brackets for bit suffix and single bit variable
        auto names = getTermAsRValue(base, castWidth > 1);
        string rdName = names.first + bit;
        string wrName = names.second + bit;
        
        putString(stmt, pair<string,string>(rdName, wrName), 1);
        
    } else {
        SCT_INTERNAL_FATAL(stmt->getBeginLoc(),
                         "putBitSelectExpr : no term for base/index part ");
    }
}

// Report warning for negative literal casted to unsigned
void ScVerilogWriter::checkNegLiterCast(const Stmt* stmt, const TermInfo& info) 
{
    if (info.literRadix && info.castSign == CastSign::UCAST) {
        APSInt val(info.str.first);
        if (val < 0) {
            ScDiag::reportScDiag(stmt->getBeginLoc(), 
                                 ScDiag::SYNTH_NEG_LITER_UCAST);
        }
    }
}

// Set SCAST for literal, SACAST for unsigned operand which is not SEXPR
void ScVerilogWriter::setExprSCast(const Expr* expr, TermInfo& info) 
{
    if (info.literRadix) {
        // Set SCAST for literal to have it signed in signed operation
        info.castSign = CastSign::SCAST;
        
    } else {
        QualType type = expr->getType().getCanonicalType();
        
        if (!isSignedType(type)) {
            // Set SACAST to have signed'{1'b0, ...} cast in SV code
            if (info.exprSign != ExprSign::SEXPR) {
                info.castSign = CastSign::SACAST;
            } else {
                ScDiag::reportScDiag(expr->getBeginLoc(), 
                                     ScDiag::SYNTH_SEXPR_UNSIGNED);
            }
        }
    }
}

// Binary operators "+", "-", "*", "/", "%", "||", "&&", "&", "|", "^",
// "<<<", ">>>", ">", "<", ">=", "<=", "==", "!="
void ScVerilogWriter::putBinary(const Stmt* stmt, string opcode, 
                                const Expr* lhs, const Expr* rhs) 
{
    if (skipTerm) return;

    if (terms.count(lhs) && terms.count(rhs)) 
    {
        bool binaryMinus = opcode == "-";
        bool bitwiseOper = opcode == "&" || opcode == "|" || opcode == "^";
        bool arithmOper  = binaryMinus || opcode == "+" || 
                           opcode == "*" || opcode == "/" || opcode == "%" ||
                           bitwiseOper;
        bool shiftOper   = opcode == ">>>" || opcode == "<<<";
        bool compareOper = opcode == ">" || opcode == "<" ||
                           opcode == ">=" || opcode == "<=" ||
                           opcode == "==" || opcode == "!=";        
        
        auto& linfo = terms.at(lhs);
        auto& rinfo = terms.at(rhs);
        
//        cout << "putBinary #" << hex << stmt << dec 
//             << " castSign " << (int)linfo.castSign << (int)rinfo.castSign
//             << " exprSign " << (int)linfo.exprSign << (int)rinfo.exprSign 
//             << " explCast " << (int)linfo.explCast << (int)rinfo.explCast << endl;
        
        // Report warning for signed to unsigned cast for non-literal
        if ((!linfo.literRadix && linfo.castSign == CastSign::UCAST) || 
            (!rinfo.literRadix && rinfo.castSign == CastSign::UCAST))  
        {
            if (arithmOper) {
                ScDiag::reportScDiag(stmt->getBeginLoc(), 
                                     ScDiag::SYNTH_SIGN_UNSIGN_MIX);
            } else 
            if (checkUnsigned && compareOper) {
                ScDiag::reportScDiag(stmt->getBeginLoc(),
                                     ScDiag::SYNTH_COMPARE_SIGN_UNSIGN_MIX);
            }
        }
        
        if (arithmOper || shiftOper) {
            if ((!linfo.literRadix && linfo.castSign == CastSign::BCAST) ||
                (!rinfo.literRadix && rinfo.castSign == CastSign::BCAST)) {
                ScDiag::reportScDiag(stmt->getBeginLoc(), 
                                     ScDiag::CPP_BOOL_BITWISE_BINARY);
            }
        }
        
        // Report warning for negative literal casted to unsigned in / and %
        if (opcode == "/" || opcode == "%") {
            checkNegLiterCast(stmt, linfo);
            checkNegLiterCast(stmt, rinfo);
        }

        // Get binary expression type
        auto bexpr = dyn_cast<Expr>(stmt);
        SCT_TOOL_ASSERT (bexpr, "No expression for binary operation");
        QualType btype = bexpr->getType();
        bool isSignedBinary = isSignedType(btype);

        if (shiftOper && isSignedBinary) {
            ScDiag::reportScDiag(stmt->getBeginLoc(), 
                                 ScDiag::SYNTH_SIGNED_SHIFT);
        }
        
        // Check both arguments are unsigned or literals or casted from @bool
        if (checkUnsigned) {
            bool isSignedLHS = isSignedType(lhs->getType()) && !linfo.literRadix &&
                               linfo.castSign != CastSign::BCAST;
            bool isSignedRHS = isSignedType(rhs->getType()) && !rinfo.literRadix &&
                               rinfo.castSign != CastSign::BCAST;
            if (isSignedLHS || isSignedRHS) {
                ScDiag::reportScDiag(stmt->getBeginLoc(), 
                                     ScDiag::SYNTH_UNSIGNED_MODE_BINARY);
            }
        }
        
        bool negBrackets = arithmOper || shiftOper;
        bool doSignCast = !skipSignCast && (arithmOper || compareOper);
        
        // For signed binary operation set SACAST for operand if its unsigned 
        if (doSignCast && isSignedBinary) {
            setExprSCast(lhs, linfo);
            setExprSCast(rhs, rinfo);
        }
        
        string s = getTermAsRValue(lhs, false, negBrackets, doSignCast).first + 
                   " " + opcode + " " +
                   getTermAsRValue(rhs, false, negBrackets, doSignCast).first;
        
        unsigned width = 0;
        unsigned lwidth = getExprWidth(lhs);
        unsigned rwidth = getExprWidth(rhs);
        unsigned maxwidth = (lwidth > rwidth) ? lwidth : rwidth;
        //cout << "lhs " << hex << lhs << dec << " lwidth " << lwidth << endl;
        //cout << "rhs " << hex << rhs << dec << " rwidth " << rwidth << endl;
        
        if (opcode == "||" || opcode == "&&" || opcode == ">" ||
            opcode == "<" || opcode == ">=" || opcode == "<=" ||
            opcode == "==" || opcode == "!=") 
        {
            width = 1;
            
        } else 
        if (lwidth) {
            if (opcode == "/" || opcode == "%" || opcode == ">>>") {
                width = lwidth;
                
            } else 
            if (rwidth) {
                if (opcode == "+" || opcode == "-") {
                    width = maxwidth + 1;
                } else 
                if (opcode == "*") {
                    width = lwidth + rwidth;
                } else 
                if (opcode == "&" || opcode == "|" || opcode == "^") {
                    width = maxwidth;
                } else 
                if (opcode == "<<<") {
                    if (rinfo.literRadix) {
                        width = lwidth + getLiteralAbs(rhs);
                    } else {
                        // Left shift width can be too long, so width if limited 
                        // with 64bit, explicit type cast should be used in SC
                        if (rwidth < 7) {
                            width = lwidth + (1 << rwidth)-1;
                            width = width > 64 ? 0 : width;
                        }
                    }
                }
            }
        }
        
        // For unsigned operation promote SEXPR/signed type from arguments and 
        // also set it for binary minus
        bool signedExpr = !skipSignCast && !isSignedBinary &&
                          (binaryMinus || linfo.exprSign == ExprSign::SEXPR || 
                           rinfo.exprSign == ExprSign::SEXPR);
        
        putString(stmt, s, width);
        clearSimpleTerm(stmt);
        setExprSign(stmt, signedExpr);
        //cout << "    signedExpr " << int(signedExpr) << " literExpr " << literExpr << endl;
        
        // Set increase result width 
        if (opcode == "+" || opcode == "-" || opcode == "*" || opcode == "<<<"){
            setIncrWidth(stmt);
        }
        
        //cout << "putBinary opcode " << opcode << " width " << width << endl;
    } else {
        cout << "putBinary : arg " << hex 
             << (size_t(terms.count(lhs) ? rhs : lhs)) << dec << endl;
        SCT_INTERNAL_FATAL(stmt->getBeginLoc(),
                           "putBinary : No term for lhs/rhs statement ");
    }
}

// Compound assignment operators "+=", "-=", "*=", "/=", "%=", "&=", "|=", "^=",
// "<<=", ">>=", for non-channel object only
void ScVerilogWriter::putCompAssign(const Stmt* stmt, string opcode, 
                                    const SValue& lval, 
                                    const Expr* lhs, const Expr* rhs) 
{
    if (skipTerm) return;

    if (terms.count(lhs) && terms.count(rhs)) {
        
        bool bitwiseOper = opcode == "&" || opcode == "|" || opcode == "^";
        bool arithmOper  = opcode == "+" || opcode == "-" || 
                           opcode == "*" || opcode == "/" || opcode == "%" ||
                           bitwiseOper;
        
        auto& linfo = terms.at(lhs);
        auto& rinfo = terms.at(rhs);
        
        SCT_TOOL_ASSERT (!linfo.isChannel, "putCompAssign for SC channel object");
        //cout << "putCompAssign castSign " << (int)linfo.castSign << (int)rinfo.castSign << endl;
        
        // Get compound expression type
        auto cexpr = dyn_cast<Expr>(stmt);
        SCT_TOOL_ASSERT (cexpr, "No expression for compound operation");
        QualType ctype = cexpr->getType();
        QualType rtype = rhs->getType();
        bool isSignedCompound = isSignedType(ctype);
        bool isSignedRHS = isSignedType(rtype);
        
        // Report warning for signed to unsigned cast and signed rvalue
        // in unsigned operation for non-literal
        if (!rinfo.literRadix && (rinfo.castSign == CastSign::UCAST ||
            (!isSignedCompound && isSignedRHS && rinfo.castSign != CastSign::BCAST)))
        {
            if (arithmOper) {
                ScDiag::reportScDiag(stmt->getBeginLoc(),
                                     ScDiag::SYNTH_SIGN_UNSIGN_MIX);
            }
        }
        
        if (checkUnsigned && isSignedRHS) {
            if (!rinfo.literRadix && rinfo.castSign != CastSign::BCAST) {
                //cout << hex << "stmt " << stmt << endl;
                ScDiag::reportScDiag(stmt->getBeginLoc(), 
                                     ScDiag::SYNTH_UNSIGNED_MODE_BINARY);
            }
        }
        
        if (!rinfo.literRadix && rinfo.castSign == CastSign::BCAST) {
            ScDiag::reportScDiag(stmt->getBeginLoc(), 
                                 ScDiag::CPP_BOOL_BITWISE_BINARY);
        }
        
        // Report warning for negative literal casted to unsigned in / and %
        if (opcode == "/" || opcode == "%") {
            checkNegLiterCast(stmt, rinfo);
        }
        
        bool doSignCast = !skipSignCast && arithmOper;

        // For signed operation set SACAST for right operand if its unsigned 
        if (doSignCast && isSignedCompound) {
            setExprSCast(rhs, rinfo);
        }
            
        auto names = getTermAsRValue(lhs);
        
        bool isReg = isRegister(lval) || isCombSig(lval) || 
                     isCombSigClear(lval) || isClearSig(lval);
        bool nbAssign = isClockThreadReset && isReg;
        bool secName = !isClockThreadReset && isReg;

        // RHS string, add brackets for negative    
        string s = getTermAsRValue(rhs, false, true, doSignCast).first;
        // Add brackets for RHS if it is not simple term w/o brackets yet
        if (!terms.at(rhs).simplTerm) {
            if (!isTermInBrackets(s)) {
                s = '(' + s + ')';
            }
        }
        
        s = (secName ? names.second : names.first) + 
            (nbAssign ? NB_ASSIGN_SYM : ASSIGN_SYM) + 
            getTermAsRValue(lhs, false, false, doSignCast).first + " " + 
            opcode + " " + s;

        unsigned width = getExprWidth(lhs); 
        
        putString(stmt, s, width);
        clearSimpleTerm(stmt);
        // Do not need to put @exprSign for compound statement
        
    } else {
        cout << "putCompAssign : arg " << hex 
             << (size_t(terms.count(lhs) ? rhs : lhs)) << dec << endl;
        SCT_INTERNAL_FATAL(stmt->getBeginLoc(), 
                         "putCompAssign : No term for lhs/rhs statement ");
    }
}

// Unary operators "-", ++", "--", "~", "!", "&", "|", "^", "~&", "~|", "~^"
void ScVerilogWriter::putUnary(const Stmt* stmt, string opcode, const Expr* rhs, 
                               bool isPrefix) 
{
    if (skipTerm) return;

    if (terms.count(rhs)) {
        
        bool unaryMinus = opcode == "-";
        bool doSignCast = !skipSignCast && unaryMinus;
        
        auto& rinfo = terms.at(rhs);
        
        // Get unary expression type
        auto uexpr = dyn_cast<Expr>(stmt);
        SCT_TOOL_ASSERT (uexpr, "No expression for unary operation");
        QualType utype = uexpr->getType();
        bool isSignedUnary = isSignedType(utype);
        bool isSignedRHS = isSignedType(rhs->getType());
        
        // Check operand is unsigned, skip literals and cast to boolean
        if (checkUnsigned && isSignedRHS && opcode != "|") {
            if (!rinfo.literRadix && rinfo.castSign != CastSign::BCAST) {
                ScDiag::reportScDiag(stmt->getBeginLoc(), 
                                     ScDiag::SYNTH_UNSIGNED_MODE_UNARY);
            }
        }
        
        // For signed unary operation set SACAST for operand if its unsigned
        if (doSignCast && isSignedUnary) {
            setExprSCast(rhs, rinfo);
        }
            
        string s;
        char literRadix = rinfo.literRadix;
        bool isLiteralMinus = (opcode == "-") && literRadix;
        
        if (isLiteralMinus) {
            auto names = rinfo.str;
            s = addLeadMinus(names.first);
            
        } else {
            auto names = getTermAsRValue(rhs, false, true, doSignCast);
            string name = (opcode == "++" || opcode == "--") ?
                          names.second : names.first;
            
            // Add brackets for "!|", as it not supported by VCS
            if (opcode == "!" && name.at(0) == '|') {
                name = "("+ name+")";
            }
           
            // Do not apply "|" for 1 bit width argument
            // Extracting width from RHS type not work as argument is integer
            unsigned baseWidth = getExprTypeWidth(rhs);
            bool skipOper = opcode == "|" && baseWidth == 1;
            
            s = skipOper ? name : isPrefix ? opcode + name : name + opcode;
        }

        unsigned width = 0;
        unsigned rwidth = getExprWidth(rhs);

        if (opcode == "!" || opcode == "&" || opcode == "|" || opcode == "^" ||
            opcode == "~&" || opcode == "~|" || opcode == "~^" ) 
        {
            width = 1;
        } else 
        if (rwidth) {
            if (opcode == "++" || opcode == "--") {
                width = rwidth+1;   
            } else {
                width = rwidth;
            }
        }
        
        // Unary minus considered as signed to avoid adding 'signed{}
        bool signedExpr = !skipSignCast && !isSignedUnary &&
                          (unaryMinus || rinfo.exprSign == ExprSign::SEXPR);
        
        putString(stmt, s, width);
        clearSimpleTerm(stmt);
        setExprSign(stmt, signedExpr);
        
        // Literal with minus preserved as literal, castWidth copied
        if (isLiteralMinus) {
            auto& sinfo = terms.at(stmt);
            sinfo.literRadix = literRadix;
            sinfo.minCastWidth = rinfo.minCastWidth;
            sinfo.lastCastWidth = rinfo.lastCastWidth;
            sinfo.explCast = rinfo.explCast;
        }
        
        // Set increase result width 
        if (opcode == "++" || opcode == "--") {
            setIncrWidth(stmt);
        }
        
        //cout << "putUnary opcode " << opcode << " width " << width << endl;
    } else {
        cout << "putUnary : arg " << hex << (size_t)rhs << dec << endl;
        SCT_INTERNAL_FATAL(stmt->getBeginLoc(), 
                           "putUnary : No term for rhs statement ");
    }
}

// Ternary statement ?
void ScVerilogWriter::putCondStmt(const Stmt* stmt, const Stmt* cond, 
                                  const Stmt* lhs, const Stmt* rhs) 
{
    if (skipTerm) return;

    if (terms.count(cond) && terms.count(lhs) && terms.count(rhs)) {
        const auto& linfo = terms.at(lhs);
        const auto& rinfo = terms.at(rhs);
        
//        cout << "putCondStmt #" << hex << stmt << dec 
//             << " castSign " << (int)linfo.castSign << (int)rinfo.castSign
//             << " exprSign " << (int)linfo.exprSign << (int)rinfo.exprSign 
//             << " explCast " << (int)linfo.explCast << (int)rinfo.explCast << endl;
        
        string conds = getTermAsRValue(cond).first + " ? ";
        string rdName = conds + getTermAsRValue(lhs).first + " : " + 
                        getTermAsRValue(rhs).first;
        string wrName = conds + getTermAsRValue(lhs).second + " : " + 
                        getTermAsRValue(rhs).second;
        
        // LHS and RHS widths should be the same, so take non-zero one
        unsigned lwidth = getExprWidth(lhs);
        unsigned rwidth = getExprWidth(rhs); 
        unsigned width = lwidth ? lwidth : rwidth; 
        
        putString(stmt, pair<string,string>(rdName, wrName), width);
        clearSimpleTerm(stmt);

        auto cexpr = dyn_cast<Expr>(stmt);
        SCT_TOOL_ASSERT (cexpr, "No expression for conditional statement");
        QualType ctype = cexpr->getType();

        // Promote signed expression flag from arguments
        bool signedExpr = linfo.exprSign == ExprSign::SEXPR || 
                          rinfo.exprSign == ExprSign::SEXPR;
        // Check no signed expression for signed type
        SCT_TOOL_ASSERT (!signedExpr || !isSignedType(ctype), 
                         "Signed expression for signed conditional statement");
        
        setExprSign(stmt, signedExpr);
        
    } else {
        cout << "putCondStmt : stmt " << hex << (size_t((terms.count(cond)) ?
                         ((terms.count(lhs)) ? rhs : lhs) : cond)) << dec << endl;
        SCT_INTERNAL_FATAL(stmt->getBeginLoc(),
                          "putCondStmt : No term for cond/lhs/rhs statement ");
    }
}

// Concatenation statement
void ScVerilogWriter::putConcat(const clang::Stmt* stmt, 
                                const clang::Stmt* first, 
                                const clang::Stmt* second)
{
    if (skipTerm) return;

    if (terms.count(first) && terms.count(second)) {
        string rdName = "{" + 
                        removeCurlyBrackets(getTermAsRValue(
                        first, false, false, false, true).first) + ", " + 
                        removeCurlyBrackets(getTermAsRValue(
                        second, false, false, false, true).first) + "}";
        string wrName = "{" + 
                        removeCurlyBrackets(getTermAsRValue(
                        first, false, false, false, true).second) + ", " + 
                        removeCurlyBrackets(getTermAsRValue(
                        second, false, false, false, true).second) + "}";
        
        // Take sum of LHS and RHS widths if both of them are known
        unsigned lwidth = getExprWidth(first, true);
        unsigned rwidth = getExprWidth(second, true); 
        unsigned width = (lwidth && rwidth) ? (lwidth + rwidth) : 0;
        
        putString(stmt, pair<string,string>(rdName, wrName), width);
        
    } else {
        cout << "putConcat : stmt " << hex 
             << (size_t((terms.count(first)) ? second : first)) << dec << endl;
        SCT_INTERNAL_FATAL(stmt->getBeginLoc(),
                         "putConcat : No term for argument statement ");
    }
}

// Put function call parameter initialization with the corresponding argument
// @stmt is function call expression 
void ScVerilogWriter::putFCallParam(const Stmt* stmt, const SValue& pval,
                                    const Expr* arg)
{
    if (skipTerm) return;
    SCT_TOOL_ASSERT (!pval.isReference() || pval.isConstReference(), 
                     "Put non-constant reference parameter");
    
    if (terms.count(arg)) {
        // Get unique variable name for @pval, cannot be register variable
        auto names = getVarName(pval);
        string s = names.first + ASSIGN_SYM + getTermAsRValue(arg).first;
        
        addString(stmt, s);
        clearSimpleTerm(stmt);
        
        if (DebugOptions::isEnabled(DebugComponent::doVerWriter)) {
            cout << "putFCallParam for stmt " << hex << stmt << dec << ", " << s << endl;
        }

    } else {
        cout << "putFCallParam : arg " << hex << (size_t)arg << dec << endl;
        SCT_INTERNAL_FATAL(stmt->getBeginLoc(),
                         "putFCallParams : No term for parameter/argument ");
    }
}

void ScVerilogWriter::putEmptyFCallParam(const clang::Stmt* stmt) 
{
    addString(stmt, "");
}

// Put wait(int n) counter assignment
void ScVerilogWriter::putWaitNAssign(const clang::Stmt* stmt, 
                                     const clang::Expr* waitn) 
{
    if (skipTerm) return;

    if (terms.count(waitn)) {
        // @waitNVarName can have empty names for incorrect argument like variable
        if (waitNVarName.first.empty() || waitNVarName.second.empty()) {
            ScDiag::reportScDiag(waitn->getBeginLoc(), ScDiag::SC_WAIT_N_EMPTY);
        }
        string s = ((isClockThreadReset) ? waitNVarName.first : waitNVarName.second) + 
                   ((isClockThreadReset) ? NB_ASSIGN_SYM : ASSIGN_SYM) + 
                   getTermAsRValue(waitn).first;
        
        addString(stmt, s);
        clearSimpleTerm(stmt);
        
        if (DebugOptions::isEnabled(DebugComponent::doVerWriter)) {
            cout << "putWaitAssign for stmt " << hex << stmt << dec << ", " << s << endl;
        }

    } else {
        cout << "putWaitNAssign : waitn " << hex << (size_t)waitn << dec << endl;
        SCT_INTERNAL_FATAL(stmt->getBeginLoc(),
                         "putWaitAssign : No term for parameter of wait(int N) ");
    }
}

// Put clock edge for SVA sensitivity
void ScVerilogWriter::putClockEdge(const clang::Stmt* stmt, 
                                   const clang::Stmt* clock, 
                                   bool posEdge, bool negEdge)
{
    if (skipTerm) return;

    if (terms.count(clock)) {
        string s = (posEdge ? "posedge " : negEdge ? "negedge " : "") + 
                    getTermAsRValue(clock).first;
        
        addString(stmt, s);
        clearSimpleTerm(stmt);
        
        if (DebugOptions::isEnabled(DebugComponent::doVerWriter)) {
            cout << "putAssert for stmt " << hex << stmt << dec  << ", " << s << endl;
        }

    } else {
        cout << "putClockEdge : clock " << hex << (size_t)clock << dec << endl;
        SCT_INTERNAL_FATAL(stmt->getBeginLoc(),
                          "putClockEdge : No clock parameter of sct_assert() ");
    }
    
}

// Put SVA for @sct_assert
void ScVerilogWriter::putAssert(const clang::Stmt* stmt, 
                                const clang::Expr* arg,
                                const std::string& msgStr)
{
    if (skipTerm) return;

    if (terms.count(arg)) {
        string s = "assert (" + getTermAsRValue(arg).first + ") else $error(\""+
                   (msgStr.empty() ? "Assertion failed" : msgStr) +  " at " +
                   getFileName(stmt->getBeginLoc().printToString(sm)) + "\")";
        
        addString(stmt, s);
        clearSimpleTerm(stmt);
        
        if (DebugOptions::isEnabled(DebugComponent::doVerWriter)) {
            cout << "putAssert for stmt " << hex << stmt << dec  << ", " << s << endl;
        }

    } else {
        cout << "putAssert : expr " << hex << (size_t)arg << dec << endl;
        SCT_INTERNAL_FATAL(stmt->getBeginLoc(),
                           "putAssert : No term for parameter of sct_assert() ");
    }
}

// Put temporal assertion in clock thread 
void ScVerilogWriter::putTemporalAssert(const clang::Stmt* stmt, 
                                        const clang::Expr* lhs,
                                        const clang::Expr* rhs,
                                        const std::string& timeStr,
                                        const clang::Expr* event,
                                        unsigned stable, unsigned timeInt
                                        )
{
    if (skipTerm) return;

    if (terms.count(lhs) && terms.count(rhs)) {
        string s;
        if (stable == 0) {
            s = (event ? "@("+getTermAsRValue(event).first+") " : "") +
                 getTermAsRValue(lhs).first + " " + timeStr + " " + 
                 getTermAsRValue(rhs).first;
        } else {
            if (stable == 1) {
                s = (event ? "@("+getTermAsRValue(event).first+") " : "") +
                     getTermAsRValue(lhs).first + " " + timeStr + " " + 
                     "$stable(" + getTermAsRValue(rhs).first+ ")"+
                     (timeInt > 0 ? ("[*"+to_string(timeInt+1)+"]") : "");
            } else {
                s = (event ? "@("+getTermAsRValue(event).first+") " : "") +
                     getTermAsRValue(lhs).first + " " + timeStr + " " + 
                     (stable == 2 ? "$rose(" : "$fell(")+ 
                     getTermAsRValue(rhs).first+ ")";
            }
        }
        
        addString(stmt, s);
        clearSimpleTerm(stmt);
        
        if (DebugOptions::isEnabled(DebugComponent::doVerWriter)) {
            cout << "putTemporalAssert for stmt " << hex << stmt << dec << ", " << s << endl;
        }

    } else {
        SCT_INTERNAL_FATAL(stmt->getBeginLoc(), "putTemporalAssert : No term"
                           " for parameter of SCT_ASSERT()");
    }
}

//=========================================================================

// Clear before next statement 
void ScVerilogWriter::startStmt() {
    // It need to clear as indices removed only for channel arrays
    arraySubIndices.clear();
    terms.clear();
}

// Clear accumulated indices, required in binary operation
//void ScVerilogWriter::clearSubIndices() {
//    arraySubIndices.clear();
//}

// Remove statement from terms
void ScVerilogWriter::clearStmt(const Stmt* stmt) {
    terms.erase(stmt);
}

// Get string for @stmt, which may be sub-expression
// \return expression string to read
std::optional<string> ScVerilogWriter::getStmtString(const Stmt* stmt) 
{
    return (terms.count(stmt)) ? std::optional<string>(terms.at(stmt).str.first) : 
                                 std::optional<string>();
}

// Get string for IF statement
string ScVerilogWriter::getIfString(const Expr* cexpr) 
{
    if (terms.count(cexpr)) {
        // Use @getTermAsRValue to support type cast for condition
        return ("if (" + getTermAsRValue(cexpr).first + ")");
        
    } else {
        SCT_INTERNAL_FATAL(cexpr->getBeginLoc(), 
                           "getIfString : No term for if condition");
        return "";
    }
}

// Get string for SWITCH statement
string ScVerilogWriter::getSwitchString(const Expr* cexpr) 
{
    if (terms.count(cexpr)) {
        // Use @getTermAsRValue to support type cast for condition
        return ("case (" + getTermAsRValue(cexpr).first + ")");
        
    } else {
        SCT_INTERNAL_FATAL(cexpr->getBeginLoc(), 
                           "getSwitchString : No term for switch variable");
        return "";
    }
}

// Get string for FOR statement
string ScVerilogWriter::getForString(const Stmt* init,const Expr* cexpr, 
                                     const Expr* incr) 
{
    // Condition can be empty 
    std::string condStr = terms.count(cexpr) ? getTermAsRValue(cexpr).first : "";
    // Use @getStmtString for initialization/increment at it cannot have type cast
    std::string initStr = "";
    if (auto initStmtStr = getStmtString(init)) initStr = *initStmtStr;
    std::string incrStr = "";
    if (auto incrStmtStr = getStmtString(incr)) incrStr = *incrStmtStr;
            
    return ("for (" + initStr + "; " + condStr + "; " + incrStr + ")");
}

// Get string for WHILE statement
string ScVerilogWriter::getWhileString(const Expr* cexpr) 
{
    if (terms.count(cexpr)) {
        // Use @getTermAsRValue to support type cast for condition
        return ("while (" + getTermAsRValue(cexpr).first + ")");
        
    } else {
        SCT_INTERNAL_FATAL(cexpr->getBeginLoc(), 
                           "getWhileString : No term for while condition");
        return "";
    }
}

// Get break statement
string ScVerilogWriter::getBreakString() {
    return ("break");
}

// Get continue statement
string ScVerilogWriter::getContinueString() {
    return ("continue");
}

//=========================================================================

// Print local variable declaration and current to next register variable assignment
void ScVerilogWriter::printLocalDeclaration(std::ostream &os, 
                                            const ScState* state)
{
    // METHOD with empty sensitivity
    bool emptySensMethod = isCombProcess && emptySensitivity;
    
    // Local variables declarations
    unordered_set<SValue> declaredVars;
    //cout << "---------- printLocalDeclaration" << endl;
    for (const auto& pair : localDeclVerilog) {
        const SValue& val = pair.first;
        // Skip constant defined in thread reset section declaration in process
        // @varTraits used for CTHREAD variables only
        if (!isCombProcess) {
            auto i = varTraits.find(val);
            if (i != varTraits.end()) {
                if (i->second.isRegister() || i->second.isReadOnlyCDR()) continue;
                if (REMOVE_BODY_UNUSED() && !i->second.isAccessAfterReset()) continue;
            }
        }
        
        // Remove constant/variable which is replaced by value
        if (initLocalVars) declaredVars.insert(val);

        printSplitString(os, pair.second, emptySensMethod ? "" : TAB_SYM);
        //cout << "   " << val << " : " << pair.second << endl;
    }
    
    // Local variables initialization with zeros (@INIT_LOCAL_VARS)
    if (initLocalVars) {
        for (const auto& pair : localDeclInitVerilog) {
            // Print initialization for declared variables only
            if (declaredVars.count(pair.first) != 0) {
                printSplitString(os, pair.second, TAB_SYM);
                //cout << "   " << pair.first << " : " << pair.second << endl;
            }
        }
    }
    
    // Skip next assignments for method process
    if (isCombProcess) return;

    // Put current to next assignment for registers
    vector<string> sortAssign;
    for (const auto& i : varTraits) {
        // Is zero element of MIF array, do not print declaration for this object
        if (i.second.skipNextAssign) continue;
        
        // MIF array element suffix
        const std::string& suffix = (i.second.mifElemSuffix) ? 
                                    *i.second.mifElemSuffix : "";
        
        // Normal register
        if (i.second.isRegister() && !i.second.isReadOnly()) {
            // Check accessed after reset that includes used in process SVA
            if (i.second.currName && i.second.nextName && 
                (i.second.isAccessAfterReset() || !REMOVE_UNUSED_NEXT()))
            {
                string s = *i.second.nextName + suffix +
                           ASSIGN_SYM + 
                           *i.second.currName + suffix;
                sortAssign.push_back(s);
            }
        } else 
        if (i.second.isCombSig()) {
            // COMBSIG w/o CLEAR flag
            if (i.second.currName && i.second.nextName) {
                string s = *i.second.currName + suffix +
                           ASSIGN_SYM + 
                           *i.second.nextName + suffix;
                sortAssign.push_back(s);
            }
        } else 
        if (i.second.isCombSigClear()) {
            // COMBSIG with CLEAR flag, @currName is assigned 
            SValue val = i.first;
            bool unkwIndex;
            //cout << "i.first " << i.first << endl;
            bool isArr = state->isArray(val, unkwIndex);
            isArr = isArr || state->getBottomArrayForAny(val, unkwIndex, 
                                                         ScState::MIF_CROSS_NUM);
            //cout << "i.first " << i.first << " isArr " << isArr << endl;
            
            // Use "default" for array only
            if (i.second.currName) {
                string s = *i.second.currName + suffix + 
                           ASSIGN_SYM + (isArr ? "'{default:0}" : "'0");
                sortAssign.push_back(s);
            }
        } else 
        if (i.second.isClearSig()) {
            // CLEARSIG, @nextName is assigned 
            SValue val = i.first;
            bool unkwIndex;
            //cout << "i.first " << i.first << endl;
            bool isArr = state->isArray(val, unkwIndex);
            isArr = isArr || state->getBottomArrayForAny(val, unkwIndex, 
                                                         ScState::MIF_CROSS_NUM);
            //cout << "i.first " << i.first << " isArr " << isArr << endl;
            
            // Use "default" for array only
            if (i.second.nextName) {
                string s = *i.second.nextName + suffix + 
                           ASSIGN_SYM + (isArr ? "'{default:0}" : "'0");
                sortAssign.push_back(s);
            }
        }
    }

    std::sort(sortAssign.begin(), sortAssign.end());
    for (const auto& s : sortAssign) {
        printSplitString(os, s, emptySensMethod ? "" : TAB_SYM);
    }
}

// Print local combinational variable declaration in @always_ff reset section
void ScVerilogWriter::printResetCombDecl(std::ostream &os)
{
    //cout << "------------ printResetCombDecl" << endl;
    unordered_set<SValue> declaredVars;
    // Normal local variable declarations
    for (const auto& pair : localDeclVerilog) {
        if (!isCombProcess) {
            auto i = varTraits.find(pair.first);
            if (i != varTraits.end()) {
                if (i->second.isCombSig() || i->second.isCombSigClear() || 
                    i->second.isClearSig() ||     
                    i->second.isRegister() || i->second.isReadOnlyCDR()) continue;
                if (REMOVE_RESET_UNUSED() && !i->second.isAccessInReset()) continue;
            }
        }
            
        declaredVars.insert(pair.first);

        // Local constant, static constant, function parameter or
        // temporary variables not stored in @varTraits, 
        // it needs to provide them for reset section as well
        printSplitString(os, pair.second, TAB_SYM);
        //cout << "   " << pair.first << " : " << pair.second  << " (1)" << endl;
    }
    
    // Reset local variables initialization with zeros (@INIT_RESET_LOCAL_VARS)
    if (initResetLocalVars) {
        for (const auto& pair : localDeclInitVerilog) {
            // Print initialization for declared variables only
            if (declaredVars.count(pair.first) != 0) {
                printSplitString(os, pair.second, TAB_SYM);
                //cout << "   " << pair.first << " : " << pair.second << " (2)" << endl;
            }
        }
    }
}

// Print in-reset initialization for local variables which become registers
// declared in CTHREAD main loop body (no reset for them)
void ScVerilogWriter::printInitLocalInReset(std::ostream &os)
{
    //cout << "=========== printInitLocalInReset =========" << endl;
    if (!initLocalRegs || isCombProcess) return;
    
    unordered_set<SValue> declaredVars;
    for (const auto& pair : localDeclVerilog) {
        const SValue& val = pair.first;
        auto i = varTraits.find(val);
        if (i != varTraits.end()) {
            // Consider register for local variable not accessed in reset
            if (i->second.isRegister() && !i->second.isModuleScope &&
                !i->second.isAccessInReset() && !i->second.isConstVar) {
                declaredVars.insert(val);
            }
        }
    }
    
    for (const auto& pair : localDeclInitVerilog) {
        // Print initialization for declared variables only
        if (declaredVars.count(pair.first) != 0) {
            printSplitString(os, pair.second, TAB_SYM);
            //cout << "   " << pair.first << " : " << pair.second << endl;
        }
    }
}

// Print variable declaration for given variable
void ScVerilogWriter::printDeclString(std::ostream &os, const SValue& val, 
                                      const std::string& sizeSuff) 
{
    // Array indices
    string indx;
    const QualType& type = val.getType();
    
    if (type->isArrayType()) {
        vector<size_t> arrSizes = getArraySizes(type);

        for (auto j : arrSizes) {
            indx += '[' + to_string(j) + ']';
        }
    }
        
    auto names = getVarName(val);
    string s = getVarDeclVerilog(val.getType(), names.first) + sizeSuff + indx;
    printSplitString(os, s, TAB_SYM);
}
