#******************************************************************************
# Copyright (c) 2020, Intel Corporation. All rights reserved.
# 
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
# 
# *****************************************************************************

#! sctool_test : register target in sc_tool regression
function(svc_target exe_target)

    # Flags:
    # REPLACE_CONST_VALUE  -- replace constant with its number value if possible
    # NO_SVA_GENERATE      -- disable SVA generating for SCT assertions
    # PORT_MAP_GENERATE    -- generate port map file and top module wrapper
    # UNSIGNED             -- design uses unsigned arithmetic only
    # NO_REMOVE_EXTRA_CODE -- disable removing unused variable and extra code
    # INIT_LOCAL_VARS      -- initialize local variables at declaration with zero
    # INIT_RESET_LOCAL_VARS-- initialize CTHREAD reset section local variables 
    #                         at declaration with zero
    # WILL_FAIL  -- test will fail on non-synthesizable code
    set(boolOptions REPLACE_CONST_VALUE 
                    NO_SVA_GENERATE
                    PORT_MAP_GENERATE
                    UNSIGNED
                    NO_REMOVE_EXTRA_CODE
                    INIT_LOCAL_VARS
                    INIT_RESET_LOCAL_VARS
                    WILL_FAIL)

    # Arguments with one value
    # GOLDEN        -- Path to golden Verilog output for diff
    # ELAB_TOP      -- Hierarchical name of design top, for example "top.dut.adder"
    # MODULE_PREFIX -- Module prefix string
    set(oneValueArgs GOLDEN 
                     ELAB_TOP 
                     MODULE_PREFIX)

    # Multiple value arguments
    set(multiValueArgs "")

    # Generate variable-value pairs: PARAM_ELAB_ONLY, PARAM_CONST_PROP, ...
    cmake_parse_arguments(PARAM "${boolOptions}" "${oneValueArgs}" 
                                "${multiValueArgs}" ${ARGN} )

    if (${PARAM_NO_SVA_GENERATE})
        set(NO_SVA_GENERATE -no_sva_generate)
    endif()

    if (${PARAM_PORT_MAP_GENERATE})
        set(PORT_MAP_GENERATE -portmap_generate)
    endif()

    if (${PARAM_UNSIGNED})
        set(UNSIGNED -check_unsigned)
    endif()

    if (${PARAM_NO_REMOVE_EXTRA_CODE})
        set(NO_REMOVE_EXTRA_CODE -no_remove_extra_code)
    endif()

    if (${PARAM_INIT_LOCAL_VARS})
        set(INIT_LOCAL_VARS -init_local_vars)
    endif()

    if (${PARAM_INIT_RESET_LOCAL_VARS})
        set(INIT_RESET_LOCAL_VARS -init_reset_local_vars)
    endif()

    if (${PARAM_REPLACE_CONST_VALUE})
        set(REPLACE_CONST_VALUE -replace_const_value)
    endif()

    if (PARAM_ELAB_TOP)
        set(ELAB_TOP -top ${PARAM_ELAB_TOP})
    endif()

    if (PARAM_MODULE_PREFIX)
        set(MODULE_PREFIX -module_prefix ${PARAM_MODULE_PREFIX})
    endif()
    

    # Include directories and options for SC are described in SVCTargets.cmake
    target_link_libraries(${exe_target} PRIVATE SVC::systemc)

    # Simulation target (exe_target), used for tool tests only 
    # Add ScTool include to provide access to sct_memory and sct_common
    target_include_directories(${exe_target} PUBLIC 
            $ENV{ICSC_HOME}/include
            $ENV{ICSC_HOME}/include/sctcommon
            $ENV{ICSC_HOME}/include/sctmemory
            $ENV{ICSC_HOME}/include/sctmemory/utils
    )

    # __SC_TOOL__ not required for SC simulation target to have sct_assert 
    # defined as assert that gives line in source code (not in inlined sct_assert)
    #target_compile_definitions(${exe_target} PUBLIC __SC_TOOL__)

    # Unity file name
    set(SCTOOL_INPUT_CPP ${CMAKE_CURRENT_BINARY_DIR}/${exe_target}.sctool.cpp)

    # Get properties form exe_target specified in target CMakeList.txt
    get_target_property(targetSourceFiles ${exe_target} SOURCES)
    get_target_property(targetIncludeDir ${exe_target} INCLUDE_DIRECTORIES)
    get_target_property(targetDefinitions ${exe_target} COMPILE_DEFINITIONS)
    get_target_property(targetLibraries ${exe_target} LINK_LIBRARIES)

    if(SYSTEMC_INCLUDE_DIR)
        # Not used
        set(INCLUDE_DIRS -I${CMAKE_SOURCE_DIR} -I${SYSTEMC_INCLUDE_DIR} 
                         -I${CMAKE_CURRENT_SOURCE_DIR})
    else()
        # Specified in sc_tool/CMakeList.txt
        get_target_property(svcIncDirs SVC::SCTool INTERFACE_INCLUDE_DIRECTORIES)
        if(svcIncDirs)
            foreach(loop_var ${svcIncDirs})
                set(INCLUDE_DIRS ${INCLUDE_DIRS} -I${loop_var})
            endforeach()
        endif()
    endif()

    # User defined includes
    if(targetIncludeDir)
        foreach(loop_var ${targetIncludeDir})
            set(INCLUDE_DIRS ${INCLUDE_DIRS} -I${loop_var})
        endforeach()
    endif()

    # Internal CLang headers
    if(CLANG_INT_INC_DIR)
        set(INCLUDE_DIRS ${INCLUDE_DIRS} -I${CLANG_INT_INC_DIR})
    endif()

    # Copy definitions with "-D" for synthesis target
    if(targetDefinitions)
        foreach(loop_var ${targetDefinitions})
            set(COMP_DEFINITIONS ${COMP_DEFINITIONS} -D${loop_var} )
        endforeach()
    endif()

    # Generate absolute paths for user sources
    foreach(srcItem ${targetSourceFiles})
        get_filename_component(targetSourceAbs ${srcItem} ABSOLUTE)
        list(APPEND targetSourcesListAbs ${targetSourceAbs})
    endforeach()

    # Verilog file name
    set(VERILOG_DIR ${CMAKE_CURRENT_BINARY_DIR}/sv_out)
    file(MAKE_DIRECTORY ${VERILOG_DIR})
    set(VERILOG_OUT ${VERILOG_DIR}/${exe_target}.sv)

    # Use SystemC pre-compiled headers
    if(DEFINED SYSTEMC_PCH)
        set(PCH_OPT -include-pch ${SYSTEMC_PCH})
    endif()

    # Merge all options
    set(TOOL_OPTS
            ${SCTOOL_INPUT_CPP}
            -sv_out ${VERILOG_OUT}
            ${ELAB_TOP}
            ${MODULE_PREFIX}
            ${REPLACE_CONST_VALUE}
            ${NO_SVA_GENERATE}
            ${PORT_MAP_GENERATE}
            ${UNSIGNED}
            ${NO_REMOVE_EXTRA_CODE}
            ${INIT_LOCAL_VARS}
            ${INIT_RESET_LOCAL_VARS}
            --
            -D__SC_TOOL__ -D__SC_TOOL_ANALYZE__ -DNDEBUG
            -Wno-logical-op-parentheses
            -std=c++17 ${INCLUDE_DIRS} ${COMP_DEFINITIONS}
            -I${CMAKE_SOURCE_DIR}/sc_elab2/lib
            ${PCH_OPT})

    # Create string from option list with adding quotes, required for file name with spaces
    foreach (TOPT ${TOOL_OPTS})
            set (TOOL_OPTS_STR "${TOOL_OPTS_STR} \"${TOPT}\"")
    endforeach()

    # string (REPLACE ";" " " TOOL_OPTS_STR "${TOOL_OPTS}")
    # Generate Unity input source file for Clang-based elaborator
    file(WRITE ${SCTOOL_INPUT_CPP}
          "#include <sc_tool/SCTool.h>  \n\n"
          "const char* __sctool_args_str = R\"\(${TOOL_OPTS_STR}\)\"; \n\n"
    )

    # Add all sources        
    foreach(srcItem ${targetSourcesListAbs})
        file(APPEND ${SCTOOL_INPUT_CPP}
                "#include \"${srcItem}\"\n")
    endforeach()

    # Synthesis target
    set(exe_target_sctool ${exe_target}_sctool )
    add_executable(${exe_target_sctool} ${SCTOOL_INPUT_CPP})

    # targetLibraries optional testbench libraries given in target CMakeList.txt
    # and SystemC added above
    target_link_libraries(${exe_target_sctool} ${targetLibraries} SVC::SCTool)
    # Add ScTool include to provide access to sct_memory and sct_common
    target_include_directories(${exe_target_sctool} PUBLIC 
            $ENV{ICSC_HOME}/include
            $ENV{ICSC_HOME}/include/sctcommon
            $ENV{ICSC_HOME}/include/sctmemory
            $ENV{ICSC_HOME}/include/sctmemory/utils
    )
                                
    # Copy user includes
    if(targetIncludeDir)
        target_include_directories(${exe_target_sctool} PRIVATE ${targetIncludeDir})
    endif()

    # Copy user definitions, add __SC_TOOL__ for synthesis target
    if(targetDefinitions)
        target_compile_definitions(${exe_target_sctool} PRIVATE __SC_TOOL__ 
                                   PRIVATE ${targetDefinitions})
    else()
        target_compile_definitions(${exe_target_sctool} PRIVATE __SC_TOOL__)
    endif()

    # Create _BUILD target for ctest, build exe_target_sctool
    add_test(NAME ${exe_target}_BUILD
             COMMAND "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR} 
             --target ${exe_target_sctool})

    # Create _SYN target for ctest, runs elaboration and synthesis
    add_test(NAME ${exe_target}_SYN COMMAND ${exe_target_sctool})
    # WILL_FAIL -- support tests which contains non-synthesizable code, parameter in svc_target
    # DEPENDS   -- waiting for BUILD is done
    set_tests_properties(${exe_target}_SYN PROPERTIES WILL_FAIL ${PARAM_WILL_FAIL} 
                         DEPENDS ${exe_target}_BUILD)

    if (PARAM_GOLDEN)
        add_test(NAME ${exe_target}_DIFF COMMAND bash -c 
                 "diff -U 3 -dHrN <(sed '/The code is generated by Intel Compiler for SystemC/d;' ${CMAKE_CURRENT_SOURCE_DIR}/${PARAM_GOLDEN}) <(sed '/The code is generated by Intel Compiler for SystemC/d;' ${VERILOG_OUT}) > ${exe_target}.diff"
                )
        set_tests_properties(${exe_target}_DIFF PROPERTIES DEPENDS ${exe_target}_SYN)
    endif()

    # Add SCT_PROPERTY file 
    target_sources(${exe_target} PRIVATE 
                   $ENV{ICSC_HOME}/include/sctcommon/sct_property.cpp)

endfunction()
