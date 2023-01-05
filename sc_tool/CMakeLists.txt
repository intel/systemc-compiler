#******************************************************************************
# Copyright (c) 2020, Intel Corporation. All rights reserved.
# 
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
# 
# *****************************************************************************

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-comment")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated")

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-class-memaccess")
endif()

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -DNDEBUG -O3 -fno-tree-vectorize")

message(STATUS "CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE}")

add_library(SysCRTTI
        lib/rtti_sysc/SystemCRTTI.cpp
        lib/rtti_sysc/SystemCRTTI.h
        )

target_link_libraries(SysCRTTI
        PUBLIC
        SVC::systemc
        PRIVATE
        LLVMCore
        LLVMSupport
        )

target_include_directories(SysCRTTI PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        )

add_library(SCTool
        lib/sc_tool/cfg/SValue.h
        lib/sc_tool/cfg/SValue.cpp
        lib/sc_tool/cfg/ScState.h
        lib/sc_tool/cfg/ScState.cpp
        lib/sc_tool/cfg/ScStmtInfo.h
        lib/sc_tool/cfg/ScStmtInfo.cpp
        lib/sc_tool/cfg/ScTraverseCommon.h
        lib/sc_tool/cfg/ScTraverseCommon.cpp
        lib/sc_tool/cfg/ScTraverseProc.h
        lib/sc_tool/cfg/ScTraverseProc.cpp
        lib/sc_tool/cfg/ScTraverseConst.h
        lib/sc_tool/cfg/ScTraverseConst.cpp

        lib/sc_tool/scope/ScScopeGraph.h
        lib/sc_tool/scope/ScScopeGraph.cpp
        lib/sc_tool/scope/ScVerilogWriter.h
        lib/sc_tool/scope/ScVerilogWriter.cpp
        lib/sc_tool/expr/ScParseExpr.cpp
        lib/sc_tool/expr/ScParseExpr.h
        lib/sc_tool/expr/ScParseExprValue.h
        lib/sc_tool/expr/ScParseExprValue.cpp
        lib/sc_tool/expr/ScGenerateExpr.h
        lib/sc_tool/expr/ScGenerateExpr.cpp
        lib/sc_tool/expr/ScParseRangeExpr.h
        lib/sc_tool/systemc/ScObject.h
        lib/sc_tool/systemc/ScObject.cpp
        lib/sc_tool/systemc/ScChannel.h
        lib/sc_tool/systemc/ScChannel.cpp
        lib/sc_tool/systemc/ScModule.cpp
        lib/sc_tool/systemc/ScModule.h

        lib/sc_tool/ScProcAnalyzer.cpp
        lib/sc_tool/ScProcAnalyzer.h

        lib/sc_tool/diag/ScToolDiagnostic.cpp
        lib/sc_tool/diag/ScToolDiagnostic.h
        lib/sc_tool/ScCommandLine.h
        lib/sc_tool/ScCommandLine.cpp

        lib/sc_tool/elab/ScObjectView.cpp
        lib/sc_tool/elab/ScObjectView.h
        lib/sc_tool/elab/ScElabDatabase.cpp
        lib/sc_tool/elab/ScElabDatabase.h
        lib/sc_tool/elab/ScElabModuleBuilder.cpp
        lib/sc_tool/elab/ScElabModuleBuilder.h
        lib/sc_tool/elab/ScVerilogModule.cpp
        lib/sc_tool/elab/ScVerilogModule.h
        lib/sc_tool/elab/ScElabProcBuilder.cpp
        lib/sc_tool/elab/ScElabProcBuilder.h

        lib/sc_tool/cthread/ScThreadBuilder.cpp
        lib/sc_tool/cthread/ScThreadBuilder.h
        lib/sc_tool/cthread/ScFindWaitCallVisitor.cpp
        lib/sc_tool/cthread/ScFindWaitCallVisitor.h
        lib/sc_tool/cthread/ScCFGAnalysis.cpp
        lib/sc_tool/cthread/ScCFGAnalysis.h
        lib/sc_tool/cthread/ScCfgCursor.h
        lib/sc_tool/cthread/ScCfgCursor.cpp
        lib/sc_tool/cthread/ScThreadBuilder.h
        lib/sc_tool/cthread/ScThreadBuilder.cpp
        lib/sc_tool/cthread/ScCThreadStates.cpp
        lib/sc_tool/cthread/ScCThreadStates.h
        lib/sc_tool/cthread/ScSingleStateThread.cpp
        lib/sc_tool/cthread/ScSingleStateThread.h

        lib/sc_tool/utils/CfgFabric.h
        lib/sc_tool/utils/CfgFabric.cpp
        lib/sc_tool/utils/DebugOptions.h
        lib/sc_tool/utils/DebugOptions.cpp
        lib/sc_tool/utils/BitUtils.cpp
        lib/sc_tool/utils/BitUtils.h
        lib/sc_tool/utils/RawIndentOstream.h
        lib/sc_tool/utils/RawIndentOstream.cpp
        lib/sc_tool/utils/CommandLine.cpp
        lib/sc_tool/utils/CommandLine.h
        lib/sc_tool/utils/InsertionOrderSet.h
        lib/sc_tool/utils/CppTypeTraits.h
        lib/sc_tool/utils/CppTypeTraits.cpp
        lib/sc_tool/utils/StringFormat.h
        lib/sc_tool/utils/StringFormat.cpp
        lib/sc_tool/utils/ScTypeTraits.cpp
        lib/sc_tool/utils/ScTypeTraits.h
        lib/sc_tool/utils/CheckCppInheritance.h
        lib/sc_tool/utils/CheckCppInheritance.cpp
        lib/sc_tool/utils/VerilogKeywords.h
        lib/sc_tool/utils/VerilogKeywords.cpp
        lib/sc_tool/utils/NameGenerator.h
        lib/sc_tool/utils/NameGenerator.cpp

        lib/sc_tool/dyn_elab/Demangle.cpp
        lib/sc_tool/dyn_elab/Demangle.h
        lib/sc_tool/dyn_elab/DesignDbGenerator.cpp
        lib/sc_tool/dyn_elab/DesignDbGenerator.h
        lib/sc_tool/dyn_elab/ElabTypeManager.cpp
        lib/sc_tool/dyn_elab/ElabTypeManager.h
        lib/sc_tool/dyn_elab/GlobalContext.cpp
        lib/sc_tool/dyn_elab/GlobalContext.h
        lib/sc_tool/dyn_elab/MangledTypeDB.cpp
        lib/sc_tool/dyn_elab/MangledTypeDB.h
        lib/sc_tool/dyn_elab/Reflection.cpp
        lib/sc_tool/dyn_elab/Reflection.h
        lib/sc_tool/SCTool.cpp
        lib/sc_tool/SCTool.h
        lib/sc_tool/SCToolFrontendAction.cpp
        lib/sc_tool/SCToolFrontendAction.h

        )

target_link_libraries(SCTool
        PUBLIC
        sc_elab_proto
        SysCRTTI
        PRIVATE
        clangFrontend
        clangSerialization
        clangDriver
        clangTooling
        clangParse
        clangSema
        clangStaticAnalyzerFrontend
        clangStaticAnalyzerCheckers
        clangStaticAnalyzerCore
        clangAnalysis
        clangEdit
        clangAST
        clangASTMatchers
        clangLex
        clangBasic
        LLVMX86AsmParser
        LLVMX86Desc
        LLVMX86Info
        LLVMipo
        LLVMScalarOpts
        LLVMInstCombine
        LLVMTransformUtils
        LLVMAnalysis
        LLVMTarget
        LLVMOption
        LLVMMCParser
        LLVMMC
        LLVMObject
        LLVMBitReader
        LLVMCore
        LLVMSupport
        LLVMFrontendOpenMP
        ${system_libs}
        )

# Include $ICSC_HOME/include, create INTERFACE_INCLUDE_DIRECTORIES
target_include_directories(SCTool PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/lib>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        )

target_compile_definitions(SCTool PRIVATE SC_ELAB2_BUILD)

target_compile_options(SCTool PRIVATE -fno-rtti)

add_dependencies(SCTool systemcPCH)

add_library (SVC::SCTool ALIAS SCTool)

# Copy libraries to $ICSC_HOME/lib
install(TARGETS SCTool SysCRTTI EXPORT SVCTargets DESTINATION lib)
# Copy one required header to $ICSC_HOME/include
# SystemC headers copied by SC make install
install(FILES lib/sc_tool/SCTool.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sc_tool)
