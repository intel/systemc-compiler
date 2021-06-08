/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * SystemC temporal assertions. 
 * SCT_ASSERT and SCT_ASSERT_LOOP macros definition.
 * 
 * Author: Mikhail Moiseev
 */

#ifndef SCT_ASSERT_H
#define SCT_ASSERT_H

#if !defined(__SC_TOOL__) && !defined(SCT_ASSERT_OFF)
#include "sct_property.h"
#endif
#include "systemc.h"
#include <cassert>

namespace sc_core {

/// Get string representation for argument
#define SCT_ONE_(X) #X
#define SCT_ONE(X) SCT_ONE_(X)

/// Get string concatenation for arguments
#define SCT_TWO_(X,Y) X##Y  
#define SCT_TWO(X,Y) SCT_TWO_(X,Y)

/// Macro argument number overload
#define SCT_GET_MACRO(_1,_2,_3,_4,NAME,...) NAME

/// Remove brackets from given argument 
#define SCT_ARGS__(...) __VA_ARGS__
#define SCT_ARGS_(X) X
#define SCT_ARGS(X) SCT_ARGS_(SCT_ARGS__ X)

//=============================================================================
// Immediate assertions

/// Immediate assertion, checked at SC simulation and SVA generated 
/// \param msg -- error message, added into generated SVA
#ifdef __SC_TOOL__
inline void sct_assert(bool expr) {}
inline void sct_assert(bool expr, const char* msg) {}

#else 
#define sct_assert1(expr) assert(expr)
#define sct_assert2(expr, msg) assert(expr && msg)
#define sct_assert(...) SCT_GET_MACRO(__VA_ARGS__, , ,\
                                      sct_assert2, sct_assert1)(__VA_ARGS__)
#endif

//=============================================================================
// Temporal assertions

/// Time parameter
#define SCT_TIME1(X) (X)
#define SCT_TIME2(X,Y) (X,Y)
#define SCT_TIME(...) SCT_GET_MACRO(__VA_ARGS__, , ,\
                                    SCT_TIME2, SCT_TIME1)(__VA_ARGS__)

/// Temporal assertion in module scope
#ifdef __SC_TOOL__
struct sct_property_mod
{
    explicit sct_property_mod() {}
    template<class T1, class T2>
    explicit sct_property_mod(bool lexpr, bool rexpr, sc_event_finder& event,
                              const char* name, T1 lotime, T2 hitime) {}
    template<class T1>
    explicit sct_property_mod(bool lexpr, bool rexpr, sc_event_finder& event,
                              const char* name, T1 time) {}
    template<class T1, class T2>
    explicit sct_property_mod(bool lexpr, bool rexpr, sc_port_base& event,
                              const char* name, T1 lotime, T2 hitime) {}
    template<class T1>
    explicit sct_property_mod(bool lexpr, bool rexpr, sc_port_base& event,
                              const char* name, T1 time) {}
};

/// __SC_TOOL_CLANG__ defined for Clang only, SVC target build without it 
#ifdef __SC_TOOL_CLANG__
    #define SCT_ASSERT4(LE, TIMES, RE, EVENT)\
        sct_property_mod SCT_TWO(sctTmpVar,__LINE__){\
            static_cast<bool>(LE), static_cast<bool>(RE), EVENT,\
            "sctAssertLine" SCT_ONE(__LINE__), SCT_ARGS(TIMES)};
#else
    // No parameter passed, LE/RE can use not-bound port/not-allocated pointers
    #define SCT_ASSERT4(LE, TIMES, RE, EVENT)\
        sct_property_mod SCT_TWO(sctTmpVar,__LINE__);
#endif

#else
#define SCT_ASSERT4(LE, TIMES, RE, EVENT)\
    sct_property* SCT_TWO(sctTmpVar,__LINE__) =\
            sct_property_storage::getProperty(\
                    [&]()->bool{return ( LE );},\
                    [&]()->bool{return ( RE );},\
                    &EVENT,\
                    [&]()->sct_time{return (sct_time(SCT_ARGS(TIMES)));},\
                    #LE " ##" #TIMES " " #RE\
            );
#endif

/// Temporal assertion in clock thread process body
#ifdef __SC_TOOL__
void sct_assert_in_proc_start() {}
template<class T1, class T2>
void sct_assert_in_proc_func(bool lhs, bool rhs,\
                             const char* name, T1 lotime, T2 hitime) {}
template<class T1>
void sct_assert_in_proc_func(bool lhs, bool rhs,\
                             const char* name, T1 time) {}

#define SCT_ASSERT3(LE, TIMES, RE) {\
                        sct_assert_in_proc_start();\
                        sct_assert_in_proc_func(\
                            LE, RE, "sctAssertLine" SCT_ONE(__LINE__),\
                            SCT_ARGS(TIMES)\
                        );}

#define SCT_ASSERT_LOOPN(LE, TIMES, RE, ...) SCT_ASSERT3(LE, TIMES, RE)

#else

#define SCT_ASSERT3(LE, TIMES, RE) {\
                sct_property_storage::getProperty(\
                    [&]()->bool{return ( LE );},\
                    [&]()->bool{return ( RE );},\
                    sc_get_current_process_handle(),\
                    [&]()->sct_time{return (sct_time(SCT_ARGS(TIMES)));},\
                    #LE " ##" #TIMES " " #RE\
                );}

/// Provide comma separated iteration variables to capture them by value
#define SCT_ITER1(ARG1) ARG1
#define SCT_ITER2(ARG1, ARG2) ARG1, ARG2
#define SCT_ITER3(ARG1, ARG2, ARG3) ARG1, ARG2, ARG3
#define SCT_ITER4(ARG1, ARG2, ARG3, ARG4) ARG1, ARG2, ARG3, ARG4
#define SCT_ITER_STR(...) SCT_GET_MACRO(__VA_ARGS__, SCT_ITER4, SCT_ITER3,\
                                        SCT_ITER2, SCT_ITER1)(__VA_ARGS__)

/// Take variables by value, required for loop counter variable
#define SCT_ASSERT_LOOPN(LE, TIMES, RE, ...) {\
                sct_property_storage::getProperty(\
                    [&, SCT_ITER_STR(__VA_ARGS__)]()->bool{return ( LE );},\
                    [&, SCT_ITER_STR(__VA_ARGS__)]()->bool{return ( RE );},\
                    sc_get_current_process_handle(),\
                    [&]()->sct_time{return (sct_time(SCT_ARGS(TIMES)));},\
                    #LE " ##" #TIMES " " #RE,\
                    __VA_ARGS__\
                );}
#endif

/// Immediate assertion activated by event in module scope
#define SCT_ASSERT2(RE, EVENT) \
        SCT_ASSERT4(true, SCT_TIME(0), RE, EVENT);

#define SCT_ASSERT1(ARG1) \
        SCT_ASSERT with 1 argument not supported;

/// Disable SCT_ASSERT and SCT_ASSERT_LOOP for HLS tools which not support it
#ifdef SCT_ASSERT_OFF
#define SCT_ASSERT(...) ;
#define SCT_ASSERT_LOOP(...) ;
#else
#define SCT_ASSERT(...) SCT_GET_MACRO(__VA_ARGS__, SCT_ASSERT4, SCT_ASSERT3,\
                                      SCT_ASSERT2, SCT_ASSERT1)(__VA_ARGS__)
#define SCT_ASSERT_LOOP(...) SCT_ASSERT_LOOPN(__VA_ARGS__)
#endif

//=============================================================================
// Special assertions of ISCS tool, used for tool testing

/// Latch assertion, assert that given variable, signal or port is latch or not
/// depends on second parameter
/// \param var -- variable, signal or port 
/// \param latch -- assert latch if true, or not latch otherwise
template <typename T>
inline void sct_assert_latch(T& var, bool latch = true) {}

/// Check given expression is true in constant propagation analysis
#ifdef __SC_TOOL__
inline void sct_assert_const(bool expr) {
    assert(expr);
}
#else
#define sct_assert_const(X) sc_assert(X)
#endif

/// Check current block level with given one
inline void sct_assert_level(unsigned level) {}

/// Check value is unknown 
template <typename T>
inline void sct_assert_unknown(T v) {}

/// Check if @v is defined if @b is true or not else
template <typename T>
inline void sct_assert_defined(T& v, bool b = true) {}

/// Check if @v is read if @b is true or not else
template <typename T>
inline void sct_assert_read(T& v, bool b = true) {}

/// Check if @v is read not defined if @b is true or not else
template <typename T>
inline void sct_assert_register(T& v, bool b = true) {}

/// Check if @v is array some element of which defined at least at on path
template <typename T>
inline void sct_assert_array_defined(T& v, bool b = true) {}

//=============================================================================
// Assert that FOR/WHILE loop has at least one iteration
// Used for loop with wait() inside and non-determinable number of iterations 
// which is placed in another loop without wait()
// 
// Usage:
//    SCT_ALIVE_LOOP(for(...){...});
//    SCT_ALIVE_LOOP(while(...){...});
//
// Usage example (N is non-determinable value):
//    for (int j = 0; j < M; j++) {
//       SCT_ALIVE_LOOP(
//       for (int i = 0; i < N; i++) {
//           wait();   
//       });
//    }
//
inline void sct_alive_loop() {}

#define SCT_ALIVE_LOOP(X) {sct_alive_loop(); X}


}

#endif /* SCT_ASSERT_H */
