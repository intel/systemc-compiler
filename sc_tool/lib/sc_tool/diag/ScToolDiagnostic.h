/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Author: Roman Popov
 * Modified by: Mikhail Moiseev
 */

#ifndef SCTOOL_SCTOOLDIAGNOSTIC_H
#define SCTOOL_SCTOOLDIAGNOSTIC_H

#include <clang/Basic/Diagnostic.h>
#include <clang/Basic/DiagnosticIDs.h>

#include <unordered_set>
#include <map>
#include <utility>

namespace std {
    
template<> 
struct hash<std::pair<unsigned, unsigned>>  
{
    std::size_t operator () (const std::pair<unsigned, unsigned>& obj) const;
};
}


namespace sc {

class ScDiagBuilder;

/// Internal fatal error exception used to catch it in ScToolFrontendAction
struct InternalErrorException : public std::exception {
    
    std::string msg;
    
    InternalErrorException() = default;
    explicit InternalErrorException(std::string msg_) : 
        msg("Fatal error : " + msg_)
    {}
    
    const char* what() const 
    _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_USE_NOEXCEPT override {
        return msg.c_str();
    }
};


/// Initialized before starting source code processing
void initDiagnosticEngine(clang::DiagnosticsEngine *diagEngine);

/// Permanent Diagnostic IDs for issues in input source code.
/// Use reportScDiag() to report a diagnostic message with permanent IDs.
/// Please do not use for internal tool assertions, instead use assert()
/// or reportErrAndDie(). For quick and dirty temporary diagnostics use
/// reportCustom()
class ScDiag {
public:
    friend class ScDiagBuilder;

    ///  Naming convention: SC_LEVEL_*    Example: SC_WARNING_UNITIALIZED_VAR
    ///  To add new diagnostic:
    ///     - Add entry to this enum
    ///     - Add FormatString to initialize()
    enum ScDiagID {

        SC_CXX_INIT_IN_PROC_FUNC    = 100,
        SC_LIST_INIT_IN_PROC_FUNC   = 101,
        SC_CXX_CTOR_IN_PROC_FUNC    = 102,
        SC_NEW_IN_PROC_FUNC         = 103,
        SC_DELETE_IN_PROC_FUNC      = 104,
        CPP_INCORRECT_REFER         = 105,
        CPP_LOOP_COMPLEX_INIT       = 106,
        CPP_LOOP_COMPLEX_INCR       = 107,
        SYNTH_UNSUPPORTED_OPER      = 108,
        CPP_ARRAY_OUT_OF_BOUND      = 109,
        SYNTH_ARRAY_INIT_LIST       = 110,
        SYNTH_UNSUPPORTED_INIT      = 111,
        CPP_GOTO_STMT               = 112,
        CPP_UNKNOWN_STMT            = 113,
        SYNTH_TYPE_NOT_SUPPORTED    = 114,
        SC_RANGE_DIFF_VARS          = 115,
        CPP_FOR_WITHOUT_INIT        = 116,
        CPP_DIFF_POINTER_COMP       = 117,
        SC_UNSUPPORTED_PORT         = 118,
        SYNTH_POINTER_NO_INIT       = 119,
        CPP_NO_VIRT_FUNC            = 120,
        CPP_PURE_FUNC_CALL          = 121,
        CPP_REFER_NO_INIT           = 122,
        SYNTH_POINTER_NONZERO_INIT  = 123,
        CPP_ASSERT_FAILED           = 124,
        SC_WAIT_IN_METHOD           = 125,
        SC_ERROR_NORETURN           = 126,
        SYNTH_NO_CHANNEL            = 127,
        SYNTH_UNSUPP_CHANNEL_METH   = 128,
        SYNTH_NOSENSITIV_METH       = 129,
        SYNTH_POINTER_OPER          = 130,
        CPP_INCORRECT_ASSERT        = 131,
        SC_FATAL_THREAD_NO_STATE    = 132,
        SYNTH_UNSUPPORTED_RETURN    = 133,
        SYNTH_MULTI_POINTER_DIFF    = 134,
        SC_PORT_NOT_BOUND           = 135,
        SYNTH_NO_ARRAY              = 136,
        SYNTH_DUPLICATE_ASSIGN      = 137,
        SYNTH_CHILD_FIELD_ACCESS    = 138,
        CPP_OPAQUE_EXPR             = 139,
        SC_WAIT_N_VARIABLE          = 140,
        SC_WAIT_N_NONPOSITIVE       = 141,
        SC_UNSUPP_WAIT_KIND         = 142,
        SC_RANGE_WRONG_INDEX        = 143,
        SC_RANGE_WRONG_WIDTH        = 144,
        SYNTH_NON_SENSTIV_2USED     = 145,
        SYNTH_SOME_PATH_DEFINED     = 146,
        SC_WAIT_N_EMPTY             = 147,
        SYNTH_SIGN_UNSIGN_MIX       = 148,
        SYNTH_SC_PORT_INCORRECT     = 149,
        SYNTH_COMB_SIG_READNOTDEF   = 150,
        SYNTH_LOCAL_MODULE_DECL     = 151,
        SYNTH_INCORRECT_RECORD      = 152,
        SYNTH_NONTRIVIAL_COPY       = 153,
        SYNTH_NONTRIVIAL_ASSIGN     = 154,
        CPP_READ_NOTDEF_VAR         = 155,
        SYNTH_CONCAT_CAST_BOOL      = 156,
        SYNTH_TEMP_EXPR_ARG         = 157,
        SYNTH_RECORD_CTOR_IN_PARAM  = 158,
        SYNTH_MULT_PROC_DRIVE_SIG   = 159,
        SYNTH_MULT_PROC_ACCESS_VAR  = 160,
        SYNTH_USEDEF_IN_SAME_PROC   = 161,
        CPP_NULL_PTR_DEREF          = 162,
        CPP_DANGLING_PTR_DEREF      = 163,
        CPP_DANGLING_PTR_CAST       = 164,
        SYNTH_UNKNOWN_TYPE_WIDTH    = 165,
        SYNTH_ARRAY_TO_POINTER      = 166,
        SYNTH_SVA_INCORRECT_TIME    = 167,
        SYNTH_SVA_IN_MAIN_LOOP      = 168,
        SYNTH_SVA_INCORRECT_LOOP    = 169,
        SC_NO_MODULE_NAME           = 170,
        SC_PORT_BOUND_SIGNAL_SAME   = 171,
        SC_RANGE_WRONG_BASE         = 172,
        SYNTH_SWITCH_LAST_EMPTY_CASE= 173,
        CPP_READ_NOTDEF_VAR_RESET   = 174,
        SYNTH_FUNC_IN_ASSERT        = 175,
        SYNTH_ZERO_TYPE_WIDTH       = 176,
        SYNTH_SWITCH_ALL_EMPTY_CASE = 177,
        SYNTH_NONCOST_PTR_CONST     = 178,
        SYNTH_CONST_VAR_MODIFIED    = 179,
        SYNTH_WIDTH_WIDENNING       = 180,
        CPP_BOOL_BITWISE_NOT        = 181,
        SYNTH_ARRAY_ELM_REFERENCE   = 182,
        SYNTH_BITWISE_SIGN_UNSIGN_MIX = 183,
        SYNTH_NEGATIVE_SHIFT        = 184,
        SYNTH_BIG_SHIFT             = 185,
        SYNTH_DIV_SIGN_UNSIGN_MIX   = 186,
        SYNTH_NEG_LITER_UCAST       = 187,
        SYNTH_CPP_COMMA             = 188,
        CPP_COMMA_SIMPLE_LHS        = 189,
        SC_CONCAT_INT_TO_BOOL       = 190,

        SC_FATAL_ELAB_TYPES_NS      = 200,
        SC_WARN_ELAB_UNSUPPORTED_TYPE,
        SC_WARN_ELAB_DANGLING_PTR,
        SC_ERROR_ELAB_MULT_PTRS,
        SC_WARN_ELAB_MULT_PTRS,
        SC_ERROR_ELAB_BASE_OFFSET_PTR,
        SC_ERROR_ELAB_UNSUPPORTED_TYPE,
        SYNTH_WAIT_LOOP_FALLTHROUGH,


        SC_ERROR_CPROP_UNROLL_MAX   = 300,
        SC_ERROR_CPROP_UNROLL_WAIT  = 301,
        SC_ERROR_CPROP_UNROLL_UNKWN   = 302,
        SC_WARN_EVAL_UNSUPPORTED_EXPR = 303,

        TOOL_INTERNAL_ERROR          = 400,
        TOOL_INTERNAL_FATAL          = 401,
        TOOL_INTERNAL_WARNING        = 402
    };

private:
    /// See Clang documentation for FormatString
    void initialize() {
        idFormatMap[SC_CXX_INIT_IN_PROC_FUNC] =
            {clang::DiagnosticIDs::Error, 
            "Default initializer (CXXDefaultInitExpr) cannot be used in "
            "process function"};
        idFormatMap[SC_LIST_INIT_IN_PROC_FUNC] =
            {clang::DiagnosticIDs::Error, 
            "List initializer (InitListExpr) cannot be used in process "
            "function"};
        idFormatMap[SC_CXX_CTOR_IN_PROC_FUNC] =
            {clang::DiagnosticIDs::Error, 
            "Class constructor (CXXConstructExpr) cannot be used in process "
            "function"};
        idFormatMap[SC_NEW_IN_PROC_FUNC] =
            {clang::DiagnosticIDs::Error, 
            "Operator new (CXXNewExpr) cannot be used in process function"};
        idFormatMap[SC_DELETE_IN_PROC_FUNC] =
            {clang::DiagnosticIDs::Error, 
            "Operator delete (CXXDeleteExpr) cannot be used in process "
            "function"};
        idFormatMap[CPP_INCORRECT_REFER] =
            {clang::DiagnosticIDs::Error, 
            "Incorrect referenced object in & statement : %0"};
        idFormatMap[CPP_LOOP_COMPLEX_INIT] =
            {clang::DiagnosticIDs::Error, 
            "Complex initialization in FOR loop is not supported"};
        idFormatMap[CPP_LOOP_COMPLEX_INCR] =
            {clang::DiagnosticIDs::Error, 
            "Complex increment in FOR loop is not supported"};
        idFormatMap[SYNTH_UNSUPPORTED_OPER] =
            {clang::DiagnosticIDs::Error, 
            "Unsupported to synthesis : %0 (%1)"};
        idFormatMap[CPP_ARRAY_OUT_OF_BOUND] =
            {clang::DiagnosticIDs::Error, 
            "Array out-of-bound"};
        idFormatMap[SYNTH_ARRAY_INIT_LIST] =
            {clang::DiagnosticIDs::Warning, 
            "Multidimensional array initialization not supported yet"};
        idFormatMap[SYNTH_UNSUPPORTED_INIT] =
            {clang::DiagnosticIDs::Warning, 
            "Unsupported initializer : %0"};
        idFormatMap[CPP_GOTO_STMT] =
            {clang::DiagnosticIDs::Fatal, 
            "Goto statement not supported"};
        idFormatMap[CPP_UNKNOWN_STMT] =
            {clang::DiagnosticIDs::Error, 
            "Unsupported statement : %0"};
        idFormatMap[SYNTH_TYPE_NOT_SUPPORTED] =
            {clang::DiagnosticIDs::Error, 
            "Type is not supported : %0"};
        idFormatMap[SYNTH_UNKNOWN_TYPE_WIDTH] =
            {clang::DiagnosticIDs::Error, 
            "Cannot determine type width : %0"};
        idFormatMap[SYNTH_ZERO_TYPE_WIDTH] =
            {clang::DiagnosticIDs::Fatal, 
            "Zero type width not allowed for variable : %0"};
        
        idFormatMap[SYNTH_WIDTH_WIDENNING] =
            {clang::DiagnosticIDs::Remark, 
            "Widening width in type cast : %0 to %1"};

        idFormatMap[SC_RANGE_DIFF_VARS] =
            {clang::DiagnosticIDs::Fatal, 
            "Incorrect range, different variables in lo/hi"};
        idFormatMap[SC_RANGE_WRONG_INDEX] =
            {clang::DiagnosticIDs::Fatal, 
            "Incorrect range, low index bigger than high or negative one"};
        idFormatMap[SC_RANGE_WRONG_WIDTH] =
            {clang::DiagnosticIDs::Fatal, 
            "Incorrect range, high index out of variable width"};
        idFormatMap[SC_RANGE_WRONG_BASE] =
            {clang::DiagnosticIDs::Error, 
            "Incorrect range, base cannot be expression or literal"};
        idFormatMap[SYNTH_SWITCH_LAST_EMPTY_CASE] =
            {clang::DiagnosticIDs::Fatal, 
            "Incorrect switch statement, no break in last case"};
        idFormatMap[SYNTH_SWITCH_ALL_EMPTY_CASE] =
            {clang::DiagnosticIDs::Fatal, 
            "Incorrect switch statement, all cases are empty"};
                
        idFormatMap[CPP_FOR_WITHOUT_INIT] =
            {clang::DiagnosticIDs::Warning, 
            "For loop without counter initialization"};
        idFormatMap[CPP_DIFF_POINTER_COMP] =
            {clang::DiagnosticIDs::Fatal, 
            "Pointers to different objects comparison"};
        idFormatMap[SC_UNSUPPORTED_PORT] =
            {clang::DiagnosticIDs::Fatal, 
            "Unsupported kind of port : %0"};
        idFormatMap[SYNTH_POINTER_NO_INIT] =
            {clang::DiagnosticIDs::Warning, 
            "Local pointer declared without initialization : %0"};
        idFormatMap[SYNTH_MULTI_POINTER_DIFF] =
            {clang::DiagnosticIDs::Fatal, 
            "Pointers to dynamically allocated object declared in different modules"};
        idFormatMap[CPP_NO_VIRT_FUNC] =
            {clang::DiagnosticIDs::Fatal, 
            "No virtual function %0 found in the dynamic class"};
        idFormatMap[CPP_PURE_FUNC_CALL] =
            {clang::DiagnosticIDs::Fatal, 
            "Pure virtual function call : %0"};
        idFormatMap[CPP_REFER_NO_INIT] =
            {clang::DiagnosticIDs::Fatal, 
            "Uninitialized reference : %0"};
        idFormatMap[SYNTH_POINTER_NONZERO_INIT] =
            {clang::DiagnosticIDs::Error, 
            "Pointer initialization with non-zero integer not supported : %0"};
        idFormatMap[CPP_ASSERT_FAILED] =
            {clang::DiagnosticIDs::Error, 
            "User assertion (sct_assert_const) failed"};
        idFormatMap[SC_WAIT_IN_METHOD] =
            {clang::DiagnosticIDs::Fatal, 
            "Call wait() in method process prohibited"};
        idFormatMap[SC_WAIT_N_VARIABLE] =
            {clang::DiagnosticIDs::Fatal, 
            "Call wait(N) where N is non-constant"};
        idFormatMap[SC_WAIT_N_EMPTY] =
            {clang::DiagnosticIDs::Fatal, 
            "No counter variable created for wait(N)"};
        idFormatMap[SC_WAIT_N_NONPOSITIVE] =
            {clang::DiagnosticIDs::Fatal, 
            "Call wait(N) where N is negative or zero"};
        idFormatMap[SC_UNSUPP_WAIT_KIND] =
            {clang::DiagnosticIDs::Fatal, 
            "Unsupported wait() kind for synthesis"};
        idFormatMap[SYNTH_NO_CHANNEL] =
            {clang::DiagnosticIDs::Fatal, 
            "No channel object for channel type access"};
        idFormatMap[SYNTH_UNSUPP_CHANNEL_METH] =
            {clang::DiagnosticIDs::Error, 
            "Channel method is not supported : %0"};
        idFormatMap[SYNTH_NOSENSITIV_METH] =
            {clang::DiagnosticIDs::Error, 
            "Prohibited statement in SC_METHOD with empty sensitivity"};
        idFormatMap[SYNTH_DUPLICATE_ASSIGN] =
            {clang::DiagnosticIDs::Error, 
            "Duplicate assignment in SC_METHOD with empty sensitivity"};
        idFormatMap[SYNTH_NON_SENSTIV_2USED] =
            {clang::DiagnosticIDs::Error, 
            "SC_METHOD %0() is non-sensitive to %1 which read inside"};
        idFormatMap[SYNTH_SOME_PATH_DEFINED] =
            {clang::DiagnosticIDs::Error, 
            "Variable %0 defined at some paths of %1() process, latch detected"};
        
        idFormatMap[CPP_BOOL_BITWISE_NOT] =
            {clang::DiagnosticIDs::Error, 
            "Bitwise not for boolean argument, use logical not instead"};
        
        idFormatMap[SYNTH_ARRAY_ELM_REFERENCE] =
            {clang::DiagnosticIDs::Warning,
            "Reference to array element at variable index may lead to wrong code"};
        
        idFormatMap[SYNTH_NEGATIVE_SHIFT] =
            {clang::DiagnosticIDs::Error, 
            "Negative shift not allowed"};
        
        idFormatMap[SYNTH_BIG_SHIFT] =
            {clang::DiagnosticIDs::Warning, 
            "Too big (>1024) shift not allowed"};
        
        idFormatMap[SYNTH_POINTER_OPER] =
            {clang::DiagnosticIDs::Error, 
            "Pointer operation not supported yet : %0"};
        idFormatMap[CPP_INCORRECT_ASSERT] =
            {clang::DiagnosticIDs::Warning, 
            "Incorrect assert expression for : %0"};
        idFormatMap[SYNTH_FUNC_IN_ASSERT] =
            {clang::DiagnosticIDs::Error, 
            "Function call in temporal assert not allowed"};
        
        
        idFormatMap[SC_FATAL_THREAD_NO_STATE] =
            {clang::DiagnosticIDs::Fatal, 
            "No states found for thread"};
        idFormatMap[SYNTH_UNSUPPORTED_RETURN] =
            {clang::DiagnosticIDs::Fatal, 
            "Unsupported return in current loop/switch (incorrect loop stack)"};
        idFormatMap[SC_PORT_NOT_BOUND] =
            {clang::DiagnosticIDs::Fatal, 
            "Port not bound : %0 (%1)"};
        idFormatMap[SC_PORT_BOUND_SIGNAL_SAME] =
            {clang::DiagnosticIDs::Warning, 
            "Port bound to signal located in the same module: %0"};

        idFormatMap[SYNTH_NO_ARRAY] =
            {clang::DiagnosticIDs::Fatal, 
            "No array or global array object in [] operator: %0"};
        idFormatMap[SYNTH_ARRAY_TO_POINTER] =
            {clang::DiagnosticIDs::Fatal, 
            "Array to pointer on zero element cast not supported"};
        

        idFormatMap[SYNTH_SVA_INCORRECT_TIME] =
            {clang::DiagnosticIDs::Error, 
            "Incorrect time parameter in temporal assertions (SCT_ASSERT)"};
        
        idFormatMap[SYNTH_SVA_IN_MAIN_LOOP] =
            {clang::DiagnosticIDs::Error, 
            "Temporal assertion (SCT_ASSERT) in main loop not supported"};

        idFormatMap[SYNTH_SVA_INCORRECT_LOOP] =
            {clang::DiagnosticIDs::Error, 
            "Temporal assertion (SCT_ASSERT) can be in for loop only"};
        
        idFormatMap[SYNTH_CHILD_FIELD_ACCESS] =
            {clang::DiagnosticIDs::Fatal, 
            "Access to child module field is prohibited"};
        idFormatMap[CPP_OPAQUE_EXPR] =
            {clang::DiagnosticIDs::Warning, 
            "OpaqueValueExpr found, check for C++ compiler error message"};
        idFormatMap[SYNTH_WAIT_LOOP_FALLTHROUGH] =
            {clang::DiagnosticIDs::Fatal,
             "Loop with wait() has potential fallthrough path"};
        
        
        idFormatMap[SYNTH_SIGN_UNSIGN_MIX] =
            {clang::DiagnosticIDs::Warning,
             "Signed/unsigned types mix may lead to non-equivalent code"};
        
        idFormatMap[SYNTH_DIV_SIGN_UNSIGN_MIX] =
            {clang::DiagnosticIDs::Warning,
             "Signed/unsigned types mix in division/remainder leads to non-equivalent code"};
        
        idFormatMap[SYNTH_BITWISE_SIGN_UNSIGN_MIX] =
            {clang::DiagnosticIDs::Warning, 
            "Signed/unsigned types mix in bitwise operation not allowed"};
        
        idFormatMap[SYNTH_NEG_LITER_UCAST] =
            {clang::DiagnosticIDs::Warning,
             "Negative literal casted to unsigned leads to non-equivalent code"};
        
        idFormatMap[SYNTH_CPP_COMMA] =
            {clang::DiagnosticIDs::Warning,
             "C++ operator comma, may be concatenation intended here"};
               
        idFormatMap[CPP_COMMA_SIMPLE_LHS] =
            {clang::DiagnosticIDs::Warning,
             "C++ operator comma with variable/literal not supported"};
        
        idFormatMap[SC_CONCAT_INT_TO_BOOL] =
            {clang::DiagnosticIDs::Warning,
             "SC concatenation operator with integer to boolean cast"};

        idFormatMap[SYNTH_SC_PORT_INCORRECT] =
            {clang::DiagnosticIDs::Fatal, 
            "Incorrect use of sc_port<IF>: %0"};
        
        idFormatMap[SC_ERROR_NORETURN] =
            {clang::DiagnosticIDs::Fatal, 
            "Functions with NORETURN attribute (like assert() "
            "or __assert_fail() ) are not supported."};
        
        
        // For MIF array access by unknown index its impossible to avoid
        // false positive for this issue, so use warning level only
        idFormatMap[SYNTH_COMB_SIG_READNOTDEF] =
            {clang::DiagnosticIDs::Warning, 
            "Read of sct_comb_sig<T, true> before write not supported"};

        idFormatMap[SYNTH_LOCAL_MODULE_DECL] =
            {clang::DiagnosticIDs::Fatal, 
            "Declaration of sc_module variable prohibited"};
        
        idFormatMap[SYNTH_INCORRECT_RECORD] =
            {clang::DiagnosticIDs::Error, 
            "No record object found for member call (not sc_interface inheritor or not statically determined)"};
        
        idFormatMap[SYNTH_NONTRIVIAL_COPY] =
            {clang::DiagnosticIDs::Fatal, 
            "Non-trivial copy constructor in local record not supported"};
        
        idFormatMap[SYNTH_NONTRIVIAL_ASSIGN] =
            {clang::DiagnosticIDs::Fatal, 
            "Non-trivial assignment operator in local record not supported"};

        idFormatMap[CPP_READ_NOTDEF_VAR] =
            {clang::DiagnosticIDs::Warning, 
            "Read not initialized non-channel variable : %0"};

        idFormatMap[CPP_READ_NOTDEF_VAR_RESET] =
            {clang::DiagnosticIDs::Error, 
            "Read not initialized non-channel variable in CTHREAD reset section : %0"};

        idFormatMap[SYNTH_CONCAT_CAST_BOOL] =
            {clang::DiagnosticIDs::Error, 
            "Operand with no length specified in concatenation operator"};

        idFormatMap[SYNTH_TEMP_EXPR_ARG] =
            {clang::DiagnosticIDs::Error, 
            "Temporary expression cannot be used in this context"};

        idFormatMap[SYNTH_RECORD_CTOR_IN_PARAM] =
            {clang::DiagnosticIDs::Fatal, 
            "Record in function parameter must have empty copy/move constructor body"};
        
        idFormatMap[SYNTH_MULT_PROC_DRIVE_SIG] =
            {clang::DiagnosticIDs::Warning, 
            "Multiple processes drive signal/port : %0"};

        idFormatMap[SYNTH_MULT_PROC_ACCESS_VAR] =
            {clang::DiagnosticIDs::Warning, 
            "Multiple processes access variable : %0"};

        idFormatMap[SYNTH_USEDEF_IN_SAME_PROC] =
            {clang::DiagnosticIDs::Warning, 
            "Use signal/port defined in the same method process : %0"};
        
        idFormatMap[CPP_NULL_PTR_DEREF] =
            {clang::DiagnosticIDs::Error, 
            "Null pointer dereference : %0"};
        
        idFormatMap[CPP_DANGLING_PTR_DEREF] =
            {clang::DiagnosticIDs::Fatal, 
            "Dangling pointer dereference : %0"};
        
        idFormatMap[CPP_DANGLING_PTR_CAST] =
            {clang::DiagnosticIDs::Error, 
            "Dangling pointer casted to bool : %0"};
        
        idFormatMap[SYNTH_NONCOST_PTR_CONST] =
            {clang::DiagnosticIDs::Error, 
            "Non-constant pointer to constant variable no allowed : %0"};
        
        idFormatMap[SYNTH_CONST_VAR_MODIFIED] =
            {clang::DiagnosticIDs::Fatal, 
            "Constant variable modified in process code : %0"};

        // Elaboration
        idFormatMap[SC_FATAL_ELAB_TYPES_NS] =
            {clang::DiagnosticIDs::Fatal,
             "Can't find elaboration types namespace '%0'"};
        idFormatMap[SC_WARN_ELAB_UNSUPPORTED_TYPE] =
            {clang::DiagnosticIDs::Warning, 
            "ScState unsupported type : '%0'"};
        idFormatMap[SC_WARN_ELAB_DANGLING_PTR] =
            {clang::DiagnosticIDs::Warning, 
            "Dangling pointer : #%0"};
        idFormatMap[SC_ERROR_ELAB_MULT_PTRS] =
            {clang::DiagnosticIDs::Error,
            "Multiple pointers point to dynamic object #%0"};
        idFormatMap[SC_WARN_ELAB_MULT_PTRS] =
            {clang::DiagnosticIDs::Warning,
            "Multiple pointers point to object #%0"};
        idFormatMap[SC_ERROR_ELAB_BASE_OFFSET_PTR] =
            {clang::DiagnosticIDs::Error,
            "Unsupported pointer to element of integer array %0"};
        idFormatMap[SC_ERROR_ELAB_UNSUPPORTED_TYPE] =
            {clang::DiagnosticIDs::Error,
            "Unsupported type: %0"};

        
        idFormatMap[SC_NO_MODULE_NAME] =
            {clang::DiagnosticIDs::Fatal, 
            "Module not found, probably constructor w/o sc_module_name parameter"};
        
        // constant propagation
        idFormatMap[SC_ERROR_CPROP_UNROLL_MAX] =
            {clang::DiagnosticIDs::Error,
             "Can't unroll loop, maximum iterations exceeded"};

        idFormatMap[SC_ERROR_CPROP_UNROLL_WAIT] =
            {clang::DiagnosticIDs::Error,
             "Possible comb path through wait loop"};

        idFormatMap[SC_ERROR_CPROP_UNROLL_UNKWN] =
            {clang::DiagnosticIDs::Error,
             "Can't evaluate loop condition to unroll a loop"};

        idFormatMap[SC_WARN_EVAL_UNSUPPORTED_EXPR] =
            {clang::DiagnosticIDs::Warning,
             "Can't evaluate unsupported Clang AST Expression: %0"};

        
        idFormatMap[TOOL_INTERNAL_WARNING] =
            {clang::DiagnosticIDs::Warning, "ScTool internal warning : %0"};

        idFormatMap[TOOL_INTERNAL_ERROR] =
            {clang::DiagnosticIDs::Error, "ScTool internal error : %0"};
        
        idFormatMap[TOOL_INTERNAL_FATAL] =
            {clang::DiagnosticIDs::Fatal, "ScTool internal fatal error : %0"};
    }

public:
    /// Report Fatal Error using diagnostic engine by creating custom ID
    /// and terminating. In most cases you don't need this function,
    /// just use asserts()
    static void reportErrAndDie(clang::SourceLocation loc,
                                llvm::StringRef message);
    static void reportErrAndDie(llvm::StringRef message);

    /// Custom (Temporary) diagnostic message. Consider using ScDiag and
    /// reportScDiag() for permanent diagnostics.
    /// Usage example: reportCustom(clang::DiagnosticsEngine::Warning,
    /// "My warning with param %0 , bye") << param_val;
    static clang::DiagnosticBuilder reportCustom(
        clang::SourceLocation loc,
        clang::DiagnosticIDs::Level level,
        llvm::StringRef formatString);

    static clang::DiagnosticBuilder reportCustom(
        clang::DiagnosticIDs::Level level,
        llvm::StringRef formatString);

    /// Reporting using permanent IDs from ScDiag
    /// Usage example: reportScDiag(SC_FATAL_ELAB_TYPES_NS) << "param";
    static clang::DiagnosticBuilder reportScDiag(clang::SourceLocation loc,
                                                 ScDiag::ScDiagID id,
                                                 bool checkDuplicate = true);
    static clang::DiagnosticBuilder reportScDiag(ScDiag::ScDiagID id);
    
    /// Reporting internal warning/error
    #define SCT_INTERNAL_WARNING(loc, msg) \
            ScDiag::reportScDiag(loc, ScDiag::TOOL_INTERNAL_WARNING) << msg; 

    #define SCT_INTERNAL_WARNING_NOLOC(msg) \
            ScDiag::reportScDiag(ScDiag::TOOL_INTERNAL_WARNING) << msg;

    #define SCT_INTERNAL_ERROR(loc, msg) \
            ScDiag::reportScDiag(loc, ScDiag::TOOL_INTERNAL_ERROR) << msg; 

    #define SCT_INTERNAL_ERROR_NOLOC(msg) \
            ScDiag::reportScDiag(ScDiag::TOOL_INTERNAL_ERROR) << msg;

    #define SCT_INTERNAL_FATAL(loc, msg) \
            {ScDiag::reportScDiag(loc, ScDiag::TOOL_INTERNAL_FATAL) << msg; \
            throw sc::InternalErrorException();}

    #define SCT_INTERNAL_FATAL_NOLOC(msg) \
            {ScDiag::reportScDiag(ScDiag::TOOL_INTERNAL_FATAL) << msg; \
            throw sc::InternalErrorException(msg);}

#ifdef NDEBUG
    #define SCT_TOOL_ASSERT(expr, msg) \
            if (!(expr)) { \
            ScDiag::reportScDiag(ScDiag::TOOL_INTERNAL_FATAL) << msg; \
            throw sc::InternalErrorException(msg);}
#else 
    #define SCT_TOOL_ASSERT(expr, msg) \
            if (!(expr)) { \
            ScDiag::reportScDiag(ScDiag::TOOL_INTERNAL_ERROR) << msg; \
            assert (false);}
#endif
    
private:
    ScDiag() = default;
    ScDiag(const ScDiag &) = delete;

    static ScDiag &instance();

    clang::DiagnosticsEngine *engine = nullptr;

    /// ID -> <Level, FormatString>
    std::map<ScDiag::ScDiagID,
             std::pair<clang::DiagnosticIDs::Level, std::string>> idFormatMap;

    /// ID -> Clang diag ID
    std::map<ScDiag::ScDiagID, unsigned> sc2clangMap;
    /// All reported issues to filter duplicates
    static std::unordered_set<std::pair<unsigned, unsigned>> diagIssues;
};

} // end namespace sc



#endif //SCTOOL_SCTOOLDIAGNOSTIC_H
