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
#include "sc_tool/ScCommandLine.h"

#include "clang/AST/ExprCXX.h"
#include "llvm/Support/ScopedPrinter.h"
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

}

using namespace sc;

// Update external names from constant propagator, required for static constants
void ScVerilogWriter::updateExtrNames(
                    const unordered_map<SValue, std::string>& valNames) 
{
    for (const auto& i : valNames) {
        extrValNames.insert(i);
        extrNames.insert(i.second);
    }
}

// Update external name strings after process analysis, required to add 
// empty sensitive METHOD local variables to provide name uniqueness
void ScVerilogWriter::updateExtrNames(
                    const unordered_set<std::string>& valNames) 
{
    extrNames.insert(valNames.begin(), valNames.end());
}

//============================================================================

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
    
    if (isUserDefinedClass(ctype)) {
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
            size_t width = typeInfo.getValue().first;
            bool isUnsigned = typeInfo.getValue().second;
            
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
            size_t width = getExprWidth(init);
            
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
            size_t width = getExprWidth(init);
            
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
unsigned ScVerilogWriter::getUniqueNameIndex(const std::string& origVarName, 
                                             const SValue& val, bool isNext) 
{
    //cout << "getUniqueNameIndex for " << origVarName << " val " << val << endl;
    unsigned index;
    pair<SValue, bool> viKey(val, isNext);
    
    // @val considers difference between variable in base classes
    if (varIndex.count(viKey) == 0) {
        // Get next index for the variable original name
        if (varNameIndex.count(origVarName) == 0) {
            index = 0;
            // Check there is any constructed name equals to this original name,
            // and suffix is numerical
            size_t i = origVarName.length()-1;
            bool numSuffix = true;
            for (; i != 0; i--) {
                const char c = origVarName.at(i);
                if (c == NAME_SUFF_SYM) break;
                numSuffix &= isdigit(c);
            }
            // Increment index to exclude coincidence
            if (i != 0 && i != origVarName.length()-1 && numSuffix) {
                // Get original name without numerical suffix
                string origVarNameBase = origVarName.substr(0, i);

                if (varNameIndex.find(origVarNameBase) != varNameIndex.end()) {
                    index += 1;
                }
            }
        } else {
            index = varNameIndex.at(origVarName) + 1;
        }

        // Check name is unique with already constructed and external names
        //cout << "------ Check name is unique with constructed/external names ----" << endl;
        string nameWithIndex = origVarName + ((index == 0) ? "" : 
                               NAME_SUFF_SYM + to_string(index));
        //cout << "Initial name " << s << endl;
        while (varNameIndex.count(nameWithIndex) || 
               extrNames.count(nameWithIndex)) {
            index += 1;
            nameWithIndex = origVarName + NAME_SUFF_SYM + to_string(index);
            //cout << "Updated name " << s << endl;
        }
        //cout << "----------------------------------------------------" << endl;

        // Store the index
        if (varNameIndex.count(origVarName) == 0) {
            varNameIndex.emplace(origVarName, index);
        } else {
            varNameIndex.at(origVarName) = index;
        }
        varIndex.emplace(viKey, index);

    } else {
        index = varIndex.at(viKey);
    }
    
    return index;
}

// Check if variable value is not registered in varTraits and extrValNames 
bool ScVerilogWriter::isLocalVariable(const SValue& val) 
{
    auto i = varTraits.find(val);
    auto j = extrValNames.find(val);
    bool local = (i == varTraits.end() || !i->second.isModuleScope) && 
                 j == extrValNames.end();
    return local;
}

// Get unique read and write names for variable in scope
// \param recvar -- used to provide unique name inside of record instance
// \return <readName, writeName>
std::pair<string, string> ScVerilogWriter::getVarName(const SValue& val)
{
    SCT_TOOL_ASSERT (val.isVariable() || val.isTmpVariable(),
                     "No variable found");
    if (DebugOptions::isEnabled(DebugComponent::doGenName)) {
        cout << endl << "getVarName for val " << val << endl;
    }

    // Do not manage external name, it should be unique by itself
    if (!isCombProcess && !singleBlockCThreads) {
        //cout << "Get thread name for " << val << endl;
        auto i = varTraits.find(val);
        
        //cout << "varTraits: " << endl;
        //for (auto& e : varTraits) {
        //    cout << "   " << e.first << " : " << (e.second.currName ? 
        //          e.second.currName.getValue() : "---") << e.second << endl;
        //}
        
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
                                i->second.nextName.getValue() : 
                                i->second.currName.getValue();

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
    } else {
        //cout << "Get external name for " << val << endl;
        auto i = extrValNames.find(val);
        if ( i != extrValNames.end() ) {
            /*cout << "extrValNames: " << endl;
            for (auto& e : extrValNames) {
                cout << "   " << e.first << " : " << e.second << endl;
            }*/
            
            if (DebugOptions::isEnabled(DebugComponent::doGenName)) {
                cout << "   External name " << i->second << endl;
            }
            
            return pair<string, string>(i->second, i->second);
        }
    }
    
    // Original name of variable
    string origVarName = val.asString(false);
    // If given name is Verilog keyword add some suffix
    if (VerilogKeywords::check(origVarName)) {
        origVarName += VERILOG_KEYWORD_SUFFIX;
    }

    //cout << "Get local name for " << val << endl;
    if (forLoopInit) {
        // Loop variable is combinatorial variable, next suffix is false
        std::pair<SValue, bool> viKey(val, false);
        
        // No uniqueness checking for loop variable, it overrides other variables
        if (varIndex.count(viKey) == 0) {
            varIndex.emplace(viKey, 0);
        } else {
            varIndex.at(viKey) = 0;
        }
        if (DebugOptions::isEnabled(DebugComponent::doGenName)) {
            cout << "   Local loop initializer name " << origVarName << endl;
        }
        
        return pair<string, string>(origVarName, origVarName);
        
    } else {
        // Normal local variable
        // In reset section current name is used, it placed in @always_ff
        bool isNext = isRegister(val) && !isClockThreadReset;
        
        // Local record field prefix
        string recPrefix = ScState::getLocalRecName(val);
        if (!recPrefix.empty()) recPrefix = recPrefix + "_";
        origVarName = (isNext) ? (origVarName + NEXT_VAR_SUFFIX) : 
                                 (recPrefix + origVarName); 

        // Get index to provide unique name for local variable
        auto index = getUniqueNameIndex(origVarName, val, isNext);

        string vname = origVarName + ((index == 0) ? "" : 
                       NAME_SUFF_SYM + to_string(index));
        
        // Add local variable name from empty sensitive METHOD as it is declared
        // in module scope, that provides variable name uniqueness
        if (emptySensitivity) {
            extrNames.insert(vname);
        }
        
        if (DebugOptions::isEnabled(DebugComponent::doGenName)) {
            cout << "   Local name " << vname << endl;
        }
        return pair<string, string>(vname, vname);
    }    
}

// Get name for ScChannel, used for port/signal
// \param cval -- channel value
// \return <readName, writeName>
pair<string, string> ScVerilogWriter::getChannelName(const SValue& cval)
{
    if (DebugOptions::isEnabled(DebugComponent::doGenName)) {
        cout << "getScChannelName for val " << cval << endl;
    }

    if (!isCombProcess && !singleBlockCThreads) {
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
                return pair<string, string>(j->second, j->second); 
            }
            
            cout << "getScChannelName for val " << cval << endl;
            cout << "varTraits: " << endl;
            for (auto& e : varTraits) {
                cout << "   " << e.first << " : " << (e.second.currName ?
                        e.second.currName.getValue() : "---") << endl;
            }
            SCT_INTERNAL_FATAL_NOLOC ("No channel in VarTraits");
        }

        SCT_TOOL_ASSERT (i->second.currName && 
                         (!i->second.isRegister() || i->second.nextName),
                         "No channel name in VarTraits");
        
        // In reset section current name is used, it placed in @always_ff
        string rdName = (i->second.isCombSig() && isClockThreadReset) ?
                         i->second.nextName.getValue() :
                         i->second.currName.getValue();

        string wrName = ((i->second.isRegister() && !isClockThreadReset) || 
                         (i->second.isCombSig() && isClockThreadReset)) ?
                         i->second.nextName.getValue() : 
                         i->second.currName.getValue();

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

// Return true if @val is register in @varTraits, only in splitting thread mode
bool ScVerilogWriter::isRegister(const SValue& val) 
{
    if (val.isUnknown()) {
        return false;
    }
    
    if (!isCombProcess && !singleBlockCThreads) {
        auto i = varTraits.find(val);
        return ((i != varTraits.end()) && i->second.isRegister());
    }
    return false;
} 

// Check @sc_comb_sig without CLEAR flag 
bool ScVerilogWriter::isCombSig(const SValue& val) 
{
    if (val.isUnknown()) {
        return false;
    }
    
    if (!isCombProcess && !singleBlockCThreads) {
        auto i = varTraits.find(val);
        return ((i != varTraits.end()) && i->second.isCombSig());
    }
    return false;
}

// Check @sc_comb_sig with CLEAR flag 
bool ScVerilogWriter::isCombSigClear(const SValue& val) 
{
    if (val.isUnknown()) {
        return false;
    }
    
    if (!isCombProcess && !singleBlockCThreads) {
        auto i = varTraits.find(val);
        return ((i != varTraits.end()) && i->second.isCombSigClear());
    }
    return false;
}

// Calculate outlined brackets number
unsigned ScVerilogWriter::getBracketNum(const string& s) 
{
    // Get number of leading open brackets
    size_t slen = s.length();
    unsigned leadNum = 0;
    for (; s[leadNum] == '(' && leadNum < slen; leadNum++) {}
    
    // Get number of trailing close brackets
    unsigned trailNum = 0;
    for (; s[slen-1-trailNum] == ')' && trailNum < slen; trailNum++) {}
    
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
    std::string res = s;
    unsigned bracketNum = getBracketNum(s);
    
    if (bracketNum != 0) {
        res.erase(s.length()-bracketNum, bracketNum);
        res.erase(0, bracketNum);
    }
    return res;
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
std::pair<llvm::Optional<uint64_t>, llvm::Optional<int64_t>> 
ScVerilogWriter::getLiteralVal(const std::string& literStr)
{
    std::pair<llvm::Optional<uint64_t>, 
              llvm::Optional<int64_t>> res(llvm::None, llvm::None);
    
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
                                            char radix, size_t minCastWidth, 
                                            size_t lastCastWidth,
                                            CastSign castSign,
                                            bool addNegBrackets)
{
    APSInt val(literStr);

    bool isZero = val.isNullValue();
    bool isOne = val == 1;
    bool isNegative = val < 0;
    
    // Do not use APSInt::getActiveBits() it provides wrong results for -2,-4,-8
    unsigned bitNeeded = APSInt::getBitsNeeded(literStr, 10);
    
    // It is possible to have no cast for non-negative literal in integer range
    bool valueCast = minCastWidth;
    // Maximal/minimal decimal value is 2147483647/-2147483647, 
    // greater values are represented as hex
    if (bitNeeded < (isNegative ? 33 : 32)) {
        if (isZero || isOne) {
            radix = (radix == 2) ? 2 : 10;
        }
    } else {
        radix = 16; valueCast = true;
    }
   
    // Zero value not casted, negative value always considered as signed
    bool isUnsignCast = !isZero && (
                        castSign == CastSign::UCAST && valueCast && !isNegative);
    bool isSignCast = !isZero && (
                      castSign == CastSign::SCAST && valueCast || isNegative);
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
    
//    cout << "literStr " << literStr << " isCast " << isCast << " minCastWidth "
//         << minCastWidth << " isSignCast " << isSignCast << " isUnsignCast " 
//         << isUnsignCast << endl;

    // Negative literal is always based, set base as value width
    if ((isSignCast || isUnsignCast) && !minCastWidth) {
        // +1 bit for sign for positive casted to signed
        minCastWidth = bitNeeded + (isSignCast && !isNegative ? 1 : 0);
    }
    
    string absVal = isNegative ? removeLeadMinus(val.toString(radix)) : 
                                 val.toString(radix);
    string s = (minCastWidth ? to_string(minCastWidth) : "") + baseStr + absVal;
    
    if (lastCastWidth && lastCastWidth != minCastWidth) {
        s = to_string(lastCastWidth) + "'(" + s + ')';
    }
    s = string(isNegative ? "-" : "") + s;
    
    if (isNegative && addNegBrackets) {
        s = '(' + s + ')';
    }
    //cout << "makeLiteralStr " << literStr << " => "  << s << endl;
    
    return s;
}

// Make non-literal term string with sign cast if required
std::string ScVerilogWriter::makeTermStr(const std::string& termStr,
                                         size_t minCastWidth, 
                                         size_t lastCastWidth,
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

    auto names = info.str;
    string rdName = names.first;
    string wrName = names.second;
    
    SCT_TOOL_ASSERT ((info.minCastWidth == 0) == 
                     (info.lastCastWidth == 0 || !info.explCast), 
                     "Only one of minCastWidth/lastCastWidth is non-zero");
    
    if (info.literRadix) {
        // Apply lastCastWidth only for explicit cast or concatenation
        size_t lastCastWidth = (info.explCast || doConcat) ? 
                               info.lastCastWidth : 0; 
        rdName = makeLiteralStr(rdName, info.literRadix, info.minCastWidth, 
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
//    cout << "putString #" << hex << stmt << dec << " " << info.str.first
//         << " exprWidth " << info.exprWidth << endl;
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
                                size_t exprWidth, bool isChannel)
{
    putString(stmt, TermInfo(s, exprWidth, isChannel));
}

// Put/replace the same string into @terms with empty flags and no range
void ScVerilogWriter::putString(const Stmt* stmt, const string& s,
                                size_t exprWidth, bool isChannel)
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

void ScVerilogWriter::setExprSign(const Stmt* stmt, bool sign, bool liter) 
{
    //cout << "setExprSign #" << hex << stmt << dec << " sign " << sign << endl;

    auto i = terms.find(stmt);
    if (i != terms.end()) {
        auto& info = i->second;
        info.exprSign = liter ? ExprSign::LEXPR :
                        sign ? ExprSign::SEXPR : ExprSign::UEXPR;
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
size_t ScVerilogWriter::getExprWidth(const Stmt* stmt, bool doConcat) {
    
    auto i = terms.find(stmt);
    if (i == terms.end()) {
        return 0;
    }
    auto& info = i->second;
    
    // Get last cast width if exists, required for concatenation
    size_t width = ((info.explCast || doConcat) && info.lastCastWidth) ? 
                    info.lastCastWidth : info.exprWidth;
    return width;
}

// Get minimal expression data width as minimal of @minCast and @exprWidth
// \return @exprWidth for given statement or 0 if width unknown
size_t ScVerilogWriter::getMinExprWidth(const Stmt* stmt) {
    
    auto i = terms.find(stmt);
    if (i == terms.end()) {
        return 0;
    }
    auto& info = i->second;
    
    // Get minimal width to avoid part/bit selection outside of variable width
    size_t width = info.minCastWidth == 0 ? info.exprWidth :
                        (info.exprWidth == 0 ? info.minCastWidth : 
                         min(info.minCastWidth, info.exprWidth));
    return width;
}

// Get expression data width from @lastCast or @exprWidth after that or 
// type information at the end
// \return expression/type width or 64 with error reporting
size_t ScVerilogWriter::getExprTypeWidth(const Expr* expr, size_t defWidth) 
{
    size_t width = getExprWidth(expr);

    if (width == 0) {
        if (auto typeInfo = getIntTraits(getTypeForWidth(expr))) {
            width = typeInfo.getValue().first;
            
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
size_t ScVerilogWriter::getMinExprTypeWidth(const Expr* expr, size_t defWidth) 
{
    size_t width = getMinExprWidth(expr);

    if (width == 0) {
        if (auto typeInfo = getIntTraits(getTypeForWidth(expr))) {
            width = typeInfo.getValue().first;
            
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
                                   size_t width, bool isChannel) 
{
    bool isReg = isRegister(lval) || isCombSig(lval) || isCombSigClear(lval);
    bool isRecord = isUserDefinedClass(lval.getType(), true);
    // Do not use non-blocking assignment for channel in METHOD
    bool nbAssign = (isClockThreadReset && isReg && !singleBlockCThreads) ||
                    (!isCombProcess && singleBlockCThreads && isChannel);
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
                             "copyTerm : No term for sub-statement " +
                             llvm::to_hexString((size_t) srcStmt, false));*/
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
                             "addTerm : No term for sub-statement " +
                             llvm::to_hexString((size_t) srcStmt, false));*/
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
                             "copyTermInBrackets : No term for sub-statement " +
                             llvm::to_hexString((size_t) srcStmt, false));*/
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
                             "copyTermRemoveBrackets : No term for sub-statement " +
                             llvm::to_hexString((size_t) srcStmt, false));*/
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

        size_t width = 64; 
        if (auto typeInfo = getIntTraits(type, true)) {
            width = typeInfo.getValue().first;
            
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
        SCT_INTERNAL_FATAL(stmt->getBeginLoc(),
                         "putTypeCast : No term for sub-statement "+
                         llvm::to_hexString((size_t)srcStmt, false));
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

        size_t width = 64;
        if (auto typeInfo = getIntTraits(type, true)) {
            width = typeInfo.getValue().first;
            
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
        SCT_INTERNAL_FATAL(stmt->getBeginLoc(),
                           "setLastCastWidth : No term for statement "+
                           llvm::to_hexString((size_t)stmt, false));
    }
}

// Extend type width for arithmetic operation  argument self-determined in SV,
// this is type cast to given @width
void ScVerilogWriter::extendTypeWidth(const clang::Stmt* stmt,
                                      const size_t width)
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
        SCT_INTERNAL_FATAL(stmt->getBeginLoc(),
                           "addTypeCast : No term for sub-statement "+
                           llvm::to_hexString((size_t)stmt, false));
    }
}

// Used for literals
void ScVerilogWriter::putLiteral(const Stmt* stmt, const SValue& val) 
{
    if (skipTerm) return;

    if (val.isInteger()) {
        string s = val.getInteger().toString(10);
        size_t width = APSInt::getBitsNeeded(s, 10);
        char radix = val.getRadix();
        
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
                                 bool replaceConstEnable) 
{
    if (skipTerm) return;
    SCT_TOOL_ASSERT (val.isVariable() || val.isTmpVariable(), 
                     "No variable found to declare");
    SCT_TOOL_ASSERT (!val.isReference() || val.isConstReference(), 
                     "No non-constant reference expected to be declared");
    
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
            
            // Loop counter not replaced by value
            notReplacedVars.insert(val);
            
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
        bool isConst = val.getType().isConstQualified();
        bool isConstRef = val.isConstReference();
        bool isRecord = isUserDefinedClass(val.getType(), true);
        SCT_TOOL_ASSERT (!isRecord || !init,
                         "Unexpected record variable with initialization");
        
        //cout << "putVarDecl val " << val << " isRecord " << isRecord
        //     << " isConst " << isConst << endl;
         
        pair<string, string> names = getVarName(val);
        
        // Do not declare/initialize record as individual fields are declared 
        if (!isRecord) {
            // Skip constant variable if they are replaced with values
            // Possible only for non-reference variable initialized with integer,
            // which is checked in @replaceConstEnable
            if (keepConstVariables || !replaceConstEnable || 
                (!isConst && !isConstRef))
            {
                // Constant/variable not replaced by value
                //cout << "add to notReplacedVars val " << val << endl;
                notReplacedVars.insert(val);
            }

            // Register assignment statement, for declared variables only
            putVarAssignStmt(val, stmt);

            if (!isReg) {
                // Combinatorial variables and combinational process variables
                auto i = localDeclVerilog.begin();
                for (; i != localDeclVerilog.end(); ++i) {
                    if (i->first == val) {
                        break;
                    }
                }
                if (i == localDeclVerilog.end()) {
                    string s = getVarDeclVerilog(type, names.first, init);
                    //cout << "put to localDeclVerilog val " << val << endl;
                    localDeclVerilog.push_back({val, s});
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
            if (stmt && (init || initLocalVars) && !removeUnusedInit) {
                if (init && terms.count(init) == 0) {
                    SCT_INTERNAL_FATAL (init->getBeginLoc(),
                                        "No term for variable initialization");
                }

                // Use read name in @assign (emptySensitivity)
                bool secName = !isClockThreadReset && isReg && !emptySensitivity;
                string lhsName = secName ? names.second : names.first;
                // If no initializer use 0 
                string rhsName = init ? getTermAsRValue(init).first : "0";

                putAssignBase(stmt, val, lhsName, rhsName, 0);
            }
        }
        
        if (isRecord) {
            // Record never replaced by integer value
            //cout << "    REC added to globalNotReplacedConst" << endl;
            notReplacedVars.insert(val);
        }
    }
}

// Array declaration statement, array initialization added as separate
// assignments for @stmt  
void ScVerilogWriter::putArrayDecl(const Stmt* stmt, const SValue& val, 
                                   const QualType& type, 
                                   const vector<size_t>& arrSizes) 
{
    if (skipTerm) return;
    SCT_TOOL_ASSERT (val.isVariable() || val.isTmpVariable(), 
                     "No variable found");
    
    bool isReg = isRegister(val);
    bool isRecord = isUserDefinedClassArray(val.getType(), true);
    //cout << "putArrayDecl val " << val << " " << isReg << isRecord << endl;
    
    // Do not declare record as individual fields are declared
    if (!isReg && !isRecord) {
        // Constant/variable array not replaced by value
        notReplacedVars.insert(val);
        
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
            pair<string, string> names = getVarName(val);

            // Get array element indices
            string indx;
            for (auto j : arrSizes) {
                indx = indx + '['+to_string(j)+']';
            }
            
            // Array variable declaration, no declaration for registers
            string s = getVarDeclVerilog(type, names.first) + indx;
            localDeclVerilog.push_back({val, s});
            //cout << "   " << s << endl;
        } 
    }
}

// Put string of @init statement to use instead of the reference variable
// Used for any non-constant reference 
void ScVerilogWriter::storeRefVarDecl(const SValue& val, const Expr* init) 
{
    if (skipTerm) return;
    SCT_TOOL_ASSERT (val.isVariable() || val.isTmpVariable(), "No variable found");

    if (terms.count(init) != 0) {
        // Replace reference with new string, required for second call of the function
        refValueDecl[val] = getTermAsRValue(init);
        
    } else {
        SCT_INTERNAL_FATAL(init->getBeginLoc(),
                           "putRefVarDecl : no term for right part " +
                            llvm::to_hexString((size_t)init, false));
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
        SCT_INTERNAL_FATAL(init->getBeginLoc(),
                           "storePointerVarDecl : no term for right part " +
                            llvm::to_hexString((size_t)init, false));
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
void ScVerilogWriter::putValueExpr(const Stmt* stmt, const SValue& val,
                                   const vector<SValue>& recarrs, 
                                   bool elemOfMifArr, bool elemOfRecArr,
                                   const string& refRecarrIndxStr)
{
    if (skipTerm) return;
//    cout << "putValueExpr for stmt " << hex << stmt << dec << ", val " << val 
//         << ", elemMifArr " << elemOfMifArr << ", elemRecArr " << elemOfRecArr 
//         << ", recarrs size " << recarrs.size() << " arraySubIndices.size "
//         << arraySubIndices.size() << endl;

    if (val.isInteger()) {
        // Integer put for evaluated expressions
        putLiteral(stmt, val);
        
    } else 
    if (val.isVariable() || val.isTmpVariable()) {
        pair<string, string> names = getVarName(val);
        
        // Do not use MIF indices in reset section as variable is declared locally
        bool isExtrCombVarNoIndx = isClockThreadReset && 
                                   extrCombVarUsedRst.count(val) != 0;
        
        // Add MIF variable prefix for its member access
        if (elemOfMifArr && !isExtrCombVarNoIndx) {
            // Access to member of MIF from its process body 
            // For local variables nothing required
            if (!isLocalVariable(val)) {
                string indxSuff = getIndexFromRecordName(MIFValueName.second);
                names.first += indxSuff;
                names.second += indxSuff;
                //cout << "  MIF array add suffix " << indxSuff << endl;
            }
        }
        
        // Add record/MIF variable prefix for its member access
        if (elemOfRecArr) {
            // Access to member of record/MIF from this record method called
            // possible from parent module process
            string indxSuff = getIndexFromRecordName(recordValueName.second);
            names.first += indxSuff;
            names.second += indxSuff;
            //cout << "  REC from its method suffix " << indxSuff << endl;
            
        } else 
        if (!recarrs.empty()) {
            // Access record/MIF member from module function or access 
            // record array elememnt
            // Add record array parameter passed by reference indices
            // Commented to support array record with array member func parameter
            //if (isLocalVariable(val)) {
                names.first += refRecarrIndxStr;
                names.second += refRecarrIndxStr;
            //}
            //cout << "  isLocalVariable " << val << " " << isLocalVariable(val) << endl;
            
            // Add record array index inside
            string indxSuff = getRecordIndxs(recarrs);
            names.first += indxSuff;
            names.second += indxSuff;
            //cout << "  REC from module suffix " << refRecarrIndxStr << indxSuff
            //     << " names.first " << names.first << endl;
        }
        
        // Get variable width 
        size_t width = 0;
        if (auto typeInfo = getIntTraits(val.getType(), true)) {
            width = typeInfo->first;
            
        } /*else {
            ScDiag::reportScDiag(stmt->getBeginLoc(),
                                 ScDiag::SYNTH_UNKNOWN_TYPE_WIDTH) << 
                                 val.getType().getAsString();
        }*/
        
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
                                     bool elemOfMifArr, bool elemOfRecArr)
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
        string indxSuff = getIndexFromRecordName(MIFValueName.second);
        names.first += indxSuff;
        names.second += indxSuff;
        //cout << "  elem of MIF array add suffix " << indxSuff << endl;
    }
    
    // Add record/MIF variable prefix for its member access
    if (elemOfRecArr) {
        // Access to member of record/MIF from this record/MIF called method
        string indxSuff = getIndexFromRecordName(recordValueName.second);
        names.first += indxSuff;
        names.second += indxSuff;
        //cout << "  REC array add suffix " << indxSuff << endl;

    } else 
    if ( !recarrs.empty() ) {
        // Access record/MIF member from module function
        // Get indices for most all records
        string indxSuff = getRecordIndxs(recarrs);
        names.first += indxSuff;
        names.second += indxSuff;
        //cout << "  recvar array add suffix " << indxSuff << endl;
    }
    
    // Get variable width, channel must always have determinable width 
    size_t width = 0;
    if (auto typeInfo = getIntTraits(cval.getScChannel()->getType())) {
        width = typeInfo->first;
        
    } else {
        ScDiag::reportScDiag(stmt->getBeginLoc(),
                             ScDiag::SYNTH_UNKNOWN_TYPE_WIDTH) << 
                             cval.getScChannel()->getType().getAsString();
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
        size_t width = 0;
        if (auto typeInfo = getIntTraits(val.getType().getNonReferenceType())) {
            width = typeInfo->first;
        }
        
        putString(stmt, names, width);

        //cout << "putLocalPtrValueExpr #" << hex << stmt << dec << " val " << val << " width " << width << endl;
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
        size_t width = 0;
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

    if (terms.count(rhs) != 0 ) {
        if (terms.count(lhs) != 0) {
            auto info = terms.at(lhs);
            
            // Get LHS names
            auto names = info.str;

            bool isReg = isRegister(lval) || isCombSig(lval) || 
                         isCombSigClear(lval);
            bool isRecord = isUserDefinedClass(lval.getType(), true);
            SCT_TOOL_ASSERT (!isRecord, "Unexpected record variable");
            
            // Use read name in @assign (emptySensitivity)
            bool secName = !isClockThreadReset && isReg && !emptySensitivity;
            string lhsName = removeBrackets(secName ? names.second:names.first);
            string rhsName = removeBrackets(getTermAsRValue(rhs).first);

            size_t width = getExprWidth(lhs); 

            putAssignBase(stmt, lval, lhsName, rhsName, width, info.isChannel);
            //cout << "+++ " << hex << (unsigned long)stmt << dec << endl;
            
        } else {
            SCT_INTERNAL_FATAL(stmt->getBeginLoc(),
                             "putAssign : no term for left part "+ 
                             llvm::to_hexString((size_t)lhs, false));
        }
    } else {
        SCT_INTERNAL_FATAL(stmt->getBeginLoc(),
                         "putAssign : no term for right part " +
                         llvm::to_hexString((size_t)rhs, false));
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
                     isCombSigClear(lval);
        bool isRecord = isUserDefinedClass(lval.getType(), true);
        SCT_TOOL_ASSERT (!isRecord, "Unexpected record variable");

        // Use read name in @assign (emptySensitivity)
        bool secName = !isClockThreadReset && isReg && !emptySensitivity;
        string lhsName = removeBrackets(secName ? names.second : names.first);
        string rhsName = removeBrackets(getTermAsRValue(rhs).first);
        
        // Get LHS variable width 
        size_t width = 0;
        if (auto typeInfo = getIntTraits(lval.getType())) {
            width = typeInfo->first;
        }
                
        putAssignBase(stmt, lval, lhsName, rhsName, width);
        
    } else {
        SCT_INTERNAL_FATAL(stmt->getBeginLoc(),
                         "putAssign : no term for right part " +
                         llvm::to_hexString((size_t)rhs, false));
    }
}

// Assignment for record variable (record copy)
void ScVerilogWriter::putRecordAssign(const Stmt* stmt, 
                                      const SValue& lvar, const SValue& lrec, 
                                      const SValue& rvar, const SValue& rrec,
                                      const string& lrecSuffix,
                                      const string& rrecSuffix) 
{
    bool isReg = isRegister(lvar) || isCombSig(lvar) || isCombSigClear(lvar);
    bool nbAssign = isClockThreadReset && isReg && !singleBlockCThreads;
    bool secName = !isClockThreadReset && isReg && !emptySensitivity;
    SCT_TOOL_ASSERT (lrec.isRecord(), "lrec is not record value");
    SCT_TOOL_ASSERT (rrec.isRecord(), "rrec is not record value");
    //cout << "recSuffix " << lrecSuffix << " " << rrecSuffix << endl;
    
    // Record assignment, assign all the fields
    string s;
    bool first = true;
    auto recDecl = lrec.getType()->getAsRecordDecl();
    
    for (auto fieldDecl : recDecl->fields()) {
        // Get name for LHS
        SValue lfval(fieldDecl, lrec);
        const auto& lnames = getVarName(lfval);
        string lhsName = (secName ? lnames.second : lnames.first) + lrecSuffix;
        
        // Get name for RHS, use read name here
        SValue rfval(fieldDecl, rrec);
        const auto& rnames = getVarName(rfval);
        string rhsName = rnames.first + rrecSuffix;

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
                                       const SValue& ival, const Expr* iexpr) 
{
    if (skipTerm) return;
    SCT_TOOL_ASSERT ((bval.isVariable() || bval.isTmpVariable()) && 
                     ival.isInteger(), "No variable or integer found");
    SCT_TOOL_ASSERT (stmt != iexpr, "Incorrect array inializer");

    if (terms.count(iexpr)) {
        // Get unique variable name for write
        auto names = getVarName(bval);

        bool isReg = isRegister(bval);
        bool nbAssign = isClockThreadReset && isReg && !singleBlockCThreads;
        bool secName = !isClockThreadReset && isReg;

        string s = (secName ? names.second : names.first) + 
                   "[" + ival.asString() + "]" + 
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
                                           const SValue& ival) 
{
    if (skipTerm) return;
    SCT_TOOL_ASSERT ((bval.isVariable() || bval.isTmpVariable()) && 
                     ival.isInteger(), "No variable or integer found");

    // Get unique variable name for write
    auto names = getVarName(bval);

    bool isReg = isRegister(bval);
    bool nbAssign = isClockThreadReset && isReg && !singleBlockCThreads;
    bool secName = !isClockThreadReset && isReg;

    string s = (secName ? names.second : names.first) + 
               "[" + ival.asString() + "]" + 
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
    //cout << "addSubscriptIndex for " << aval << hex << " indx " << indx << dec << endl;
}


// Get string from indices "[index1][index2]..." stored in @arraySubIndices
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
            //cout << "---- " << hex << (unsigned long)(i->second) << dec << endl;
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
        
        size_t width = 0;
        if (auto typeWidth = getAnyTypeWidth(type, true, true)) {
            width = *typeWidth;
            
        }/* else {
            ScDiag::reportScDiag(stmt->getBeginLoc(),
                                 ScDiag::SYNTH_UNKNOWN_TYPE_WIDTH) << 
                                 type.getAsString();
        }*/

        putString(stmt, pair<string,string>(rdName, wrName), width);
        
    } else {
        SCT_INTERNAL_FATAL(stmt->getBeginLoc(),
                         "putArrayIndexExpr : no term for base/index part " +
                         llvm::to_hexString((size_t)((terms.count(base)) ? 
                                            index : base),false) );
    }
}

// Constant and variable based range part-select
void ScVerilogWriter::putPartSelectExpr(const Stmt* stmt,  const SValue& val,
                                        const Expr* base,
                                        const Expr* hindx, const Expr* lindx,
                                        bool useDelta)
{
    if (skipTerm) return;
    
    // Part select not replaced by value, base need to be declared
    notReplacedVars.insert(val);
    
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
        size_t castWidth = getExprTypeWidth(base);
        // Internal (minimal) type width, this number of bits can be accessed 
        size_t intrWidth = getMinExprTypeWidth(base);

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
            if (intrWidth == 1 && (!lval.isNullValue() || !hval.isNullValue())) {
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
        
        size_t width = useDelta ? getLiteralAbs(lindx) : 
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
    
    // Bit select not replaced by value, base need to be declared
    notReplacedVars.insert(val);
    //cout << "notReplacedVars add val " << val << endl;
    
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
        size_t castWidth = getExprTypeWidth(base);
        // Internal (minimal) type width, this number of bits can be accessed 
        size_t intrWidth = getMinExprTypeWidth(base);
        
        // Check incorrect bit index
        if (indexInfo.literRadix) {
            APSInt lval(indexInfo.str.first);
            if (lval < 0) {
                ScDiag::reportScDiag(index->getBeginLoc(), 
                                     ScDiag::SC_BIT_WRONG_INDEX);
            }
            if (intrWidth == 1 && !lval.isNullValue()) {
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

// Binary operators "+", "-", "*", "/", "%", "||", "&&", "&", "|", "^",
// "<<<", ">>>", ">", "<", ">=", "<=", "==", "!="
void ScVerilogWriter::putBinary(const Stmt* stmt, string opcode, 
                                const Expr* lhs, const Expr* rhs) 
{
    if (skipTerm) return;

    if (terms.count(lhs) && terms.count(rhs)) {
        bool negBrackets = opcode == "+" || opcode == "-" || 
                           opcode == "*" || opcode == "/" || opcode == "%" ||
                           opcode == "&" || opcode == "|" || opcode == "^" ||
                           opcode == ">>>" || opcode == "<<<";
        bool doSignCast = !skipSignCast && (
                            opcode == "+" || opcode == "-" || 
                            opcode == "*" || opcode == "/" || opcode == "%" ||
                            opcode == "&" || opcode == "|" || opcode == "^");
        bool checkSignedExpr = doSignCast;
        
        auto& linfo = terms.at(lhs);
        auto& rinfo = terms.at(rhs);
        
        //cout << "putBinary #" << hex << stmt << dec << " castSign " << (int)linfo.castSign << (int)rinfo.castSign
        //     << " exprSign " << (int)linfo.exprSign << (int)rinfo.exprSign << endl;
        
        // Warning reported as division/reminder results for SC types 
        // are different in SC and SV, warning for other operations
        if (!linfo.literRadix && linfo.exprSign != ExprSign::LEXPR && 
            linfo.castSign == CastSign::UCAST || 
            !rinfo.literRadix && rinfo.exprSign != ExprSign::LEXPR &&
            rinfo.castSign == CastSign::UCAST)  
        {
            if (opcode == "+" || opcode == "-" || opcode == "*") {
                ScDiag::reportScDiag(stmt->getBeginLoc(), 
                                     ScDiag::SYNTH_SIGN_UNSIGN_MIX);
            } else
            if (opcode == "/" || opcode == "%") {
                ScDiag::reportScDiag(stmt->getBeginLoc(), 
                                     ScDiag::SYNTH_DIV_SIGN_UNSIGN_MIX);
            } else 
            if (opcode == "&" || opcode == "|" || opcode == "^") {
                ScDiag::reportScDiag(stmt->getBeginLoc(),
                                     ScDiag::SYNTH_BITWISE_SIGN_UNSIGN_MIX);
            }
        }
        
        // Error reported for negative literal casted to unsigned
        auto checkNegLiterCast = 
            [stmt](TermInfo& first) 
            {
                if (first.literRadix && first.castSign == CastSign::UCAST) {
                    APSInt val(first.str.first);
                    if (val < 0) {
                        ScDiag::reportScDiag(stmt->getBeginLoc(), 
                                    ScDiag::SYNTH_NEG_LITER_UCAST);
                    }
                }
            };
            
        if (opcode == "/" || opcode == "%") {
            checkNegLiterCast(linfo);
            checkNegLiterCast(rinfo);
        }

        // Expression with both literals not considered as signed expression
        bool literExpr = linfo.literRadix && rinfo.literRadix;
        // Expression is signed in generated code
        bool signedExpr = false;

        // Evaluate if expression is signed in generated code
        auto setSignedExpr = 
            [&signedExpr](TermInfo& first, const Expr* fexpr, 
                          TermInfo& secnd, const Expr* sexpr) 
            {
                QualType ftype = fexpr->getType().getCanonicalType();
                QualType stype = sexpr->getType().getCanonicalType();
                
                if (!first.literRadix && first.exprSign != ExprSign::LEXPR &&
                    (first.castSign == CastSign::SCAST || 
                     first.castSign == CastSign::SACAST || 
                     first.exprSign == ExprSign::SEXPR || 
                     isSignedType(ftype)) &&
                     (secnd.literRadix || secnd.exprSign == ExprSign::LEXPR ||
                      secnd.castSign == CastSign::SCAST || 
                      secnd.castSign == CastSign::SACAST || 
                      secnd.exprSign == ExprSign::SEXPR || 
                      isSignedType(stype)))
                {
                    signedExpr = true;
                }
            };
        
        if (checkSignedExpr) {
            setSignedExpr(linfo, lhs, rinfo, rhs);
            setSignedExpr(rinfo, rhs, linfo, lhs);
        }
        //cout << "   signedExpr " << signedExpr << endl;
            
        // Set SCAST for operand if another non-literal operand has UCAST or
        // signed expression, that converts signed+unsigned mix to signed in SV.
        // No cast by literal to keep unsigned arithmetic, commonly used.
        // Do not apply cast if there is explicit cast.
        // That is not equal to SC but better than have unsigned semantic in SV
        // which is also not equal to SC
        auto setExprSCast = 
            [&signedExpr](TermInfo& first, const Expr* fexpr, 
                          TermInfo& secnd, const Expr* sexpr) 
            {
                QualType ftype = fexpr->getType().getCanonicalType();
                QualType stype = sexpr->getType().getCanonicalType();
                
                if (!first.literRadix && first.exprSign != ExprSign::LEXPR &&
                    (first.castSign == CastSign::UCAST ||
                     first.exprSign == ExprSign::SEXPR) && 
                    secnd.castSign == CastSign::NOCAST && !secnd.explCast)
                {
                    // Do not apply sign cast to signed expression
                    if (secnd.exprSign != ExprSign::SEXPR && !isSignedType(stype)) {
                        first.castSign = CastSign::NOCAST;
                        secnd.castSign = CastSign::SACAST;
                        //cout << "setExprSCast executed" << endl;
                    }
                    signedExpr = true;
                }
            };
        
        // Set non-literal SCAST for one operand if another operand is @sc_bigint   
        // which is casted to @sc_signed, convert sc_bigint+unsigned 
        // to signed operation
        auto setBigIntSCast = 
            [&signedExpr](TermInfo& first, const Expr* fexpr, 
                          TermInfo& secnd, const Expr* sexpr)  
            {
                QualType ftype = fexpr->getType().getCanonicalType();
                QualType stype = sexpr->getType().getCanonicalType();
                
                if (!first.literRadix && first.exprSign != ExprSign::LEXPR &&
                    isScBigInt(ftype))
                {
                    // Do not apply sign cast to signed expression
                    if (!secnd.literRadix && !isSignedType(stype) &&
                        secnd.exprSign != ExprSign::SEXPR) 
                    {
                        secnd.castSign = CastSign::SACAST;
                        //cout << "setBigIntSCast executed" << endl;
                    }
                    signedExpr = true;
                }
            };    
            
        // Set non-literal SCAST for @sc_biguint operand if another operand is 
        // signed expression or literal, convert sc_bigint+int/sc_int/sc_bigint
        // to signed operation
        auto setBigUintSCast = 
            [&signedExpr](TermInfo& first, const Expr* fexpr, 
               TermInfo& secnd, const Expr* sexpr) 
            {
                QualType ftype = fexpr->getType().getCanonicalType();
                QualType stype = sexpr->getType().getCanonicalType();
                
                if (!first.literRadix && first.exprSign != ExprSign::LEXPR &&
                    isScBigUInt(ftype) && isSignedType(stype)) 
                {
                    // Do not apply sign cast to signed expression
                    if (first.exprSign != ExprSign::SEXPR) {
                        first.castSign = CastSign::SACAST;
                        //cout << "setBigUintSCast executed" << endl;
                    }
                    signedExpr = true;
                }
            };     
            
        // Set SCAST for literal if another operand is signed, 
        // that adds "s" for literal if it is in sized form ('d -> 'sd),
        // Required for all operations
        auto setLiterSCast = 
            [](TermInfo& first, const Expr* fexpr, 
               TermInfo& secnd, const Expr* sexpr) 
            {
                QualType stype = sexpr->getType().getCanonicalType();

                // Check NOCAST to avoid break UCAST`ed literal
                if (first.literRadix && first.castSign == CastSign::NOCAST && 
                    isSignedType(stype)) {
                    first.castSign = CastSign::SCAST;
                }
            };  
            
        if (doSignCast) {
            setExprSCast(linfo, lhs, rinfo, rhs);
            setExprSCast(rinfo, rhs, linfo, lhs);
            
            setBigIntSCast(linfo, lhs, rinfo, rhs);
            setBigIntSCast(rinfo, rhs, linfo, lhs);
            
            setBigUintSCast(linfo, lhs, rinfo, rhs);
            setBigUintSCast(rinfo, rhs, linfo, lhs);

            setLiterSCast(linfo, lhs, rinfo, rhs);
            setLiterSCast(rinfo, rhs, linfo, lhs);
        }
        
        string s = getTermAsRValue(lhs, false, negBrackets, doSignCast).first + 
                   " " + opcode + " " +
                   getTermAsRValue(rhs, false, negBrackets, doSignCast).first;
        
        size_t width = 0;
        size_t lwidth = getExprWidth(lhs); 
        size_t rwidth = getExprWidth(rhs);
        size_t maxwidth = (lwidth > rwidth) ? lwidth : rwidth;
        
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
                    if (terms.at(rhs).literRadix) {
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
        
        putString(stmt, s, width);
        clearSimpleTerm(stmt);
        setExprSign(stmt, signedExpr, literExpr);
        //cout << "    signedExpr " << int(signedExpr) << " literExpr " << literExpr << endl;
        
        // Set increase result width 
        if (opcode == "+" || opcode == "-" || opcode == "*" || opcode == "<<<"){
            setIncrWidth(stmt);
        }
        
        //cout << "putBinary opcode " << opcode << " width " << width << endl;
    } else {
        SCT_INTERNAL_FATAL(stmt->getBeginLoc(),
                         "putBinary : No term for lhs/rhs statement " +
                         llvm::to_hexString((size_t)(terms.count(lhs) ? 
                                            rhs : lhs), false) );
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
        
        bool doSignCast = !skipSignCast && (
                            opcode == "+" || opcode == "-" || 
                            opcode == "*" || opcode == "/" || opcode == "%" ||
                            opcode == "&" || opcode == "|" || opcode == "^");
        
        auto& linfo = terms.at(lhs);
        auto& rinfo = terms.at(rhs);
        SCT_TOOL_ASSERT (!linfo.isChannel, "putCompAssign for SC channel object");
        
        //cout << "putCompAssign castSign " << (int)linfo.castSign << (int)rinfo.castSign << endl;
        
        // Report warning for signed type in bitwise operators
        if (opcode == "&" || opcode == "|" || opcode == "^") {
            if (!rinfo.literRadix && (rinfo.castSign == CastSign::UCAST ||
                rinfo.castSign == CastSign::SCAST)) {
                ScDiag::reportScDiag(stmt->getBeginLoc(),
                                     ScDiag::SYNTH_BITWISE_SIGN_UNSIGN_MIX);
            }
        }
        
        // Set non-literal SCAST for @sc_biguint operand if another operand is 
        // signed expression or literal, convert sc_bigint+int/sc_int/sc_bigint
        // to signed operation
        auto setBigUintExprSCast = 
            [](TermInfo& first, const Expr* fexpr, 
               TermInfo& secnd, const Expr* sexpr) 
            {
                QualType ftype = fexpr->getType().getCanonicalType();
                QualType stype = sexpr->getType().getCanonicalType();
                
                if (!first.literRadix && isScBigUInt(ftype) && 
                    isSignedType(stype)) 
                {
                    // Do not apply sign cast to signed expression
                    if (first.exprSign != ExprSign::SEXPR) {
                        first.castSign = CastSign::SACAST;
                    }
                }
            }; 
            
        // Do not need another rules used in binary operation
        
        if (doSignCast) {
            setBigUintExprSCast(linfo, lhs, rinfo, rhs);
            setBigUintExprSCast(rinfo, rhs, linfo, lhs);
        }    

        auto names = getTermAsRValue(lhs);
        
        bool isReg = isRegister(lval) || isCombSig(lval) || isCombSigClear(lval);
        bool nbAssign = isClockThreadReset && isReg && !singleBlockCThreads;
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

        size_t width = getExprWidth(lhs); 
        
        putString(stmt, s, width);
        clearSimpleTerm(stmt);
        // Do not need to put @exprSign for compound statement
        
    } else {
        SCT_INTERNAL_FATAL(stmt->getBeginLoc(), 
                         "putCompAssign : No term for lhs/rhs statement " +
                         llvm::to_hexString((size_t)(terms.count(lhs) ? 
                                             rhs : lhs), false) );
    }
}

// Unary operators "-", ++", "--", "~", "!", "&", "|", "^", "~&", "~|", "~^"
void ScVerilogWriter::putUnary(const Stmt* stmt, string opcode, const Expr* rhs, 
                               bool isPrefix) 
{
    if (skipTerm) return;

    if (terms.count(rhs)) {
        
        bool doSignCast = !skipSignCast && opcode == "-";
        bool checkSignedExpr = doSignCast || opcode == "++" || 
                               opcode == "--" || opcode == "~";
        
        auto& info = terms.at(rhs);
        
        // Is this expression signed in generated code
        bool signedExpr = false;
        
        // Evaluate if expression is signed in generated code
        auto setSignedExpr = 
            [&signedExpr](TermInfo& first, const Expr* fexpr) 
            {
                QualType ftype = fexpr->getType().getCanonicalType();
                
                if (!first.literRadix && first.exprSign != ExprSign::LEXPR &&
                    (first.castSign == CastSign::SCAST || 
                     first.castSign == CastSign::SACAST || 
                     first.exprSign == ExprSign::SEXPR || 
                     isSignedType(ftype)))
                {
                    signedExpr = true;
                }
            };
            
        if (checkSignedExpr) {
            setSignedExpr(info, rhs);
        }
        
        // Set non-literal SCAST for @sc_biguint operand for unary minus
        // For ++ and -- operators @sc_uint/@sc_biguint remain unsigned
        auto setBigUintExprSCast = 
            [&signedExpr](TermInfo& first, const Expr* fexpr) 
            {
                QualType ftype = fexpr->getType().getCanonicalType();
                
                if (!first.literRadix && isScBigUInt(ftype)) {
                    // Do not apply sign cast to signed expression
                    if (first.exprSign != ExprSign::SEXPR) {
                        first.castSign = CastSign::SACAST;
                    }
                    signedExpr = true;
                }
            }; 
        
        if (doSignCast) {
            setBigUintExprSCast(info, rhs);
        }
            
        string s;
        char literRadix = info.literRadix;
        bool isLiteralMinus = (opcode == "-") && literRadix;
        
        if (isLiteralMinus) {
            auto names = info.str;
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
            size_t baseWidth = getExprTypeWidth(rhs);
            bool skipOper = opcode == "|" && baseWidth == 1;
            
            s = skipOper ? name : isPrefix ? opcode + name : name + opcode;
        }

        size_t width = 0;
        size_t rwidth = getExprWidth(rhs);

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
        
        putString(stmt, s, width);
        clearSimpleTerm(stmt);
        setExprSign(stmt, signedExpr, literRadix);
        
        // Literal with minus preserved as literal, castWidth copied
        if (isLiteralMinus) {
            auto& sinfo = terms.at(stmt);
            sinfo.literRadix = literRadix;
            sinfo.minCastWidth = info.minCastWidth;
            sinfo.lastCastWidth = info.lastCastWidth;
            sinfo.explCast = info.explCast;
        }
        
        // Set increase result width 
        if (opcode == "++" || opcode == "--") {
            setIncrWidth(stmt);
        }
        
        //cout << "putUnary opcode " << opcode << " width " << width << endl;
    } else {
        SCT_INTERNAL_FATAL(stmt->getBeginLoc(), 
                        "putUnary : No term for rhs statement " +
                        llvm::to_hexString((size_t)rhs, false));
    }
}

// Put sign cast for literals and expressions
void ScVerilogWriter::putSignCast(const clang::Stmt* stmt, CastSign castSign)
{
    if (skipTerm || skipSignCast) return;

    if (terms.count(stmt)) {
        auto& info = terms.at(stmt);
        // Apply SACAST after explicit cast to get @signed'({1'b0, ...});
        info.castSign = info.explCast && castSign == CastSign::SCAST ? 
                        CastSign::SACAST : castSign;
        
        //cout << "putSignCast #" << hex << stmt << dec << " castSign " << (int)castSign << endl;
    } else {
        SCT_INTERNAL_FATAL(stmt->getBeginLoc(), 
                           "putSignCast : No term for statement " +
                           llvm::to_hexString((size_t)stmt, false));
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
        
        string conds = getTermAsRValue(cond).first + " ? ";
        string rdName = conds + getTermAsRValue(lhs).first + " : " + 
                        getTermAsRValue(rhs).first;
        string wrName = conds + getTermAsRValue(lhs).second + " : " + 
                        getTermAsRValue(rhs).second;
        
        // LHS and RHS widths should be the same, so take non-zero one
        size_t lwidth = getExprWidth(lhs);
        size_t rwidth = getExprWidth(rhs); 
        size_t width = lwidth ? lwidth : rwidth; 
        
        putString(stmt, pair<string,string>(rdName, wrName), width);
        clearSimpleTerm(stmt);

        // Keep expression signed cast
        bool signedExpr = linfo.exprSign == ExprSign::SEXPR || 
                          rinfo.exprSign == ExprSign::SEXPR;
        // Expression with both literals not considered as signed expression
        bool literExpr = linfo.literRadix && rinfo.literRadix;
        setExprSign(stmt, signedExpr, literExpr);
        
    } else {
        SCT_INTERNAL_FATAL(stmt->getBeginLoc(),
                         "putCondStmt : No term for cond/lhs/rhs statement " +
                         llvm::to_hexString((size_t)((terms.count(cond)) ?
                         ((terms.count(lhs)) ? rhs : lhs) : cond), false));
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
                getTermAsRValue(first, false, false, false, true).first + ", " + 
                getTermAsRValue(second, false, false, false, true).first + "}";
        string wrName = "{" + 
                getTermAsRValue(first, false, false, false, true).second + ", " + 
                getTermAsRValue(second, false, false, false, true).second + "}";
        
        // Take sum of LHS and RHS widths if both of them are known
        size_t lwidth = getExprWidth(first, true);
        size_t rwidth = getExprWidth(second, true); 
        size_t width = (lwidth && rwidth) ? (lwidth + rwidth) : 0;
        
        putString(stmt, pair<string,string>(rdName, wrName), width);
        
    } else {
        SCT_INTERNAL_FATAL(stmt->getBeginLoc(),
                         "putConcat : No term for argument statement " +
                          llvm::to_hexString((size_t)((terms.count(first)) ? 
                                             second : first), false) );
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
        SCT_INTERNAL_FATAL(stmt->getBeginLoc(),
                         "putFCallParams : No term for parameter/argument " +
                         llvm::to_hexString((size_t)arg, false) );
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
        string s = ((!singleBlockCThreads && isClockThreadReset) ? 
                    waitNVarName.first : waitNVarName.second) + 
                   ((!singleBlockCThreads && isClockThreadReset) ? 
                    NB_ASSIGN_SYM : ASSIGN_SYM) + 
                   getTermAsRValue(waitn).first;
        
        addString(stmt, s);
        clearSimpleTerm(stmt);
        
        if (DebugOptions::isEnabled(DebugComponent::doVerWriter)) {
            cout << "putWaitAssign for stmt " << hex << stmt << dec << ", " << s << endl;
        }

    } else {
        SCT_INTERNAL_FATAL(stmt->getBeginLoc(),
                         "putWaitAssign : No term for parameter of wait(int N) " +
                         llvm::to_hexString((size_t)waitn, false) );
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
        SCT_INTERNAL_FATAL(stmt->getBeginLoc(),
                          "putClockEdge : No clock parameter of sct_assert() " +
                          llvm::to_hexString((size_t)clock, false) );
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
        SCT_INTERNAL_FATAL(stmt->getBeginLoc(),
                         "putAssert : No term for parameter of sct_assert() " +
                         llvm::to_hexString((size_t)arg, false) );
    }
}

// Put temporal assertion in clock thread 
void ScVerilogWriter::putTemporalAssert(const clang::Stmt* stmt, 
                                        const clang::Expr* lhs,
                                        const clang::Expr* rhs,
                                        const std::string& timeStr,
                                        const clang::Expr* event
                                        )
{
    if (skipTerm) return;

    if (terms.count(lhs) && terms.count(rhs)) {
        string s = (event ? "@("+getTermAsRValue(event).first+") " : "") +
                   getTermAsRValue(lhs).first + " " + timeStr + " " + 
                   getTermAsRValue(rhs).first;
        
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

// Remove statement from terms
void ScVerilogWriter::clearStmt(const Stmt* stmt) {
    terms.erase(stmt);
}

// Get string for @stmt, which may be sub-expression
// \return expression string to read
llvm::Optional<string> ScVerilogWriter::getStmtString(const Stmt* stmt) 
{
    return (terms.count(stmt)) ? llvm::Optional<string>(terms.at(stmt).str.first) : 
                                 llvm::Optional<string>();
}

// Get string for IF statement
string ScVerilogWriter::getIfString(const Expr* cexpr) 
{
    return ("if (" +  getStmtString(cexpr).getValue() + ")");
}

// Get string for SWITCH statement
string ScVerilogWriter::getSwitchString(const Expr* cexpr) 
{
    return ("case (" + getStmtString(cexpr).getValue() + ")");
}

// Get string for FOR statement
string ScVerilogWriter::getForString(const Stmt* init,const Expr* cexpr, 
                                     const Expr* incr) 
{
    return ("for (" + getStmtString(init).getValueOr("") + "; " +
                      getStmtString(cexpr).getValueOr("") + "; " + 
                      getStmtString(incr).getValueOr("") + ")");
}

// Get string for WHILE statement
string ScVerilogWriter::getWhileString(const Expr* cexpr) 
{
    return ("while (" + getStmtString(cexpr).getValue() + ")");
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
    
    //cout << "---------- printLocalDeclaration" << endl;
    for (const auto& pair : localDeclVerilog) {
        const SValue& val = pair.first;
        // Skip constant defined in thread reset section declaration in process
        // @varTraits used for CTHREAD variables only
        if (REMOVE_BODY_UNUSED() && !isCombProcess) {
            auto i = varTraits.find(val);
            if (i != varTraits.end()) {
                if (i->second.isReadOnlyCDR()) continue;
                if (!i->second.isAccessAfterReset()) continue;
            }
        }
        
        // Remove constant/variable which is replaced by value
        if (notReplacedVars.count(val) == 0) {
            continue;
        }
        
        printSplitString(os, pair.second, emptySensMethod ? "" : TAB_SYM);
        //cout << "   " << val << " : " << pair.second << endl;
    }

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
                string s = i.second.nextName.getValue() + suffix +
                           ASSIGN_SYM + 
                           i.second.currName.getValue() + suffix;
                sortAssign.push_back(s);
            }
        } else 
        if (i.second.isCombSig()) {
            // COMBSIG w/o CLEAR flag
            if (i.second.currName && i.second.nextName) {
                string s = i.second.currName.getValue() + suffix +
                           ASSIGN_SYM + 
                           i.second.nextName.getValue() + suffix;
                sortAssign.push_back(s);
            }
        } else 
        if (i.second.isCombSigClear()) {
            // COMBSIG with CLEAR flag is assigned instead of @currName
            SValue val = i.first;
            bool unkwIndex;
            //cout << "i.first " << i.first << endl;
            bool isArr = state->isArray(val, unkwIndex);
            isArr = isArr || state->getBottomArrayForAny(val, unkwIndex, 
                                                         ScState::MIF_CROSS_NUM);
            //cout << "i.first " << i.first << " isArr " << isArr << endl;
            
            // Use "default" for array only
            if (i.second.nextName) {
                string s = i.second.currName.getValue() + suffix + 
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
    // Normal local variable declarations
    for (const auto& pair : localDeclVerilog) {
        if (REMOVE_RESET_UNUSED() && !isCombProcess) {
            auto i = varTraits.find(pair.first);
            if (i != varTraits.end()) {
                if (i->second.isCombSig() || i->second.isCombSigClear() || 
                    i->second.isRegister() || i->second.isReadOnlyCDR()) continue;
                if (!i->second.isAccessInReset()) continue;
            }
        }
            
        if (!notReplacedVars.count(pair.first)) {
            continue;
        }
        
        // Local constant, static constant, function parameter or
        // temporary variables not stored in @varTraits, 
        // it needs to provide them for reset section as well
        printSplitString(os, pair.second, TAB_SYM);    
        //cout << "   " << pair.first << " : " << pair.second << endl;
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
