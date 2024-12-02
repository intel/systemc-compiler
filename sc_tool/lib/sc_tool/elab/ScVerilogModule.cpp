/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Author: Roman Popov
 */

#include <sc_tool/elab/ScVerilogModule.h>
#include <sc_tool/utils/ScTypeTraits.h>
#include <sc_tool/utils/CppTypeTraits.h>
#include <sc_tool/utils/StringFormat.h>
#include <sc_tool/diag/ScToolDiagnostic.h>
#include <sc_tool/utils/VerilogKeywords.h>
#include <sc_tool/ScCommandLine.h>
#include "ScVerilogModule.h"
#include "sc_tool/scope/ScVerilogWriter.h"

namespace sc_elab
{
using std::cout; using std::endl;


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

static unsigned long getVerVarBitNum(const VerilogVar &var);

// Remove assignments for unused variables (@assignments) and 
// update required variables (@requiredVars)
void VerilogModule::filterAssignments()
{
    using namespace std;
    // <var, <assignment where var in LHS, assignment where var in RHS>>
    // If there is no or more than one assignment, @nullptr is placed 
    using AsnPair = std::pair<const Assignment*, const Assignment*>;
    std::unordered_map<const VerilogVar*, AsnPair> varCounts;
            
    // Count variable entry numbers in left and right parts        
    for (auto& asn : assignments) {
        // Skip variables with indices and required variables
        if (asn.getLeftIdx().empty() && 
            requiredVars.count(asn.getLeftVar()) == 0) 
        {
            auto res = varCounts.emplace(asn.getLeftVar(), 
                                         AsnPair(&asn, nullptr));
            if (!res.second) {
                // Pair of assignments
                AsnPair& i = res.first->second;
                // Both @nullptr means it has multiple @lvar/@rvar entries
                i.second = (i.first) ? nullptr : i.second;
                i.first = (i.second) ? &asn : nullptr;
            }
        }
        if (asn.getRightIdx().empty() && 
            requiredVars.count(asn.getRightVar()) == 0) 
        {
            auto res = varCounts.emplace(asn.getRightVar(), 
                                         AsnPair(nullptr, &asn));
            if (!res.second) {
                // Pair of assignments
                AsnPair& i = res.first->second;
                // Both @nullptr means it has multiple @lvar/@rvar entries
                i.first = (i.second) ? nullptr : i.first;
                i.second = (i.first) ? &asn : nullptr;
            }
        }
    }
//    cout << "---------- varCounts" << endl;
//    for (auto& i : varCounts) {
//        cout << i.first->getName() << " ptrs " << i.second.first 
//             << " " << i.second.second << endl;
//    }
//    
//    cout << "---------- assignments 0: " << endl;
//    for (auto& i : assignments) {
//        cout << &i << " " <<  i.getLeftVar()->getName() << " = " << i.getRightVar()->getName() << endl;
//    }
    
    // Replace two transitive assignments with one 
    // @a = @b & @b = @c replace with  @a = @c, if @b is not required 
    std::vector<Assignment> filtered;
    for (const auto& asn : assignments) {
        // Check LHS variable
        auto i = varCounts.find(asn.getLeftVar());
        if (i != varCounts.end()) {
            AsnPair& asnPtr = i->second;
            if (asnPtr.first && asnPtr.second) {
                // Do not put this assignment into @filtered
                continue;
            }
        }
        
        // Check RHS variable
        i = varCounts.find(asn.getRightVar());
        if (i != varCounts.end()) {
            AsnPair& asnPtr = i->second;
            if (asnPtr.first && asnPtr.second) {
                // Create joined assignment and put into @filtered
                filtered.push_back(Assignment(asn.getLeft(), 
                                   VerilogVarRef(asnPtr.first->getRightVar(), 
                                                 asnPtr.first->getRightIdx())));
            } else {
                filtered.push_back(asn);
            }
            continue;
        }
        
        // No variable in @varCounts if variable indices not empty
        filtered.push_back(asn);
    }
    assignments.swap(filtered); 

//    cout << "---------- assignments 1: " << endl;
//    for (auto& i : assignments) {
//        cout << &i << " " << i.getLeftVar()->getName() << " = " << i.getRightVar()->getName() << endl;
//    }
    
    // Remove assignments for not required variable
    filtered.clear();
    std::unordered_set<const Assignment*> required;
    bool changed;
    
    do {
        changed = false;
        // Add assignment with required LHS variable
        for (const auto& asn : assignments) {
            if (requiredVars.count(asn.getLeftVar()) != 0) {
                required.insert(&asn);
            }
        }
        // Add RHS variable of required assignment
        for (auto* asn : required) {
            auto i = requiredVars.insert(asn->getRightVar());
            changed = changed || i.second;
        } 
    } while (changed);
    
    for (const auto& asn : assignments) {
        if (required.count(&asn)) {
            filtered.push_back(asn);
        }
    }
    assignments.swap(filtered);

//    cout << "---------- assignments 2: " << endl;
//    for (auto& i : assignments) {
//        cout << i.getLeftVar()->getName() << " = " << i.getRightVar()->getName() << endl;
//    }
}


void VerilogModule::removeUnusedVariables()
{
    using namespace std;
    using namespace sc;
    SCT_TOOL_ASSERT (requiredVars.empty(), "requiredVars is not empty");

    //cout << "----- svaUseVars " << endl;
    for (const auto& entry : svaUseVars) {
        //cout << "   " << entry.first->getName() << " " << unsigned(entry.second) << endl;
        requiredVars.insert(entry.first);
    }
    
    //cout << "----- procUseVars " << endl;
    for (const auto& proc : procUseVars) {
        //cout << "Proc #" << proc.first.getID() << endl;
        for (const auto& entry : proc.second) {
            //cout << "   " << entry.first->getName() << " " << unsigned(entry.second) << endl;
            requiredVars.insert(entry.first);
        }
    }
    
    //cout << "----- procDefVars " << endl;
    for (const auto& proc : procDefVars) {
        //cout << "Proc #" << proc.first.getID() << endl;
        for (const auto& entry : proc.second) {
            //cout << "   " << entry.first->getName() << " " << unsigned(entry.second) << endl;
            requiredVars.insert(entry.first);
        }
    }
    
    for (auto* var : procBindVars) {
        requiredVars.insert(var);
    }
    for (const auto& port : verilogPorts) {
        requiredVars.insert(port.getVariable());
    }
    
//    cout << "----- Required variables : " << endl;
//    for (auto* var : requiredVars) {
//        cout << var->name << endl;
//    }
    
    // Remove assignments for unused variables (@assignments) and 
    // required variables (@requiredVars)
    filterAssignments();
}

// Detect multiple used/defined variable/channel in different processes
void VerilogModule::detectUseDefErrors()
{
    using namespace sc;
    
//    std::cout << "----- Used variables: " << std::endl;
//    for (const auto& entry : procUseVars) {
//        std::cout << "Process #" << entry.first.getID() << std::endl;
//        for (const auto& procVars : entry.second) {
//            std::cout << procVars.first->getName() << std::endl;
//        } 
//    }
//    std::cout << "----- Defined variables: " << std::endl;
//    for (const auto& entry : procDefVars) {
//        std::cout << "Process #" << entry.first.getID() << std::endl;
//        for (const auto& procVars : entry.second) {
//            std::cout << procVars.first->getName() << std::endl;
//        }
//    }
    
    std::unordered_map<const VerilogVar*, ProcessView> defVars;
    std::unordered_map<const VerilogVar*, ProcessView> useVars;
    
    // Checking defined variables
    for (const auto& entry : procDefVars) {
        const ProcessView& procView = entry.first;
        bool isCombProc = procView.isCombinational();
        auto procDecl = procView.getLocation().second; 

        for (const auto& procVars : entry.second) {
            const VerilogVar* verVar = procVars.first;
            const auto& defRes = defVars.emplace(verVar, procView);
            
            // Variable/channel already defined in other process 
            if (!defRes.second) {
                const ProcessView& othProcView = defRes.first->second;
                auto othProcDecl = othProcView.getLocation().second;
                // Check process functions are different (not same in MIF array elements)
                if (procDecl != othProcDecl) {
                    ScDiag::ScDiagID diagId;
                    if (procVars.second == VarKind::vkChannel) {
                        // Check both process defined the variable are combinational
                        isCombProc = isCombProc && othProcView.isCombinational();
                        diagId = isCombProc ? ScDiag::SYNTH_MULT_COMB_DRIVE_SIG :
                                              ScDiag::SYNTH_MULT_SEQ_DRIVE_SIG;
                    } else {
                        diagId = ScDiag::SYNTH_MULT_PROC_ACCESS_VAR;
                    }
                    SCT_TOOL_ASSERT (procDecl, "Process has null declaration");
                    ScDiag::reportScDiag(procDecl->getBeginLoc(), diagId) <<
                                         verVar->getName();
                }
            }
        }
    }
        
    // Checking used variables
    for (const auto& entry : procUseVars) {
        const ProcessView& procView = entry.first;
        auto procDecl = procView.getLocation().second; 
        
        for (const auto& procVars : entry.second) {
            const VerilogVar* verVar = procVars.first;
            // Skip constants and non-variable
            if (verVar->isConstant() || 
                procVars.second != VarKind::vkVariable) continue;
            
            const auto& useRes = useVars.emplace(verVar, procView);

            // Variable already used in other process 
            if (!useRes.second) {
                const ProcessView& othProcView = useRes.first->second;
                auto othProcDecl = othProcView.getLocation().second;
                // Check process functions are different (not same in MIF array elements)
                if (procDecl != othProcDecl) {
                    SCT_TOOL_ASSERT (procDecl, "Process has null declaration");
                    ScDiag::reportScDiag(procDecl->getBeginLoc(),
                                         ScDiag::SYNTH_MULT_PROC_ACCESS_VAR) <<
                                         verVar->getName();
                }
            }
        }
    }
    
    // Checking use of assigned channel in this method process
    for (const auto& entry : procDefVars) {
        const ProcessView& procView = entry.first;
        if (!procView.isCombinational()) continue;
        
        for (const auto& procVars : entry.second) {
            const VerilogVar* verVar = procVars.first;
            // Skip non-channels
            if (procVars.second != VarKind::vkChannel) continue;

            if (procUseVars[procView].count(verVar) != 0) {
                auto* procDecl = procView.getLocation().second;
                SCT_TOOL_ASSERT (procDecl, "Process has null declaration");
                
                // Print warning for non-array channel only, else print remark
                if (verVar->getArrayDims().empty()) {
                    ScDiag::reportScDiag(procDecl->getBeginLoc(), 
                                         ScDiag::SYNTH_USEDEF_IN_SAME_PROC) <<
                                         verVar->getName();
                } else {
                    ScDiag::reportScDiag(procDecl->getBeginLoc(), 
                                         ScDiag::SYNTH_USEDEF_ARR_IN_SAME_PROC) <<
                                         verVar->getName();
                }
            }
        }
    }
    
    // Checking method sensitivity list is complete
    for (const auto& entry : procUseVars) {
        const auto& procView = entry.first;
        if (!procView.isScMethod()) continue;
        
        unsigned nonSensFound = 0;
        std::string nonSensChannels;
        for (const auto& procVar : entry.second) {
            const VerilogVar* verVar = procVar.first;
            // Skip non-channels
            if (procVar.second != VarKind::vkChannel) continue;

            bool found = false;
            for (const auto& event : procView.staticSensitivity()) {
                if (!event.isDefault()) continue;

                auto eventVars = event.sourceObj.getVerilogVars();
                for (const auto& eventVar : eventVars) {
                    if (verVar == eventVar.var) {
                        found = true; break;
                    }
                }
                if (found) break;
            }
            
            if (!found) {
                if (nonSensFound) nonSensChannels += ", ";
                nonSensChannels += verVar->getName();
                if (++nonSensFound > 5) break;
            }
        }    
        if (nonSensFound) {
            auto* procDecl = procView.getLocation().second;
            ScDiag::reportScDiag(procDecl->getBeginLoc(), 
                                 ScDiag::SYNTH_NON_SENSTIV_2USED) << 
                                 nonSensChannels;
        }                
    }
    
    // Checking @SC_THREAD sensitivity to @sc_signal (not @sct_signal)
    for (const auto& entry : procUseVars) {
        const ProcessView& procView = entry.first;
        if (!procView.isScThread()) continue;
        
        for (const auto& event : procView.staticSensitivity()) {
            auto eventVars = event.sourceObj.getVerilogVars();
            for (const auto& eventVar : eventVars) {
                auto i = verVar2Value.find(eventVar.var);
                if (i != verVar2Value.end()) {
                    const SValue& val = i->second;
                    if (val.asString().find("sc_signal") != std::string::npos) {
                        if (auto procdecl = procView.getLocation().second) {
                            ScDiag::reportScDiag(procdecl->getLocation(),
                                                 ScDiag::SYNTH_SS_SIG_SENS_THREAD);
                        } else {
                            ScDiag::reportScDiag(ScDiag::SYNTH_SS_SIG_SENS_THREAD);
                        }
                    }
                }
            }
        }
    }
}

void VerilogModule::serializeToStream(llvm::raw_ostream &os) const
{
    using namespace sc;
    
    std::string modLoc = "";
    if (elabModObj.getFieldDecl()) {
        auto& sm = elabModObj.getFieldDecl()->getASTContext().getSourceManager();
        modLoc = elabModObj.getFieldDecl()->getBeginLoc().printToString(sm);
        modLoc = sc::getFileName(modLoc);
    }
    
    os << "\n//==============================================================================\n";
    os << "//\n";
    os << "// Module: " << commentName << " (" << modLoc << ")\n";
    os << "//\n";
    os << "module " << name << " // \"" << comment << "\"\n";
    os << "(";

    bool firstPort = true;
    for (auto &vPort : verilogPorts) {
        if (!firstPort) {
            os << ",\n";
        }
        else {
            os << "\n";
            firstPort = false;
        }

        switch (vPort.getDirection()) {
            case PortDirection::IN:os << "    input ";
                break;
            case PortDirection::OUT:os << "    output ";
                break;
            case PortDirection::INOUT:os << "    inout ";
                break;
            case PortDirection::NONE:break;
        }

        serializeVerVar(os, *vPort.getVariable());
    }

    os << "\n);\n\n";

    if (!verilogSignals.empty()) {
        os << "// Variables generated for SystemC signals\n";
        //bool firstVar = true;
        //bool lastVar = false;
        for (auto *var : verilogSignals) {
            // Skip not required variables
            if (requiredVars.count(var) == 0) {
                continue;
            }
            //if (firstVar) {
            //    os << "// Variables generated for SystemC signals\n";
            //    firstVar = false;
            //}
            serializeVerVar(os, *var);
            os << ";\n";
            //lastVar = true;
        }
        //if (lastVar) os << "\n"; 
        os << "\n";
    }

    if (!dataVars.empty()) {
        bool first = true;
        for (const VerilogVar& var : dataVars) {
            // Skip not required variables
            if (requiredVars.count(&var) == 0) {
                continue;
            }
            // Print constants with filtering duplicates
            if (var.isConstant()) {
                if (first) {
                    os << "// Local parameters generated for C++ constants\n";
                    first = false;
                }
                serializeVerVar(os, var);
                os << ";\n";
            }
        }
        if (!first) os << "\n";
    }

    if (!assignments.empty()) {
        // Filtering assignment to remove intermediate signals
        os << "// Assignments generated for C++ channel arrays\n";
        for (auto& assign : assignments) {
            os << "assign " << assign.getLeftVar()->getName();
            for (auto dim : assign.getLeftIdx())
                os << "[" << dim << "]";

            os << " = " << assign.getRightVar()->getName();

            for (auto dim : assign.getRightIdx())
                os << "[" << dim << "]";

            os << ";\n";
        }
        os << "\n";
    }

    // Print processes
    for (auto procObj : processes) {
        serializeProcess(os, procObj);
    }

    if (!instances.empty()) {
        os << "\n";
        os << "//------------------------------------------------------------------------------\n";
        os << "// Child module instances\n\n";
    }

    // Normal module instances
    bool hasModInst = false;
    for (auto& instance : instances) {

        auto *verMod = instance.getModObj().getVerilogModule();
        if (!hasModInst) {
            hasModInst = true;
        }

        os << verMod->name;
        os << " " << instance.getName() << "\n(\n";

        bool firstBinding = true;
        for (auto &binding : instance.getBindings()) {

            if (!firstBinding) {
                os << ",\n";
            } else {
                firstBinding = false;
            }

            os << "  ." << binding.first << "(" << binding.second << ")";
        }

        os << "\n);\n\n";
    }

    // SVA property instances
    if (!svaPropCode.empty()) {
        if (hasModInst) {
            os << "\n";
            os << "//------------------------------------------------------------------------------\n";
            os << "// SVA generated for SystemC temporal assertions\n\n";
        }
        os << "`ifndef INTEL_SVA_OFF\n";
        os << svaPropCode;
        os << "`endif // INTEL_SVA_OFF\n\n";
    }
    
    os << "endmodule\n\n\n";
    
}

// Get array indices in multi-dimensional for given @indx
// \param allSizes -- record array and field array joined sizes
IndexVec getArrayIndices(const IndexVec& arrSizes, std::size_t indx) 
{
    IndexVec arrInds(arrSizes);

    // Fill @arrInds with element indices
    for (auto i = arrInds.rbegin(); i != arrInds.rend(); ++i) {
        unsigned a = indx % (*i);
        indx = indx / (*i);
        *i = a;
    }
    return arrInds;
}

// Get total element number in one/multi-dimensional array, 
// for one-dimensional array the same as its size
unsigned getArrayElementNumber(const IndexVec& arrSizes)
{
    unsigned elmnum = (arrSizes.size() > 0) ? 1 : 0;

    for (auto s : arrSizes) {
        elmnum = elmnum*s;
    }

    return elmnum;
}


void VerilogModule::createTopWrapper(llvm::raw_ostream &os) const
{
    using namespace sc;
    using namespace std;
    
    string modLoc = "";
    if (elabModObj.getFieldDecl()) {
        auto& sm = elabModObj.getFieldDecl()->getASTContext().getSourceManager();
        modLoc = elabModObj.getFieldDecl()->getBeginLoc().printToString(sm);
        modLoc = sc::getFileName(modLoc);
    }
    
    os << "\n//==============================================================================\n";
    os << "//\n";
    os << "// Module wrapper with array port flatten \n";
    os << "// for : " << commentName << " (" << modLoc << ")\n";
    os << "//\n";
    
    os << "module " << name << "_wrapper\n";
    os << "(";

    UniqueNamesGenerator portNameGen;
    portNameGen.addTakenName(name);
    for (auto &port : verilogPorts) {
        const VerilogVar& var = *port.getVariable();
        portNameGen.addTakenName(var.getName());
    }
    
    // Generated array ports for array port variable
    unordered_map<VerilogPort, vector<VerilogVar>> arrPorts;

    bool firstPort = true;
    for (auto &port : verilogPorts) {
        string dirStr = port.getDirection() == PortDirection::IN ?
                        "    input " : 
                        port.getDirection() == PortDirection::OUT ?
                        "    output " :
                        port.getDirection() == PortDirection::INOUT ?
                        "    inout " : "";
        const VerilogVar& var = *port.getVariable();
        
        if (var.isArray()) {
            auto entry = arrPorts.emplace(port, vector<VerilogVar>());
            SCT_TOOL_ASSERT(entry.second, 
                            "Non-unique port variable at top module interface");
            
            auto dims = var.getArrayDims();
            unsigned elemNum = getArrayElementNumber(dims);
            
            for (unsigned i = 0; i < elemNum; i++) {
                string arrName = var.getName();
                auto indices = getArrayIndices(dims, i);
                for (auto indx : indices) {
                    arrName += "_" + to_string(indx);
                }
                //cout << "arrName " << arrName << endl;
                
                const auto& arrPort = entry.first->second.emplace_back(
                            VerilogVar(portNameGen.getUniqueName(arrName),
                            var.getBitwidth(), IndexVec(), var.isSigned()));
                
                if (!firstPort) os << ",\n"; else os << "\n"; 
                os << dirStr;
                serializeVerVar(os, arrPort);
            }
            
        } else {
            if (!firstPort) os << ",\n"; else os << "\n"; 
            os << dirStr;
            serializeVerVar(os, var);
        }
        firstPort = false;
    }
    os << "\n);\n\n";
    
    // Declare local array variables
    for (auto& entry : arrPorts) {
        const VerilogVar& var = *entry.first.getVariable();
        serializeVerVar(os, var);
        os << ";\n";
    }
    os << "\n";

    // Generate intermediate assignments
    for (auto& entry : arrPorts) {
        const VerilogVar& var = *entry.first.getVariable();
        bool inPort = entry.first.getDirection() == PortDirection::IN;
        auto dims = var.getArrayDims();
        unsigned elemNum = getArrayElementNumber(dims);
        
        for (unsigned i = 0; i < elemNum; i++) {
            string arrName = var.getName();
            auto indices = getArrayIndices(dims, i);
            for (auto indx : indices) {
                arrName += "[" + to_string(indx) + "]";
            }
            
            os << "assign ";
            if (inPort) {
                os << arrName << " = " << entry.second.at(i).getName(); 
            } else {
                os << entry.second.at(i).getName() << " = " << arrName; 
            }
            os << ";\n";
        }
    }
    os << "\n";
    
    // Instantiate this module
    string instName = portNameGen.getUniqueName(name + "_inst");
    os << name << " " << instName << "\n(\n";

    firstPort = true;
    for (auto& port : verilogPorts) {
        const VerilogVar& var = *port.getVariable();
        if (!firstPort) os << ",\n";
        os << "  .";
        os << var.getName();
        os << "(";
        os << var.getName();
        os << ")";
        firstPort = false;
    }

    os << "\n);\n\n";
    
    os << "endmodule\n\n";
}

void VerilogModule::createPortMap(llvm::raw_ostream &os) const
{
    using namespace sc;
    
    os << "# Module: " << commentName << " (" << name << ")\n\n";

    for (auto& port : verilogPorts) {
        
        auto var = port.getVariable();

        if (var->isConstant()) continue;
        if (var->getBitwidth() == 1) continue;

        // Empty arrays are not generated
        bool isEmpty = false;
        for (auto dim : var->getArrayDims()) {
            if (dim == 0) {
                isEmpty = true;
            }
        }
        if (isEmpty) continue;

        os << var->getName() << " " << var->getBitwidth() << " bit_vector ";
        
        if (var->getBitwidth() <= 64) {
            if (var->isSigned()) {
                os << "sc_int";
            } else {
                os << "sc_uint";
            }
        } else {
            if (var->isSigned()) {
                os << "sc_bigint";
            } else {
                os << "sc_biguint";
            }
        }
        os << "\n";
    }
    os << "\n";
}

VerilogModStatistic VerilogModule::getStatistic() const {
    VerilogModStatistic res;

    for (const auto& proc : procBodies) {
        res.stmtNum += proc.second.statStmtNum;
        res.termNum += proc.second.statTermNum;
        res.asrtNum += proc.second.statAsrtNum;
        res.waitNum += proc.second.statWaitNum;
    }

    for (auto* var : verilogSignals) {
        // Skip not required variables
        if (requiredVars.count(var) == 0) continue;
        res.regBits += getVerVarBitNum(*var);
        //cout << "   " << var->getName() << " " << res.regBits << endl;
    }

    for (const auto& entry : procBodies) {
        auto i = procVarMap.find(entry.first);
        if (i != procVarMap.end()) {
            for (auto* var : i->second) {
                if (requiredVars.count(var) == 0) continue;
                res.varBits += getVerVarBitNum(*var);
                //cout << "   " << var->getName() << " " << res.varBits << endl;
            }
        }
    }

    return res;
}

// Translate SystemC assertion string into SVA string
/*std::optional<std::string> VerilogModule::transSvaString(
                                        const std::string& origStr) const
{
    using namespace sc;
    using namespace std;
    //cout << "origStr " << origStr << endl;

    // Parse string
    size_t ssize = origStr.size();
    size_t tstart = origStr.find_first_of('#');
    if (tstart == 0) {
        std::string errMsg = "No antecedent (left) expression in \"" + 
                             origStr + "\"";
        SCT_INTERNAL_ERROR_NOLOC (errMsg);
    }
    std::string lhs = origStr.substr(0, tstart);
    
    size_t tend = origStr.find_last_of('#')+1;
    if (tend == ssize) {
        std::string errMsg = "No consequent (RHS) expression in \"" + 
                             origStr + "\"";
        SCT_INTERNAL_ERROR_NOLOC (errMsg);
    }
    std::string rhs = origStr.substr(tend, ssize-tend);
    std::string timeTail = origStr.substr(tstart, tend-tstart);
    
    // Get time string
    auto timeStr = parseSvaTime(timeTail);
    
    if (!timeStr) {
        ScDiag::reportScDiag(ScDiag::SYNTH_SVA_INCORRECT_TIME) << timeTail;
        return std::nullopt;
    }
    
    // Check variable names not changed during name conflict
    auto lhsStr = parseSvaArg(lhs);
    auto rhsStr = parseSvaArg(rhs);
    
    if (lhsStr && rhsStr) {
        std::string svaStr = *lhsStr + " " + *timeStr + " " + *rhsStr;
        //cout << "svaStr " << svaStr << endl;
        return svaStr;
    }
    return std::nullopt;
}*/

// Check SVA argument does not have changed names, and do some modifications:
// remove spaces and extra symbols like "this->" and "read()"
std::optional<std::string> VerilogModule::parseSvaArg(
                                            const std::string& origStr) const 
{
    using namespace sc;

    // Remove all spaces
    std::string str = origStr;
    str.erase(remove_if(str.begin(), str.end(), isspace), str.end());
    
    std::string name;
    for (auto i = str.begin(); i != str.end(); ++i) {
        char c = *i;
        
        if (name.empty()) {
            if (c == '_' || isalpha(c)) {
                name = c;
            }
        } else {
            if (c == '_' || isalpha(c) || isdigit(c)) {
                name += c;
            } else {
                if (nameGen.isChanged(name)) {
                    std::string errMsg = "Changed variable name in \"" + 
                                         origStr + "\"";
                    SCT_INTERNAL_ERROR_NOLOC (errMsg);
                    return std::nullopt;
                }
                name.clear();
            }
        }
    }
    if (nameGen.isChanged(name)) {
        std::string errMsg = "Changed variable name in \"" + origStr + "\"";
        SCT_INTERNAL_ERROR_NOLOC (errMsg);
        return std::nullopt;
    }
    
    // Removing extra symbols
    removeAllSubstr(str, ".read()");
    removeAllSubstr(str, "->read()");

    removeAllSubstr(str, "this->");
    removeAllSubstr(str, "(*this).");

    // Checking array access at non-literal index
    if (checkArrayAccessStr(str)) {
        std::string errMsg = "Array access in \"" + origStr + "\"";
        SCT_INTERNAL_ERROR_NOLOC (errMsg);
        return std::nullopt;
    }
    
    // Check for @bit() and @range()
    std::string errMsg = "Incorrect bit()/range() in \"" + origStr + "\"";
    if (!replaceBitRangeStr(str, ".bit(")) {
        SCT_INTERNAL_ERROR_NOLOC (errMsg);
    }
    if (!replaceBitRangeStr(str, "->bit(")) {
        SCT_INTERNAL_ERROR_NOLOC (errMsg);
    }
    if (!replaceBitRangeStr(str, ".range(")) {
        SCT_INTERNAL_ERROR_NOLOC (errMsg);
    }
    if (!replaceBitRangeStr(str, "->range(")) {
        SCT_INTERNAL_ERROR_NOLOC (errMsg);
    }
    
    // Check for incorrect symbols
    if (str.find("::") != std::string::npos ||
        str.find("->") != std::string::npos ||
        str.find("&") != std::string::npos ||
        str.find(".") != std::string::npos) {
        std::string errMsg = "Unsupported symbol in \"" + origStr + "\"";
        SCT_INTERNAL_ERROR_NOLOC (errMsg);
        return std::nullopt;
    }
    
    // Check de-reference in first symbol
    size_t i = 0;
    while (str[i] == '(') i++;
    if (str[i] == '*') {
        std::string errMsg = "De-reference in \"" + origStr + "\"";
        SCT_INTERNAL_ERROR_NOLOC (errMsg);
        return std::nullopt;
    }
    
    return str;
}


void VerilogModule::serializeProcess(llvm::raw_ostream &os,
                                     ProcessView procObj) const
{
    bool isCthread = procObj.isScThread() || procObj.isScCThread();

    if (isCthread) {
        serializeProcSplit(os, procObj);
    } else {
        serializeProcSingle(os, procObj);
    }
}

void VerilogModule::addModuleInstance(ModuleMIFView modObj, 
                                      const std::string& name)
{
    auto instanceName = nameGen.getUniqueName(name);
    auto *newInstance = &instances.emplace_back(instanceName, modObj);
    instanceMap[modObj] = newInstance;
}

// \param isMIFArrElmnt -- used to do not report error for sensitivity list
//                         as it could be false for MIF array element
VerilogVar *VerilogModule::createChannelVariable(sc_elab::ObjectView systemcObject,
                                                 const std::string& suggestedName,
                                                 size_t bitwidth,
                                                 sc_elab::IndexVec arrayDims,
                                                 bool isSigned,
                                                 bool isMIFArrElmnt,       
                                                 sc_elab::APSIntVec initVals,
                                                 const std::string& comment)
{
//    cout << "    createChannelVariable cppObject " << systemcObject.getID() 
//         << ", suggestedName " << std::string(suggestedName) 
//         << ", isMIFArrElmnt = " << isMIFArrElmnt << endl;

    VerilogVar* var = &channelVars.emplace_back(VerilogVar(
                                nameGen.getUniqueName(suggestedName),
                                bitwidth,
                                std::move(arrayDims),
                                isSigned,
                                std::move(initVals),
                                comment));
    
    channelVarMap[systemcObject].push_back(var);
    
    return var;
}

//void VerilogModule::addChannelVariable(ObjectView scObj, VerilogVar var)
//{
//    channelVars.push_back(std::move(var));
//    channelVarMap[scObj].push_back(&channelVars.back());
//}

VerilogVar *VerilogModule::createDataVariable(ObjectView cppObject,
                                              const std::string& suggestedName,
                                              size_t bitwidth,
                                              IndexVec arrayDims,
                                              bool isSigned,
                                              APSIntVec initVals,
                                              const std::string& comment)
{
//    cout << "    createDataVariable suggestedName " 
//         << std::string(suggestedName) << " ";    
//    for(auto i : initVals) cout << i.toString(10) << " ";
//    cout << " arrayDims.size " << arrayDims.size() << endl;

    VerilogVar *var = &dataVars.emplace_back(VerilogVar(
                            nameGen.getUniqueName(suggestedName), 
                            bitwidth, 
                            std::move(arrayDims),
                            isSigned, 
                            std::move(initVals), 
                            comment));

    dataVarMap[cppObject] = var;
    return var;
}

// The same as previous, but for member variable of MIF array element,
// provides the same name for all array instances
VerilogVar* VerilogModule::createDataVariableMIFArray(ObjectView cppObject,
                                              ObjectView parentObject,
                                              const std::string& suggestedName,
                                              size_t bitwidth,
                                              IndexVec arrayDims,
                                              bool isSigned,
                                              APSIntVec initVals,
                                              const std::string& comment)
{
    using namespace sc;
    
//    cout << "    createDataVariableMIFArray parentObject " 
//         << parentObject.getID() << ", cppObject " 
//         << cppObject.getID() << ", suggestedName " 
//         << std::string(suggestedName) << ", InitVals = ";
//    for(auto i : initVals) cout << i.toString(10) << " ";
//    cout << " arrayDims.size " << arrayDims.size() << endl;
    
    // Get variable for dynamic object to have field declaration
    ObjectView varObj = cppObject;
    if (auto v = varObj.getVerilogNameOwner()) {
        varObj = *v;
    }
    if (auto v = varObj.getTopmostParentArrayOrPointer()) {
        varObj = *v;
    }
    SCT_TOOL_ASSERT (varObj.getValueDecl(),
                     "No ValueDecl for object in createDataVariableMIFArray");

    // Create name key with parent module/MIF and all parent records
    ObjectView objView = varObj;
    RecordMemberNameKey nameKey(parentObject, varObj.getValueDecl()); 
    //std::cout << "    parent to key " << parentObject.getDebugString() << std::endl;
        
    while (objView.isDataMember() || objView.isArrayElement()) {
        objView = objView.getParent();
        if (objView.isModule() || objView.isModularInterface()) break;
    
        nameKey.parDecls.push_back(objView.getValueDecl());
        //std::cout << "    add to key " << objView.getDebugString() << " " << objView.getID() << std::endl;
    }
    
    VerilogVar* var;
    auto i = memberMIFArrayVars.find(nameKey);
    if (i == memberMIFArrayVars.end()) {
        // Create new variable
        std::string uniqueName = nameGen.getUniqueName(suggestedName);
        var = &dataVars.emplace_back( VerilogVar(
                                uniqueName, bitwidth, std::move(arrayDims),
                                isSigned, std::move(initVals), comment) );
        memberMIFArrayVars[nameKey] = var;
        
        //cout << "  create var, name " << uniqueName << endl;
    } else {
        // Add initialization value to existing variable
        var = i->second;
        var->addInitVals( std::move(initVals) );
        //cout << "  add init vals to existing name  " << var->getName() << endl;
    }
        
    dataVarMap[cppObject] = var;
    return var;
}

//void VerilogModule::addDataVariable(ObjectView objView, VerilogVar var)
//{
//    dataVars.push_back(std::move(var));
//    dataVarMap[objView] = &dataVars.back();
//}

// Create process local variable or member variable used in the process
VerilogVar *VerilogModule::createProcessLocalVariable(
                                            ProcessView procView, 
                                            const std::string& suggestedName,
                                            size_t bitwidth, IndexVec arrayDims, 
                                            bool isSigned, APSIntVec initVals, 
                                            const std::string& comment)
{
    // Avoid name conflict with local names
    auto* procVar = &procVars.emplace_back(
        VerilogVar(nameGen.getUniqueName(suggestedName, true), bitwidth,
            std::move(arrayDims), isSigned, std::move(initVals), comment) );

//    std::cout << "createProcessLocalVariable procVar " << procVar->getName() 
//              << " arrayDims " << procVar->getArrayDims().size() << " arraDims[0] "
//              << (procVar->getArrayDims().size() > 0 ? 
//                  std::to_string(procVar->getArrayDims()[0]) : "-") << std::endl;

    return procVar;
}

// Create process local variable or member variable used in the process for 
// members of non zero elements of MIF array
// Do not register variable in @procVarMap to avoid its declaration, and 
// do not change given name (it is unique because of zero element)
VerilogVar *VerilogModule::createProcessLocalVariableMIFNonZero(
                                            ProcessView procView, 
                                            const std::string& suggestedName,
                                            size_t bitwidth, IndexVec arrayDims, 
                                            bool isSigned, APSIntVec initVals, 
                                            const std::string& comment)
{
    auto* procVar = &procVars.emplace_back(
        VerilogVar(suggestedName, bitwidth, std::move(arrayDims), 
                   isSigned, std::move(initVals), comment) );

//    std::cout << "createProcessLocalVariableMIFNonZero procVar " << procVar->getName() 
//              << " arrayDims " << procVar->getArrayDims().size() << " arraDims[0] "
//              << (procVar->getArrayDims().size() > 0 ? 
//                  std::to_string(procVar->getArrayDims()[0]) : "-") << std::endl;

    return procVar;
}

VerilogVar *VerilogModule::createAuxilarySignal(
                                            const std::string& suggestedName,
                                            size_t bitwidth, IndexVec arrayDims, 
                                            bool isSigned, APSIntVec initVals,
                                            const std::string& comment)
{
    auto *newVar = &channelVars.emplace_back(VerilogVar(
                            nameGen.getUniqueName(suggestedName),
                            bitwidth, arrayDims, isSigned, initVals, comment));

    //cout << this->getName() << " createAuxilarySignal " << suggestedName.data() << " " << newVar->getName() << endl;

    verilogSignals.emplace_back(newVar);
    return newVar;
}

VerilogVar* VerilogModule::createAuxilaryPort(
                                            PortDirection dir,
                                            const std::string& suggestedName, 
                                            size_t bitwidth, IndexVec arrayDims,
                                            bool isSigned, APSIntVec initVals, 
                                            const std::string& comment)
{
    auto *newVar = &channelVars.emplace_back(
        VerilogVar(nameGen.getUniqueName(suggestedName), bitwidth, arrayDims,
            isSigned, initVals, comment));

    //cout << "--  " << this->getName() << " createAuxilaryPort " << suggestedName.data() << " " << newVar->getName() << endl;

    verilogPorts.emplace_back(dir, newVar);
    return newVar;
}

// Create auxiliary Verilog port for port binding purposes and remove 
// signal variable from this Verilog module
// Used to keep signal name when signal connected to port in another module
VerilogVar* VerilogModule::createAuxilaryPortForSignal(PortDirection dir,
                                            VerilogVar* verVar, 
                                            APSIntVec initVals, 
                                            const std::string& comment)
{
    auto *newVar = &channelVars.emplace_back(
        VerilogVar(verVar->getName(), verVar->getBitwidth(), 
                   verVar->getArrayDims(), verVar->isSigned(), 
                   initVals, comment));

    //cout << "--  " << this->getName() << " createAuxilaryPortForSignal " << newVar->getName() << endl;

    verilogPorts.emplace_back(dir, newVar);
    
    // Remove signal variable create before
    auto i = std::find(verilogSignals.begin(), verilogSignals.end(), verVar);
    if (i != verilogSignals.end()) {
        verilogSignals.erase(i);
    }
    return newVar;
}


void VerilogModule::addProcRegisterNextValPair(ProcessView procView,
                                               VerilogVar *regVar,
                                               VerilogVar *nextVar, 
                                               const std::string& suffix)
{
    //std::cout << "addProcRegisterNextValPair regVar " << regVar->getName()  
    //          << " nextVar " << nextVar->getName() << std::endl;
    
    procRegNextPairs[procView].push_back({{regVar, nextVar}, suffix});
}

// Check uniqueness of given variable to prevent multiple 
// @var = @next_var assignments for a member of MIF array elements
bool VerilogModule::checkProcUniqueVar(ProcessView procView, VerilogVar *regVar)
{
    auto res = procRegVars[procView].insert(regVar);
    return res.second;
}

void VerilogModule::convertToProcessLocalVar(const VerilogVar *var,
                                             ProcessView procView)
{
    //std::cout << "convertToProcessLocalVar var " << var->getName() << std::endl;
    procVarMap[procView].insert(var);
}

void VerilogModule::addSignal(VerilogVar *var)
{
    //cout << this->getName() << " addSignal " << var->getName() << endl;
    verilogSignals.push_back(var);
}

VerilogVarsVec VerilogModule::getVerVariables(ObjectView scObj) const
{
    if (scObj.isSignal()) {
        if (channelVarMap.count(scObj))
            return channelVarMap.at(scObj);
        else
            return {};
    }

    if (auto primitive = scObj.primitive()) {
        if (primitive->isPort() && primitive->port()->isSignalPort()) {
            if (channelVarMap.count(scObj))
                return channelVarMap.at(scObj);
            else
                return {};
        }
    }

    VerilogVarsVec res;
    auto vi = dataVarMap.find(scObj);

    if (vi != dataVarMap.end()) {
        res.push_back(vi->second);
    }

    return res;
}

VerilogModuleInstance *VerilogModule::getInstance(ModuleMIFView modObj)
{
    return instanceMap.at(modObj);
}

void VerilogModule::convertToPort(const VerilogVar *var, PortDirection dir)
{
    using namespace sc;
    SCT_TOOL_ASSERT(std::find_if(channelVars.begin(), channelVars.end(),
                                [=](const VerilogVar &v) {
                                    return &v == var;
                                }) != channelVars.end(),
                    "Duplicated variable in convertToPort");
    
    //cout << "--  " << this->getName() << " convertToPort " << var->getName() << endl;
    verilogPorts.emplace_back(dir, var);
}

void VerilogModule::convertToSignal(const VerilogVar *var)
{
    //cout << "--  " <<this->getName() << " convertToSignal " << var->getName() << endl;
    verilogSignals.push_back(var);
}

void VerilogModule::addVerilogPort(VerilogVar *var, PortDirection dir)
{
    //cout << "--  " << this->getName() << " addVerilogPort " << var->getName() << endl;
    verilogPorts.emplace_back(dir, var);
}

void VerilogModule::addProcess(ProcessView proc)
{
    processes.push_back(proc);
}

void VerilogModule::addProcessBody(ProcessView proc, VerilogProcCode code)
{
    procBodies[proc] = std::move(code);
}

static void serializeVerilogBool(llvm::raw_ostream &os, llvm::APSInt val)
{
    os << (val.isZero() ? '0' : '1');
}

static void serializeVerilogInt(llvm::raw_ostream &os, llvm::APSInt val)
{
    using namespace llvm;
    using namespace sc;
    
    std::string s = ScVerilogWriter::makeLiteralStr(val, 10, 0, 0, 
                                                    CastSign::NOCAST, false);
    os << s;
}

static void serializeInitVals(llvm::raw_ostream &os,
                              IndexVec dims,
                              const APSIntVec& initVals,
                              size_t& initIdx,
                              bool isBool)
{

    os << "'{ ";

    for (size_t i = 0; i < dims[0]; ++i) {
        bool last = (i == dims[0] - 1);
        if (dims.size() == 1) {
            if (initVals.size() <= initIdx) {
                cout << "Size " << initVals.size() << " index " << initIdx << endl;
                assert (false);
            }
            
            if (isBool) {
                serializeVerilogBool(os, initVals[initIdx]);
            } else {
                serializeVerilogInt(os, initVals[initIdx]);
            }
            initIdx++;
        } else {
            auto newDims = dims;
            newDims.erase(newDims.begin());
            serializeInitVals(os, newDims, initVals, initIdx, isBool);
        }

        if (!last)
            os << ", ";

    }

    os << " }";

}

void VerilogModule::serializeVerVar(llvm::raw_ostream& os, 
                                    const VerilogVar& var) const
{
    using namespace sc;
    
    // empty arrays are not generated
    bool isEmpty = false;
    for (auto dim : var.getArrayDims())
        if (dim == 0)
            isEmpty = true;

    if (isEmpty)
        os << "//";

    bool isConst = var.isConstant();
    bool fullInitVals = isConst && var.checkInitVals();
    
    if (isConst) {
        if (fullInitVals) {
            os << "localparam ";
        } else {
            auto i = verVar2Value.find(&var);
            if (i != verVar2Value.end()) {
                SValue val = i->second; 
                if (val.isVariable()) {
                    ScDiag::reportScDiag(val.getVariable().getDecl()->getBeginLoc(),
                            ScDiag::SYNTH_PART_INIT_VAR) << var.getName();
                } else {
                    ScDiag::reportScDiag(ScDiag::SYNTH_PART_INIT_VAR) << 
                                         var.getName();
                }
            } else {
               ScDiag::reportScDiag(ScDiag::SYNTH_PART_INIT_VAR) << 
                                 var.getName();
            }
        }
    }
    
    os << "logic ";
    if (var.isSigned())
        os << "signed ";

    if (var.getBitwidth() > 1)
        os << "[" << var.getBitwidth() - 1 << ":0] ";

    os << var.getName();

    auto dims = var.getArrayDims();
    for (auto dim : dims)
        os << "[" << dim << "]";

    if (isConst && fullInitVals) {

        bool isBool = !var.isSigned() && var.getBitwidth() == 1;
        
        if (dims.empty()) {
            os << " = ";
            if (isBool) {
                serializeVerilogBool(os, var.getInitVals()[0]);
            } else {
                serializeVerilogInt(os, var.getInitVals()[0]);
            }
        }
        else {
            os << " = ";
            size_t initIdx = 0;
            serializeInitVals(os, dims, var.getInitVals(), initIdx, isBool);
        }

    }

    if (!var.getComment().empty())
        os << " /*" << var.getComment() << "*/";

}

static unsigned long getVerVarBitNum(const VerilogVar &var)
{
    if (var.isConstant()) return 0;
    
    unsigned long res = var.getBitwidth();
    for (auto dim : var.getArrayDims()) res *= dim;
    
    return res;
}


// Use @isSigned as @sc_bigint has uint64Val instead of int64Val
void VerilogModule::fillInitVal(APSIntVec& initVals, 
                                bool isSigned, ValueView valueView)
{
    size_t bitwidth = valueView.bitwidth();
    
    if (valueView.int64Val()) {
        initVals.emplace_back(
            llvm::APSInt(llvm::APInt(bitwidth,*valueView.int64Val()), !isSigned));
    } else 
    if (valueView.uint64Val()) {
        initVals.emplace_back(
            llvm::APSInt(llvm::APInt(bitwidth,*valueView.uint64Val()), !isSigned));
    }
}

void VerilogModule::fillInitVals(APSIntVec& initVals, 
                                 bool isSigned, ArrayView arrayView)
{
   for (size_t i = 0; i < arrayView.size(); ++i) {
        ObjectView elem = arrayView.at(i);
        if (elem.isArrayLike())
            fillInitVals(initVals, isSigned, elem);
        
        else {
            ValueView valueView = *elem.primitive()->value();
            fillInitVal(initVals, isSigned, valueView);
        }
    }
}

void VerilogModule::addConstDataVariable(ObjectView objView,
                                         const std::string &name)
{
    using namespace sc;
    //cout << "addConstDataVariable " << name << endl;
    
    if (auto primitive = objView.primitive()) {
        SCT_TOOL_ASSERT(primitive->isValue(), "No value found");
        ValueView valueView = *primitive->value();
        size_t bitwidth = valueView.bitwidth();
        bool isSigned = isSignedOrArrayOfSigned(objView.getType());

        APSIntVec initVals;
        fillInitVal(initVals, isSigned, valueView);
        createDataVariable(objView,
                           name,
                           bitwidth,
                           {},
                           isSigned,
                           initVals);
    } else {
        SCT_TOOL_ASSERT(objView.isArrayLike(), "No array found");
        ArrayView arrayView = *objView.array();
        SCT_TOOL_ASSERT(arrayView.isConstPrimitiveArray(), 
                        "Non-constant array");

        IndexVec arrayDims;
        ObjectView childObj = arrayView;
        while (childObj.isArrayLike()) {
            ArrayView childArray = *childObj.array();
            arrayDims.push_back(childArray.size());
            childObj = childArray.at(0);
        }
        size_t bitwidth = childObj.primitive()->value()->bitwidth();
        bool isSigned = isSignedOrArrayOfSigned(childObj.getType());

        APSIntVec initVals;
        fillInitVals(initVals, isSigned, arrayView);
        createDataVariable(objView,
                           name,
                           bitwidth,
                           arrayDims,
                           isSigned,
                           initVals);
    }
}

bool VerilogModule::isEquivalentTo(VerilogModule &otherMod) const
{
    using namespace sc;
    
    if (getModObj().getType().getAsString()
        != otherMod.getModObj().getType().getAsString())
        return false;

    if (isIntrinsic() != otherMod.isIntrinsic())
        return false;

    // Different name can be caused by @__SC_TOOL_MODULE_NAME__
    if (isIntrinsic()) {
        if (name != otherMod.name)
            return false;
    }
    
    if (isIntrinsic()) {
        return (*getVerilogIntrinsic() == *otherMod.getVerilogIntrinsic());
    }

    if (dataVars != otherMod.dataVars)
        return false;

    if (channelVars != otherMod.channelVars)
        return false;

    if (instances != otherMod.instances)
        return false;

    if (assignments != otherMod.assignments)
        return false;

    if (processes.size() != otherMod.processes.size())
        return false;

    for (size_t i = 0; i < processes.size(); ++i) {
        auto thisProc = processes[i];
        auto otherProc = otherMod.processes[i];

        if (thisProc.getLocation().second != otherProc.getLocation().second)
            return false;

        if (thisProc.staticSensitivity().size()
            != otherProc.staticSensitivity().size())
            return false;

        for (size_t si = 0; si < thisProc.staticSensitivity().size(); ++si) {

            auto thisSens = thisProc.staticSensitivity()[si];
            auto otherSens = otherProc.staticSensitivity()[si];

            if (thisSens.kind != otherSens.kind)
                return false;

            auto thisSrcObj = thisSens.sourceObj;
            auto otherSrcObj = otherSens.sourceObj;

            auto thisArrayEl = thisSrcObj.getAsArrayElementWithIndicies();
            auto otherArrayEl = otherSrcObj.getAsArrayElementWithIndicies();

            if (thisArrayEl.indices != otherArrayEl.indices)
                return false;

            thisSrcObj = thisArrayEl.obj;
            otherSrcObj = otherArrayEl.obj;

            // Check for ZWI as it is not in @channelVarMap
            if ( isZeroWidthType(thisSrcObj.getType()) ) {
                if ( !isZeroWidthType(otherSrcObj.getType()) ) return false;
                continue;
            }

            if (!channelVarMap.count(thisSrcObj)) {
                llvm::outs() << "SENS " << thisSrcObj << "\n";
                SCT_TOOL_ASSERT(false, "No sensitivity object in object-to-verilog");
            }
            auto &thisSources = channelVarMap.at(thisSrcObj);
            auto &otherSources = otherMod.channelVarMap.at(otherSrcObj);

            if (thisSources.size() != otherSources.size())
                return false;

            for (size_t so = 0; so < thisSources.size(); ++so) {
                if (*thisSources[so] != *otherSources[so])
                    return false;
            }
        }

        if (thisProc.resets().size() != otherProc.resets().size())
            return false;

        for (size_t ri = 0; ri < thisProc.resets().size(); ++ri) {
            auto thisReset = thisProc.resets()[ri];
            auto otherReset = otherProc.resets()[ri];

            if (thisReset.level != otherReset.level)
                return false;

            if (thisReset.isAsync != otherReset.isAsync)
                return false;

            auto thisSrcObj = thisReset.sourceObj;
            auto otherSrcObj = otherReset.sourceObj;

            auto thisArrayEl = thisSrcObj.getAsArrayElementWithIndicies();
            auto otherArrayEl = otherSrcObj.getAsArrayElementWithIndicies();

            if (thisArrayEl.indices != otherArrayEl.indices)
                return false;

            thisSrcObj = thisArrayEl.obj;
            otherSrcObj = otherArrayEl.obj;

            auto &thisSources = channelVarMap.at(thisSrcObj);
            auto &otherSources = otherMod.channelVarMap.at(otherSrcObj);

            if (thisSources.size() != otherSources.size())
                return false;

            for (size_t so = 0; so < thisSources.size(); ++so) {
                if (*thisSources[so] != *otherSources[so])
                    return false;
            }

        }
    }

    return true;
}

std::optional<std::string> VerilogModule::getModularIfName(ProcessView procObj) const
{
    using namespace sc;
    using std::cout; using std::endl;
    // Get Only last inheritor of @sc_interface is modular IF
    const auto& procMod = procObj.getParent();
    bool isModularIF = procMod.isModularInterface();
    //cout << "procObj " << procObj.getDebugString() << endl;
    
    if (isModularIF) {
        if (procMod.isDynamic() || procMod.isArrayElement()) {
            auto parent = procMod.getTopmostParentArrayOrPointer();
            //cout << "parent " << (parent ? parent->getDebugString() : "NO parent") << endl;
            
            SCT_TOOL_ASSERT (parent, "No parent found for dynamic object");
            
            clang::ValueDecl* decl = parent->getValueDecl();
            SCT_TOOL_ASSERT (decl, "No ValueDecl in dynamic object");
            return decl->getNameAsString();
            
        } else {
            clang::ValueDecl* decl = procMod.getValueDecl();
            SCT_TOOL_ASSERT (decl, "No ValueDecl in non-dynamic object");
            return decl->getNameAsString();
        }
    }
    return std::optional<std::string>();
}

// Get process name unique in the module
std::string VerilogModule::getProcName(ProcessView procObj) const
{
    std::string procName = procObj.getLocation().second->getNameAsString();
    // Add modular IF instance name to support multiple/array instance 
    // of the same modular IF class
    if (auto modifName = getModularIfName(procObj)) { 
        procName = *modifName + "_" + procName;
    }
    procName = nameGen.getUniqueName(procName);
    //std::cout << "getProcName " << procName << std::endl;
    return procName;
}

// Generate always block for method process 
void VerilogModule::serializeProcSingle(llvm::raw_ostream &os,
                                        ProcessView procObj) const
{
    auto procCode = procBodies.at(procObj);
    bool generateAlways = !procObj.staticSensitivity().empty();

    // Get process start location
    auto& sm = procObj.getLocation().second->getASTContext().getSourceManager();
    auto procLoc = procObj.getLocation().second->getBeginLoc().printToString(sm);
    procLoc = sc::getFileName(procLoc);

    os << "//------------------------------------------------------------------------------\n";
    os << "// Method process: " << procObj.procName << " (" << procLoc << ") \n";

    if (procCode.emptyProcess) {
        os << "// Empty process, no code generated \n\n";
        return;
    }

    os << "\n";
    
    if (procVarMap.count(procObj)) {
        os << "// Process-local variables\n";
        for (auto *verVar : procVarMap.at(procObj)) {
            serializeVerVar(os, *verVar);
            os << ";\n";
        }
        os << "\n";
    }

    if (generateAlways) {
        if (procObj.isScMethod()) {
            if (procObj.getHasLatch()) {
                os << "always_latch \n";
            } else {
                os << "always_comb \n";
            }
        } else {
            // For non-split cthread and thread
            os << "always @(";
            serializeSensList(os, procObj);
            os << ") \n";
        }
    }
    
    if (generateAlways)
        os << "begin : " << procObj.procName << "     // " << procLoc << "\n";

    if (!procObj.isScMethod()) {
        os << procCode.localVars;

        os << "    ";
        if (!procObj.resets().empty()) {
            serializeResetCondition(os, procObj);
            os << procCode.resetSection;
            os << "    else ";
        }
    }

    os << procBodies.at(procObj).body;

    if (generateAlways)
        os << "end\n";
    os << "\n";

}

// Generate pair of always_comb/always_ff for thread process 
void VerilogModule::serializeProcSplit(llvm::raw_ostream &os,
                                       ProcessView procObj) const
{
    auto procCode = procBodies.at(procObj);

    // Get process start location
    auto& sm = procObj.getLocation().second->getASTContext().getSourceManager();
    auto procLoc = procObj.getLocation().second->getBeginLoc().printToString(sm);
    procLoc = sc::getFileName(procLoc);
    
    os << "//------------------------------------------------------------------------------\n";
    os << "// Clocked THREAD: " << procObj.procName << " (" << procLoc << ") \n";
    
    if (procCode.emptyProcess) {
        os << "// Empty process, no code generated \n\n";
        return;
    }

    os << "\n";

    if (procVarMap.count(procObj)) {
        os << "// Thread-local variables\n";
        for (auto *verVar : procVarMap.at(procObj)) {
            serializeVerVar(os, *verVar);
            os << ";\n";
        }
        os << "\n";
    }

    // Process-local constants defined in reset section
    if (procConstMap.count(procObj)) {
        os << "// Thread-local constants\n";
        for (const VerilogVar* verVar : procConstMap.at(procObj)) {
            serializeVerVar(os, *verVar);
            os << ";\n";
        }
        os << "\n";
    }

    auto combName = procObj.procName + "_comb";
    auto ffName = procObj.procName + "_ff";
    auto funcName = procObj.procName + "_func";

    os << "// Next-state combinational logic\n";
    os << "always_comb begin : " << combName << "     // " << procLoc << "\n";
    os << "    " << funcName << ";\n";
    os << "end\n";
    os << "function void " << funcName << ";\n";
    os << procCode.localVars;
    os << procCode.body;

    os << "endfunction\n\n";

    os << "// Synchronous register update\n";

    os << "always_ff @(";
    serializeSensList(os, procObj);
    os << ") \n";
    os << "begin : " << ffName << "\n";
    os << "    ";
    if (!procObj.resets().empty()) {
        serializeResetCondition(os, procObj);
        os << procCode.resetSection;
        
        if (!procCode.tempRstAsserts.empty()) {
            os << "\n    `ifndef INTEL_SVA_OFF\n";
            os << procCode.tempRstAsserts;
            os << "    `endif // INTEL_SVA_OFF\n";
        }
        
        os << "    end\n";
        os << "    else ";
    }
    os << "begin\n";

    if (procRegNextPairs.count(procObj)) {
        for (const auto &regNextPair : procRegNextPairs.at(procObj)) {
            os << "        " << regNextPair.first.first->getName() 
               << regNextPair.second; // Add MIF element suffix
            os << " <= ";
            os << regNextPair.first.second->getName() 
               << regNextPair.second << ";\n";  // Add MIF element suffix
        }
    }
    
    if (!procCode.tempAsserts.empty()) {
        os << "\n    `ifndef INTEL_SVA_OFF\n";
        os << procCode.tempAsserts;
        os << "    `endif // INTEL_SVA_OFF\n";
    }

    os << "    end\n";
    os << "end\n\n";
}

void VerilogModule::serializeSensList(llvm::raw_ostream &os,
                                      ProcessView procObj) const
{
    bool first = true;
    for (auto sensEvent : procObj.staticSensitivity()) {

        ArrayElemObjWithIndices source{sensEvent.sourceObj};
        source = source.obj.getAsArrayElementWithIndicies();

        auto &verSources = channelVarMap.at(source.obj);
        for (auto *var : verSources) {
            if (first)
                first = false;
            else
                os << " or ";

            if (sensEvent.isNegedge())
                os << "negedge ";
            else 
            if (sensEvent.isPosedge())
                os << "posedge ";

            os << var->getName();

            for (auto idx : source.indices)
                os << "[" << idx << "]";
            // Only one clock is required
            break;  
        }
        // Only one clock is required
        break;
    }

    for (auto reset : procObj.resets()) {

        ArrayElemObjWithIndices source{reset.sourceObj};
        source = source.obj.getAsArrayElementWithIndicies();
        auto &verSources = channelVarMap.at(source.obj);
        
        for (auto *var : verSources) {
            if (reset.isAsync) {
                if (!first) 
                    os << " or ";
                first = false;

                if (reset.level)
                    os << "posedge ";
                else
                    os << "negedge ";

                os << var->getName();
                for (auto idx : source.indices)
                    os << "[" << idx << "]";

            } else {
                os << " /*sync " << var->getName();
                for (auto idx : source.indices)
                    os << "[" << idx << "]";
                os << "*/";
            }
        }
    }
}

// Get process sensitivity string
std::string VerilogModule::getSensEventStr(const ProcessView& procObj) const
{
    std::string res;
    bool first = true;
    
    for (auto sensEvent : procObj.staticSensitivity()) {
        
        ArrayElemObjWithIndices source{sensEvent.sourceObj};
        source = source.obj.getAsArrayElementWithIndicies();

        auto &verSources = channelVarMap.at(source.obj);
        for (auto *var : verSources) {
            if (first)
                first = false;
            else
                res += " or ";

            if (sensEvent.isNegedge())
                res += "negedge ";
            else 
            if (sensEvent.isPosedge())
                res += "posedge ";

            res += var->getName();

            for (auto idx : source.indices)
                res += "[" + std::to_string(idx) + "]";
        }
    }
    return res;
}


void VerilogModule::serializeResetCondition(llvm::raw_ostream &os,
                                            ProcessView procObj) const
{
    os << "if ( ";
    bool first = true;
    for (auto reset : procObj.resets()) {

        ArrayElemObjWithIndices source{reset.sourceObj};
        source = source.obj.getAsArrayElementWithIndicies();
        auto &verSources = channelVarMap.at(source.obj);
        
        for (auto *var : verSources) {
            if (!first)
                os << " || ";
            first = false;

            if (!reset.level)
                os << "~";

            os << var->getName();
            for (auto idx : source.indices)
                os << "[" << idx << "]";
            
            // Previous version:
//            if (auto signal = reset.sourceObj.signal()) {
//                os << signal->getVerilogVars()[0].var->getName();
//
//            } else 
//            if (auto prim = reset.sourceObj.primitive()) {
//                assert(prim->isPort());
//                auto port = prim->port();
//                os << port->getVerilogVars()[0].var->getName();
//            }
        }
    }

    os << " ) ";
}

void VerilogModuleInstance::addBinding(VerilogVar *instancePort,
                                       VerilogVarRef hostVar)
{
    //cout << "addBinding " << instancePort->getName() << " -> " 
    //     << hostVar.var->getName() << endl;
    std::string bindedName = hostVar.var->getName();
    for (const auto idx : hostVar.indicies) {
        bindedName = bindedName + "[" + std::to_string(idx) + "]";
    }

    bindings.emplace_back(instancePort->getName(), bindedName);
}

} // end namespace sc_elab
