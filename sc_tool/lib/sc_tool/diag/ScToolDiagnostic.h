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

/// Report error exception in HandleTranslationUnit
void reportErrorException();

/// Return -1 for error and -2 for fatal error
int getDiagnosticStatus();

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
        SYNTH_INCRDECR_LHS          = 179,
        SYNTH_WIDTH_WIDENNING       = 180,
        CPP_BOOL_BITWISE_NOT        = 181,
        SYNTH_ARRAY_ELM_REFERENCE   = 182,
        SYNTH_SIGNED_SHIFT          = 183,
        SYNTH_NEGATIVE_SHIFT        = 184,
        SYNTH_BIG_SHIFT             = 185,
        SYNTH_DIV_SIGN_UNSIGN_MIX   = 186,
        SYNTH_NEG_LITER_UCAST       = 187,
        SYNTH_CPP_COMMA             = 188,
        CPP_COMMA_SIMPLE_LHS        = 189,
        SC_CONCAT_INT_TO_BOOL       = 190,
        SC_BIT_WRONG_INDEX          = 191,
        SC_BIT_WRONG_BASE           = 192,
        CPP_NONSTD_TYPE_WIDTH       = 193,
        SYNTH_MEMORY_NON_UNIQUE     = 194,
        CPP_LOOP_COMPOUND_COND      = 195,
        SYNTH_FUNC_CALL_COND_LOOP   = 196,
        CPP_UNKNOWN_STD_FUNC        = 197,
        SYNTH_ALIVE_LOOP_ERROR      = 198,        
        SYNTH_ALIVE_LOOP_NULL_COND  = 199,
        SC_CHAN_READ_IN_RESET       = 200,
        SC_CTHREAD_NO_MAIN_LOOP     = 201,
        SYNTH_RETURN_FROM_LOOP      = 202,
        SYNTH_CODE_AFTER_RETURN     = 203,
        SYNTH_NO_RESET_VIOLATION    = 204,
        SYNTH_POINTER_INCORRECT_INIT= 205,
        SC_BIT_CONSTPTR_BASE        = 206,
        SC_RANGE_CONSTPTR_BASE      = 207,
        SYNTH_ASSIGN_IN_COND        = 208,
        SYNTH_SIDE_EFFECT_IN_COND   = 209,
        SYNTH_LVALUE_BIT_CAST       = 210,
        SYNTH_CONST_CAST            = 211,
        ELAB_BAD_RECORD_OBJ         = 212,
        SYNTH_READ_REG_IN_RESET     = 213,
        SYNTH_MODIFY_REG_IN_RESET   = 214,
        SC_RANGE_NO_INTEGER         = 215,
        SYNTH_USEDEF_ARR_IN_SAME_PROC = 216,
        SYNTH_COMPARE_SIGN_UNSIGN_MIX = 217,
        SYNTH_SEXPR_UNSIGNED        = 218,    
        CPP_NOT_NUMBER_LITER        = 219, 
        SYNTH_FUNC_CALL_II_LOOP     = 220,
        SYNTH_FUNC_CALL_WAIT_LOOP   = 221,
        SYNTH_MULT_ASSIGN_REC       = 222,
        SYNTH_INTERNAL_ASSIGN_OPER  = 223,
        SYNTH_UNSIGNED_MODE_BINARY  = 224,
        SYNTH_UNSIGNED_MODE_UNARY   = 225,
        SYNTH_UNSIGNED_MODE_DECL    = 226,
        CPP_BOOL_BITWISE_BINARY     = 227,
        SYNTH_MULTI_CROSS_BOUND     = 228,
        CPP_FOR_WITHOUT_DECL        = 229,
        SYNTH_PART_INIT_VAR         = 230,
        CPP_STRING_BINARY_OPER      = 231,
        CPP_STATIC_STD_VECTOR       = 232,
        SYNTH_LITER_OVERFLOW        = 233,
        SYNTH_RECORD_CTOR_NONEMPTY  = 234,
        CPP_DEFINED_LOCAL_PARAM     = 235,
        SYNTH_INCORRECT_FUNC_CALL   = 236,
        SYNTH_PARENT_SAME_OBJECT    = 237,
        SYNTH_NO_RESET_PROCESS      = 238,
        SYNTH_FUNC_TYPE_IN_ASSERT   = 239,
        SYNTH_NON_SENSTIV_THREAD    = 240,
        SYNTH_EXTRA_SENSTIV_THREAD  = 241,
        SYNTH_RECORD_INIT_LIST      = 242,
        SYNTH_CHAN_RECORD_CONST     = 243,
        SYNTH_CHAN_RECORD_TEMP      = 244,
        SYNTH_INNER_RECORD          = 245,
        SYNTH_SC_PORT_ARRAY         = 246,
        
        SC_FATAL_ELAB_TYPES_NS      = 300,
        SC_WARN_ELAB_UNSUPPORTED_TYPE,
        SC_WARN_ELAB_DANGLING_PTR,
        SC_ERROR_ELAB_MULT_PTRS,
        SC_WARN_ELAB_MULT_PTRS,
        SC_ERROR_ELAB_BASE_OFFSET_PTR,
        SC_ERROR_ELAB_UNSUPPORTED_TYPE,
        ELAB_PORT_BOUND_PORT_ERROR,
        ELAB_PORT_BOUND_SIGNAL_ERROR,
        ELAB_PROCESS_ERROR,
        SYNTH_WAIT_LOOP_FALLTHROUGH,


        SC_ERROR_CPROP_UNROLL_MAX   = 400,
        SC_ERROR_CPROP_UNROLL_WAIT  = 401,
        SC_ERROR_CPROP_UNROLL_UNKWN   = 402,
        SC_WARN_EVAL_UNSUPPORTED_EXPR = 403,

        TOOL_INTERNAL_ERROR          = 500,
        TOOL_INTERNAL_FATAL          = 501,
        TOOL_INTERNAL_WARNING        = 502
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
        idFormatMap[CPP_LOOP_COMPOUND_COND] =
            {clang::DiagnosticIDs::Error, 
            "Compound condition in loop is not supported"};
        idFormatMap[SYNTH_UNSUPPORTED_OPER] =
            {clang::DiagnosticIDs::Error, 
            "Unsupported to synthesis : %0 (%1)"};
        idFormatMap[CPP_ARRAY_OUT_OF_BOUND] =
            {clang::DiagnosticIDs::Error, 
            "Array out-of-bound"};
        idFormatMap[SYNTH_ARRAY_INIT_LIST] =
            {clang::DiagnosticIDs::Error, 
            "Partial initialization of sub-arrays not supported"};
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
        idFormatMap[CPP_NONSTD_TYPE_WIDTH] =
            {clang::DiagnosticIDs::Warning, 
            "C++ type width differs from SC synth standard requirements : %0"};
        
        
        idFormatMap[SYNTH_WIDTH_WIDENNING] =
            {clang::DiagnosticIDs::Remark, 
            "Widening width in type cast : %0 to %1"};

        idFormatMap[SC_RANGE_NO_INTEGER] =
            {clang::DiagnosticIDs::Fatal, 
            "No integer value in range lo/hi"};
        idFormatMap[SC_RANGE_DIFF_VARS] =
            {clang::DiagnosticIDs::Fatal, 
            "Incorrect range access, different variables in lo/hi"};
        idFormatMap[SC_RANGE_WRONG_INDEX] =
            {clang::DiagnosticIDs::Fatal, 
            "Incorrect range access, low index bigger than high or negative one"};
        idFormatMap[SC_RANGE_WRONG_WIDTH] =
            {clang::DiagnosticIDs::Fatal, 
            "Incorrect range access, high index out of variable width"};
        idFormatMap[SC_RANGE_WRONG_BASE] =
            {clang::DiagnosticIDs::Error, 
            "Incorrect range access, base cannot be expression or literal"};
        idFormatMap[SC_RANGE_CONSTPTR_BASE] =
            {clang::DiagnosticIDs::Error, 
            "Range access for constant pointer to dynamic memory not supported"};
        idFormatMap[SYNTH_SWITCH_LAST_EMPTY_CASE] =
            {clang::DiagnosticIDs::Fatal, 
            "Incorrect switch statement, no break in last case"};
        idFormatMap[SYNTH_SWITCH_ALL_EMPTY_CASE] =
            {clang::DiagnosticIDs::Fatal, 
            "Incorrect switch statement, all cases are empty"};
                
        idFormatMap[SC_BIT_WRONG_INDEX] =
            {clang::DiagnosticIDs::Fatal, 
            "Incorrect bit access, index out of variable width"};
        idFormatMap[SC_BIT_WRONG_BASE] =
            {clang::DiagnosticIDs::Error, 
            "Incorrect bit access, base cannot be expression or literal"};
        idFormatMap[SC_BIT_CONSTPTR_BASE] =
            {clang::DiagnosticIDs::Error, 
            "Bit access for constant pointer to dynamic memory not supported"};
        
        idFormatMap[CPP_FOR_WITHOUT_INIT] =
            {clang::DiagnosticIDs::Warning, 
            "For loop without counter initialization"};
        idFormatMap[SC_UNSUPPORTED_PORT] =
            {clang::DiagnosticIDs::Fatal, 
            "Unsupported kind of port : %0"};
        idFormatMap[CPP_NO_VIRT_FUNC] =
            {clang::DiagnosticIDs::Fatal, 
            "No virtual function %0 found in the dynamic class"};
        idFormatMap[CPP_PURE_FUNC_CALL] =
            {clang::DiagnosticIDs::Fatal, 
            "Pure virtual function call : %0"};
        idFormatMap[CPP_REFER_NO_INIT] =
            {clang::DiagnosticIDs::Fatal, 
            "Uninitialized reference : %0"};
        idFormatMap[CPP_ASSERT_FAILED] =
            {clang::DiagnosticIDs::Error, 
            "User assertion (sct_assert_const) failed"};
        idFormatMap[SC_WAIT_IN_METHOD] =
            {clang::DiagnosticIDs::Fatal, 
            "Call wait() in method process prohibited"};
        idFormatMap[SC_WAIT_N_VARIABLE] =
            {clang::DiagnosticIDs::Fatal, 
            "Call wait(N) where N is unknown"};
        idFormatMap[SC_WAIT_N_EMPTY] =
            {clang::DiagnosticIDs::Fatal, 
            "Call wait(N) with variable parameter"};
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
            "SC_METHOD is non-sensitive to (%0) which read inside"};
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
        
        idFormatMap[CPP_INCORRECT_ASSERT] =
            {clang::DiagnosticIDs::Warning, 
            "Incorrect assert expression for : %0"};
        idFormatMap[SYNTH_FUNC_IN_ASSERT] =
            {clang::DiagnosticIDs::Error, 
            "Non-trivial function call in temporal assert not allowed"};
        idFormatMap[SYNTH_FUNC_TYPE_IN_ASSERT] =
            {clang::DiagnosticIDs::Error, 
            "Function called in temporal assert should return integral type"};
        
        
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
            {clang::DiagnosticIDs::Remark, 
            "Port bound to signal located in the same module: %0"};

        idFormatMap[SYNTH_NO_ARRAY] =
            {clang::DiagnosticIDs::Fatal, 
            "No array or global array object in [] operator: %0"};
        

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
        
        // Not used
        idFormatMap[SYNTH_DIV_SIGN_UNSIGN_MIX] =
            {clang::DiagnosticIDs::Warning,
             "Signed/unsigned types mix in division/remainder leads to non-equivalent code"};
        
        idFormatMap[SYNTH_SIGNED_SHIFT] =
            {clang::DiagnosticIDs::Warning, 
            "Signed shift operation may lead to non-equivalent code"};
        
        idFormatMap[SYNTH_COMPARE_SIGN_UNSIGN_MIX] =
            {clang::DiagnosticIDs::Warning, 
            "Signed/unsigned types mix in comparison operation not allowed"};

        // More dangerous as leads to negative expression casted to unsigned 
        // with signed'{1'b0, ...}
        idFormatMap[SYNTH_SEXPR_UNSIGNED] =
            {clang::DiagnosticIDs::Warning,
             "Negative expression with unsigned type leads to non-equivalent code"};

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
            "No record object found for member call : %0 %1"};
        
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
            {clang::DiagnosticIDs::Warning, 
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
            {clang::DiagnosticIDs::Error, 
            "Multiple processes access non-channel variable : %0"};

        idFormatMap[SYNTH_USEDEF_IN_SAME_PROC] =
            {clang::DiagnosticIDs::Warning, 
            "Use signal/port defined in the same method process : %0"};
        
        idFormatMap[SYNTH_USEDEF_ARR_IN_SAME_PROC] =
            {clang::DiagnosticIDs::Remark, 
            "Possibly use signal/port defined in the same method process : %0"};
        
        idFormatMap[SYNTH_FUNC_CALL_II_LOOP] =
            {clang::DiagnosticIDs::Error, 
            "Function call in FOR loop initialization/increment not supported"};
        idFormatMap[SYNTH_FUNC_CALL_COND_LOOP] =
            {clang::DiagnosticIDs::Error, 
            "Function call in condition of loop without wait() not supported"};
        idFormatMap[SYNTH_FUNC_CALL_WAIT_LOOP] =
            {clang::DiagnosticIDs::Error, 
            "Function with wait() call in loop condition not supported"};
        idFormatMap[SYNTH_MULT_ASSIGN_REC] =
            {clang::DiagnosticIDs::Error, 
            "Multiple assignment for records not supported yet"};
        idFormatMap[SYNTH_INTERNAL_ASSIGN_OPER] =
            {clang::DiagnosticIDs::Error, 
            "Internal assignment in binary/unary operation not supported yet"};

        idFormatMap[SYNTH_UNSIGNED_MODE_BINARY] =
            {clang::DiagnosticIDs::Warning, 
            "Signed assignment or other binary operator in unsigned mode"};

        idFormatMap[SYNTH_UNSIGNED_MODE_UNARY] =
            {clang::DiagnosticIDs::Warning, 
            "Signed unary operator in unsigned mode"};
        
        idFormatMap[SYNTH_UNSIGNED_MODE_DECL] =
            {clang::DiagnosticIDs::Warning, 
            "Signed type variable declaration in unsigned mode"};
                 
         idFormatMap[CPP_BOOL_BITWISE_BINARY] =
            {clang::DiagnosticIDs::Remark, 
            "Bitwise operation with boolean, use logical operation instead"};
         
         idFormatMap[SYNTH_MULTI_CROSS_BOUND] =
            {clang::DiagnosticIDs::Error, 
            "Multiple bound at different levels of module hierarchy : signal %0 at module %1 and module %2"};
         

        idFormatMap[CPP_UNKNOWN_STD_FUNC] =
            {clang::DiagnosticIDs::Warning, 
            "Call of unknown function from namespace std : %0"};
        
        idFormatMap[SYNTH_ALIVE_LOOP_ERROR] =
            {clang::DiagnosticIDs::Error, 
            "SCT_ALIVE_LOOP macro can be applied to FOR or WHILE loops only"};

        idFormatMap[SYNTH_ALIVE_LOOP_NULL_COND] =
            {clang::DiagnosticIDs::Error, 
            "SCT_ALIVE_LOOP macro cannot be applied to loop with false condition"};
        
        idFormatMap[SC_CHAN_READ_IN_RESET] =
            {clang::DiagnosticIDs::Warning, 
            "Channel read in CTHREAD reset section is prohibited : %0"};
        
        idFormatMap[SC_CTHREAD_NO_MAIN_LOOP] =
            {clang::DiagnosticIDs::Error, 
            "No main loop in CTHREAD found"};
        
        idFormatMap[SYNTH_RETURN_FROM_LOOP] =
            {clang::DiagnosticIDs::Fatal, 
            "Function return in loop is prohibited"};
        
        idFormatMap[SYNTH_CODE_AFTER_RETURN] =
            {clang::DiagnosticIDs::Fatal, 
            "Code after function return is prohibited"};
        
        idFormatMap[SYNTH_NO_RESET_VIOLATION] =
            {clang::DiagnosticIDs::Error, 
            "CTHREAD without reset cannot have reset code or multiple states"};
        
        
        idFormatMap[CPP_NULL_PTR_DEREF] =
            {clang::DiagnosticIDs::Error, 
            "Null pointer dereference : %0"};
        idFormatMap[CPP_DANGLING_PTR_DEREF] =
            {clang::DiagnosticIDs::Fatal, 
            "Dangling pointer dereference : %0"};
        idFormatMap[CPP_DANGLING_PTR_CAST] =
            {clang::DiagnosticIDs::Fatal, 
            "Dangling pointer casted to bool : %0"};
        idFormatMap[SYNTH_NONCOST_PTR_CONST] =
            {clang::DiagnosticIDs::Error, 
            "Non-constant pointer to constant variable no allowed : %0"};
        
        idFormatMap[SYNTH_INCRDECR_LHS] =
            {clang::DiagnosticIDs::Error, 
            "Increment/decrement in left side of assignment is prohibited"};
        
        idFormatMap[SYNTH_ARRAY_TO_POINTER] =
            {clang::DiagnosticIDs::Fatal, 
            "Array to pointer on zero element cast not supported"};
        idFormatMap[SYNTH_POINTER_OPER] =
            {clang::DiagnosticIDs::Error, 
            "Pointer operation not supported yet : %0"};
        idFormatMap[SYNTH_POINTER_INCORRECT_INIT] =
            {clang::DiagnosticIDs::Error, 
            "Pointer initialization with incorrect object : %0"};
        idFormatMap[SYNTH_POINTER_NONZERO_INIT] =
            {clang::DiagnosticIDs::Error, 
            "Pointer initialization with non-zero integer not supported : %0"};
        idFormatMap[SYNTH_POINTER_NO_INIT] =
            {clang::DiagnosticIDs::Warning,
            "Local pointer declared without initialization : %0"};
        idFormatMap[SYNTH_MULTI_POINTER_DIFF] =
            {clang::DiagnosticIDs::Fatal, 
            "Pointers to dynamically allocated object declared in different modules"};
        idFormatMap[SYNTH_PARENT_SAME_OBJECT] =
            {clang::DiagnosticIDs::Warning, 
            "One of dynamically allocated object parent is the same object"};
        
        idFormatMap[SYNTH_NO_RESET_PROCESS] =
            {clang::DiagnosticIDs::Warning, 
            "No process for reset found, reset is not sc_in<> port"};
        
        
        idFormatMap[CPP_DIFF_POINTER_COMP] =
            {clang::DiagnosticIDs::Fatal, 
            "Pointers to different objects comparison"};
        
        idFormatMap[SYNTH_LVALUE_BIT_CAST] =
            {clang::DiagnosticIDs::Error, 
            "LValue cast is not supported"};
        idFormatMap[SYNTH_CONST_CAST] =
            {clang::DiagnosticIDs::Error, 
            "Constant cast in process code is ignored"};
        
        idFormatMap[ELAB_BAD_RECORD_OBJ] =
            {clang::DiagnosticIDs::Fatal, 
            "Bad record object at elaboration phase, a port is bound twice"};
        
        idFormatMap[SYNTH_READ_REG_IN_RESET] =
            {clang::DiagnosticIDs::Warning, 
            "Register variable cannot be read in reset section"};
        
        idFormatMap[SYNTH_MODIFY_REG_IN_RESET] =
            {clang::DiagnosticIDs::Error, 
            "Register variable cannot be read-and-modified in reset section"};

        idFormatMap[CPP_NOT_NUMBER_LITER] =
            {clang::DiagnosticIDs::Error, 
            "String liter does not contain valid number"};
        
        idFormatMap[CPP_FOR_WITHOUT_DECL] =
            {clang::DiagnosticIDs::Warning, 
            "For loop without local counter declaration"};
        
        idFormatMap[SYNTH_PART_INIT_VAR] =
            {clang::DiagnosticIDs::Warning, 
            "Initialization value number differs from array size : %0"};
        
        idFormatMap[CPP_STRING_BINARY_OPER] =
            {clang::DiagnosticIDs::Error, 
            "Binary operation with string argument not supported"};
        
        idFormatMap[CPP_STATIC_STD_VECTOR] =
            {clang::DiagnosticIDs::Error, 
            "Static std::vector is not supportd yet"};
        
        idFormatMap[SYNTH_LITER_OVERFLOW] =
            {clang::DiagnosticIDs::Error, 
            "Literal is too big, up to 4096 symbols supported"};
        
        idFormatMap[SYNTH_RECORD_CTOR_NONEMPTY] =
            {clang::DiagnosticIDs::Error, 
            "Base class or member class non-empty constructor not supported"};
        
        idFormatMap[CPP_DEFINED_LOCAL_PARAM] =
            {clang::DiagnosticIDs::Error, 
            "Member variable used before initialization (re-defined localparam) : %0"};
        
        idFormatMap[SYNTH_INCORRECT_FUNC_CALL] =
            {clang::DiagnosticIDs::Error, 
             "Incorrect user defined method or function : %0"};
        
        idFormatMap[SYNTH_NON_SENSTIV_THREAD] =
            {clang::DiagnosticIDs::Warning, 
            "Process is not sensitive to SS channel used : %0"};
        
        idFormatMap[SYNTH_EXTRA_SENSTIV_THREAD] =
            {clang::DiagnosticIDs::Warning, 
            "Process is sensitive to SS channel not used : %0"};
        
        idFormatMap[SYNTH_RECORD_INIT_LIST] =
            {clang::DiagnosticIDs::Error, 
            "List initializer for record is not supported"};
        
        idFormatMap[SYNTH_CHAN_RECORD_CONST] =
            {clang::DiagnosticIDs::Error, 
            "Constant field is not allowed in channel type record, use static constant instead"};

        idFormatMap[SYNTH_CHAN_RECORD_TEMP] =
            {clang::DiagnosticIDs::Error, 
            "No record found, temporary object should use default constructor"};
        
        idFormatMap[SYNTH_INNER_RECORD] =
            {clang::DiagnosticIDs::Fatal, 
            "Inner record is not supported"};
        
        idFormatMap[SYNTH_SC_PORT_ARRAY] =
            {clang::DiagnosticIDs::Fatal, 
            "Array/vector of sc_port is not supported"};
        
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
            "Non-initialized pointer or pointer to non-zero element of array"};
        idFormatMap[SC_ERROR_ELAB_UNSUPPORTED_TYPE] =
            {clang::DiagnosticIDs::Error,
            "Unsupported type: %0"};
        
        idFormatMap[ELAB_PORT_BOUND_PORT_ERROR] =
            {clang::DiagnosticIDs::Error,
            "Port bound to incorrect port: %0"};
        idFormatMap[ELAB_PORT_BOUND_SIGNAL_ERROR] =
            {clang::DiagnosticIDs::Error,
            "Port bound to incorrect signal: %0"};
        idFormatMap[ELAB_PROCESS_ERROR] =
            {clang::DiagnosticIDs::Error,
            "Process object is expected: %0"};
        
        idFormatMap[SYNTH_ASSIGN_IN_COND] =
            {clang::DiagnosticIDs::Error,
            "Assignment in condition not supported"};
        idFormatMap[SYNTH_SIDE_EFFECT_IN_COND] =
            {clang::DiagnosticIDs::Error,
            "Side effects in condition not supported"};

        idFormatMap[SYNTH_MEMORY_NON_UNIQUE] =
            {clang::DiagnosticIDs::Error,
            "Memory module name is not unique: %0"};
        
        idFormatMap[SC_NO_MODULE_NAME] =
            {clang::DiagnosticIDs::Fatal, 
            "Module not found, probably constructor w/o sc_module_name parameter"};
        
        // constant propagation
        idFormatMap[SC_ERROR_CPROP_UNROLL_MAX] =
            {clang::DiagnosticIDs::Error,
             "Can't unroll loop, maximum iterations exceeded"};

        idFormatMap[SC_ERROR_CPROP_UNROLL_WAIT] =
            {clang::DiagnosticIDs::Error,
             "Possible combinational path through wait loop"};

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
    static clang::DiagnosticBuilder reportScDiag(ScDiag::ScDiagID id,
                                                 bool checkDuplicate = true);
    
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

    /// Any error/fatal error/exception occured
    bool hasError() {return engine->hasErrorOccurred();};
    bool hasFatal() {return engine->hasFatalErrorOccurred();};
    bool hasException = false;
    
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
