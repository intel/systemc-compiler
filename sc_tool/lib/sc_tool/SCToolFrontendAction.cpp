/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Author: Roman Popov
 */

#include <sc_tool/SCToolFrontendAction.h>

#include <sc_tool/dyn_elab/GlobalContext.h>
#include <sc_tool/dyn_elab/DesignDbGenerator.h>
#include <sc_tool/dyn_elab/MangledTypeDB.h>
#include <sc_tool/elab/ScElabModuleBuilder.h>
#include <sc_tool/elab/ScElabDatabase.h>
#include <sc_tool/diag/ScToolDiagnostic.h>
#include <sc_tool/utils/DebugOptions.h>
#include <sc_tool/utils/StringFormat.h>
#include <sc_tool/ScCommandLine.h>
#include <rtti_sysc/SystemCRTTI.h>
#include <llvm/Support/CommandLine.h>

#include <clang/ASTMatchers/ASTMatchers.h>
#include <sc_tool/utils/DebugOptions.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>

#include <exception>
#include <fstream>

using namespace llvm;
using namespace clang;
using namespace clang::ast_matchers;
using namespace sc_elab;

static cl::opt<std::string> topModuleName(
    "top",
    cl::desc("Specify top-level module hierarchical name"),
    cl::value_desc("top-level module"),
    cl::cat(ScToolCategory));

namespace sc
{

/// Get parent module for pointer to given dynamically allocated object
/// If there are several pointers belong to different modules error is reported 
/// \return parent module or @nullptr if there several such modules
Object* getParentModule(SCDesign& designDB, Object* memberObj)
{
    Object* parentObj = nullptr;
    
    for (auto& id : memberObj->pointer_ids()) {
        // Get next pointer to object
        auto& pointerObj = designDB.objects(id);
        
        // Skip sc_port which is pointer to sc_signal/sc_port bound to
        bool isPointer = pointerObj.kind() == Object::PRIMITIVE &&
                         pointerObj.primitive().kind() == Primitive::POINTER;
        if (!isPointer) continue;
        
        // Get always first parent
        SCT_TOOL_ASSERT (pointerObj.parent_ids_size() > 0, 
                         "No parent object found");
        Object* modParent = designDB.mutable_objects(pointerObj.parent_ids(0));

        // Go up until find first module
        while (modParent->sckind() != Object::SC_MODULE) {
            SCT_TOOL_ASSERT (modParent->parent_ids_size() > 0, 
                             "No parent object found");
            modParent = designDB.mutable_objects(modParent->parent_ids(0));
        }
        
        if (parentObj == nullptr) {
            parentObj = modParent;
            
        } else 
        // TODO: check me
        if (memberObj == modParent) {
            // If there is a module inside of MIF, it could have two pointers
            // and one of them is the same object
            ScDiag::reportScDiag(ScDiag::SYNTH_PARENT_SAME_OBJECT);
            continue;
        //~TODO    
        } else
        if (parentObj != modParent) {
            // Pointers to this object located in different modules
            parentObj->PrintDebugString();
            modParent->PrintDebugString();
            memberObj->PrintDebugString();
            ScDiag::reportScDiag(ScDiag::SYNTH_MULTI_POINTER_DIFF);
            return nullptr;
        }
    }
    return parentObj;
}

/// Get most outer array or @nullptr if @memberObj is not array/array element
Object* getOuterArray(SCDesign& designDB, Object* memberObj)
{
    Object* arrayObj = nullptr;
    Object* arrParent = nullptr;

    // TODO support array of records, if @memberObj is non-module record member
    // or non-module record itself, it need to go up to find array or module
    
    // Get @memObject if it is array or its parent which is possibly array 
    if (memberObj->has_array()) {
        arrParent = memberObj;
    } else {
        SCT_TOOL_ASSERT (memberObj->parent_ids_size() > 0, 
                         "No parent object found");
        arrParent = designDB.mutable_objects(memberObj->parent_ids(0));
    }

    while (arrParent->kind() == Object::ARRAY) {
        arrayObj = arrParent;
        SCT_TOOL_ASSERT (arrParent->parent_ids_size() > 0, 
                         "No parent object found");
        arrParent = designDB.mutable_objects(arrParent->parent_ids(0));
    }
    return arrayObj;
}

const std::string SCElabASTConsumer::TOOL_VERSION = "1.5.11";
const std::string SCElabASTConsumer::TOOL_DATE = "May 19,2023";

void SCElabASTConsumer::HandleTranslationUnit(clang::ASTContext &astCtx)
{
    using namespace std;
    using namespace DebugOptions;

    cout << "--------------------------------------------------------------" << endl;
    cout << " Intel Compiler for SystemC (ICSC) version "  <<
                  TOOL_VERSION << ", " << TOOL_DATE << endl;
    cout << "--------------------------------------------------------------" << endl;
    
    //const char* optNames[] = {doElab};
    //const char* optNames[] = {doConstCfg, doGenCfg};
    //const char* optNames[] = {doGenCfg, doGenLoop, doGenBlock, doModuleBuilder}; 
    //const char* optNames[] = {doModuleBuilder, doConstStmt, doConstCfg/*, doState*/};
    //const char* optNames[] = {doModuleBuilder, doGenStmt};
    //const char* optNames[] = {doGenTerm, doGenCfg, doGenStmt, doModuleBuilder};
    //const char* optNames[] = {doConstCfg, doConstLoop, doConstStmt, doConstBlock, doModuleBuilder};
    //const char* optNames[] = {doGenStmt, doConstStmt, doModuleBuilder};  
    //const char* optNames[] = {doGenStmt, doModuleBuilder};  
    const char* optNames[] = {doModuleBuilder};
    size_t optSize = sizeof(optNames)/sizeof(const char*);
    //DebugOptions::enable(optNames, optSize);
    
    //keepConstVariables = true;
 
    // Find pointer to top-level module by SystemC name
    void *topPtr = getTopModulePtr(topModuleName);

    // Generate a map from mangled type name to clang::QualType
    MangledTypeDB typeDB(astCtx);

    // Since we only support single translation unit we can store
    // AST context and mangled types DB globally
    initGlobalContext(&astCtx, &typeDB);

    // Generates global database of known QualTypes (SystemC built-in types)
    initTypeTraits(astCtx);

    // SCDesign is a Protobuf Database (Protobuf Message) that will store
    // elaborated design
    SCDesign designDB;

    // In Protobuf Database each Type has unique ID
    // ElabTypeManager maintains a map that maps ID to clang::QualType
    ElabTypeManager elabTypeManager(designDB);

    // Get type of top-level module
    clang::QualType topType = typeDB.getType(getDynamicMangledTypeName(topPtr));

    if (auto recordDecl = topType->getAsCXXRecordDecl()) {

        std::cout << "Top module is " << recordDecl->getName().str() << std::endl;

        try { 
            // Run Dynamic elaborator, it will fill Protobuf SCDesign
            DesignDbGenerator(designDB, elabTypeManager).run(recordDecl, topPtr);

            // Moved objects <member id, parent module id>
            auto movedObjs = moveDynamicObjects(designDB);

            // elabDB stores designDB, elabTypeManager, astCtx and generated Verilog modules
            ElabDatabase elabDB(designDB, elabTypeManager, astCtx);
            DEBUG_WITH_TYPE(DebugOptions::doElab, elabDB.dump(););
            std::cout << "Elaboration database created\n" << std::endl;

            // To get Clang AST FieldDecl for Object it needs to create 
            // ObjectView for Object and elabDB and call getFieldDecl():
            // clang::FieldDecl f = ObjectView->getFieldDecl()
        
            // Do all the analysis and generate Verilog modules
            runVerilogGeneration(elabDB, movedObjs);

            std::cout << "----------------------------------------------------------------" << std::endl;
            std::cout << " SystemC-to-Verilog translation, OK " << std::endl;
        
        } catch (std::exception& e) {
            std::cout << "----------------------------------------------------------------" << std::endl;
            std::cout << " SystemC-to-Verilog translation, ERROR " << std::endl;
            
            // Report exception to return error code at exit
            reportErrorException();
        }
        
    } else {
        std::cout << "Fatal error : Top module is not a record" << std::endl;
        std::cout << "----------------------------------------------------------------" << std::endl;
        std::cout << " SystemC-to-Verilog translation, ERROR " << std::endl;
    }
    std::cout << "----------------------------------------------------------------" << std::endl;
    
}

// Move dynamic objects from module where allocated to module where 
// pointer to this object
std::unordered_map<size_t, size_t> 
SCElabASTConsumer::moveDynamicObjects(SCDesign& designDB) 
{
    std::unordered_map<size_t, size_t> movedObjs;
            
    for (auto moduleId : designDB.module_ids()) {
        Object* moduleObj = designDB.mutable_objects(moduleId);
        Record* modRecObj = moduleObj->mutable_record();
        auto* modMembers  = modRecObj->mutable_member_ids();

        for (auto i = modMembers->begin(); i != modMembers->end(); ) {
            //std::cout << "-------- " << *i << std::endl;
            Object* memberObj = designDB.mutable_objects(*i);

            // For dynamically allocated objects
            if (memberObj->rel_type() == Object::DYNAMIC) {
                // Skip processes
                if (memberObj->has_primitive()) {
                    if (memberObj->primitive().kind() == Primitive::PROCESS) {
                        i++; continue;
                    }
                }

                // Parent module for pointer(s) to object
                Object* parentMod = getParentModule(designDB, memberObj);

                // Parent module for pointer(s) to array first element
                Object* arrParentMod = nullptr;
                // Get most outer array to check pointer to its first element
                Object* arrayObj = getOuterArray(designDB, memberObj);
                // Get parent module for array object first element
                if (arrayObj) {
                    SCT_TOOL_ASSERT (arrayObj->has_array(), "No array object");

                    if (arrayObj->array().element_ids_size() > 0) {
                    Object* firstElemObj = 
                                designDB.mutable_objects(
                                    arrayObj->array().element_ids(0));
                    arrParentMod = getParentModule(designDB, firstElemObj);
                    }
                }

                // Get one parent module 
                if (parentMod == nullptr) {
                    if (arrParentMod == nullptr) {
                        // No pointers to this object, do not print error
                        // as it is possible for SVA
                        i++; continue;

                    } else {
                        // Get parent from pointer to first array element 
                        parentMod = arrParentMod;
                    }
                } else 
                if (parentMod != arrParentMod && arrParentMod != nullptr) {
                    // Pointers to this object located in different modules
                    memberObj->PrintDebugString();
                    ScDiag::reportScDiag(ScDiag::SYNTH_MULTI_POINTER_DIFF);
                    i++; continue;
                }

                // If dynamic object and its pointer have different parents
                if (moduleObj->id() != parentMod->id()) {
                    //std::cout << "------ Move object " << std::endl;
                    //memberObj->PrintDebugString();
                    // Register object as moved
                    movedObjs.emplace(memberObj->id(), moduleObj->id());
                    // For signal pointer add its field which used to identify it 
                    if (memberObj->sckind() == Object_SCAggregateKind_SC_SIGNAL) {
                        movedObjs.emplace(memberObj->record().member_ids(0), 
                                          moduleObj->id());
                    }

                    // Change object parent
                    memberObj->mutable_parent_ids()->Clear();
                    memberObj->mutable_parent_ids()->Add(parentMod->id());
                    // Add object to pointer parent module
                    auto* parMembers = parentMod->mutable_record()->
                                       mutable_member_ids();
                    parMembers->Add(memberObj->id());
                    // Remove object from module where is was allocated
                    i = modMembers->erase(i);

                } else {
                    i++;
                }
            } else {
                i++;
            }
        }
    }
    
    return movedObjs;
}

// Create *.sv output file, generate all Verilog modules and intrinsics
void SCElabASTConsumer::runVerilogGeneration(
                            sc_elab::ElabDatabase& elabDB,
                            const std::unordered_map<size_t, size_t>& movedObjs) 
{
    using namespace sc_elab;
    using std::cout; using std::endl;

    std::string toolStr;
    llvm::raw_string_ostream tstr(toolStr);
    
    tstr << "//==============================================================================\n";
    tstr << "//\n";
    tstr << "// The code is generated by Intel Compiler for SystemC, version " << 
           TOOL_VERSION << "\n";
    tstr << "// see more information at https://github.com/intel/systemc-compiler\n";
    tstr << "//\n";
    tstr << "//==============================================================================\n";

    // output file stream
    std::ofstream ofs;

    std::string svFile("out.sv");
    if (!verilogFileName.empty()) {
        svFile = verilogFileName;
    }

    ofs.open(svFile);
    if (!ofs.is_open()) {
        ScDiag::reportErrAndDie("Can't open " + svFile);
    }

    ofs << tstr.str();
    
    // Dump elaboration information
    //elabDB.dump(); 
    
    // Generate all Verilog Modules
    buildVerilogModules(&elabDB, movedObjs);

    // Set of Verilog intrinsic modules that are already generated
    // to avoid duplication
    std::unordered_set<std::string> generatedIntrinsics;
    
    // Serialize all generated modules
    bool topModule = true;
    for (auto& verMod : elabDB.getVerilogModules()) {

        if (verMod.isIntrinsic()) {
            if (!generatedIntrinsics.count(verMod.getName())) {
                std::string verCode = *verMod.getVerilogIntrinsic();
                if (!verCode.empty()) {
                    std::string modLoc = "";
                    if (verMod.elabModObj.getFieldDecl()) {
                        auto& sm = verMod.elabModObj.getFieldDecl()->
                                   getASTContext().getSourceManager();
                        modLoc = verMod.elabModObj.getFieldDecl()->
                                 getBeginLoc().printToString(sm);
                        modLoc = sc::getFileName(modLoc);
                    }
                    
                    ofs << "\n//==============================================================================\n";
                    ofs << "//\n";
                    ofs << "// Verilog intrinsic for module: " << verMod.getName() 
                        << " (" << modLoc << ")\n";
                    ofs << "//";
                    ofs << verCode << "\n";
                }
                generatedIntrinsics.insert(verMod.getName());
            }
        } else {
            std::string modStr;
            llvm::raw_string_ostream ostr(modStr);
            verMod.serializeToStream(ostr);
            ofs << ostr.str();
            
            // Generate top module wrapper file
            if (portMapGenerate) {
                if (topModule) {
                    std::string svWrapFile = removeFileExt(svFile) + "_wrapper.sv";

                    std::ofstream wfs;
                    wfs.open(svWrapFile);
                    if (!wfs.is_open()) {
                        ScDiag::reportErrAndDie("Can't open " + svWrapFile);
                    }

                    wfs << tstr.str();
                    std::string wrapStr;
                    llvm::raw_string_ostream wstr(wrapStr);
                    verMod.createTopWrapper(wstr);
                    wfs << wstr.str();

                    wfs.close();
                    topModule = false;
                }
            }
        }
    }

    ofs.close();
    
    // Generate port map file for vendor simulation tool
    if (portMapGenerate) {
        svFile = removeFileExt(svFile) + ".port_map";
        ofs.open(svFile);
        if (!ofs.is_open()) {
            ScDiag::reportErrAndDie("Can't open " + svFile);
        }

        ofs << "###############################################################################\n";    
        ofs << "#\n";
        ofs << "# The file is generated by Intel Compiler for SystemC, version " << 
               TOOL_VERSION << "\n";
        ofs << "# see more information at https://github.com/intel/systemc-compiler\n";
        ofs << "#\n";
        ofs << "###############################################################################\n\n";    

        // First module is top 
        auto i = elabDB.getVerilogModules().begin();
        if (i != elabDB.getVerilogModules().end()) {
            std::string modStr;
            llvm::raw_string_ostream ostr(modStr);
            i->createPortMap(ostr);
            ofs << ostr.str();
        }

        ofs.close();
    }
}

// ----------------------------------------------------------------------------

std::unique_ptr<clang::ASTConsumer>
SCElabFrontendAction::CreateASTConsumer(
    clang::CompilerInstance &compiler, llvm::StringRef )
{
    // Initialize Diagnostic Engine before doing any work
    initDiagnosticEngine(&compiler.getDiagnostics());

    return std::make_unique<SCElabASTConsumer>();
}

std::unique_ptr<clang::tooling::FrontendActionFactory>
getNewSCElabActionFactory()
{

    class SCElabActionFactory : public clang::tooling::FrontendActionFactory
    {
    public:
        SCElabActionFactory()
        {}

        std::unique_ptr<FrontendAction> create() override
        {
            return std::unique_ptr<FrontendAction>(new SCElabFrontendAction());
        }
    };

    return std::unique_ptr<clang::tooling::FrontendActionFactory>(
        new SCElabActionFactory() );
}


} // namespace sc_elab
